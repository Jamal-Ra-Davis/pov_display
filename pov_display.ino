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

#define LED_PIN 9
#define HALL_PIN 6
#define CPU_HZ 48000000
#define CPU_HZ_DIV 6000000
#define TIMER_PRESCALER_DIV 1024

#define CPU_HZ_SCALE 48ull
#define CPU_HZ_SCALE_DIV 6ull

#define MAX_PERIOD_US 100000ULL
#define MIN_PERIOD_US 16666ULL

#define REFRESH_HZ 30

#define TICK_DELAY 5
#define MS_TO_TICKS(x) (x/TICK_DELAY) 

volatile uint8_t buf_idx = 0;
const int buf_offset[HEIGHT] = {2*(LENGTH/6), 3*(LENGTH/6), 4*(LENGTH/6), 5*(LENGTH/6), 0*(LENGTH/6), 1*(LENGTH/6)};
doubleBuffer frame_buffer;
Shell shell;
SpaceGame space_game;
RTCZero rtc;

int hall;

pov_state_t exec_state = SPACE_GAME;
bool pov_state_change = true;
int change_state(pov_state_t state)
{
  if (exec_state != state)
  {
    exec_state = state;
    pov_state_change = true;
  }
  return 0;
}
void scratch_loop();
void test_exec();
void ds4_test();
void ds4_analog_test();
void clock_test();

SPIClass mySPI (&sercom1, 12, 13, 11, SPI_PAD_0_SCK_1, SERCOM_RX_PAD_3);//MOSI: 11, SCK: 13


void startTimer(int frequencyHz);
void setTimerFrequency(int frequencyHz);
void TC3_Handler();
void hallTrigger();

long timer_0, timer_delta;

