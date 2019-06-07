#include <SPI.h>
#include "wiring_private.h" // pinPeripheral() function
#include "FrameBuffer.h"
#include "Text.h"
#include "test_animations.h"

#define NUMPIXELS 24 // Number of LEDs in strip
//#define BLUE 2
//#define GREEN 1
//#define RED 0

#define LED_PIN 9
#define HALL_PIN 6
#define CPU_HZ 48000000
#define TIMER_PRESCALER_DIV 1024

#define CPU_HZ_SCALE 48ull



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


void hallEffectSetup()
{
  //Setup hall effect
  pinMode(6, INPUT);
  hall = digitalRead(6); // Don't think is necessary
  timer_delta = 0;
  timer_0 = micros();
  
  attachInterrupt(6, hallTrigger, FALLING);
  delay(5);// Allow time for timer delta to setup
}
void sercomSetup()
{
  //writeString("HELLO", 15, 2, 255, 64, 128);
  mySPI.begin();

  // Assign pins 11, 12, 13 to SERCOM functionality
  pinPeripheral(11, PIO_SERCOM);
  pinPeripheral(12, PIO_SERCOM);
  pinPeripheral(13, PIO_SERCOM);
}
void setup()
{
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  frame_buffer.reset();
  sercomSetup();
  hallEffectSetup();
  startTimer(1800);
}

void setup_() {
  pinMode(6, INPUT);
  hall = digitalRead(6);
  timer_delta = 0;
  timer_0 = micros();
  
  attachInterrupt(6, hallTrigger, FALLING);
  delay(5); // Allow time for t
  

  //clearBuf();
  frame_buffer.reset();
  
  //writeString("HELLO", 15, 2, 255, 64, 128);
  mySPI.begin();

  // Assign pins 11, 12, 13 to SERCOM functionality
  pinPeripheral(11, PIO_SERCOM);
  pinPeripheral(12, PIO_SERCOM);
  pinPeripheral(13, PIO_SERCOM);
  
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
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
      delay(7);  
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




/*********************************************************************/
/*                                                                   */
/*                       LOOP STARTS HERE                            */
/*                                                                   */
/*********************************************************************/
void loop() 
{
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
  frame_buffer.reset();
  /*
  while(1)
  {
    int r, g, b;
    
    frame_buffer.clear();
    for (int j=0; j<WIDTH; j++)
    {
      doubleBuffer::randColor(&r, &g, &b);
      for (int k=0; k<HEIGHT; k++)
      {
        frame_buffer.setColors(0, j, k, r, g, b);
        frame_buffer.setColors(15, j, k, r, g, b);
        frame_buffer.setColors(30, j, k, r, g, b);
        frame_buffer.setColors(45, j, k, r, g, b);
      }
    }
    frame_buffer.update();
    delay(1000);  
  }
  return;
  */
  //pulseAnimation(&frame_buffer);
  //return;


  textAnimation(&frame_buffer);
  pinWheelAnimation_0(&frame_buffer);
  vortexAnimation(&frame_buffer);

  frame_buffer.clear();
  frame_buffer.update();
  delay(500);

  multicolorFillAnimation(&frame_buffer);
  pinWheelAnimation_1(&frame_buffer);
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
void setTimerPeriod(int period_us) {//period in us
  int compareValue = ((CPU_HZ_SCALE*period_us) / TIMER_PRESCALER_DIV) - 1;
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

void startTimerPeriod(int period_us) {
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

  setTimerPeriod(period_us);

  // Enable the compare interrupt
  TC->INTENSET.reg = 0;
  TC->INTENSET.bit.MC0 = 1;

  NVIC_EnableIRQ(TC3_IRQn);

  TC->CTRLA.reg |= TC_CTRLA_ENABLE;
  while (TC->STATUS.bit.SYNCBUSY == 1); // wait for sync
}



int temp_offset[HEIGHT];

frameBuffer* read_buffer;
void TC3_Handler() {
  TcCount16* TC = (TcCount16*) TC3;
  // If this interrupt is due to the compare register matching the timer count
  // we toggle the LED.
  if (TC->INTFLAG.bit.MC0 == 1) {
    TC->INTFLAG.bit.MC0 = 1;
    // Write callback here!!!
    if (digitalRead(HALL_PIN))
      digitalWrite(LED_PIN, LOW);
    if (buf_idx >= LENGTH)
        return;
  
    driveLEDS(buf_idx, (int*)buf_offset, &frame_buffer, &mySPI);
    buf_idx++;
  }
}


void hallTrigger()
{
  digitalWrite(LED_PIN, HIGH);

  //Reset timer to zero
  TcCount16* TC = (TcCount16*) TC3;
  TC->COUNT.reg = 0;
  
  
  driveLEDS(buf_idx, (int*)buf_offset, &frame_buffer, &mySPI);
  buf_idx = 1;
  while (TC->STATUS.bit.SYNCBUSY == 1);
  return;
  
  /*
  long timer_temp = micros();
  timer_delta = timer_temp - timer_0;
  if (timer_delta < 15000)
    timer_delta = 15000;
  timer_0 = timer_temp;
  */
  //buf_idx = 0; 
  //startTimerPeriod(timer_delta/LENGTH);
  //return;
}


