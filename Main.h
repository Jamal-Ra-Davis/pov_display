#ifndef MAIN_LIB
#define MAIN_LIB

#ifdef CONFIG_POV_SIMULATOR
#include "Arduino.h"
#endif

#include <string.h>
#include "FrameBuffer.h"
#include "Shell.h"
#include "Space_Game.h"
#include "Events.h"
#include "test_animations.h"
#include "Color.h"


union POV_Games{
    SpaceGame space_game;
    MazeGame maze_game;
    POV_Games() {}
} pov_games;


void scratch_loop(doubleBuffer* frame_buffer);
void block_test(doubleBuffer* frame_buffer);
void test_exec(doubleBuffer* frame_buffer);
void ds4_test(doubleBuffer* frame_buffer);
void ds4_analog_test(doubleBuffer* frame_buffer);
void clock_test(doubleBuffer* frame_buffer);

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


typedef void(*init_function_t)(void*);
void init_spacegame(void* args)
{
    SpaceGame* game = (SpaceGame*)args;
    game->reset();
}
void init_mazegame(void* args)
{
    MazeGame* game = (MazeGame*)args;
    game->init();
}
void check_state_change(init_function_t init_func, void *args)
{
    if (pov_state_change)
    {
        pov_state_change = false;
        //do init function
        if (init_func != NULL)
            init_func(args);
    }
}

void main_setup(doubleBuffer* frame_buffer)
{
    SERIAL_PRINTF(SerialUSB, "sizeof(space_game) = %d, sizeof(maze_game) = %d\n", sizeof(SpaceGame), sizeof(MazeGame));
    frame_buffer->reset();
    change_state(POV_SCRATCH_LOOP);
}

void main_exec(doubleBuffer* frame_buffer)
{
    int num_events = eventBuffer.size();
    for (int i = 0; i < num_events; i++) 
    {
        Event e;
        if (!eventBuffer.pop(e))
        {
            SERIAL_PRINTF(SerialUSB, "Error: Failed to pop event from buffer");
            continue;
        }

        if (e.type == Event::ON_PRESS && e.data.button_idx == SHARE)
        {
            pov_state_t next_state = (pov_state_t)((exec_state + 1) % NUM_POV_STATES);
            change_state(next_state);
        }
        else
        {
            eventBuffer.push(e);
        }
    }
    

    switch(exec_state)
    {
    case POV_SCRATCH_LOOP:
        //scratch_loop(frame_buffer);
        sine_wave_3d(frame_buffer);
        //rainbow_swirl(frame_buffer);
        break;
    case POV_TEST:
        test_exec(frame_buffer); //Uncomment after testing
        //textAnimation(frame_buffer);
        //pinWheelAnimation_0(frame_buffer);
        //vortexAnimation(frame_buffer);
        //pulseAnimation(frame_buffer);
        break;
    case DS4_TEST:
        ds4_test(frame_buffer);
        //ds4_analog_test(frame_buffer);
        break;
    case MAZE_GAME:
        check_state_change(init_mazegame, &pov_games.maze_game);
        pov_games.maze_game.update();
        pov_games.maze_game.draw(frame_buffer);
        break;
    case SPACE_GAME:
        check_state_change(init_spacegame, &pov_games.space_game);
        pov_games.space_game.update();
        pov_games.space_game.draw(frame_buffer);
        break;
    case CLOCK_DISPLAY:
        clock_test(frame_buffer);
        break;
    default:
        break;
    }
}