void processSerialCommands()
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
}
void main_exec()
{
  switch(exec_state)
  {
    case POV_SCRATCH_LOOP:
      scratch_loop();
      break;
    case POV_TEST:
      test_exec();
      //textAnimation(&frame_buffer);
      //pinWheelAnimation_0(&frame_buffer);
      //vortexAnimation(&frame_buffer);
      //pulseAnimation(&frame_buffer);
      break;
    case DS4_TEST:
      //ds4_test();
      ds4_analog_test();
      break;
    case SPACE_GAME:
      space_game.update();
      space_game.draw(&frame_buffer);
      break;
    case CLOCK_DISPLAY:
      clock_test();
      break;
    default:
      break;
  }
}
void superLoop()
{
  long tick_start = millis();
  processSerialCommands();
  frame_buffer.clear();
  
  main_exec();//exec function responsible for managing event buffer

  frame_buffer.update();
  while((millis() - tick_start) < TICK_DELAY);
}

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
  SerialUSB.begin(115200);
  Serial1.begin(115200);    //Hold off on until you know it's using a different SERCOM than SPI
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  frame_buffer.reset();
  sercomSetup();
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
void scratch_loop() 
{
  int hue_offset = 0;
  int width_offset = 60/(WIDTH-1);
  //uint8_t height_transform[4] = {3, 4, 3, 1};
  uint8_t height_transform[15] = {2, 4, 4, 5, 5, 5, 5, 4, 4, 3, 2, 1, 1, 1, 1};
  uint8_t trans_size = 15;
  while(1)
  {
    frame_buffer.clear();  
    for (int i=0; i<LENGTH; i++)
    {
      int hue = (i*255)/LENGTH;
      int k = 0;
      if ((i >= 20) && i < (20 + trans_size))
      {
        int idx = i - 20;//i from 0 to trans_size - 1
        k = height_transform[trans_size - 1 - idx];
        //char ser_buf[128];
        //sprintf(ser_buf, "i = %d, idx = %d, t_sz = %d, k = %d\n", i, idx, trans_size, k);
        //SerialUSB.print(ser_buf);
      }
      
      for (int j=0; j<WIDTH; j++)
      {
        CRGB color = CHSV(hue+hue_offset+((WIDTH-1-j)*width_offset), 255, 255);
        frame_buffer.setColors(i, j, k, color.r, color.g, color.b);
      }
    }
    hue_offset += 3;

    
    frame_buffer.update();
    delay(33);
  }

  
  MazeGame maze;
  maze.init();
  while (1)
  {
    
    process_serial_commands(&frame_buffer);
    frame_buffer.clear();

    maze.update();
    maze.draw(&frame_buffer);
    
    frame_buffer.update();
    delay(33);
  }
  
  RingBuf<char, 32> serialBuffer;
  serialBuffer.clear();
  while(0)
  {
  while (Serial1.available())
  {
    char c = Serial1.read();
    if (c == '\n')
    {
      //Parse
      
      serialBuffer.pop(c);
      int bytes_remaining = serialBuffer.size();
      switch(c)
      {
        case 'c':
        {
          frame_buffer.clear();
          SerialUSB.println("c");
          break;
        }
        case 'u':
        {
          frame_buffer.update();
          SerialUSB.println("u");
          break;
        }
        case 's':
        {
          char parse_buf[32] = {'\0'};
          int parse_idx = 0;
          for (parse_idx = 0; parse_idx < bytes_remaining; parse_idx++)
          {
            serialBuffer.pop(parse_buf[parse_idx]);
          }
          parse_buf[parse_idx] = '\0';

          int x, y, z, r, g, b;
          sscanf(parse_buf, "%d %d %d %d %d %d", &x, &y, &z, &r, &g, &b);
          char command[32];
          sprintf(command, "s %d %d %d %d %d %d\n", x, y, z, (uint8_t)r, (uint8_t)g, (uint8_t)b);
          SerialUSB.print(command);
          frame_buffer.setColors(x, y, z, r, g, b);
          break;
        }
        case 'p':
        case 'r':
        {
          char state = c;
          serialBuffer.pop(c);
          int idx = (int)c - 48;

          SerialUSB.print(state);
          SerialUSB.print(idx);
          
          if (state == 'p')
          {
            eventBuffer.push(Event(Event::ON_PRESS, idx));
            SerialUSB.print("ON_PRESS: ");
            SerialUSB.println(idx);
          }
          else if (state = 'r')
          {
            eventBuffer.push(Event(Event::ON_RELEASE, idx));
            SerialUSB.print("ON_RELEASE: ");
            SerialUSB.println(idx);
          }
          break;
        }
        default:
        {
          //Don't recognize first character of packet, clear and move on to next
          serialBuffer.clear();
        }
      }
    }
    else
    {
      serialBuffer.push(c);
    }
  }
  }
  
  while(0)
  {
    while(Serial1.available())
    {
      if (Serial1.available() < 2)
        delay(3);
      char c = Serial1.read();
      SerialUSB.println(c);
      switch(c)
      {
        case 'c':
        {
          //while(Serial1.read() != '\n');
          frame_buffer.clear();
          SerialUSB.println("c");
          break;
        }
        case 'u':
        {
          //while(Serial1.read() != '\n');
          frame_buffer.update();
          SerialUSB.println("u");
          break;
        }
        case 's':
        {
          int x = Serial1.parseInt();
          int y = Serial1.parseInt();
          int z = Serial1.parseInt();
          int r = Serial1.parseInt();
          int g = Serial1.parseInt();
          int b = Serial1.parseInt();
          //while(Serial1.read() != '\n');
          char command[32];
          sprintf(command, "s %d %d %d %d %d %d\n", x, y, z, (uint8_t)r, (uint8_t)g, (uint8_t)b);
          SerialUSB.print(command);
          frame_buffer.setColors(x, y, z, r, g, b);
          break;
        }
        case 'b':
        case 'r':
        {
          char state = c;
          int idx = (int)Serial1.read() - 48;
          char dummy = Serial1.read();
          SerialUSB.print(state);
          SerialUSB.print(idx);
          SerialUSB.print(dummy);
          
          if (state == 'p')
          {
            eventBuffer.push(Event(Event::ON_PRESS, idx));
            SerialUSB.print("ON_PRESS: ");
            SerialUSB.println(idx);
          }
          else if (state = 'r')
          {
            eventBuffer.push(Event(Event::ON_RELEASE, idx));
            SerialUSB.print("ON_RELEASE: ");
            SerialUSB.println(idx);
          }
          
          break;
        }
        default:
        {
          //while(Serial1.read() != '\n');
        }
      }
    }
  }
  
  //random_walk(&frame_buffer);
  /*
  SerialUSB.println("In Loop");
  SerialUSB.println(rand() % 100);
  Snake snake_game;
  
  //Snake game
  snake_game.reset();
  
  while(0)
  {
    
    snake_game.update();
    if (snake_game.game_over())
    {
      snake_game.reset();
    }
    snake_game.draw(&frame_buffer);
    delay(250);
  }
  
  SerialUSB.println();
  delay(250);
  return;
  */
  
  //Draw cartesian coord
  Vector3d p0(-8, 8, 5);
  Vector3d p1(8, 8, 5);
  int rad_offset = 5;
  while(0)
  {
    
  }

  //Clock
  int sec = 0;
  int min = 0;
  int hour = 0;
  int h_off = 0;
  int h_up = 1;
  int h_inc = 0;
  while(0)
  {
    frame_buffer.clear();
    for (int i=0; i<LENGTH; i++)
    {
      frame_buffer.setColors(i, WIDTH-1, h_off, 255, 255, 255);
      if (i % (LENGTH/12) == 0)
      {
        frame_buffer.setColors(i, WIDTH-2, h_off, 255, 255, 255);
        frame_buffer.setColors(i, WIDTH-3, h_off, 255, 255, 255);
      }
    }
    for (int j=0; j<WIDTH; j++)
    {
      //Draw sec hand
      int sec_idx = (sec*LENGTH)/60;
      if (j <= WIDTH-3)
      {
        frame_buffer.setColors(sec_idx, j, 1+h_off, 0, 0, 255);
      }

      //Draw min hand
      int min_idx = (min*LENGTH)/60;
      if (j <= WIDTH-3)
      {
        frame_buffer.setColors(min_idx, j, 2+h_off, 0, 255, 0);
      }

      //Draw hour hand
      int hour_idx = (hour*LENGTH)/12;
      if (j <= WIDTH-5)
      {
        frame_buffer.setColors(hour_idx, j, 3+h_off, 255, 0, 0);
      }
    }

    frame_buffer.update();
    sec++;
    if (sec >= 60)
    {
      sec = 0;
      min++;
    }
    if (min >= 60)
    {
      min = 0;
      hour++;
    }
    if (hour >= 12)
    {
      hour = 0;
    }

    h_inc++;
    if ((h_inc % 77) == 0)
    {
      if (h_up)
      {
        h_off++;
        if (h_off > 2)
        {
          h_off = 2;
          h_up = 0;
        }
      }
      else
      {
        h_off--;
        if (h_off < 0)
        {
          h_off = 0;
          h_up = 1;
        }
      }
    }
    
    delay(10);
  }

  //3d sine wave
  int sin_idx = 0;
  while(1)
  {
    frame_buffer.clear();
    float val2;
    int g_value;
    for (int j=0; j<WIDTH; j++)
    {
      val2 = 0.5*cos((j+sin_idx)*pi_frac) + 0.5;
      g_value = (int)(255*val2);  
    }
    for (int i=0; i<LENGTH; i++)
    {
      float brightness = 0.5*sin((i+sin_idx)*pi_frac) + 0.5;
      int r_value = (int)(255*brightness);
      int b_value = (int)((1 - brightness)*255);
      int height = (int)(brightness*(HEIGHT-1) + 0.5);
      for (int j=0; j<WIDTH; j++)
      {
        frame_buffer.setColors(i, j, height, r_value, g_value, b_value);
      }
    }
    frame_buffer.update();
    sin_idx++;
    if (sin_idx == LENGTH)
      sin_idx = 0;
    delay(10);
  }

  
  Vector3d from(20,0,0);
  Vector3d to(30, 0, 0);
  int pi_idx = 0;
  //float pi_frac = M_PI/12.0;
  while(1)
  {
    frame_buffer.clear();

    float x_val0 = 3*cos(pi_idx*pi_frac) + 40;
    float y_val0 = 3*sin(pi_idx*pi_frac) + 3;
    
    float x_val1 = 3*cos(pi_idx*pi_frac + M_PI/2) + 40;
    float y_val1 = 3*sin(pi_idx*pi_frac + M_PI/2) + 3;
    
    float x_val2 = 3*cos(pi_idx*pi_frac + M_PI) + 40;
    float y_val2 = 3*sin(pi_idx*pi_frac + M_PI) + 3;
    
    float x_val3 = 3*cos(pi_idx*pi_frac + 3*M_PI/2) + 40;
    float y_val3 = 3*sin(pi_idx*pi_frac + 3*M_PI/2) + 3;
    
    
    pi_idx+=1;
    if (pi_idx >= 24)
      pi_idx = 0;
    
    //frame_buffer.drawLine(Vector3d(40, 3, 0), Vector3d((int)(x_val + 40), (int)(y_val + 3), 5), 0, 255, 0);
    
    frame_buffer.drawLine(Vector3d((int)x_val0, (int)y_val0, 5), Vector3d((int)x_val1, (int)y_val1, 5), 255, 0, 0);
    frame_buffer.drawLine(Vector3d((int)x_val1, (int)y_val1, 5), Vector3d((int)x_val2, (int)y_val2, 5), 0, 255, 0);
    frame_buffer.drawLine(Vector3d((int)x_val2, (int)y_val2, 5), Vector3d((int)x_val3, (int)y_val3, 5), 0, 0, 255);
    frame_buffer.drawLine(Vector3d((int)x_val3, (int)y_val3, 5), Vector3d((int)x_val0, (int)y_val0, 5), 255, 255, 0);

    frame_buffer.drawLine(Vector3d((int)x_val2, (int)y_val2, 4), Vector3d((int)x_val1, (int)y_val1, 4), 255, 255, 255);
    frame_buffer.drawLine(Vector3d(40, 6, 3), Vector3d(37, 3, 3), 255, 0, 255);
    
    //frame_buffer.setColors((int)(x_val + 40), (int)(y_val + 3), 5, 255, 0, 0);
    
    //frame_buffer.drawLine(from, to, 0, 0, 255);
    frame_buffer.update();
    to.y++;
    if (to.y >= WIDTH)
    {
      to.y = 0;
      to.z++;
    }
    if (to.z >= HEIGHT)
    {
      to.z = 0;
    }
    delay(10);
  }
  ball_collision(&frame_buffer);
  
  struct pixel rgb;
  rgb.r = 20;
  rgb.g = 60;
  rgb.b = 120;
  char message[10] = "HELLO";
  frame_buffer.clear();
  for (int i=0; i<LENGTH; i++)
  {
    for (int j=0; j<WIDTH; j++)
    {
      if (j == 0 || j == 1 || j == (WIDTH -1))
      {
        frame_buffer.setColors(i, j, 0, rgb.r, rgb.g, rgb.b);
      }
      else if (i == 0 || i == 1 || i == 5 || i == 6)
      {
        frame_buffer.setColors(i, j, 0, rgb.r, rgb.g, rgb.b);
      }
      else
      {
        frame_buffer.setColors(i, j, 0, 128, 128, 128);
      }
    }
  }
  int height_idx = (offset_val/2 % 5) + 1;
  writeString(message, offset_val, height_idx, rgb.r, rgb.g, rgb.b, &frame_buffer);
  frame_buffer.update();
  offset_val++;
  if (offset_val > 40)
  {
    offset_val = 0;
  }
  delay(100);
  return;
  

  
  block_test();
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
  //int compareValue = (CPU_HZ / (TIMER_PRESCALER_DIV * frequencyHz)) - 1;
  int compareValue = (CPU_HZ_DIV / frequencyHz) - 1;
  TcCount16* TC = (TcCount16*) TC3;
  
  // Make sure the count is in a proportional position to where it was
  // to prevent any jitter or disconnect when changing the compare value.
  TC->COUNT.reg = map(TC->COUNT.reg, 0, TC->CC[0].reg, 0, compareValue);
  TC->CC[0].reg = compareValue;
  while (TC->STATUS.bit.SYNCBUSY == 1);
}
void setTimerPeriod(int period_us) {//period in us
  int compareValue = (CPU_HZ_SCALE_DIV*period_us) - 1;
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
  TC->CTRLA.reg |= TC_CTRLA_PRESCALER_DIV8;
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
  TC->CTRLA.reg |= TC_CTRLA_PRESCALER_DIV8;
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
    if (!transfer_complete)
    {
      buf_idx++;

      //If interrupt triggers before transfer can complete, still prepare next DMA array
      //so next slice will display correctly
      int next_idx = buf_idx % LENGTH;
      convert_fb_to_dma(next_idx, buf_offset, frame_buffer.getReadBuffer(), pArray_next);
      return;
    }
  
    //driveLEDS(buf_idx, (int*)buf_offset, &frame_buffer, &mySPI);
    start_dma_transaction();
    buf_idx++;
    
    int next_idx = buf_idx % LENGTH;
    convert_fb_to_dma(next_idx, buf_offset, frame_buffer.getReadBuffer(), pArray_next);
  }
}


