#ifndef TEST_ANI_LIB
#define TEST_ANI_LIB

#include "FrameBuffer.h"
#include "Vector3d.h"
#include "Events.h"
#include "Color.h"
#include "Shell.h"
#include "MathLookup.h"

void textAnimation(doubleBuffer *frame_buffer)
{
  static const char *text[4] = {"CMU ECE",
                          "HELLO WORLD",
                          "EMBEDDED SYSTEMS ARE FUN",
                          "DONT LET YOUR DREAMS BE DREAMS"};
 
  static bool start = true;
  static uint8_t text_sel = 0;
  static uint8_t height = 0; 
  static uint16_t text_len = 0;
  static uint8_t r, g, b;
  
  static uint16_t cnt = 0;
  static const uint16_t delay_cnt = 8; 
  static int16_t idx = 0;

  if (cnt++ % delay_cnt == 0)
  {
    if (start == true)
    {
      text_sel = rand() % 4;
      height = rand() % HEIGHT;
      text_len = strlen(text[text_sel]);
      doubleBuffer::randColor(&r, &g, &b);
      idx = LENGTH + 5;
      start = false;
    }
    else
    {
      idx--;
      if (idx <= (-2*text_len - 5))
      {
        start = true;
      }
    }
  } 
  writeString(text[text_sel], idx, height, r, g, b, frame_buffer);
}

void pinWheelAnimation_0(doubleBuffer *frame_buffer)
{
  static const int8_t lookup[10] = {0, 1, 2, 3, 4, 5, 4, 3, 2, 1};
  static const uint16_t N_CYCLE = 60;

  static uint16_t cnt = 0;
  static const uint16_t delay_cnt = 10; 

  static bool start = true;
  static uint8_t sel = 0;
  static uint8_t r_, g_, b_;
  static uint16_t cycles = 0;

  if (cnt++ % delay_cnt == 0)
  {
    if (start == true)
    {
      sel = rand() % 6;
      doubleBuffer::randColor(&r_, &g_, &b_);
      frame_buffer->forceDoubleBuffer();
      cycles = 0;
      start = false;
    }
    else
    {
      cycles++;
      if (cycles == N_CYCLE)
      {
        frame_buffer->forceSingleBuffer();
      }
      if (cycles >= 2*N_CYCLE)
      {
        start = true;
      }
    }
  }

  uint16_t cycles_ = cycles % N_CYCLE;
  for (int i=0; i<LENGTH; i++)
  {        
    int W_0 = lookup[(i+cycles_)%10];
    int W_1 = lookup[(i+N_CYCLE-cycles_)%10]; 
    int W_2 = lookup[(i+cycles_+6)%10];

    frame_buffer->setColorChannel(i, W_0, 0, RED, 255);
    frame_buffer->setColorChannel(i, W_1, 2, GREEN, 255);
    frame_buffer->setColors(i, W_2, 5, r_, g_, b_);
  }
}

void vortexAnimation(doubleBuffer *frame_buffer)
{
  static const uint8_t lookup2[10] = {0, 0, 0, 0, 0, 7, 7, 7, 7, 7};
  static bool start = true;

  
  static uint8_t color_r, color_g, color_b;
  static uint16_t cycles = 0;
  static uint16_t cnt = 0;
  static const uint16_t delay_cnt = 10; 

  if (cnt++ % delay_cnt == 0)
  {  
    if (start == true)
    {
      cycles = 0;
      frame_buffer->forceDoubleBuffer();
      doubleBuffer::randColor(&color_r, &color_g, &color_b);
      start = false;
    }
    else
    {
      cycles++;
      cycles %= 400;
    }
  }

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
        uint8_t r_, g_, b_;
        doubleBuffer::randColor(&r_, &g_, &b_);

        frame_buffer->setColors(i, j, k, r_, g_, b_);
        frame_buffer->update();
        delayMicroseconds(50);
      }
    }
    //delay(500);
  }
