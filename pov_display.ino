#include <SPI.h>
#include "wiring_private.h" // pinPeripheral() function

#include "FrameBuffer.h"
#include "Text.h"

#define NUMPIXELS 24 // Number of LEDs in strip
//#define BLUE 2
//#define GREEN 1
//#define RED 0

#define LED_PIN 13

#define CPU_HZ 48000000
#define TIMER_PRESCALER_DIV 1024


//#define LENGTH 60
//#define WIDTH 8
//#define HEIGHT 3
//#define COLORS 3


//volatile uint8_t fbuf[LENGTH][WIDTH][HEIGHT][COLORS];
volatile uint8_t buf_idx = 0;
volatile int buf_offset[HEIGHT] = {20, 30, 40, 50, 0, 10};

doubleBuffer frame_buffer;

unsigned long long timer;
bool flag = false;
int hall;

//MOSI on Digital 11
//SCK on 13
SPIClass mySPI (&sercom1, 12, 13, 11, SPI_PAD_0_SCK_1, SERCOM_RX_PAD_3);

//void clearBuf()
//{
//  for (int i=0; i<LENGTH; i++)
//  {
//    for (int j=0; j<WIDTH; j++)
//    {
//      for (int k=0; k<HEIGHT; k++)
//      {
//        fbuf[i][j][k][0] = 0;
//        fbuf[i][j][k][1] = 0;
//        fbuf[i][j][k][2] = 0;
//      }
//    }
//  }
//}


class test_class{
  public:
    int height;
    int spd_top;
    int spd;
    int vel;
    uint8_t r, g, b;

    test_class();
    void update();
  
};

test_class::test_class()
{
  height = rand() % (HEIGHT+1);
  vel = rand() % 2;
  spd_top = rand() % 6 + 2;
  spd = 0;

  r = 0;
  b = 0;
  g = 0;

  while (r + b + g < 128)
  {
    r = rand() % 256;
    g = rand() % 256;
    b = rand() % 256;
  }
}
void test_class::update()
{
  spd++;
  if (spd >= spd_top)
  {
   if (vel)//up
   {
     height++;
     if (height > HEIGHT)
     {
      height = HEIGHT;
      vel = 1;
     }
   }
   else//down
   {
    height--;
    if (height < 0)
    {
      height = 0;
      vel = 0;
    }
   }
    
    spd = 0;
  }
}

test_class tests[LENGTH];

void startTimer(int frequencyHz);
void setTimerFrequency(int frequencyHz);
void TC3_Handler();
void hallTrigger();

void setColor(int length, int width, int height, byte red, byte green, byte blue) {
  fbuf[length][width][height][RED] = red;
  fbuf[length][width][height][GREEN] = green;
  fbuf[length][width][height][BLUE] = blue;
}

