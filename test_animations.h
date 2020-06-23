#ifndef TEST_ANI_LIB
#define TEST_ANI_LIB

#include "FrameBuffer.h"
#include "Vector3d.h"

void textAnimation(doubleBuffer *frame_buffer)
{
  frame_buffer->reset();
  char *text[4] = {"CMU ECE",
                  "HELLO WORLD",
                  "EMBEDDED SYSTEMS ARE FUN",
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
      int W_0 = (lookup2[(i+cycles)%10] + cycles) % 8;
      int W_1 = (lookup2[(i+cycles+3)%10] + cycles + 1) % 8; 
      int W_2 = (lookup2[(i+cycles+6)%10] + cycles + 2) % 8; 
      int W_3 = (lookup2[(i+cycles+9)%10] + cycles + 3) % 8;
      int W_4 = (lookup2[(i+cycles+12)%10] + cycles + 4) % 8;
      int W_5 = (lookup2[(i+cycles+15)%10] + cycles + 5) % 8;      

      frame_buffer->setColors(i, W_0, 0, color_r, color_g, color_b);
      frame_buffer->setColors(i, W_1, 1, color_r, color_g, color_b);
      frame_buffer->setColors(i, W_2, 2, color_r, color_g, color_b);
      frame_buffer->setColors(i, W_3, 3, color_r, color_g, color_b);
      frame_buffer->setColors(i, W_4, 4, color_r, color_g, color_b);
      frame_buffer->setColors(i, W_5, 5, color_r, color_g, color_b);
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
      int W_0 = lookup[(i+cycles)%10];
      int W_1 = lookup[(i+400-cycles)%10]; 
      int W_2 = lookup[(i+cycles+6)%10];

      frame_buffer->setColorChannel(i, W_0, 0, RED, 255);
      frame_buffer->setColorChannel(i, W_0, 2, GREEN, 255);
      frame_buffer->setColorChannel(i, W_0, 5, BLUE, 255);
    }
    frame_buffer->update();
    delay(50);
  }
}

void pulseAnimation(doubleBuffer *frame_buffer)
{
  uint8_t pixels[LENGTH][WIDTH];
  uint8_t pixels_target[LENGTH][WIDTH];
  frame_buffer->reset();
  frame_buffer->clear();
  
  int r, g, b;
  doubleBuffer::randColor(&r, &g, &b);
  for (int i=0; i<LENGTH; i++)
  {
    for (int j=0; j<WIDTH; j++)
    {
      pixels[i][j] = 0;
      pixels_target[i][j] = rand()%HEIGHT;
      frame_buffer->setColors(i, j, 0, r, g, b);
    }
  }
  frame_buffer->update();
  delay(1000);

  
  for (int k=0; k<HEIGHT; k++)
  {
    frame_buffer->clear();
    for (int i=0; i<LENGTH; i++)
    {
      for (int j=0; j<WIDTH; j++)
      {
        if (k <= pixels_target[i][j])
        {
          frame_buffer->setColors(i, j, k, r, g, b);
        }
        else
        {
          frame_buffer->setColors(i, j, pixels_target[i][j], r, g, b);
        }
      }
    }
    frame_buffer->update();
    delay(35);
  }
  delay(965);

  for (int k=HEIGHT-2; k>=1; k--)
  {
    frame_buffer->clear();
    for (int i=0; i<LENGTH; i++)
    {
      for (int j=0; j<WIDTH; j++)
      {
        if (k <= pixels_target[i][j])
        {
          frame_buffer->setColors(i, j, k, r, g, b);
        }
        else
        {
          frame_buffer->setColors(i, j, pixels_target[i][j], r, g, b);
        }
      }
    }
    frame_buffer->update();
    delay(35);
  }
}

void alignment_test(doubleBuffer *frame_buffer)
{
  while(1)
  {
    int r, g, b;
    
    frame_buffer->clear();
    for (int j=0; j<WIDTH; j++)
    {
      doubleBuffer::randColor(&r, &g, &b);
      for (int k=0; k<HEIGHT; k++)
      {
        frame_buffer->setColors(0, j, k, r, g, b);
        frame_buffer->setColors(15, j, k, r, g, b);
        frame_buffer->setColors(30, j, k, r, g, b);
        frame_buffer->setColors(45, j, k, r, g, b);
      }
    }
    frame_buffer->update();
    delay(1000);  
  }
}

void wobbly_words(doubleBuffer *frame_buffer)
{
  int offset = 0;
  int r__=128;
  int g__=128;
  int b__=128;
  char *words[] = {"HELLO", "WORLD", "POV", "11:30"};
  int word_idx = 0;
  while(1)
  {
    frame_buffer->clear();
    
    

    switch (rand() % 6)
    {
      case 0:
        r__ += 2;
        if (r__ > 255)
          r__ = 255;
        break;
      case 1:
        r__ -= 2;
        if (r__ < 0)
          r__ = 0;
        break;
      case 2:
        g__ += 2;
        if (g__ > 255)
          g__ = 255;
        break;
      case 3:
        g__ += 2;
        if (g__ > 255)
          g__ = 255;
        break;
      case 4:
        b__ += 2;
        if (b__ > 255)
          b__ = 255;
        break;
      case 5:
        b__ += 2;
        if (b__ > 255)
          b__ = 255;
        break;
    }
    writeString(words[word_idx], 0, 0, r__, g__, b__, frame_buffer);
    frameBuffer *fb = frame_buffer->getWriteBuffer();
    for (int i=0; i<LENGTH; i++)
    {
      int i_ = (i + offset)/2 % 10;
      int h_idx;
      if (i_ < 6)
      {
        h_idx = i_;
      }
      else
      {
        h_idx = 10 - i_;
      }
      for (int j=0; j<WIDTH; j++)
      {
        for (int k=0; k<h_idx; k++)
        {
        fb->fbuf_[i][j][k][RED] = fb->fbuf_[i][j][0][RED];
        fb->fbuf_[i][j][k][GREEN] = fb->fbuf_[i][j][0][GREEN];
        fb->fbuf_[i][j][k][BLUE] = fb->fbuf_[i][j][0][BLUE];
        }
      }
    }
    frame_buffer->update();
    offset++;
    if (offset >= LENGTH)
    {
      offset = 0;
      word_idx++;
      if (word_idx >= 4)
        word_idx = 0;
    }
    delay(100);
  }
}