/*
  static uint16_t idx = 0;
  uint16_t x = idx % LENGTH;
  uint16_t y = (idx / LENGTH) % WIDTH;
  uint16_t z = (idx / (LENGTH * WIDTH)) % HEIGHT;
*/
}
void pinWheelAnimation_1(doubleBuffer *frame_buffer)
{
  static const uint8_t lookup[10] = {0, 1, 2, 3, 4, 5, 4, 3, 2, 1};
  static bool start = true;
  static uint16_t cycles = 0;
  static uint16_t cnt = 0;
  static const uint16_t delay_cnt = 10; 

  if (cnt++ % delay_cnt == 0)
  {
    if (start == true)
    {
      frame_buffer->forceDoubleBuffer();
    }
    else
    {
      cycles++;
      cycles %= 400;
    }
  }

  for (int i=0; i<LENGTH; i++)
  {
    int W_0 = lookup[(i+cycles)%10];
    int W_1 = lookup[(i+400-cycles)%10]; 
    int W_2 = lookup[(i+cycles+6)%10];

    frame_buffer->setColorChannel(i, W_0, 0, RED, 255);
    frame_buffer->setColorChannel(i, W_0, 2, GREEN, 255);
    frame_buffer->setColorChannel(i, W_0, 5, BLUE, 255);
  }
}

void pulseAnimation(doubleBuffer *frame_buffer)
{
  uint8_t pixels[LENGTH][WIDTH];
  uint8_t pixels_target[LENGTH][WIDTH];
  frame_buffer->reset();
  frame_buffer->clear();
  
  uint8_t r, g, b;
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
  static uint16_t cnt = 0;
  static const uint16_t delay_cnt = 200; 
  static Color colors[WIDTH];

  if (cnt++ % delay_cnt == 0)
  {
    for (int i=0; i<WIDTH; i++)
    {
      doubleBuffer::randColor(&colors[i].r, &colors[i].g, &colors[i].b);
    }
  }

  for (int j=0; j<WIDTH; j++)
  {
    uint8_t r, g, b;
    r = colors[j].r;
    g = colors[j].g;
    b = colors[j].b;
    for (int k=0; k<HEIGHT; k++)
    {
      frame_buffer->setColors(0, j, k, r, g, b);
      frame_buffer->setColors(15, j, k, r, g, b);
      frame_buffer->setColors(30, j, k, r, g, b);
      frame_buffer->setColors(45, j, k, r, g, b);
    }
  }  
}

//TODO: Make non-blocking
void wobbly_words(doubleBuffer *frame_buffer)
{
  static uint16_t cnt = 0;
  static const uint16_t delay_cnt = 20; 
  const char *words[] = {"HELLO", "WORLD", "POV", "11:30"};

  int offset = 0;
  uint8_t r__=128;
  uint8_t g__=128;
  uint8_t b__=128;
  
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
    uint8_t r, g, b;
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


//TODO: Make non-blocking
//Give ball random velocity, it bounces when it collides with edges of display
//Collisions cause radial hit effect of randome color
void ball_collision(doubleBuffer *frame_buffer)
{
  //setup
  Vector3d pos(rand()%LENGTH, rand()%WIDTH, rand()%HEIGHT);
  Vector3d collide_pos;
  Vector3d vel(1, -1, -1);
  uint8_t r, g, b;
  doubleBuffer::randColor(&r, &g, &b);
  char rBuf[4];
  char gBuf[4];
  char bBuf[4];

  uint8_t r_coll = 0;
  uint8_t g_coll = 0;
  uint8_t b_coll = 0;
  
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
        //frame_buffer->setColors(55 - i, j, 5, sprite0[j][i][RED], sprite0[j][i][GREEN], sprite0[j][i][BLUE]);
      }
    }
    
    frame_buffer->update();
    delay(40);
  }
}