static inline void adjust_timing(TcCount16* TC, long timer_temp, long *timer_0)
{
  timer_delta = timer_temp - *timer_0;
  *timer_0 = timer_temp;
  if (timer_delta < MIN_PERIOD_US)
  {
    timer_delta = MIN_PERIOD_US;
  }
  else if (timer_delta > MAX_PERIOD_US)
  {
    timer_delta = MAX_PERIOD_US;
  }
  
  timer_delta -= (timer_delta >> 8);
  uint16_t period_us = (timer_delta/LENGTH);
  uint16_t compareValue = (CPU_HZ_SCALE_DIV*period_us) - 1;
  TC->CC[0].reg = compareValue;
  while (TC->STATUS.bit.SYNCBUSY == 1);
}
void hallTrigger()
{
  digitalWrite(LED_PIN, HIGH);

  //Reset timer to zero
  TcCount16* TC = (TcCount16*) TC3;
  TC->COUNT.reg = 0;
  long timer_temp = micros();
  buf_idx = 1;
  
  if (!transfer_complete)
  {
    //Update timing even if, DMA transaction not finished
    adjust_timing(TC, timer_temp, &timer_0);
  
    //If interrupt triggers before transfer can complete, still prepare next DMA array
    //so next slice will display correctly
    convert_fb_to_dma(buf_idx, buf_offset, frame_buffer.getReadBuffer(), pArray_next);
    return;
  }
  
  start_dma_transaction();
  convert_fb_to_dma(buf_idx, buf_offset, frame_buffer.getReadBuffer(), pArray_next);

  adjust_timing(TC, timer_temp, &timer_0);
}