void draw_triange_wave(doubleBuffer *frame_buffer)
{
  for (int i=0; i<LENGTH; i++)
  {
    int r, g, b;
    doubleBuffer::randColor(&r, &g, &b);
    int i_ = i % 10;
    int h_idx;
    if (i_ < 6)
    {
      h_idx = i_;
    }
    else
    {
      h_idx = 10 - i_;
    }
    for (int j=0; j<WIDTH; j++)
    {
      frame_buffer->setColors(i, j, h_idx, r, g, b);
    }
  }
}

//Give ball random velocity, it bounces when it collides with edges of display
//Collisions cause radial hit effect of randome color
void ball_collision(doubleBuffer *frame_buffer)
{
  //setup
  Vector3d pos(rand()%LENGTH, rand()%WIDTH, rand()%HEIGHT);
  Vector3d collide_pos;
  Vector3d vel(1, -1, -1);
  int r, g, b;
  doubleBuffer::randColor(&r, &g, &b);
  char rBuf[4];
  char gBuf[4];
  char bBuf[4];

  int r_coll = 0;
  int g_coll = 0;
  int b_coll = 0;
  
  int walls[2][2] = {{20, 0}, {60, 1}};
  

  //loop
  while(1)
  {
    bool collide = false;
    //Update game logic
    pos.addVector3d(vel);

    if (pos.x >= LENGTH)
    {
      pos.x = 0;
    }
    if (pos.x < 0)
    {
      pos.x = LENGTH - 1;
    }
    
    if (pos.y < 0)
    {
      pos.y = 0;
      vel.y *= -1;
      collide = true;
    }
    if (pos.y >= WIDTH)
    {
      pos.y = WIDTH - 1; 
      vel.y *= -1;
      collide = true;
    }

    if (pos.z < 0)
    {
      pos.z = 0;
      vel.z *= -1;
      collide = true;
    }
    if (pos.z >= HEIGHT)
    {
      pos.z = HEIGHT - 1;
      vel.z *= -1;
      collide = true; 
    }

    if (pos.x == walls[0][0] && pos.y < 4)
    {
      vel.x *= -1;
      collide = true;
    }
    if (pos.x == walls[1][0] && pos.y >= 4)
    {
      vel.x *= -1;
      collide = true;
    }


    //Dim the colors
    if (r_coll > 0)
    {
      r_coll -= 16;
    }
    if (r_coll < 0)
    {
      r_coll = 0;
    }
    if (b_coll > 0)
    {
      b_coll -= 16;
    }
    if (b_coll < 0)
    {
      b_coll = 0;
    }
    if (g_coll > 0)
    {
      g_coll -= 16;
    }
    if (g_coll < 0)
    {
      g_coll = 0;
    }


    if (collide && (r_coll == 0 && b_coll == 0 && g_coll == 0))
    {
      collide_pos = pos;
      doubleBuffer::randColor(&r_coll, &g_coll, &b_coll);
    }

    sprintf(rBuf, "%d", pos.x);
    sprintf(gBuf, "%d", pos.y);
    sprintf(bBuf, "%d", pos.z);

    //Draw stuff
    frame_buffer->clear();
    /*
    for (int i=0; i<LENGTH; i++)
    {
      for (int k=0; k<HEIGHT; k++)
      {
        if (k == 0 || k == HEIGHT-1)
          frame_buffer->setColors(i, 0, k, 255, 255, 255);
        else
          frame_buffer->setColors(i, 0, k, 12, 12, 12);
      }
    }
    */
    frame_buffer->drawBlock(Vector3d(walls[0][0], 0, 0), Vector3d(walls[0][0], 3, HEIGHT-1),
                            255, 255, 255, false);
    frame_buffer->drawBlock(Vector3d(walls[1][0], 4, 0), Vector3d(walls[1][0], WIDTH-1, HEIGHT-1),
                            255, 255, 255, false);
    
    if (r_coll || b_coll || g_coll)
    {
      frame_buffer->setColors(collide_pos.x, collide_pos.y, collide_pos.z, r_coll, g_coll, b_coll);
    }
    
    frame_buffer->setColors(pos.x, pos.y, pos.z, r, g, b);
    /*
    writeString(rBuf, 0, 5, 255, 0, 0, frame_buffer);
    writeString(gBuf, 20, 5, 0, 255, 0, frame_buffer);
    writeString(bBuf, 40, 5, 0, 0, 255, frame_buffer);
    */

    for (int i=0; i<8; i++)
    {
      for (int j=0; j<8; j++)
      {
        frame_buffer->setColors(55 - i, j, 5, sprite0[j][i][RED], sprite0[j][i][GREEN], sprite0[j][i][BLUE]);
      }
    }
    
    frame_buffer->update();
    delay(40);
  }
}




#endif