//TODO: Make non-blocking
void random_walk(doubleBuffer *frame_buffer)
{
  enum DIRECTION{CW_DIR, CCW_DIR, IN_DIR, OUT_DIR, UP_DIR, DOWN_DIR, NUM_DIR};
  Vector3d pos;
  uint8_t shift_cnt = 0;
  
  pos.x = rand() % LENGTH;
  pos.y = rand() % WIDTH;
  pos.z = rand() % HEIGHT;

  frame_buffer->forceSingleBuffer();
  frameBuffer* fb = frame_buffer->getWriteBuffer();
  frame_buffer->clear();
  while(1)
  {
    if (shift_cnt == 2)
    {
      for (int i=0; i<LENGTH; i++)
      {
        for (int j=0; j<WIDTH; j++)
        {
          for (int k=0; k<HEIGHT; k++)
          {
            fb->fbuf_[i][j][k][RED] >>= 1;
            fb->fbuf_[i][j][k][GREEN] >>= 1;
            fb->fbuf_[i][j][k][BLUE] >>= 1;
          }
        }
      }
    }
    uint8_t dir = rand() % NUM_DIR;
    switch(dir)
    {
      case CW_DIR:
        pos.x++;
        if (pos.x >= LENGTH)
        {
          pos.x = 0;
        }
        break;
      case CCW_DIR:
        pos.x--;
        if (pos.x < 0)
        {
          pos.x = LENGTH - 1;
        }
        break;
      case IN_DIR:
        pos.y--;
        if (pos.y < 0)
        {
          pos.y = WIDTH - 1;
        }
        break;
      case OUT_DIR:
        pos.y++;
        if (pos.y >= WIDTH)
        {
          pos.y = 0;
        }
        break;
      case UP_DIR:
        pos.z++;
        if (pos.z >= HEIGHT)
        {
          pos.z = 0;
        }
        break;
      case DOWN_DIR:
        pos.z--;
        if (pos.z < 0)
        {
          pos.z = HEIGHT - 1;
        }
        break;
    }
    
    doubleBuffer::randColor(&fb->fbuf_[pos.x][pos.y][pos.z][RED], &fb->fbuf_[pos.x][pos.y][pos.z][GREEN], &fb->fbuf_[pos.x][pos.y][pos.z][BLUE]);
    shift_cnt++;
    shift_cnt %= 3;
    delay(33);
  }
}


const uint8_t maze_wall_data[16][3] = {
                                  {0, 0, 0}, {0, 7, 5},
                                  {10, 4, 0}, {10, 7, 5},
                                  {20, 0, 0}, {20, 3, 5},
                                  {30, 4, 0}, {30, 7, 5},
                                  
                                  {40, 0, 0}, {40, 7, 2},
                                  {50, 0, 3}, {50, 7, 5},
                                  {60, 0, 0}, {60, 7, 2},
                                  {70, 0, 3}, {70, 7, 5},
                               };

class MazeWall {
  private:
    uint8_t brightness;
  
  public:
    Vector3d p0;
    Vector3d p1;
    
