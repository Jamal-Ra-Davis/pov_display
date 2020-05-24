#include <SPI.h>
#include <math.h>
#include "wiring_private.h" // pinPeripheral() function
#include "FrameBuffer.h"
#include "Text.h"
#include "test_animations.h"
#include "Vector3d.h"
#include "HSV.h"
#include "Space_Game.h"
#include "Events.h"
#include "Snake.h"


#define LED_PIN 9
#define HALL_PIN 6
#define CPU_HZ 48000000
#define TIMER_PRESCALER_DIV 1024

#define CPU_HZ_SCALE 48ull


volatile uint8_t buf_idx = 0;
volatile int buf_offset[HEIGHT] = {20, 30, 40, 50, 0, 10};

doubleBuffer frame_buffer;

int hall;

//MOSI on Digital 11
//SCK on 13
SPIClass mySPI (&sercom1, 12, 13, 11, SPI_PAD_0_SCK_1, SERCOM_RX_PAD_3);


void startTimer(int frequencyHz);
void setTimerFrequency(int frequencyHz);
void TC3_Handler();
void hallTrigger();

long timer_0, timer_delta;


void hallEffectSetup()
{
  //Setup hall effect
  pinMode(HALL_PIN, INPUT);
  hall = digitalRead(HALL_PIN); // Don't think is necessary
  timer_delta = 0;
  timer_0 = micros();
  
  attachInterrupt(HALL_PIN, hallTrigger, FALLING);
  delay(5);// Allow time for timer delta to setup
}
void sercomSetup()
{
  mySPI.begin();

  // Assign pins 11, 12, 13 to SERCOM functionality
  pinPeripheral(11, PIO_SERCOM);
  pinPeripheral(12, PIO_SERCOM);
  pinPeripheral(13, PIO_SERCOM);
}
void setup()
{
  SerialUSB.begin(9600);
  //while(!SerialUSB);
  Serial1.begin(9600);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  frame_buffer.reset();
  sercomSetup();
  hallEffectSetup();
  startTimer(1800);
  frame_buffer.reset();
}


/*********************************************************************/
/*                                                                   */
/*                       LOOP STARTS HERE                            */
/*                                                                   */
/*********************************************************************/


void block_test();
void endless_runner();
void ship_game();
void loop() 
{
  ship_loop(&frame_buffer);
  return;
  
  frame_buffer.forceSingleBuffer();
  frame_buffer.clear();
  uint16_t color = 0;
  int r = 0;
  int g =0;
  int b = 0;
  int idx = 0;
  while(1)
  {
    r += 5;
    if (r >= 256)
    {
      r = 0;
      g += 5;
      if (g >= 256)
      {
        g = 0;
        b += 5;
        if (b >= 256)
        {
          b = 0;
        }
      }
    }
    
    color++;
    if (color >= 0x1FF)
      color = 0;
      
    idx++;
    if (idx >= LENGTH)
      idx = 0;

    for (int j=0; j<WIDTH; j++)
    {
      frame_buffer.setColors(idx, j, 0, color & (0x03), (color >> 3) & (0x03), (color >> 6) & 0x03);
      frame_buffer.setColors(idx, j, 3, r, g, b);
      frame_buffer.setColors(idx, j, 5, 255, 0, 0);
    }
    frame_buffer.update();
    delay(35);
  } 
}


void setTimerFrequency(int frequencyHz) {
  int compareValue = (CPU_HZ / (TIMER_PRESCALER_DIV * frequencyHz)) - 1;
  TcCount16* TC = (TcCount16*) TC3;
  
  // Make sure the count is in a proportional position to where it was
  // to prevent any jitter or disconnect when changing the compare value.
  TC->COUNT.reg = map(TC->COUNT.reg, 0, TC->CC[0].reg, 0, compareValue);
  TC->CC[0].reg = compareValue;
  while (TC->STATUS.bit.SYNCBUSY == 1);
}
void setTimerPeriod(int period_us) {//period in us
  int compareValue = ((CPU_HZ_SCALE*period_us) / TIMER_PRESCALER_DIV) - 1;
  TcCount16* TC = (TcCount16*) TC3;
  
  // Make sure the count is in a proportional position to where it was
  // to prevent any jitter or disconnect when changing the compare value.
  TC->COUNT.reg = map(TC->COUNT.reg, 0, TC->CC[0].reg, 0, compareValue);
  TC->CC[0].reg = compareValue;
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
  NVIC_SetPriority(TC3_IRQn, 4); 

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
  NVIC_SetPriority(TC3_IRQn, 4); 

  TC->CTRLA.reg |= TC_CTRLA_ENABLE;
  while (TC->STATUS.bit.SYNCBUSY == 1); // wait for sync
}


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