long timer_0, timer_delta;
void setup() {
  pinMode(6, INPUT);
  hall = digitalRead(6);
  timer_0 = millis();
  timer_delta = 0;
  attachInterrupt(6, hallTrigger, FALLING);


  

  //clearBuf();
  frame_buffer.clear();

  for (int j=0; j<WIDTH; j++)
  {
    for (int k=0; k<HEIGHT; k++)
    {
      fbuf[0][j][k][GREEN] = 255;
      fbuf[0][j][k][BLUE] = 255;
    }
  }
  for (int j=0; j<WIDTH; j++)
  {
    for (int k=0; k<HEIGHT; k++)
    {
      fbuf[LENGTH-1][j][k][RED] = 255;
    }
  }
  
  writeString("HELLO", 15, 2, 255, 64, 128);
  mySPI.begin();

  // Assign pins 11, 12, 13 to SERCOM functionality
  pinPeripheral(11, PIO_SERCOM);
  pinPeripheral(12, PIO_SERCOM);
  pinPeripheral(13, PIO_SERCOM);
  
  //pinMode(LED_PIN, OUTPUT);
  timer = micros();
  startTimer(1800);



  frame_buffer.forceSingleBuffer();
  for (int c=0; c<3; c++)
  {
    frame_buffer.clear();
    for (int i=0; i<LENGTH; i++)
    {
      for (int j=0; j<WIDTH; j++)
      {
        for (int k=0; k<HEIGHT; k++)
        {
          frame_buffer.setColorChannel(i, j, k, c, 255); 
        }
      }
      frame_buffer.update();
      delay(20);  
    }
  }
  frame_buffer.reset();

  //clearBuf();
  frame_buffer.clear();

  
  //int h = 0;
  for (int i=0; i<LENGTH; i++)
  {
    for (int j = 0; j<WIDTH; j++)
    {
      //fbuf[i][j][i%HEIGHT][BLUE] = 255;
      frame_buffer.setColorChannel(i, j, i%HEIGHT, BLUE, 255);
    }
    continue;
    
    int j = i % 2*WIDTH - 2;
    if (j < WIDTH)
    {
      fbuf[i][j][0][BLUE] = 255;
    }
    else
    {
      j = 2*WIDTH - 2 - j;
      fbuf[i][j][0][BLUE] = 255;
    }
  }
  for (int i=0; i<WIDTH; i++)
  {
    for (int j=0; j<HEIGHT; j++)
      fbuf[0][i][j][RED] = 255;
    
  }
  frame_buffer.update();
  delay(5000);
}







int lookup[10] = {0, 1, 2, 3, 4, 5, 4, 3, 2, 1};
int lookup2[10] = {0, 0, 0, 0, 0, 7, 7, 7, 7, 7};


int N_CYCLE = 60;
int num_cnt = 0;
char cnt_str[10];


void randColor(int *r, int *g, int *b)
{
  int sel = rand() % 6;
    int r_, g_, b_;
    switch (sel)
    {
      case 0:
        r_ = 255;
        b_ = 0;
        g_ = rand() % 256;
        break;
      case 1:
        r_ = 255;
        b_ = rand() % 256;
        g_ = 0;
        break;
      case 2:
        r_ = 0;
        b_ = 255;
        g_ = rand() % 256;
        break;
      case 3:
        r_ = rand() % 256;
        b_ = 255;
        g_ = 0;
        break;
      case 4:
        r_ = 0;
        b_ = rand() % 256;
        g_ = 255;
        break;
      case 5:
        r_ = rand() % 256;
        b_ = 0;
        g_ = 255;
        break;
    }
    *r = r_;
    *g = g_;
    *b = b_;
}