void scratch_loop(doubleBuffer* frame_buffer)
{
    static float pi_frac = M_PI/16.0;
    static int offset_val = 0;
    int hue_offset = 0;
    int width_offset = 60/(WIDTH-1);
    //uint8_t height_transform[4] = {3, 4, 3, 1};
    uint8_t height_transform[15] = {2, 4, 4, 5, 5, 5, 5, 4, 4, 3, 2, 1, 1, 1, 1};
    uint8_t trans_size = 15;
    while(0)
    {
        frame_buffer->clear();  
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
              Color color = Color::getColorHSV(hue+hue_offset+((WIDTH-1-j)*width_offset), 255, 255);
              frame_buffer->setColors(i, j, k, color.r, color.g, color.b);
          }
        }
        hue_offset += 3;

        
        frame_buffer->update();
        delay(33);
    }

    /*
    MazeGame maze;
    maze.init();
    while (1)
    {
        
        process_serial_commands(frame_buffer);
        frame_buffer->clear();

        maze.update();
        maze.draw(frame_buffer);
        
        frame_buffer->update();
        delay(33);
    }
    */

    /*
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
            frame_buffer->clear();
            SerialUSB.println("c");
            break;
            }
            case 'u':
            {
            frame_buffer->update();
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
            frame_buffer->setColors(x, y, z, r, g, b);
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
            frame_buffer->clear();
            SerialUSB.println("c");
            break;
            }
            case 'u':
            {
            //while(Serial1.read() != '\n');
            frame_buffer->update();
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
            frame_buffer->setColors(x, y, z, r, g, b);
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
    */

    //random_walk(frame_buffer);
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
        snake_game.draw(frame_buffer);
        delay(250);
    }
    
    SerialUSB.println();
    delay(250);
    return;
    */
    
    /*
    //Draw cartesian coord
    Vector3d p0(-8, 8, 5);
    Vector3d p1(8, 8, 5);
    int rad_offset = 5;
    while(0)
    {
        
    }
    */

    /*
    //Clock
    int sec = 0;
    int min = 0;
    int hour = 0;
    int h_off = 0;
    int h_up = 1;
    int h_inc = 0;
    while(0)
    {
        frame_buffer->clear();
        for (int i=0; i<LENGTH; i++)
        {
        frame_buffer->setColors(i, WIDTH-1, h_off, 255, 255, 255);
        if (i % (LENGTH/12) == 0)
        {
            frame_buffer->setColors(i, WIDTH-2, h_off, 255, 255, 255);
            frame_buffer->setColors(i, WIDTH-3, h_off, 255, 255, 255);
        }
        }
        for (int j=0; j<WIDTH; j++)
        {
        //Draw sec hand
        int sec_idx = (sec*LENGTH)/60;
        if (j <= WIDTH-3)
        {
            frame_buffer->setColors(sec_idx, j, 1+h_off, 0, 0, 255);
        }

        //Draw min hand
        int min_idx = (min*LENGTH)/60;
        if (j <= WIDTH-3)
        {
            frame_buffer->setColors(min_idx, j, 2+h_off, 0, 255, 0);
        }

        //Draw hour hand
        int hour_idx = (hour*LENGTH)/12;
        if (j <= WIDTH-5)
        {
            frame_buffer->setColors(hour_idx, j, 3+h_off, 255, 0, 0);
        }
        }

        frame_buffer->update();
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
       */

    /*
    //3d sine wave
    SERIAL_PRINTF(SerialUSB, "3d sine wave\n");
    int sin_idx = 0;
    while(1)
    {
        frame_buffer->clear();
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
            frame_buffer->setColors(i, j, height, r_value, g_value, b_value);
        }
        }
        frame_buffer->update();
        sin_idx++;
        if (sin_idx == LENGTH)
        sin_idx = 0;
        delay(35);
    }
    */
    
    //Think i was trying to do rotating boxes or something
    Vector3d from(20,0,0);
    Vector3d to(30, 0, 0);
    int pi_idx = 0;
    //float pi_frac = M_PI/12.0;
    while(0)
    {
        frame_buffer->clear();

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
        
        //frame_buffer->drawLine(Vector3d(40, 3, 0), Vector3d((int)(x_val + 40), (int)(y_val + 3), 5), 0, 255, 0);
        
        frame_buffer->drawLine(Vector3d((int)x_val0, (int)y_val0, 5), Vector3d((int)x_val1, (int)y_val1, 5), 255, 0, 0);
        frame_buffer->drawLine(Vector3d((int)x_val1, (int)y_val1, 5), Vector3d((int)x_val2, (int)y_val2, 5), 0, 255, 0);
        frame_buffer->drawLine(Vector3d((int)x_val2, (int)y_val2, 5), Vector3d((int)x_val3, (int)y_val3, 5), 0, 0, 255);
        frame_buffer->drawLine(Vector3d((int)x_val3, (int)y_val3, 5), Vector3d((int)x_val0, (int)y_val0, 5), 255, 255, 0);

        frame_buffer->drawLine(Vector3d((int)x_val2, (int)y_val2, 4), Vector3d((int)x_val1, (int)y_val1, 4), 255, 255, 255);
        frame_buffer->drawLine(Vector3d(40, 6, 3), Vector3d(37, 3, 3), 255, 0, 255);
        
        //frame_buffer->setColors((int)(x_val + 40), (int)(y_val + 3), 5, 255, 0, 0);
        
        //frame_buffer->drawLine(from, to, 0, 0, 255);
        frame_buffer->update();
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

    //ball_collision(frame_buffer);
    
    
    //Laughing man text attempt
    if (false) {
        Color rgb;
        rgb.r = 20;
        rgb.g = 60;
        rgb.b = 120;
        char message[10] = "HELLO";
        frame_buffer->clear();
        for (int i = 0; i < LENGTH; i++)
        {
            for (int j = 0; j < WIDTH; j++)
            {
                if (j == 0 || j == 1 || j == (WIDTH - 1))
                {
                    frame_buffer->setColors(i, j, 0, rgb.r, rgb.g, rgb.b);
                }
                else if (i == 0 || i == 1 || i == 5 || i == 6)
                {
                    frame_buffer->setColors(i, j, 0, rgb.r, rgb.g, rgb.b);
                }
                else
                {
                    frame_buffer->setColors(i, j, 0, 128, 128, 128);
                }
            }
        }
        int height_idx = (offset_val / 2 % 5) + 1;
        writeString(message, offset_val, height_idx, rgb.r, rgb.g, rgb.b, frame_buffer);
        frame_buffer->update();
        offset_val++;
        if (offset_val > 40)
        {
            offset_val = 0;
        }
        delay(100);
        return;
    }

    
    //block_test(frame_buffer);
    //return;
    
    /*
    frame_buffer->forceSingleBuffer();
    frame_buffer->clear();
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
        frame_buffer->setColors(idx, j, 0, color & (0x03), (color >> 3) & (0x03), (color >> 6) & 0x03);
        frame_buffer->setColors(idx, j, 3, r, g, b);
        frame_buffer->setColors(idx, j, 5, 255, 0, 0);
        }
        frame_buffer->update();
        delay(35);
    } 
    */
}
void block_test(doubleBuffer* frame_buffer)
{
#ifdef CONFIG_POV_SIMULATOR  
  static const int buf_offset[HEIGHT] = { 2 * (LENGTH / 6), 3 * (LENGTH / 6), 4 * (LENGTH / 6), 5 * (LENGTH / 6), 0 * (LENGTH / 6), 1 * (LENGTH / 6) };
#endif
  SerialUSB.println("Entering blocktest");
  Vector3d vec0_s(0, 0, 0), vec0_e(9, 4, 1);
  uint8_t r0, g0, b0;
  doubleBuffer::randColor(&r0, &g0, &b0);
  
  Vector3d vec1_s(15, 0, 3), vec1_e(17, 6, 5);
  uint8_t r1, g1, b1;
  doubleBuffer::randColor(&r1, &g1, &b1);
  
  while(1)
  {
    frame_buffer->clear();
    frame_buffer->drawBlock(vec0_s, vec0_e, r0, g0, b0);
    frame_buffer->drawBlock(vec1_s, vec1_e, r1, g1, b1);
   
    for (int i=0; i<WIDTH; i++)
    {
      frame_buffer->setColors(buf_offset[0], i, 0, 0xFF, 0x00, 0x00);
      frame_buffer->setColors(buf_offset[0]-1, i, 0, 0x80, 0x00, 0xFF);
    }
    frame_buffer->update();
    

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

void test_exec(doubleBuffer* frame_buffer)
{
  static const uint8_t delay_cnt = 7;
  static uint8_t cnt = 0;
  static const char* test_message = "HACKADAY";

  static int message_len = strlen(test_message);
  static int start_offset = -1*message_len*8;
  static int offset = start_offset;

  static int start_offset2 = LENGTH;
  static int offset2 = start_offset;
  
  //Draw
  writeString(test_message, offset, 1, 50, 0, 255, frame_buffer);
  writeString(test_message, offset2, 4, 0, 255, 100, frame_buffer);

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

void ds4_test(doubleBuffer* frame_buffer)
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
      Color color = Color::getColorHSV(hue, 255, 255);
      frame_buffer->drawBlock(x1, y1, 0, x2, y2, HEIGHT-1, color.r, color.g, color.b);
    }
  }
}