    MazeWall() {brightness = 255;}
    MazeWall(Vector3d p0_, Vector3d p1_) {p0 = p0_; p1 = p1_; brightness=255;}
    void setMazeWall(Vector3d p0_, Vector3d p1_) {p0 = p0_; p1 = p1_; brightness=255;}
    bool checkCollision(Vector3d in0, Vector3d in1);
    void update();//Maybe colors or flashing stuff
    void draw(doubleBuffer *frame_buffer, Vector3d offset);
};
bool MazeWall::checkCollision(Vector3d in0, Vector3d in1)
{
  /*
  if ((in0.x >= p0.x) && (in1.x <= p1.x) &&
      (in0.y >= p0.y) && (in1.y <= p1.y) &&
      (in0.z >= p0.z) && (in1.z <= p1.z))
  {
    return true;
  }
  return false;
  */

  if (
        (in0.x > p1.x) || //Left edge of player is further right than right edge of wall
        (in1.x < p0.x) || //right edge of player is further left than left edge of wall
        (in1.y < p0.y) || //outmost edge of player is further in than inmost edge of wall
        (in0.y > p1.y) || //inmost edge of player is further out than outmost edge of wall
        (in1.z < p0.z) || //Top of player is below bottom of wall 
        (in0.z > p1.z)    //Bottom of player is abobe top of wall          
     )
  {
    return false;  
  }
  return true;
  
}
void MazeWall::update()
{
  
}
void MazeWall::draw(doubleBuffer *frame_buffer, Vector3d offset)
{
  for (int i=p0.x; i <= p1.x; i++)
  {
    int x_idx = i + offset.x;
    if (x_idx < 0)
      x_idx += LENGTH;
    x_idx %= LENGTH;
    for (int j=p0.y; j <= p1.y; j++)
    {
      int y_idx = j + offset.y;
      //if (y_idx < 0)
      //  y_idx += WIDTH;
      //y_idx %= WIDTH;
      for (int k=p0.z; k <= p1.z; k++)
      {
        int z_idx = k + offset.z;
        //if (z_idx < 0)
        //  z_idx += HEIGHT;
        //z_idx %= HEIGHT;

        frame_buffer->setColors(x_idx, y_idx, z_idx, 255, 255, 255);
      }
    }
  }
}


class MazePlayer {
  private:
    static const uint8_t size = 2;
    Vector3d pos;
    MazeWall *walls;
    uint8_t N_walls;
    bool cw, ccw, in, out, up, down;
    