void block_test()
{
  SerialUSB.println("Entering blocktest");
  Vector3d vec0_s(0, 0, 0), vec0_e(9, 4, 1);
  int r0, g0, b0;
  doubleBuffer::randColor(&r0, &g0, &b0);
  
  Vector3d vec1_s(15, 0, 3), vec1_e(17, 6, 5);
  int r1, g1, b1;
  doubleBuffer::randColor(&r1, &g1, &b1);
  
  while(1)
  {
    frame_buffer.clear();
    frame_buffer.drawBlock(vec0_s, vec0_e, r0, g0, b0);
    frame_buffer.drawBlock(vec1_s, vec1_e, r1, g1, b1);
   
    for (int i=0; i<WIDTH; i++)
    {
      frame_buffer.setColors(20, i, 0, 0xFF, 0x00, 0x00);
      frame_buffer.setColors(20-1, i, 0, 0x80, 0x00, 0xFF);
    }
    frame_buffer.update();
    

    vec0_s.addVector3d(1, 0, 0);
    vec0_e.addVector3d(1, 0, 0);
    vec1_s.addVector3d(1, 0, 0);
    vec1_e.addVector3d(1, 0, 0);
    if (vec0_s.x > LENGTH)
    {
      vec0_s.addVector3d(-(LENGTH+10), 0, 0);
      vec0_e.addVector3d(-(LENGTH+10), 0, 0);
      doubleBuffer::randColor(&r0, &g0, &b0);
    }
    if (vec1_s.x > LENGTH)
    {
      vec1_s.addVector3d(-(LENGTH+10), 0, 0);
      vec1_e.addVector3d(-(LENGTH+10), 0, 0);
      doubleBuffer::randColor(&r1, &g1, &b1);
    }
  
    delay(20);
  }
}
void endless_runner()
{
  Vector3d p_pos(50, 3, 0);
  Vector3d p_size(2, 2, 4);

  Vector3d blocks[5][2];
  bool block_active[5] = {false, false, false, false, false};
  int block_spawn = 10;


  bool crouch = false;
  bool left = false;
  bool right = false;
  while(1)
  {
    bool p1 = false;
    bool r1 = false;
    bool p2 = false;
    bool r2 = false;
    bool p3 = false;
    bool r3 = false;
    while (Serial1.available() > 1)
    {
      SerialUSB.print("Bytes to be read: ");
      SerialUSB.println(Serial1.available());
      char idx = Serial1.read();
      SerialUSB.println(idx);
      if (idx != '1' && idx != '2' && idx != '3')
        continue;

      
      char status = Serial1.read();
      Serial1.read();
      switch(idx)
      {
        case '1':
        {
          if (status == 'p')
          {
            SerialUSB.println("Press 1");
            left = true;
          }
          else if (status = 'r')
          {
            SerialUSB.println("Release 1");
            left = false;
          }
          break;
        }
        case '2':
        {
          if (status == 'p')
          {
            SerialUSB.println("Press 2");
            crouch = true;
          }
          else if (status = 'r')
          {
            SerialUSB.println("Release 2");
            crouch = false;
          }
          break;
        }
        case '3':
        {
          if (status == 'p')
          {
            SerialUSB.println("Press 3");
            right = true;
          }
          else if (status = 'r')
          {
            SerialUSB.println("Release 3");
            right = false;
          }
          break;
        }
      }
    }

    if (crouch)
    {
      p_size.setVector3d(2, 2, 2);
    }
    else 
    {
      p_size.setVector3d(2, 2, 4); 
    }

    if (left)
    {
      p_pos.y--;
      if (p_pos.y < 0)
      {
        p_pos.y = 0;
      }
    }
    else if (right)
    {
      p_pos.y++;
      if ((p_pos.y + p_size.y) > WIDTH )
      {
        p_pos.y = WIDTH - p_size.y;
      }
    }

    //Spawn blocks
    if (block_spawn <= 0)
    {
      block_spawn = rand() % 5 + 15;
      int idx = -1;
      for (int i=0; i<5; i++)
      {
        if (!block_active[i])
        {
          idx = i;
          break;
        }
      }
      if (idx >= 0)
      {
        block_active[idx] = true;
        int spawn_type = rand() % 3;
        switch(spawn_type)
        {
          case 0:
          {
            //left
            
            blocks[idx][0].setVector3d(-10, 0, 0);
            blocks[idx][1].setVector3d(-6, 3, 5);
            break;
          }
          case 1:
          {
            //right
            blocks[idx][0].setVector3d(-10, 4, 0);
            blocks[idx][1].setVector3d(-6, 7, 5);
            break;
          }
          case 2:
          {
            //top
            blocks[idx][0].setVector3d(-10, 0, 3);
            blocks[idx][1].setVector3d(-6, 7, 5);
            break;
          }
        }
      }
    }
    block_spawn--;

    frame_buffer.clear();
    //Update/draw blocks
    for (int i=0; i<5; i++)
    {
      if (!block_active[i])
        continue;
      blocks[i][0].addVector3d(1, 0, 0);
      blocks[i][1].addVector3d(1, 0, 0);
      if (blocks[i][0].x >= LENGTH)
        block_active[i] = false;

      frame_buffer.drawBlock(blocks[i][0], blocks[i][1], 255, 0, 0);
    }

    frame_buffer.drawBlock(Vector3d(0, 0, 0), Vector3d(3, 7, 5), 255, 255, 255);
    
    Vector3d pos_ = p_pos;
    pos_.addVector3d(p_size.x - 1, p_size.y - 1, p_size.z - 1);
    frame_buffer.drawBlock(p_pos, pos_, 0, 0, 255);

    for (int i=0; i<WIDTH; i++)
    {
      frame_buffer.setColors(20, i, 0, 0xFF, 0x00, 0x00);
      frame_buffer.setColors(20-1, i, 0, 0x80, 0x00, 0xFF);
    }
    frame_buffer.update();
    delay(33);
  }
}

