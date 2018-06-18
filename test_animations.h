#ifndef TEST_ANI_LIB
#define TEST_ANI_LIB

#include "FrameBuffer.h"


void textAnimation(doubleBuffer *frame_buffer)
{
  frame_buffer->reset();
  char *text[4] = {"HACK A DAY",
                  "HI NOLAN",
                  "MAKERFAIRE 2018",
                  "DONT LET YOUR DREAMS BE DREAMS"};
  int text_sel = rand() % 4;
  int height = rand()%HEIGHT;
  int text_len = strlen(text[text_sel]);
  int r, g, b;
  doubleBuffer::randColor(&r, &g, &b);

  for (int i=LENGTH + 5; i>(-2*text_len - 5); i--)
  {
    frame_buffer->clear();
    writeString(text[text_sel], i, height, r, g, b, frame_buffer);
    frame_buffer->update();
    delay(40);
  }
}

void pinWheelAnimation_0(doubleBuffer *frame_buffer)
{
  int lookup[10] = {0, 1, 2, 3, 4, 5, 4, 3, 2, 1};
  int N_CYCLE = 60;
  frame_buffer->reset();

  int sel = rand() % 6;
    int r_, g_, b_;
    doubleBuffer::randColor(&r_, &g_, &b_);

    frame_buffer->reset();
    for (int cycles=0; cycles<N_CYCLE; cycles++)
    {
      frame_buffer->clear();
      for (int i=0; i<LENGTH; i++)
      {
        //int idx = (i+cycles)% (LENGTH-10);
        //float W = 3*sin(2*PI*i/10.0 + cycles)+3;
        
        int W_0 = lookup[(i+cycles)%10];
        int W_1 = lookup[(i+N_CYCLE-cycles)%10]; 
        int W_2 = lookup[(i+cycles+6)%10];
        //if (W_ < WIDTH && W_ >= 0)
  
        frame_buffer->setColorChannel(i, W_0, 0, RED, 255);
        frame_buffer->setColorChannel(i, W_1, 2, GREEN, 255);
        //frame_buffer->setColorChannel(i, W_2, 5, BLUE, 255);
  
        frame_buffer->setColors(i, W_2, 5, r_, g_, b_);
        
        //fbuf[i][W_0][0][RED] = 255; 
        //fbuf[i][W_1][2][GREEN] = 255;
        //fbuf[i][W_2][5][BLUE] = 255;
      }
      frame_buffer->update();
      delay(50);
    }

    frame_buffer->forceSingleBuffer();
    for (int cycles=0; cycles<N_CYCLE; cycles++)
    {
      frame_buffer->clear();
      for (int i=0; i<LENGTH; i++)
      {
        //int idx = (i+cycles)% (LENGTH-10);
        //float W = 3*sin(2*PI*i/10.0 + cycles)+3;
        
        int W_0 = lookup[(i+cycles)%10];
        int W_1 = lookup[(i+N_CYCLE-cycles)%10]; 
        int W_2 = lookup[(i+cycles+6)%10];
        //if (W_ < WIDTH && W_ >= 0)
  
        frame_buffer->setColorChannel(i, W_0, 0, RED, 255);
        frame_buffer->setColorChannel(i, W_1, 2, GREEN, 255);
        //frame_buffer->setColorChannel(i, W_2, 5, BLUE, 255);
  
        frame_buffer->setColors(i, W_2, 5, r_, g_, b_);
        
        //fbuf[i][W_0][0][RED] = 255; 
        //fbuf[i][W_1][2][GREEN] = 255;
        //fbuf[i][W_2][5][BLUE] = 255;
      }
      frame_buffer->update();
      delay(50);
    }
  
}

void vortexAnimation(doubleBuffer *frame_buffer)
{
  int lookup2[10] = {0, 0, 0, 0, 0, 7, 7, 7, 7, 7};
  
  //frame_buffer->forceSingleBuffer();
  frame_buffer->reset();
  int color_r, color_g, color_b;
  doubleBuffer::randColor(&color_r, &color_g, &color_b);
  
  
  for (int cycles = 0; cycles<400; cycles++)
  {
    frame_buffer->clear();
    if (cycles % 10 == 0)
      doubleBuffer::randColor(&color_r, &color_g, &color_b);
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

      

      frame_buffer->setColors(i, W_0, 0, color_r, color_g, color_b);
      frame_buffer->setColors(i, W_1, 1, color_r, color_g, color_b);
      frame_buffer->setColors(i, W_2, 2, color_r, color_g, color_b);
      frame_buffer->setColors(i, W_3, 3, color_r, color_g, color_b);
      frame_buffer->setColors(i, W_4, 4, color_r, color_g, color_b);
      frame_buffer->setColors(i, W_5, 5, color_r, color_g, color_b);

      /*
      frame_buffer->setColors(i, W_0, 0, 10, 200, 200 + (rand() % 29));
      frame_buffer->setColors(i, W_1, 1, 10, 200, 200 + (rand() % 55));
      frame_buffer->setColors(i, W_2, 2, 10, 200, 200 + (rand() % 55));
      frame_buffer->setColors(i, W_3, 3, 10, 200, 200 + (rand() % 55));
      frame_buffer->setColors(i, W_4, 4, 10, 200, 200 + (rand() % 55));
      frame_buffer->setColors(i, W_5, 5, 10, 200, 200 + (rand() % 55));
      */
    }
    frame_buffer->update();
    delay(50);
  }
}
void multicolorFillAnimation(doubleBuffer *frame_buffer)
{
  frame_buffer->forceSingleBuffer();
  frame_buffer->clear();
  frame_buffer->update();
  for (int k=0; k<HEIGHT; k++)
  {
    for (int j=0; j<WIDTH; j++)
    {
      for (int i=LENGTH-1; i>=0; i--)
      {
        int r_, g_, b_;
        doubleBuffer::randColor(&r_, &g_, &b_);

        frame_buffer->setColors(i, j, k, r_, g_, b_);
        frame_buffer->update();
        delayMicroseconds(50);
      }
    }
    //delay(500);
  }
}
void pinWheelAnimation_1(doubleBuffer *frame_buffer)
{
  int lookup[10] = {0, 1, 2, 3, 4, 5, 4, 3, 2, 1};
  frame_buffer->reset();
  
  for (int cycles = 0; cycles<400; cycles++)
  {
    frame_buffer->clear();
    for (int i=0; i<LENGTH; i++)
    {
      //int idx = (i+cycles)% (LENGTH-10);
      //float W = 3*sin(2*PI*i/10.0 + cycles)+3;

      
      
      int W_0 = lookup[(i+cycles)%10];
      int W_1 = lookup[(i+400-cycles)%10]; 
      int W_2 = lookup[(i+cycles+6)%10];
      //if (W_ < WIDTH && W_ >= 0)

      frame_buffer->setColorChannel(i, W_0, 0, RED, 255);
      frame_buffer->setColorChannel(i, W_0, 2, GREEN, 255);
      frame_buffer->setColorChannel(i, W_0, 5, BLUE, 255);
      //fbuf[i][W_0][0][RED] = 255; 
      //fbuf[i][W_1][2][GREEN] = 255;
      //fbuf[i][W_2][5][BLUE] = 255;
    }
    frame_buffer->update();
    delay(50);
  }
}




#endif