void loop() 
{
  frame_buffer.reset();
   char *text[4] = {"HACK A DAY",
                   "HI NOLAN",
                   "MAKERFAIRE 2018",
                   "DONT LET YOUR DREAMS BE DREAMS"};
  int text_sel = rand() % 4;
  int height = rand()%HEIGHT;
  int text_len = strlen(text[text_sel]);
  uint8_t r = 0;
  uint8_t g = rand() % 128 + 128;
  uint8_t b = 255;
  for (int i=LENGTH + 5; i>(-2*text_len - 5); i--)
  {
    frame_buffer.clear();
    writeString(text[text_sel], i, height, r, g, b, &frame_buffer);
    frame_buffer.update();
    delay(40);
  }



  /*
    sprintf(cnt_str, "%d", timer_delta);
    frame_buffer.clear();
    writeString(cnt_str, 20, 5, 180, 60, 255, &frame_buffer);
    frame_buffer.update();
    
    //num_cnt++;
    //if (num_cnt > 150)
    //  num_cnt = 0;
    //delay(200);
    return;
    */
    
    int sel = rand() % 6;
    int r_, g_, b_;
    switch (sel)
    {
      case 0:
        r_ = 255;
        b_ = 0;
        g_ = rand() % 256;
        break;
      case 1:
        r_ = 255;
        b_ = rand() % 256;
        g_ = 0;
        break;
      case 2:
        r_ = 0;
        b_ = 255;
        g_ = rand() % 256;
        break;
      case 3:
        r_ = rand() % 256;
        b_ = 255;
        g_ = 0;
        break;
      case 4:
        r_ = 0;
        b_ = rand() % 256;
        g_ = 255;
        break;
      case 5:
        r_ = rand() % 256;
        b_ = 0;
        g_ = 255;
        break;
    }

    frame_buffer.reset();
    for (int cycles=0; cycles<N_CYCLE; cycles++)
    {
    frame_buffer.clear();
    for (int i=0; i<LENGTH; i++)
    {
      //int idx = (i+cycles)% (LENGTH-10);
      //float W = 3*sin(2*PI*i/10.0 + cycles)+3;

      
      
      int W_0 = lookup[(i+cycles)%10];
      int W_1 = lookup[(i+N_CYCLE-cycles)%10]; 
      int W_2 = lookup[(i+cycles+6)%10];
      //if (W_ < WIDTH && W_ >= 0)

      frame_buffer.setColorChannel(i, W_0, 0, RED, 255);
      frame_buffer.setColorChannel(i, W_1, 2, GREEN, 255);
      //frame_buffer.setColorChannel(i, W_2, 5, BLUE, 255);

      frame_buffer.setColors(i, W_2, 5, r_, g_, b_);
      
      //fbuf[i][W_0][0][RED] = 255; 
      //fbuf[i][W_1][2][GREEN] = 255;
      //fbuf[i][W_2][5][BLUE] = 255;
    }
    frame_buffer.update();
    delay(50);
    }


    frame_buffer.forceSingleBuffer();
    for (int cycles=0; cycles<N_CYCLE; cycles++)
    {
    frame_buffer.clear();
    for (int i=0; i<LENGTH; i++)
    {
      //int idx = (i+cycles)% (LENGTH-10);
      //float W = 3*sin(2*PI*i/10.0 + cycles)+3;

      
      
      int W_0 = lookup[(i+cycles)%10];
      int W_1 = lookup[(i+N_CYCLE-cycles)%10]; 
      int W_2 = lookup[(i+cycles+6)%10];
      //if (W_ < WIDTH && W_ >= 0)

      frame_buffer.setColorChannel(i, W_0, 0, RED, 255);
      frame_buffer.setColorChannel(i, W_1, 2, GREEN, 255);
      //frame_buffer.setColorChannel(i, W_2, 5, BLUE, 255);

      frame_buffer.setColors(i, W_2, 5, r_, g_, b_);
      
      //fbuf[i][W_0][0][RED] = 255; 
      //fbuf[i][W_1][2][GREEN] = 255;
      //fbuf[i][W_2][5][BLUE] = 255;
    }
    frame_buffer.update();
    delay(50);
    }


   // return;

    
  

    frame_buffer.clear();

  
/*
  for (int i=0; i<LENGTH; i++)
  {
    for (int j = 0; j<WIDTH; j++)
    {
      //fbuf[i][j][i%HEIGHT][BLUE] = 255;
      frame_buffer.setColorChannel(i, j, i%HEIGHT, GREEN, 255);
    }
    continue;
    
    int j = i % 2*WIDTH - 2;
    if (j < WIDTH)
    {
      fbuf[i][j][0][BLUE] = 255;
    }
    else
    {
      j = 2*WIDTH - 2 - j;
      fbuf[i][j][0][BLUE] = 255;
    }
  }
  for (int i=0; i<WIDTH; i++)
  {
    for (int j=0; j<HEIGHT; j++)
      fbuf[0][i][j][RED] = 255;
    
  }
  frame_buffer.update();
  delay(5000);
*/




  /*
  for (int i=0; i<LENGTH; i++)
  {
    tests[i].update();
    for (int j=0; j<WIDTH; j++)
    {
      if (tests[i].height > 0)
      {
        int h = tests[i].height - 1;
        fbuf[i][j][h][RED] = tests[i].r;
        fbuf[i][j][h][GREEN] = tests[i].g;
        fbuf[i][j][h][BLUE] = tests[i].b;
      }
    }
  }
  delay(100);

  return;
  */

  frame_buffer.forceSingleBuffer();
  int color_r, color_g, color_b;
  randColor(&color_r, &color_g, &color_b);
  
  
  for (int cycles = 0; cycles <400; cycles++)
  {
    frame_buffer.clear();
    if (cycles % 10 == 0)
      randColor(&color_r, &color_g, &color_b);
    for (int i=0; i<LENGTH; i++)
    {
      //int idx = (i+cycles)% (LENGTH-10);
      //float W = 3*sin(2*PI*i/10.0 + cycles)+3;
     
      
      int W_0 = (lookup2[(i+cycles)%10] + cycles) % 8;
      int W_1 = (lookup2[(i+cycles+3)%10] + cycles + 1) % 8; 
      int W_2 = (lookup2[(i+cycles+6)%10] + cycles + 2) % 8; 
      int W_3 = (lookup2[(i+cycles+9)%10] + cycles + 3) % 8;
      int W_4 = (lookup2[(i+cycles+12)%10] + cycles + 4) % 8;
      int W_5 = (lookup2[(i+cycles+15)%10] + cycles + 5) % 8;
      //if (W_ < WIDTH && W_ >= 0)

      

      frame_buffer.setColors(i, W_0, 0, color_r, color_g, color_b);
      frame_buffer.setColors(i, W_1, 1, color_r, color_g, color_b);
      frame_buffer.setColors(i, W_2, 2, color_r, color_g, color_b);
      frame_buffer.setColors(i, W_3, 3, color_r, color_g, color_b);
      frame_buffer.setColors(i, W_4, 4, color_r, color_g, color_b);
      frame_buffer.setColors(i, W_5, 5, color_r, color_g, color_b);

      /*
      frame_buffer.setColors(i, W_0, 0, 10, 200, 200 + (rand() % 29));
      frame_buffer.setColors(i, W_1, 1, 10, 200, 200 + (rand() % 55));
      frame_buffer.setColors(i, W_2, 2, 10, 200, 200 + (rand() % 55));
      frame_buffer.setColors(i, W_3, 3, 10, 200, 200 + (rand() % 55));
      frame_buffer.setColors(i, W_4, 4, 10, 200, 200 + (rand() % 55));
      frame_buffer.setColors(i, W_5, 5, 10, 200, 200 + (rand() % 55));
      */
    }
    frame_buffer.update();
    delay(50);
  }
  

  frame_buffer.clear();
  frame_buffer.update();
  delay(500);
  for (int k=0; k<HEIGHT; k++)
  {
    for (int j=0; j<WIDTH; j++)
    {
      for (int i=LENGTH-1; i>=0; i--)
      {
        int sel = rand() % 6;
        int r_, g_, b_;
        switch (sel)
        {
          case 0:
            r_ = 255;
            b_ = 0;
            g_ = rand() % 256;
            break;
          case 1:
            r_ = 255;
            b_ = rand() % 256;
            g_ = 0;
            break;
          case 2:
            r_ = 0;
            b_ = 255;
            g_ = rand() % 256;
            break;
          case 3:
            r_ = rand() % 256;
            b_ = 255;
            g_ = 0;
            break;
          case 4:
            r_ = 0;
            b_ = rand() % 256;
            g_ = 255;
            break;
          case 5:
            r_ = rand() % 256;
            b_ = 0;
            g_ = 255;
            break;
        }

        frame_buffer.setColors(i, j, k, r_, g_, b_);
        frame_buffer.update();
        delayMicroseconds(50);
      }
    }
    //delay(500);
  }



  //for (int i=0; i<


  for (int cycles = 0; cycles <400; cycles++)
  {
    frame_buffer.clear();
    for (int i=0; i<LENGTH; i++)
    {
      //int idx = (i+cycles)% (LENGTH-10);
      //float W = 3*sin(2*PI*i/10.0 + cycles)+3;

      
      
      int W_0 = lookup[(i+cycles)%10];
      int W_1 = lookup[(i+400-cycles)%10]; 
      int W_2 = lookup[(i+cycles+6)%10];
      //if (W_ < WIDTH && W_ >= 0)

      frame_buffer.setColorChannel(i, W_0, 0, RED, 255);
      frame_buffer.setColorChannel(i, W_0, 2, GREEN, 255);
      frame_buffer.setColorChannel(i, W_0, 5, BLUE, 255);
      //fbuf[i][W_0][0][RED] = 255; 
      //fbuf[i][W_1][2][GREEN] = 255;
      //fbuf[i][W_2][5][BLUE] = 255;
    }
    frame_buffer.update();
    delay(50);
  }
  return;

  
  clearBuf();

  int h = 0;
  for (int i=0; i<LENGTH; i++)
  {
    for (int j = 0; j<WIDTH; j++)
    {
      fbuf[i][j][i%HEIGHT][BLUE] = 255;
    }
    continue;
    
    int j = i % 2*WIDTH - 2;
    if (j < WIDTH)
    {
      fbuf[i][j][0][BLUE] = 255;
    }
    else
    {
      j = 2*WIDTH - 2 - j;
      fbuf[i][j][0][BLUE] = 255;
    }
  }
  for (int i=0; i<WIDTH; i++)
  {
    for (int j=0; j<HEIGHT; j++)
      fbuf[0][i][j][RED] = 255;
    
  }
  delay(1000);
  return;


/*
  
  //resolveChar(CHAR_BUF[0]);
  char *text[4] = {"HELLO LAURYN  HOW ARE YOU",
                   "HI NOLAN",
                   "I SURE HOPE THIS ACTUALLY WORKS",
                   "DONT LET YOUR DREAMS BE DREAMS"};
  int text_sel = rand() % 4;
  int height = rand()%HEIGHT;
  int text_len = strlen(text[text_sel]);
  uint8_t r = 0;
  uint8_t g = rand() % 128 + 128;
  uint8_t b = 255;
  for (int i=LENGTH + 5; i>(-2*text_len - 5); i--)
  {
    clearBuf();
    writeString(text[text_sel], i, height, r, g, b);
    delay(100);
  }


  */
  //if (attach && (millis() - timer > 3))
  //{
  //  attachInterrupt(10, hallTrigger, FALLING);
  //}

  /*
  clearBuf();
  for (int i=0; i<LENGTH; i++)
  {
    for (int j=0; j<WIDTH; j++)
    {
      fbuf[i][j][0][RED] = 255;
      fbuf[i][j][1][GREEN] = 255;
      fbuf[i][j][2][BLUE] = 255;
    }
  }
  delay(500);
  */
}