void block_test()
{
  SerialUSB.println("Entering blocktest");
  Vector3d vec0_s(0, 0, 0), vec0_e(9, 4, 1);
  uint8_t r0, g0, b0;
  doubleBuffer::randColor(&r0, &g0, &b0);
  
  Vector3d vec1_s(15, 0, 3), vec1_e(17, 6, 5);
  uint8_t r1, g1, b1;
  doubleBuffer::randColor(&r1, &g1, &b1);
  
  while(1)
  {
    frame_buffer.clear();
    frame_buffer.drawBlock(vec0_s, vec0_e, r0, g0, b0);
    frame_buffer.drawBlock(vec1_s, vec1_e, r1, g1, b1);
   
    for (int i=0; i<WIDTH; i++)
    {
      frame_buffer.setColors(buf_offset[0], i, 0, 0xFF, 0x00, 0x00);
      frame_buffer.setColors(buf_offset[0]-1, i, 0, 0x80, 0x00, 0xFF);
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

void test_exec()
{
  static const uint8_t delay_cnt = 7;
  static uint8_t cnt = 0;
  static const char* test_message = "AR PRODUCT FW";

  static int message_len = strlen(test_message);
  static int start_offset = -1*message_len*8;
  static int offset = start_offset;

  static int start_offset2 = LENGTH;
  static int offset2 = start_offset;
  
  //Draw
  writeString(test_message, offset, 1, 50, 0, 255, &frame_buffer);
  writeString(test_message, offset2, 4, 0, 255, 100, &frame_buffer);

  //Update logic
  if (cnt++ % delay_cnt == 0)
  {
    offset++;
    if (offset >= LENGTH)
    {
      offset = start_offset;
    } 

    //SERIAL_PRINTF(SerialUSB, "Offset2 = %d\n", offset2);
    offset2--;
    //SERIAL_PRINTF(SerialUSB, "Offset2(post) = %d\n", offset2);
    if (offset2 <= -1*message_len*8)
    {
      //SERIAL_PRINTF(SerialUSB, "Offset2 = %d, -1*message_len*8 = %d\n", offset2, -1*message_len*8);
      offset2 = start_offset2;
    }
  }
}

void ds4_test()
{
  //static enum {TRIANGE, SQUARE, CROSS, CIRCLE, LBUMP, RBUMP, LSTICK,
  //             RSTICK, SHARE, OPTIONS, DUP, DLEFT, DDOWN, DRIGHT, DX, DY, NUM_KEYS} ds4_keys_t;
  static bool buttons[NUM_KEYS] = {0};

  uint8_t num_events = eventBuffer.size();
  for (int i=0; i<num_events; i++)
  {
    Event e;
    eventBuffer.pop(e);
    if (e.type >= Event::NUM_BTN_EVENTS || e.data.button_idx >= NUM_KEYS)
    {
      continue;
    }
    if (e.type == Event::ON_RELEASE && (e.data.button_idx == DX || e.data.button_idx == DY))
    {
      if (e.data.button_idx == DX)
      {
        buttons[DLEFT] = false;
        buttons[DRIGHT] = false;
      }
      else
      {
        buttons[DUP] = false;
        buttons[DDOWN] = false;
      }
      continue;
    }

    switch(e.type)
    {
      case Event::ON_PRESS:
      {
        buttons[e.data.button_idx] = true;
        break;
      }
      case Event::ON_RELEASE:
      {
        buttons[e.data.button_idx] = false;
        break;
      }
      case Event::TAP:
      {
        break;
      }
      default:
      {
        break;
      }
    }
  }

  static const uint8_t N = (2*LENGTH)/(NUM_KEYS-2);
  static const uint8_t HALF = (NUM_KEYS-2)/2; 
  for (int i=0; i<NUM_KEYS-2; i++)
  {
    if (buttons[i])
    {
      uint8_t x1 = (i >= HALF) ? N*(i-HALF) : N*i; 
      uint8_t x2 = (i >= HALF) ? N*(i+1-HALF) : N*(i+1);
      uint8_t y1 = (i >= HALF) ? WIDTH/2-1 : 0; 
      uint8_t y2 = (i >= HALF) ? WIDTH-1 : WIDTH/2;
      int hue = (i*N*255)/96;
      CRGB color = CHSV(hue, 255, 255);
      frame_buffer.drawBlock(x1, y1, 0, x2, y2, HEIGHT-1, color.r, color.g, color.b);
    }
  }
}

void clock_test()
{
  //Clock
  int sec = rtc.getSeconds();
  int min = rtc.getMinutes();
  int hour = rtc.getHours() % 12;
  static int h_off = 0;
  static int h_up = 1;
  static int h_inc = 0;

  static const uint8_t delay_cnt = 5;
  static uint8_t cnt = 0;
  
  //Draw back plane
  for (int i=0; i<LENGTH; i++)
  {
    frame_buffer.setColors(i, WIDTH-1, h_off, 255, 255, 255);
    if (i % (LENGTH/12) == 0)
    {
      frame_buffer.setColors(i, WIDTH-2, h_off, 255, 255, 255);
      frame_buffer.setColors(i, WIDTH-3, h_off, 255, 255, 255);
    }
  }

  //Draw Hands
  for (int j=0; j<WIDTH; j++)
  {
    //Draw sec hand
    int sec_idx = (sec*LENGTH)/60;
    if (j <= WIDTH-3)
    {
      frame_buffer.setColors(sec_idx, j, 1+h_off, 0, 0, 255);
    }

    //Draw min hand
    int min_idx = (min*LENGTH)/60;
    if (j <= WIDTH-3)
    {
      frame_buffer.setColors(min_idx, j, 2+h_off, 0, 255, 0);
    }

    //Draw hour hand
    int hour_idx = (hour*LENGTH)/12;
    if (j <= WIDTH-5)
    {
      frame_buffer.setColors(hour_idx, j, 3+h_off, 255, 0, 0);
    }
  }

  //Update logic
  if (cnt++ % delay_cnt == 0)
  {
    h_inc++;
    if ((h_inc % 77) == 0)
    {
      if (h_up)
      {
        h_off++;
        if (h_off > 2)
        {
          h_off = 2;
          h_up = 0;
        }
      }
      else
      {
        h_off--;
        if (h_off < 0)
        {
          h_off = 0;
          h_up = 1;
        }
      }
    }
    SERIAL_PRINTF(SerialUSB, "%02d:%02d:%02d\n", hour, min, sec);
  }
}

void ds4_analog_test()
{
  static int16_t lstick_x = 0;
  static int16_t lstick_y = 0;
  static uint16_t lstick_angle = 0;
  static uint8_t lstick_mag = 0;

  static int16_t rstick_x = 0;
  static int16_t rstick_y = 0;
  static uint16_t rstick_angle = 0;
  static uint8_t rstick_mag = 0;

  static uint8_t l_trig = 0;
  static uint8_t r_trig = 0;

  uint8_t num_events = eventBuffer.size();
  for (int i=0; i<num_events; i++)
  {
    Event e;
    eventBuffer.pop(e);
    if (e.type != Event::ABS_VAL)
    {
      continue;
    }

    switch(e.data.abs_data.type)
    {
      case Event::L_STICK:
      {
        lstick_x = e.data.abs_data.x;
        lstick_y = e.data.abs_data.y;
        lstick_angle = e.data.abs_data.angle;
        lstick_mag = e.data.abs_data.mag;
        break;
      }
      case Event::R_STICK:
      {
        rstick_x = e.data.abs_data.x;
        rstick_y = e.data.abs_data.y;
        rstick_angle = e.data.abs_data.angle;
        rstick_mag = e.data.abs_data.mag;
        break;
      }
      case Event::L_TRIG:
      {
        l_trig = e.data.abs_data.trigger;
        break;
      }
      case Event::R_TRIG:
      {
        r_trig = e.data.abs_data.trigger;
        break;
      }
      default:
      {
        break;
      }
    }
  }
  
  /*
  int16_t norm_lstick_x = (100*lstick_x)/0xEFFF;
  int16_t norm_lstick_y = (100*lstick_y)/0xEFFF;
  //int16_t norm_rstick_x = (100*rstick_x)/0xEFFF;
  //int16_t norm_rstick_y = (100*rstick_y)/0xEFFF;

  int16_t l_mag = sqrt(norm_lstick_x*norm_lstick_x + norm_lstick_y*norm_lstick_y);
  //int16_t r_mag = sqrt(norm_rstick_x*norm_rstick_x + norm_rstick_y*norm_rstick_y);
  */

  uint16_t norm_l_trig = ((LENGTH-1)*l_trig)/255;
  uint16_t norm_r_trig = ((LENGTH-1)*r_trig)/255;
  

  frame_buffer.drawBlock(0, 0, HEIGHT-1, norm_l_trig, 3, HEIGHT-1, 255, 0, 255);//Purple
  frame_buffer.drawBlock(0, 4, HEIGHT-1, norm_r_trig, 7, HEIGHT-1, 255, 255, 0);//Yellow

  if (lstick_mag >= 64)
  {
    //angle = 0 to 359 => 0 to LENGTH - 1 
    uint16_t l_idx = ((LENGTH-1)*lstick_angle)/359;
    for (int j=0; j<WIDTH; j++)
    {
      frame_buffer.setColors(l_idx, j, 0, 0, 255, 0); 
    }
  }
  if (rstick_mag >= 64)
  {
    //angle = 0 to 359 => 0 to LENGTH - 1 
    uint16_t r_idx = ((LENGTH-1)*rstick_angle)/359;
    for (int j=0; j<WIDTH; j++)
    {
      frame_buffer.setColors(r_idx, j, 1, 0, 0, 255); 
    }
  }
}