  public:
    MazePlayer();
    MazePlayer(Vector3d pos_, MazeWall *walls_, uint8_t N_walls_);
    void setPlayer(Vector3d pos_, MazeWall *walls_, uint8_t N_walls_);
    void update();
    void draw(doubleBuffer *frame_buffer, Vector3d offset);
    bool intersects(Vector3d p0, Vector3d p1);
    Vector3d getPos() {return pos;}
    void setMoveCW(bool b);// {cw = b;}
    void setMoveCCW(bool b);// {ccw = b;}
    void setMoveIn(bool b);// {in = b;}
    void setMoveOut(bool b);// {out = b;}
    void setMoveUp(bool b);// {up = b;}
    void setMoveDown(bool b);// {down = b;}
    void getEndPoints(Vector3d *p0, Vector3d *p1);
};
MazePlayer::MazePlayer()
{
  walls = NULL;
  N_walls = 0;
  cw = false;
  ccw = false;
  in = false;
  out = false;
  up = false;
  down = false;
}
MazePlayer::MazePlayer(Vector3d pos_, MazeWall *walls_, uint8_t N_walls_)
{
  pos = pos_;
  walls = walls_;
  N_walls = N_walls_;
  cw = false;
  ccw = false;
  in = false;
  out = false;
  up = false;
  down = false;
}
void MazePlayer::setPlayer(Vector3d pos_, MazeWall *walls_, uint8_t N_walls_)
{
  pos = pos_;
  walls = walls_;
  N_walls = N_walls_;
  cw = false;
  ccw = false;
  in = false;
  out = false;
  up = false;
  down = false;
}
void MazePlayer::update()
{
  //Check if should be moving
  Vector3d next_pos = pos;


  //Decide if you want to be able to move in mutiple directions at once
  if (cw)
  {
    //SerialUSB.println("Moving CW");
    next_pos.x += 1;
  }
  else if (ccw)
  {
    //SerialUSB.println("Moving CCW");
    next_pos.x -= 1;
  }
  else if (out)
  {
    //SerialUSB.println("Moving OUT");
    next_pos.y += 1;
  }
  else if (in)
  {
    //SerialUSB.println("Moving IN");
    next_pos.y -= 1;
  }
  else if (up)
  {
    //SerialUSB.println("Moving UP");
    next_pos.z += 1;
  }
  else if (down)
  {
    //SerialUSB.println("Moving DOWN");
    next_pos.z -= 1;
  }


  //X wraps
  if (next_pos.x >= LENGTH)
  {
    next_pos.x = 0;
  }
  if (next_pos.x < 0)
  {
    next_pos.x = LENGTH-1;
  }

  //Y is bounded
  if ((next_pos.y + size - 1)  >= WIDTH)
  {
    next_pos.y = WIDTH-size;//(pos.y)
  }
  if (next_pos.y < 0)
  {
    next_pos.y = 0;
  }

  //Z is bounded (eventually by arbitary values)
  if ((next_pos.z + size - 1)  >= HEIGHT)
  {
    next_pos.z = HEIGHT-size;//(pos.z)
  }
  if (next_pos.z < 0)
  {
    next_pos.z = 0;
  }
  

  Vector3d p1 = Vector3d(next_pos.x + size - 1, next_pos.y + size - 1, next_pos.z + size - 1);
  //Check for collisions with maze walls
  if (walls != NULL)
  {
    for (uint8_t i=0; i<N_walls; i++)
    {
      if (walls[i].checkCollision(next_pos, p1))
      {
        next_pos = pos;
        break;
      }
    }
  }

  pos = next_pos;
}
void MazePlayer::draw(doubleBuffer *frame_buffer, Vector3d offset)
{
  for (int i=pos.x; i<pos.x+size; i++)
  {
    int x_idx = i + offset.x;
    if (x_idx < 0)
      x_idx += LENGTH;
    x_idx %= LENGTH;
    for (int j=pos.y; j<pos.y+size; j++)
    {
      int y_idx = j + offset.y;
      for (int k=pos.z; k<pos.z+size; k++)
      {
        int z_idx = k + offset.z;
        frame_buffer->setColors(x_idx, y_idx, z_idx, 0, 0, 255);
      }
    }
  }
}
void MazePlayer::setMoveCW(bool b) 
{
  cw = b;
}
void MazePlayer::setMoveCCW(bool b) 
{
  ccw = b;
}
void MazePlayer::setMoveIn(bool b) 
{
  in = b;
}
void MazePlayer::setMoveOut(bool b) 
{
  out = b;
}
void MazePlayer::setMoveUp(bool b) 
{
  up = b;
}
void MazePlayer::setMoveDown(bool b) 
{
  down = b;
}
bool MazePlayer::intersects(Vector3d p0, Vector3d p1)
{
  
  Vector3d pos1 = Vector3d(pos.x+size-1, pos.y+size-1, pos.z+size-1);
  if (
        (pos.x > p1.x) || //Left edge of player is further right than right edge of object
        (pos1.x < p0.x) || //right edge of player is further left than left edge of object
        (pos1.y < p0.y) || //outmost edge of player is further in than inmost edge of object
        (pos.y > p1.y) || //inmost edge of player is further out than outmost edge of object
        (pos1.z < p0.z) || //Top of player is below bottom of object
        (pos.z > p1.z)    //Bottom of player is abobe top of object          
     )
  {
    return false;  
  }
  return true;
}
void MazePlayer::getEndPoints(Vector3d *p0, Vector3d *p1)
{
  Vector3d pos1 = Vector3d(pos.x + size - 1, pos.y + size - 1, pos.z + size - 1);
  *p0 = pos;
  *p1 = pos1;
}

 
class MazeGoal {
  private:
    Vector3d pos;
    uint8_t brightness;
    static const uint8_t size=3;