void setTimerFrequency(int frequencyHz) {
  int compareValue = (CPU_HZ / (TIMER_PRESCALER_DIV * frequencyHz)) - 1;
  TcCount16* TC = (TcCount16*) TC3;
  // Make sure the count is in a proportional position to where it was
  // to prevent any jitter or disconnect when changing the compare value.
  TC->COUNT.reg = map(TC->COUNT.reg, 0, TC->CC[0].reg, 0, compareValue);
  TC->CC[0].reg = compareValue;
  //Serial.println(TC->COUNT.reg);
  //Serial.println(TC->CC[0].reg);
  while (TC->STATUS.bit.SYNCBUSY == 1);
}

void startTimer(int frequencyHz) {
  REG_GCLK_CLKCTRL = (uint16_t) (GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID_TCC2_TC3) ;
  while ( GCLK->STATUS.bit.SYNCBUSY == 1 ); // wait for sync

  TcCount16* TC = (TcCount16*) TC3;

  TC->CTRLA.reg &= ~TC_CTRLA_ENABLE;
  while (TC->STATUS.bit.SYNCBUSY == 1); // wait for sync

  // Use the 16-bit timer
  TC->CTRLA.reg |= TC_CTRLA_MODE_COUNT16;
  while (TC->STATUS.bit.SYNCBUSY == 1); // wait for sync

  // Use match mode so that the timer counter resets when the count matches the compare register
  TC->CTRLA.reg |= TC_CTRLA_WAVEGEN_MFRQ;
  while (TC->STATUS.bit.SYNCBUSY == 1); // wait for sync

  // Set prescaler to 1024
  TC->CTRLA.reg |= TC_CTRLA_PRESCALER_DIV1024;
  while (TC->STATUS.bit.SYNCBUSY == 1); // wait for sync

  setTimerFrequency(frequencyHz);

  // Enable the compare interrupt
  TC->INTENSET.reg = 0;
  TC->INTENSET.bit.MC0 = 1;

  NVIC_EnableIRQ(TC3_IRQn);

  TC->CTRLA.reg |= TC_CTRLA_ENABLE;
  while (TC->STATUS.bit.SYNCBUSY == 1); // wait for sync
}