void ds4_analog_test(doubleBuffer* frame_buffer)
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
  

  frame_buffer->drawBlock(0, 0, HEIGHT-1, norm_l_trig, 3, HEIGHT-1, 255, 0, 255);//Purple
  frame_buffer->drawBlock(0, 4, HEIGHT-1, norm_r_trig, 7, HEIGHT-1, 255, 255, 0);//Yellow

  if (lstick_mag >= 64)
  {
    //angle = 0 to 359 => 0 to LENGTH - 1 
    uint16_t l_idx = ((LENGTH-1)*lstick_angle)/359;
    for (int j=0; j<WIDTH; j++)
    {
      frame_buffer->setColors(l_idx, j, 0, 0, 255, 0); 
    }
  }
  if (rstick_mag >= 64)
  {
    //angle = 0 to 359 => 0 to LENGTH - 1 
    uint16_t r_idx = ((LENGTH-1)*rstick_angle)/359;
    for (int j=0; j<WIDTH; j++)
    {
      frame_buffer->setColors(r_idx, j, 1, 0, 0, 255); 
    }
  }
}

void clock_test(doubleBuffer* frame_buffer)
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
    frame_buffer->setColors(i, WIDTH-1, h_off, 255, 255, 255);
    if (i % (LENGTH/12) == 0)
    {
      frame_buffer->setColors(i, WIDTH-2, h_off, 255, 255, 255);
      frame_buffer->setColors(i, WIDTH-3, h_off, 255, 255, 255);
    }
  }

  //Draw Hands
  for (int j=0; j<WIDTH; j++)
  {
    //Draw sec hand
    int sec_idx = (sec*LENGTH)/60;
    if (j <= WIDTH-3)
    {
      frame_buffer->setColors(sec_idx, j, 1+h_off, 0, 0, 255);
    }

    //Draw min hand
    int min_idx = (min*LENGTH)/60;
    if (j <= WIDTH-3)
    {
      frame_buffer->setColors(min_idx, j, 2+h_off, 0, 255, 0);
    }

    //Draw hour hand
    int hour_idx = (hour*LENGTH)/12;
    if (j <= WIDTH-5)
    {
      frame_buffer->setColors(hour_idx, j, 3+h_off, 255, 0, 0);
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

#endif