void ship_game()
{
   Vector3d pos(0, 4, 3);
   Vector3d vel(1, 0, 0);

   float color_scale;
   float color_idx = 0;

   Vector3d stars[10];
   for (int i=0; i<10; i++)
   {
    stars[i].setVector3d(rand() % LENGTH, rand() % WIDTH, rand() % HEIGHT);
   }
   int star_update = 100;
   int len = 3;
   Vector3d bullets[5];
   int bullet_vel[5];
   int active_bullets[5] = {-1, -1, -1, -1, -1};


   while(1)
   {
      frame_buffer.clear();
      
      //Update
      pos.addVector3d(vel);
      if (pos.x >= LENGTH)
        pos.x = 0;
      if (pos.x < 0)
        pos.x = LENGTH - 1;


      if (rand() % 50 == 0)
      {
        switch (rand() % 4)
        {
          case 0:
          {
            pos.y++;
            if (pos.y >= WIDTH)
              pos.y = WIDTH - 1;
            break;
          }
          case 1:
          {
            pos.y--;
            if (pos.y < 0)
              pos.y = 0;
            break;
          }
          case 2:
          {
            pos.z++;
            if (pos.z >= HEIGHT)
              pos.z = HEIGHT - 1;
            break;
          }
          case 3:
          {
            pos.z--;
            if (pos.z < 0)
              pos.z = 0;
            break;
          }
        }
      }

     
      if (rand() % 10 == 0)
      {
        //Shoot bullet
        int bullet_idx = -1;
        for (int i=0; i<5; i++)
        {
          if (active_bullets[i] < 0)
          {
            bullet_idx = i;
            break;
          }
        }
        bullets[bullet_idx] = pos;
        bullets[bullet_idx].addVector3d(vel);
        if (vel.x > 0)
          bullet_vel[bullet_idx] = 2;
        else
          bullet_vel[bullet_idx] = -2;
        active_bullets[bullet_idx] = 15;
      }

      //Update bullets
      for (int i=0; i<5; i++)
      {
        if (active_bullets[i] >= 0)
        {
          bullets[i].x += bullet_vel[i];
          if(bullets[i].x >= LENGTH)
            bullets[i].x = bullets[i].x % LENGTH;
          active_bullets[i] = active_bullets[i] - 1;
        }
        else
          continue;
      }
      star_update--;
      if (star_update <= 0)
      {
        star_update = 100;
        for (int i=0; i<10; i++)
        {
          stars[i].addVector3d(-1, 0, 0);  
          if (stars[i].x < 0)
            stars[i].x += LENGTH;      
        }
      }



      float flame_int = rand() % 256 / 256.0;
      //Draw
      for (int i=0; i<10; i++)
      {
        frame_buffer.setColors(stars[i].x, stars[i].y, stars[i].z, 255, 255, 255);       
      }
      
      if (vel.x > 0)
      {
        frame_buffer.setColors(pos.x, pos.y, pos.z, 100, 30, 255);
        if (pos.x - 1 < 0)
          frame_buffer.setColors(LENGTH - 1, pos.y, pos.z, 100, 30, 255);
        else
          frame_buffer.setColors(pos.x - 1, pos.y, pos.z, 100, 30, 255);

        if (pos.x - 2 < 0)
          frame_buffer.setColors((pos.x + LENGTH) % LENGTH, pos.y, pos.z, (uint8_t)(255*flame_int), (uint8_t)(128*flame_int), 0);
        else
          frame_buffer.setColors(pos.x - 2, pos.y, pos.z, (uint8_t)(255*flame_int), (uint8_t)(128*flame_int), 0);
      }
      else
      {
        
      }

      
      for (int i=0; i<5; i++)
      {
        if (active_bullets[i] >= 0)
        {
          frame_buffer.setColors(bullets[i].x, bullets[i].y, bullets[i].z, 0, 255, 64);
        }
      }


      color_scale = sin(2*M_PI*color_idx);
      color_idx += 0.001;

      for (int i=0; i<LENGTH; i++)
      {
        //0, 1, 2, 3, 4, 5,
        for (int k=0; k<HEIGHT/2; k++)
        {
          frame_buffer.setColors(i, 0, k, 255, (uint8_t)(255*color_scale), 0); 
          if (k < HEIGHT/2-1)
          {
            frame_buffer.setColors(i, 1, k, 255, (uint8_t)(255*color_scale), 0);
          }
        }
      }

      

      frame_buffer.update();
      delay(33);
   }
   
}