int temp_offset[HEIGHT];

void TC3_Handler() {
  //digitalWrite(10, HIGH);
  TcCount16* TC = (TcCount16*) TC3;
  // If this interrupt is due to the compare register matching the timer count
  // we toggle the LED.
  if (TC->INTFLAG.bit.MC0 == 1) {
    TC->INTFLAG.bit.MC0 = 1;
    // Write callback here!!!

  uint8_t hall_ = digitalRead(6);
  if(!hall_ && hall)
  {
    buf_idx = 0;
  }
  hall = hall_;
  if (buf_idx >= LENGTH)
      return;
  

  frameBuffer* read_buffer = frame_buffer.getReadBuffer();

   for (int k=0; k<HEIGHT; k++)
   {
    temp_offset[k] = (buf_idx + buf_offset[k]) % LENGTH;
   }
    
    mySPI.beginTransaction(SPISettings(12000000, MSBFIRST, SPI_MODE0));

    //Start Frame
    mySPI.transfer(0x00);
    mySPI.transfer(0x00);
    mySPI.transfer(0x00);
    mySPI.transfer(0x00);
    mySPI.transfer(0x00);

    for (int k=0; k<HEIGHT; k++)
    {
      for (int j=0; j<WIDTH; j++)
      {
        mySPI.transfer(0xFF);
        /*
        mySPI.transfer(fbuf[temp_offset[k]][j][k][BLUE]);
        mySPI.transfer(fbuf[temp_offset[k]][j][k][GREEN]);
        mySPI.transfer(fbuf[temp_offset[k]][j][k][RED]);
        */
        mySPI.transfer(read_buffer->fbuf_[temp_offset[k]][j][k][BLUE]);
        mySPI.transfer(read_buffer->fbuf_[temp_offset[k]][j][k][GREEN]);
        mySPI.transfer(read_buffer->fbuf_[temp_offset[k]][j][k][RED]);
      }
    }

    //End Frame
      mySPI.transfer(0xFF);
      mySPI.transfer(0xFF);
      mySPI.transfer(0xFF);
      mySPI.transfer(0xFF);
      mySPI.transfer(0xFF);

    mySPI.endTransaction();


//    for (int k=0; k<HEIGHT; k++)
//    {
//      buf_offset[k]++;
//      if (buf_offset[k] >= LENGTH)
//        buf_offset[k] = 0;
//    }
    buf_idx++;
    //if (buf_idx >= LENGTH)
    //  buf_idx = 0;
  }
  //digitalWrite(10, LOW);
}


void hallTrigger()
{
  long timer_temp = millis();
  timer_delta = timer_temp - timer_0;
  timer_0 = timer_temp;
  return;
  
  buf_idx = 0;
  
  return;
  //delay(1);
  if (millis() - timer < 15)
    return;
  //buf_idx = 0;
  //detachInterrupt(10);
  flag = true;
  timer = millis();
}