  public:
    MazeGoal() {brightness = 255;}
    MazeGoal(Vector3d pos_) {pos = pos_; brightness = 255;}
    void init(Vector3d pos_) {pos = pos_; brightness = 255;}
    void update();
    void draw(doubleBuffer *frame_buffer, Vector3d offset);
    void getEndPoints(Vector3d *p0, Vector3d *p1);
};
void MazeGoal::update()
{
  brightness -= 5;
}
void MazeGoal::draw(doubleBuffer *frame_buffer, Vector3d offset)
{
  uint8_t val = brightness;
  if (val < 128)
  {
    val = 255 - val;
  }

  int z_idx = pos.z + offset.z;
  for (int i=pos.x; i<pos.x+size; i++)
  {
    int x_idx = i + offset.x;
    if (x_idx < 0)
      x_idx += LENGTH;
    x_idx %= LENGTH;
    for (int j=pos.y; j<pos.y+size; j++)
    {
      int y_idx = j + offset.y;
      frame_buffer->setColors(x_idx, y_idx, z_idx, 0, val, 0);
    }
  }
}
void MazeGoal::getEndPoints(Vector3d *p0, Vector3d *p1)
{
  Vector3d pos1 = Vector3d(pos.x + size - 1, pos.y + size - 1, pos.z + size - 1);
  *p0 = pos;
  *p1 = pos1;
}

class MazeGame {
  private:
    enum BUTTON_MAP{
      LEFT_DIR=DLEFT, 
      FIRE = SQUARE, 
      RIGHT_DIR=DRIGHT, 
      UP_DIR=RBUMP, 
      DOWN_DIR=LBUMP, 
      IN_DIR = DUP, 
      OUT_DIR = DDOWN,
    };
    enum STATE{START_STATE, PLAY_STATE, END_STATE} game_state = START_STATE;
    MazeWall walls[8];
    MazePlayer player;
    MazeGoal goal;
    bool goal_reached;
    uint8_t cnt;
    static const uint8_t delay_cnt = 7;

    int handleInputs();
      
  public:
    MazeGame();
    void init();
    void update();
    void draw(doubleBuffer *frame_buffer);
};
MazeGame::MazeGame()
{
  //goal_reached = false;
}
void MazeGame::init()
{
  //Populate walls
  for (int i=0; i<16; i+=2)
  {
    walls[i/2].setMazeWall(Vector3d(maze_wall_data[i][0], maze_wall_data[i][1], maze_wall_data[i][2]), 
                           Vector3d(maze_wall_data[i+1][0], maze_wall_data[i+1][1], maze_wall_data[i+1][2])); 
  }
  
  player.setPlayer(Vector3d(2, 0, 0), walls, 8);
  goal.init(Vector3d(LENGTH - 6, 2, 0));
  goal_reached = false;
  cnt = 0;
}
void MazeGame::update()
{
  if (cnt++ % delay_cnt != 0)
  {
    return;
  }

  handleInputs();
  
  player.update();
  goal.update();
  for (int i=0; i<8; i++)
  {
    walls[i].update();
  }

  Vector3d goal0, goal1;
  goal.getEndPoints(&goal0, &goal1);
  if (player.intersects(goal0, goal1))
  {
    goal_reached = true;
  }
  
}
void MazeGame::draw(doubleBuffer *frame_buffer)
{
  Vector3d offset = Vector3d(0,0,0);
  Vector3d player_pos = player.getPos();
  int target = 60;
  offset.x = target - player_pos.x;
  
  player.draw(frame_buffer, offset);
  for (int i=0; i<8; i++)
  {
    walls[i].draw(frame_buffer, offset);
  }
  goal.draw(frame_buffer, offset);
  
  if (goal_reached)
  {
    for (int i=0; i<LENGTH; i++)
      frame_buffer->setColors(i, 3, 2, 0, 255, 0);  
  }
}
int MazeGame::handleInputs()
{
  while (!eventBuffer.isEmpty())
  {
    Event e;
    if (!eventBuffer.pop(e))
    {
      //Handle some error condition
      SerialUSB.println("Error: Failed to pop event from buffer");
    }

    switch (e.type)
    {
      case Event::ON_PRESS:
      {
        switch (e.data.button_idx)
        {
          case LEFT_DIR:
          {
            player.setMoveCCW(true);
            SerialUSB.println("set move CCW");
            break;
          }
          case RIGHT_DIR:
          {
            player.setMoveCW(true);
            SerialUSB.println("set move CW");
            break;
          }
          case UP_DIR:
          {
            player.setMoveUp(true);
            SerialUSB.println("set move UP");
            break;
          }
          case DOWN_DIR:
          {
            player.setMoveDown(true);
            SerialUSB.println("set move DOWN");
            break;
          }
          case IN_DIR:
          {
            player.setMoveIn(true);
            SerialUSB.println("set move IN");
            break;
          }
          case OUT_DIR:
          {
            player.setMoveOut(true);
            SerialUSB.println("set move OUT");
            break;
          }
          default:
          {
            break;
          }
        }
        break;
      }
      case Event::ON_RELEASE:
      {
        switch (e.data.button_idx)
        {
          case LEFT_DIR:
          {
            player.setMoveCCW(false);
            break;
          }
          case RIGHT_DIR:
          {
            player.setMoveCW(false);
            break;
          }
          case UP_DIR:
          {
            player.setMoveUp(false);
            break;
          }
          case DOWN_DIR:
          {
            player.setMoveDown(false);
            break;
          }
          case IN_DIR:
          {
            player.setMoveIn(false);
            SerialUSB.println("set move IN");
            break;
          }
          case OUT_DIR:
          {
            player.setMoveOut(false);
            SerialUSB.println("set move OUT");
            break;
          }
          default:
          {
            break;
          }
        }
        break;
      }
    }
  }
  return 0;
}

