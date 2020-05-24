#ifndef FRAME_BUFFER_LIB
#define FRAME_BUFFER_LIB

#include "Vector3d.h"

enum COLORS {RED, GREEN, BLUE, NUM_COLORS};

#define LENGTH 60
#define WIDTH 8
#define HEIGHT 6

class frameBuffer
{
  public:
    uint8_t fbuf_[LENGTH][WIDTH][HEIGHT][NUM_COLORS];
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
    bool isSingleBuffered() {return read_buffer == write_buffer;}
    void setColorChannel(int l, int w, int h, int c_idx, int c_val);
    void setColors(int l, int w, int h, int rVal, int gVal, int bVal);
    void clear();
    void update();
    frameBuffer* getWriteBuffer() {return write_buffer;}
    frameBuffer* getReadBuffer() {return read_buffer;}

    static void randColor(int *r, int *g, int *b);

    //Drawing functions
    void drawBlock(Vector3d v0, Vector3d v1, int r, int g, int b, bool fill = true);
    void drawBlock(int x0, int y0, int z0, int x1, int y1, int z1, int r, int g, int b, bool fill=true);
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
void doubleBuffer::drawBlock(Vector3d v0, Vector3d v1, int r, int g, int b, bool fill)
{
  if (v0.x > v1.x || v0.y > v1.y || v0.z > v1.z)
    return;
  for (int i=v0.x; i <= v1.x; i++)
  {
    if (i < 0 || i >= LENGTH)
      continue;
    for (int j=v0.y; j <= v1.y; j++)
    {
      if (j < 0 || j >= WIDTH)
        continue;
      for (int k=v0.z; k <= v1.z; k++)
      {
        if (k < 0 || k >= HEIGHT)
          continue;
        setColors(i, j, k, r, g, b);
//        write_buffer->fbuf_[i][j][k][RED] = r;
//        write_buffer->fbuf_[i][j][k][GREEN] = g;
//        write_buffer->fbuf_[i][j][k][BLUE] = b;
      }
    }
  }
}
void doubleBuffer::drawBlock(int x0, int y0, int z0, int x1, int y1, int z1, int r, int g, int b, bool fill)
{
  if (x0 > x1 || y0 > y1 || z0 > z1)
    return;
  for (int i=x0; i <= x1; i++)
  {
    SerialUSB.print("i: ");
    SerialUSB.println(i);
    if (i < 0 || i >= LENGTH)
      continue;
    for (int j=y0; j <= y1; j++)
    {
      SerialUSB.print("j: ");
      SerialUSB.println(j);
      if (j < 0 || j >= WIDTH)
        continue;
      for (int k=z0; k <= z1; k++)
      {
        SerialUSB.print("k: ");
        SerialUSB.println(k);
        if (k < 0 || k >= HEIGHT)
          continue;
        setColors(i, j, k, r, g, b);
      }
    }
  }
}

void driveLEDS(int buf_idx, int *buf_offset, doubleBuffer* frame_buffer, SPIClass* mySPI)
{
  frameBuffer* read_buffer = frame_buffer->getReadBuffer();
  mySPI->beginTransaction(SPISettings(12000000, MSBFIRST, SPI_MODE0));

  //Start Frame
  mySPI->transfer(0x00);
  mySPI->transfer(0x00);
  mySPI->transfer(0x00);
  mySPI->transfer(0x00);
  mySPI->transfer(0x00);

  for (int k=0; k<HEIGHT; k++)
  {
    int offset = (buf_idx + buf_offset[k]) % LENGTH;
    for (int j=0; j<WIDTH; j++)
    {
      mySPI->transfer(0xFF);
      mySPI->transfer(read_buffer->fbuf_[offset][j][k][BLUE]);
      mySPI->transfer(read_buffer->fbuf_[offset][j][k][GREEN]);
      mySPI->transfer(read_buffer->fbuf_[offset][j][k][RED]);
    }
  }

  //End Frame
  mySPI->transfer(0xFF);
  mySPI->transfer(0xFF);
  mySPI->transfer(0xFF);
  mySPI->transfer(0xFF);
  mySPI->transfer(0xFF);

  mySPI->endTransaction();
  //buf_idx++;
}



#endif
