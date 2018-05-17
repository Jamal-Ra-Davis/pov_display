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

#endif
