#ifndef FRAME_BUFFER_LIB
#define FRAME_BUFFER_LIB

#include "Vector3d.h"

enum COLORS {RED, GREEN, BLUE, NUM_COLORS};

#define LENGTH 96
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
    void setColorChannel(int l, int w, int h, uint8_t c_idx, uint8_t c_val);
    void setColors(int l, int w, int h, uint8_t rVal, uint8_t gVal, uint8_t bVal);
    void clear();
    void update();
    frameBuffer* getWriteBuffer() {return write_buffer;}
    frameBuffer* getReadBuffer() {return read_buffer;}

    static void randColor(uint8_t *r, uint8_t *g, uint8_t *b);

    //Drawing functions
    void drawBlock(Vector3d v0, Vector3d v1, uint8_t r, uint8_t g, uint8_t b, bool fill = true);
    void drawBlock(int x0, int y0, int z0, int x1, int y1, int z1, uint8_t r, uint8_t g, uint8_t b, bool fill=true);
    void drawLine(Vector3d p0, Vector3d p1, uint8_t r, uint8_t g, uint8_t b);
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
void doubleBuffer::setColorChannel(int l, int w, int h, uint8_t c_idx, uint8_t c_val)
{
  if (l < 0 || w < 0 || h < 0 || c_idx < 0)
    return;
  if (l >= LENGTH || w >= WIDTH || h >= HEIGHT || c_idx >= 3)
    return;

  write_buffer->fbuf_[l][w][h][c_idx] = c_val;
}
void doubleBuffer::setColors(int l, int w, int h, uint8_t rVal, uint8_t gVal, uint8_t bVal)
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

void doubleBuffer::randColor(uint8_t *r, uint8_t *g, uint8_t *b)
{
  uint8_t sel = rand() % 6;
  uint8_t r_, g_, b_;
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
void doubleBuffer::drawBlock(Vector3d v0, Vector3d v1, uint8_t r, uint8_t g, uint8_t b, bool fill)
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
void doubleBuffer::drawBlock(int x0, int y0, int z0, int x1, int y1, int z1, uint8_t r, uint8_t g, uint8_t b, bool fill)
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
void doubleBuffer::drawLine(Vector3d p0, Vector3d p1, uint8_t r, uint8_t g, uint8_t b)
{
  enum AXIS {LX, LY, LZ};
  
  Vector3d del = Vector3d::subVector3d(p1, p0);
  float mag = sqrt((del.x*del.x) + (del.y*del.y) + (del.z*del.z));
  float ratio_x = del.x / mag;
  if (ratio_x < 0)
    ratio_x *= -1;
  float ratio_y = del.y / mag;
  if (ratio_y < 0)
    ratio_y *= -1;
  float ratio_z = del.z / mag;
  if (ratio_z < 0)
    ratio_z *= -1;

  Serial1.print("ratiox: ");
  Serial1.println(ratio_x);
  Serial1.print("ratioy: ");
  Serial1.println(ratio_y);
  Serial1.print("ratioz: ");
  Serial1.println(ratio_z);
  
  Serial1.print("delx: ");
  Serial1.println(del.x);
  Serial1.print("dely: ");
  Serial1.println(del.y);
  Serial1.print("delz: ");
  Serial1.println(del.z);

  Serial1.print("mag: ");
  Serial1.println(mag);
  
  enum AXIS axis = LX;
  if ((ratio_y > ratio_x) && (ratio_y > ratio_z))
  {
    axis = LY;
  }
  else if((ratio_z > ratio_x) && (ratio_z > ratio_y))
  {
    axis = LZ;
  }

  Vector3d from = p0;
  Vector3d to = p1;
  /*
  if (p0.x > p1.x)
  {
    from.x = p1.x;
    to.x = p0.x;
  }
  if (p0.y > p1.y)
  {
    from.y = p1.y;
    to.y = p0.y;
  }
  if (p0.z > p1.z)
  {
    from.z = p1.z;
    to.z = p0.z;
  }
  */

  switch (axis)
  {
    case LX:
    {
      Serial1.println("LX");
      if (p0.x > p1.x)
      {
        from = p1;
        to = p0;
      }
      int y_idx = from.y;
      int z_idx = from.z;
      float slope_y = (1.0*del.y)/del.x;
      //if (slope_y < 0)
      //  slope_y *= -1;
      float slope_z = (1.0*del.z)/del.x;
      //if (slope_z < 0)
      //  slope_z *= -1;
        
      for (int x_idx = from.x; x_idx <= to.x; x_idx++)
      {
        //Need to handle wrapping in x axis, should be able to support numbers larger or smaller than LENGTH bounds
        //Will need to test for wrapping to know what to do
        int y_val = (int)(slope_y * (x_idx - from.x) + 0.5) + from.y; 
        int z_val = (int)(slope_z * (x_idx - from.x) + 0.5) + from.z;
        if (x_idx == from.x)
          setColors(from.x, from.y, from.z, r, g, b);
        else
          setColors(x_idx, y_val, z_val, r, g, b);
        Serial1.print("x_idx: ");
        Serial1.print(x_idx);
        Serial1.print(", y_val: ");
        Serial1.print(y_val);
        Serial1.print(", z_val: ");
        Serial1.println(z_val);
        Serial1.println();
      }
      break;
    }
    case LY:
    {
      Serial1.println("LY");
      if (p0.y > p1.y)
      {
        from = p1;
        to = p0;
      }
      int x_idx = from.x;
      int z_idx = from.z;
      float slope_x = (1.0*del.x)/del.y;
      //if (slope_x < 0)
      //  slope_x *= -1;
      float slope_z = (1.0*del.z)/del.y;
      //if (slope_z < 0)
      //  slope_z *= -1;
        
      for (int y_idx = from.y; y_idx <= to.y; y_idx++)
      {
        //Need to handle wrapping in x axis, should be able to support numbers larger or smaller than LENGTH bounds
        //Will need to test for wrapping to know what to do
        int x_val = (int)(slope_x * (y_idx - from.y) + 0.5) + from.x; 
        int z_val = (int)(slope_z * (y_idx - from.y) + 0.5) + from.z;
        if (y_idx == from.y)
          setColors(from.x, from.y, from.z, r, g, b);
        else
          setColors(x_val, y_idx, z_val, r, g, b);
      }
      break;
    }
    case LZ:
    {
      Serial1.println("LZ");
      if (p0.z > p1.z)
      {
        from = p1;
        to = p0;
      }
      int x_idx = from.x;
      int y_idx = from.y;
      float slope_x = (1.0*del.x)/del.z;
      //if (slope_x < 0)
      //  slope_x *= -1;
      float slope_y = (1.0*del.y)/del.z;
      //if (slope_y < 0)
      //  slope_y *= -1;
        
      for (int z_idx = from.z; z_idx <= to.z; z_idx++)
      {
        //Need to handle wrapping in x axis, should be able to support numbers larger or smaller than LENGTH bounds
        //Will need to test for wrapping to know what to do
        int x_val = (int)(slope_x * (z_idx - from.z) + 0.5) + from.x; 
        int y_val = (int)(slope_y * (z_idx - from.z) + 0.5) + from.y;
        if (z_idx == from.z)
          setColors(from.x, from.y, from.z, r, g, b);
        else
          setColors(x_val, y_val, z_idx, r, g, b);
      }
      break;
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
