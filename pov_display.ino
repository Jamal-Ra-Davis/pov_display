#include<FastLED.h>
#include <SPI.h>
#include <math.h>
#include "wiring_private.h" // pinPeripheral() function
#include "FrameBuffer.h"
#include "Text.h"
#include "test_animations.h"
#include "Vector3d.h"
//#include "HSV.h"
#include "Space_Game.h"
#include "Events.h"
#include "Snake.h"
#include "DMA_SPI.h"
#include "Shell.h"
#include <RTCZero.h>
#include "Main.h"
#include "Color.h"
#include "Util.h"


#define REFRESH_HZ 30

#define TICK_DELAY 5
#define MS_TO_TICKS(x) (x/TICK_DELAY) 

volatile uint8_t buf_idx = 0;
const int buf_offset[HEIGHT] = {2*(LENGTH/6), 3*(LENGTH/6), 4*(LENGTH/6), 5*(LENGTH/6), 0*(LENGTH/6), 1*(LENGTH/6)};
doubleBuffer frame_buffer;
Shell shell;
RTCZero rtc;

long timer_0, timer_delta;
SPIClass mySPI (&sercom1, 12, 13, 11, SPI_PAD_0_SCK_1, SERCOM_RX_PAD_3);//MOSI: 11, SCK: 13

class MathLookup {
  private:
    float *raw_sin;
    float *raw_cos;
    float *raw_tan;

    uint16_t sin_size;
    uint16_t cos_size;
    uint16_t tan_size;
  public:
    MathLookup(float *sin_array, uint16_t sin_sz, float *cos_array, uint16_t cos_sz, float *tan_array, uint16_t tan_sz);
    float sin(float rad);
    float cos(float rad);
    float tan(float rad);
};
MathLookup::MathLookup(float *sin_array, uint16_t sin_sz, float *cos_array, uint16_t cos_sz, float *tan_array, uint16_t tan_sz)
{
  raw_sin = sin_array;
  sin_size = sin_sz;

  raw_cos = cos_array;
  cos_size = cos_sz;

  raw_tan = tan_array;
  tan_size = tan_sz;
}
float MathLookup::sin(float rad)
{
  float f_idx = (rad*sin_size)/TWO_PI;//Still need to do floating point math 
  int idx = round(f_idx);
  if (idx == sin_size) 
  {
    idx = 0;
  }
  return raw_sin[idx];
}

void superLoop()
{
  long tick_start = millis();
  processSerialCommands(&shell);
  frame_buffer.clear();
  
  main_exec(&frame_buffer);//exec function responsible for managing event buffer

  frame_buffer.update();
  while((millis() - tick_start) < TICK_DELAY);
}

void setup()
{
  SerialUSB.begin(115200);
  Serial1.begin(115200);    //Hold off on until you know it's using a different SERCOM than SPI
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  frame_buffer.reset();
  sercomSetup(&mySPI);
  init_dma();
  mySPI.beginTransaction(SPISettings(12000000, MSBFIRST, SPI_MODE0));
  hallEffectSetup();
  startTimer(REFRESH_HZ*LENGTH);
  rtc.begin();
  frame_buffer.reset();
  delay(3000);

  //while(!SerialUSB);

  SerialUSB.println("Exiting setup");

  int shell_cnt = 0;
  //shell_testing(&shell);
  shell.reset();
  while(0) 
  {
    //Capture Serial Data if Available
    while(Serial1.available() > 0)
    {
      uint8_t val = Serial1.read();
      SerialUSB.print("Received: ");
      SerialUSB.println(val, HEX);
      shell.receive_data(val);
    }

    //Parse Messages if Available
    for (int i=0; i < shell.get_ready_messages(); i++)
    {
      if (shell.parse_data() < 0)
      {
        SerialUSB.print("Error: Failed to parse #");
        SerialUSB.println(i);
        break;
      }
    }
    shell_cnt++;
    if (shell_cnt == 500)
    {
      char out_buf[32] = {0};
      //SERIAL_PRINTF(SerialUSB, "hello_usb %d\n", shell_cnt);

      //LOG_POV_SHELL((&shell), "%d HELLO %d\n", shell_cnt, shell_cnt);
      shell_cnt = 0;
    }
    delay(5);
  }


  while (0) 
  {
    Color test_color = Color::getColorHSV(200, 255, 255);
    //CRGB test_color = CHSV(200, 255, 255);
    LOG_POV_SHELL((&shell), "CRGB: (%d, %d, %d)\n", test_color.r, test_color.g, test_color.b);
    SERIAL_PRINTF(SerialUSB, "CRGB: (%d, %d, %d)\n", test_color.r, test_color.g, test_color.b);

    //Color c;
    //c.setColorHSV(200, 255, 255);
    //LOG_POV_SHELL((&shell), "Color: (%d, %d, %d)\n", c.r, c.g, c.b);
    delay(5000);
  }
  main_setup(&frame_buffer);
}


/*********************************************************************/
/*                                                                   */
/*                       LOOP STARTS HERE                            */
/*                                                                   */
/*********************************************************************/

int offset_val = 0;
void block_test();
void endless_runner();
void ship_game();
float pi_frac = M_PI/16.0;

void loop() 
{
  superLoop();
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