void rainbow_swirl(doubleBuffer* frame_buffer)
{
    static const int width_offset = 60 / (WIDTH - 1);
    static const uint8_t height_transform[15] = { 2, 4, 4, 5, 5, 5, 5, 4, 4, 3, 2, 1, 1, 1, 1 };
    static const uint8_t trans_size = 15;
    static const uint8_t delay_cycles = 6;

    static int hue_offset = 0;
    static uint8_t delay_cnt = 0;
    
    for (int i = 0; i < LENGTH; i++)
    {
        int hue = (i * 255) / LENGTH;
        int k = 0;
        if ((i >= 20) && i < (20 + trans_size))
        {
            int idx = i - 20;//i from 0 to trans_size - 1
            k = height_transform[trans_size - 1 - idx];
        }

        for (int j = 0; j < WIDTH; j++)
        {
            Color color = Color::getColorHSV(hue + hue_offset + ((WIDTH - 1 - j) * width_offset), 255, 255);
            frame_buffer->setColors(i, j, k, color.r, color.g, color.b);
        }
    }

    delay_cnt++;
    if (delay_cnt % delay_cycles == 0)
    {
        hue_offset += 3;
        delay_cnt = 0;
    }
}

void sine_wave_3d(doubleBuffer* frame_buffer)
{
  static const float PI_FRAC = M_PI/16.0;
  static const uint8_t delay_cycles = 7;
  static uint8_t delay_cnt = 0;
  static int sin_idx = 0;

  float val2;
  int g_value;
  for (int j=0; j<WIDTH; j++)
  {
    int c_idx = (j+sin_idx) % 32;
    val2 = 0.5*cos_lookup_alt(c_idx*PI_FRAC) + 0.5;
    g_value = (int)(255*val2);  
  }
  for (int i=0; i<LENGTH; i++)
  {
    int s_idx = (i+sin_idx) % 32;
    float brightness = 0.5*sin_lookup_alt(s_idx*PI_FRAC) + 0.5;
    int r_value = (int)(255*brightness);
    int b_value = 255 - r_value;
    int height = (int)(brightness*(HEIGHT-1) + 0.5);
    for (int j=0; j<WIDTH; j++)
    {
        frame_buffer->setColors(i, j, height, r_value, g_value, b_value);
    }
  }

  delay_cnt++;
  if (delay_cnt % delay_cycles == 0)
  {
    delay_cnt = 0;
    sin_idx++;
    if (sin_idx == LENGTH)
      sin_idx = 0;
  }
}

#endif
