#ifndef FRAME_BUFFER_LIB
#define FRAME_BUFFER_LIB

#define BLUE 2
#define GREEN 1
#define RED 0

#define LENGTH 60
#define WIDTH 8
#define HEIGHT 6
#define COLORS 3

volatile static uint8_t fbuf[LENGTH][WIDTH][HEIGHT][COLORS];


void clearBuf()
{
  for (int i=0; i<LENGTH; i++)
  {
    for (int j=0; j<WIDTH; j++)
    {
      for (int k=0; k<HEIGHT; k++)
      {
        fbuf[i][j][k][0] = 0;
        fbuf[i][j][k][1] = 0;
        fbuf[i][j][k][2] = 0;
      }
    }
  }
}


class frameBuffer
{
  public:
    uint8_t fbuf_[LENGTH][WIDTH][HEIGHT][COLORS];
    frameBuffer();
    void clear();
};

frameBuffer::frameBuffer()
{
  clear();
}
void frameBuffer::clear()
{
  for (int i=0; i<LENGTH; i++)
  {
    for (int j=0; j<WIDTH; j++)
    {
      for (int k=0; k<HEIGHT; k++)
      {
        fbuf_[i][j][k][RED] = 0;
        fbuf_[i][j][k][GREEN] = 0;
        fbuf_[i][j][k][BLUE] = 0;
      }
    }
  }
}

class doubleBuffer
{
  private:
    frameBuffer *read_buffer;
    frameBuffer *write_buffer;
    frameBuffer buf1, buf2;


  public:
    doubleBuffer();
    void reset();
    void forceSingleBuffer();
    void setColorChannel(int l, int w, int h, int c_idx, int c_val);
    void setColors(int l, int w, int h, int rVal, int gVal, int bVal);
    void clear();
    void update();
    frameBuffer* getWriteBuffer() {return write_buffer;}
    frameBuffer* getReadBuffer() {return read_buffer;}

    static void randColor(int *r, int *g, int *b);
    
};

doubleBuffer::doubleBuffer()
{
  read_buffer = &buf1;
  write_buffer = &buf2;
}
void doubleBuffer::reset()
{
  read_buffer = &buf1;
  write_buffer = &buf2;
  read_buffer->clear();
  write_buffer->clear();
}
void doubleBuffer::forceSingleBuffer()
{
  read_buffer = &buf1;
  write_buffer = &buf1;
  write_buffer->clear();
}
void doubleBuffer::clear()
{
  write_buffer->clear();
}
void doubleBuffer::setColorChannel(int l, int w, int h, int c_idx, int c_val)
{
  if (l < 0 || w < 0 || h < 0 || c_idx < 0)
    return;
  if (l >= LENGTH || w >= WIDTH || h >= HEIGHT || c_idx >= 3)
    return;

  write_buffer->fbuf_[l][w][h][c_idx] = c_val;
}
void doubleBuffer::setColors(int l, int w, int h, int rVal, int gVal, int bVal)
{
  if (l < 0 || w < 0 || h < 0 || rVal < 0 || gVal < 0 || bVal < 0)
    return;
  if (l >= LENGTH || w >= WIDTH || h >= HEIGHT || rVal >= 256 || gVal >= 256 || bVal >= 256)
    return;

  write_buffer->fbuf_[l][w][h][RED] = rVal;
  write_buffer->fbuf_[l][w][h][GREEN] = gVal;
  write_buffer->fbuf_[l][w][h][BLUE] = bVal;
}
void doubleBuffer::update()
{
  frameBuffer *temp = read_buffer;
  read_buffer = write_buffer;
  write_buffer = temp;
}

void doubleBuffer::randColor(int *r, int *g, int *b)
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


#endif
