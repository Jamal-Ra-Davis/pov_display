#ifndef DMA_SPI_LIB
#define DMA_SPI_LIB

#include "FrameBuffer.h"

#define N_DMACHANNELS 2
#define DMA_DESCRIPTOR_MEM_SZ (16*(N_DMACHANNELS + 1))

extern doubleBuffer frame_buffer;
extern volatile uint8_t buf_idx;
extern const int buf_offset[HEIGHT];


uint8_t DMA_DES_BASE[DMA_DESCRIPTOR_MEM_SZ];
uint8_t DMA_DES_WRB[DMA_DESCRIPTOR_MEM_SZ];

volatile bool transfer_complete = true;

struct dmaDescriptor {
  uint16_t bt_ctrl;
  uint16_t bt_cnt;
  uint32_t src_addr;
  uint32_t dst_addr;
  uint32_t desc_addr;
};

volatile dmaDescriptor dmaDescriptorArray[1] __attribute__ ((aligned (16)));
volatile dmaDescriptor dmaDescriptorWBArray[1] __attribute__ ((aligned (16)));

struct pixel {
  uint8_t dummy;
  uint8_t b;
  uint8_t g;
  uint8_t r;
};

#define N_PIXELS (WIDTH*HEIGHT)
#define HEAD_FOOT_SZ 4
struct pixel_array {
  uint8_t header[HEAD_FOOT_SZ];
  struct pixel pixels[N_PIXELS];
  uint8_t footer[HEAD_FOOT_SZ];
};

uint8_t dma_buffer0[sizeof(struct pixel_array)];
uint8_t dma_buffer1[sizeof(struct pixel_array)];  
struct pixel_array* pArray_curr = (struct pixel_array*)dma_buffer0;
struct pixel_array* pArray_next = (struct pixel_array*)dma_buffer1;

static void init_pArray(struct pixel_array* pArray)
{
  memset(pArray, 0x00, sizeof(struct pixel_array));
  memset(pArray->header, 0x00, HEAD_FOOT_SZ);
  memset(pArray->footer, 0xFF, HEAD_FOOT_SZ);

  for (int i=0; i<N_PIXELS; i++)
  {
    pArray->pixels[i].dummy = 0xFF;
    pArray->pixels[i].r = 0;
    pArray->pixels[i].g = 0;
    pArray->pixels[i].b = 0;
  }
}

static inline void swap_dma_buffer()
{
  struct pixel_array* temp = pArray_curr;
  pArray_curr = pArray_next;
  pArray_next = temp;
}

void setup_dma_channel()
{
  dmaDescriptorArray[0].bt_ctrl = (1 << 0)  | //Valid
                                  (0 << 1)  | //EVOSEL: Disable event generation
                                  (0 << 3)  | //BLOCKACT=NOACT
                                  (0 << 8)  | //Beatsize: BYTE
                                  (1 << 10) | //SRCINC: Source address inc enable
                                  (0 << 11) | //DSTINC: Dest address inc disable
                                  (1 << 12) | //STEPSEL=SRC: Step selection
                                  (0 << 13);  //Step size = X1 
  dmaDescriptorArray[0].bt_cnt = sizeof(struct pixel_array);
  dmaDescriptorArray[0].src_addr = (uint32_t)((uint8_t*)pArray_curr + sizeof(struct pixel_array));
  dmaDescriptorArray[0].dst_addr = (uint32_t)&(SERCOM1->SPI.DATA.reg);
  dmaDescriptorArray[0].desc_addr = 0;
}
void init_dma()
{
  //Configure DMA
  DMAC->CTRL.bit.DMAENABLE = 0;
  
  //Configure DMA controller
  DMAC->BASEADDR.reg = (uint32_t)dmaDescriptorArray;    //set BASEADDR for descriptor memeory section
  DMAC->WRBADDR.reg = (uint32_t)dmaDescriptorWBArray;   //set WRBADDR for 
  DMAC->CTRL.reg |= (0xF << 8);                         //set priority level x of arbiter CTRL.LVLENx (probably optional)

  
  //Configure DMA channel
  //Set channel number of dma channel with CHID
  //Set Trigger action CHCTRLB.TRIGACT
  //Set Trigger source CHCTRLB.TRIGSRC
  DMAC->CHID.reg = 0;
  DMAC->CHCTRLB.bit.TRIGACT = 0x2;//Beat
  DMAC->CHCTRLB.bit.TRIGSRC = 0x04;//SERCOM1_TX
  DMAC->CHCTRLB.bit.LVL = 0;

  //Set beat size BTCTRL.BEATSIZE
  //Setup transfer descriptor
    //struct transfer_descriptor des_base* = DMA_DES_BASE;
    //des_base[0].
    //uint16_t bt_ctrl;
  //Make transfer descriptor valid by writing 1 to BTCTRL.VALID
  //Set number of beats in block transfer BTCNT
  //Set source address for block transfer SRCADDR
  //Set dest address for block transfet DSTADDR
  setup_dma_channel();

  //CRC Calculation optional



  //Enable stuff
  PM->AHBMASK.bit.DMAC_ = 1;
  PM->APBBMASK.bit.DMAC_ = 1;
  //DMAC->CHCTRLA.bit.ENABLE = 1;   //Write Channel ID CHID.ID, then write to CHCTRLA.ENABLE to enable channel
  DMAC->CTRL.bit.DMAENABLE = 1;
  
  DMAC->CHINTENSET.bit.TCMPL = 1;

  init_pArray(pArray_curr);
  init_pArray(pArray_next);
  
  NVIC_EnableIRQ(DMAC_IRQn);
}

static inline void convert_fb_to_dma(int buf_idx, const int* buf_offset, frameBuffer* frame_buffer, struct pixel_array* pArray)
{
  int idx = 0;
  for (int k=0; k<HEIGHT; k++)
  {
    int offset = (buf_idx + buf_offset[k]) % LENGTH;
    for (int j=0; j<WIDTH; j++)
    {
      pArray->pixels[idx].b = frame_buffer->fbuf_[offset][j][k][BLUE];
      pArray->pixels[idx].g = frame_buffer->fbuf_[offset][j][k][GREEN];
      pArray->pixels[idx].r = frame_buffer->fbuf_[offset][j][k][RED];
      idx++;
    }
  }
}


void DMAC_Handler()
{
  DMAC->CHINTFLAG.bit.TCMPL = 1;

  //Setup next transfer array
  swap_dma_buffer();
  dmaDescriptorArray[0].src_addr = (uint32_t)((uint8_t*)pArray_curr + sizeof(struct pixel_array));
  transfer_complete = true;
}

static inline void start_dma_transaction()
{
  DMAC->CHCTRLA.bit.ENABLE = 1;
  transfer_complete = false;
}


#endif
