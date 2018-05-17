#ifndef TEXT_LIB
#define TEXT_LIB

//#include <stdlib.h>
#include "FrameBuffer.h"
#define CHAR_WIDTH 8

enum CHAR_TYPE {LETTER, NUMBER};

const static uint8_t NUM_BUF[10][CHAR_WIDTH] = {
                            {0x00, 0x00, 0x3C, 0x42, 0x42, 0x3C, 0x00, 0x00},//0
                            {0x00, 0x00, 0x00, 0x22, 0x7E, 0x02, 0x00, 0x00},//1
                            {0x00, 0x00, 0x26, 0x4A, 0xA2, 0x22, 0x00, 0x00},//2
                            {0x00, 0x00, 0x42, 0x52, 0x52, 0x2C, 0x00, 0x00},//3
                            {0x00, 0x00, 0x78, 0x08, 0x7E, 0x08, 0x00, 0x00},//4
                            {0x00, 0x00, 0x72, 0x52, 0x52, 0x4C, 0x00, 0x00},//5
                            {0x00, 0x00, 0x3C, 0x52, 0x52, 0x4C, 0x00, 0x00},//6
                            {0x00, 0x00, 0x40, 0x4E, 0x50, 0x60, 0x00, 0x00},//7
                            {0x00, 0x00, 0x2C, 0x52, 0x52, 0x2C, 0x00, 0x00},//8
                            {0x00, 0x00, 0x30, 0x48, 0x48, 0x3E, 0x00, 0x00} //9
                        };

const static uint8_t CHAR_BUF[26][CHAR_WIDTH] =  {
                      {0x00,  0x3F, 0x48, 0x48, 0x48, 0x3F, 0x00, 0x00},  //A
                      {0x00,  0x7F, 0x49, 0x49, 0x49, 0x36, 0x00, 0x00},  //B
                      {0x00,  0x3E, 0x41, 0x41, 0x41, 0x22, 0x00, 0x00},  //C
                      {0x00,  0x7F, 0x41, 0x41, 0x41, 0x3E, 0x00, 0x00},  //D
                      {0x00,  0x7F, 0x49, 0x49, 0x49, 0x49, 0x00, 0x00},  //E
                      {0x00,  0x7F, 0x48, 0x48, 0x48, 0x48, 0x00, 0x00},  //F
                      {0x00,  0x3E, 0x41, 0x49, 0x49, 0x2E, 0x00, 0x00},  //G
                      {0x00,  0x7F, 0x08, 0x08, 0x08, 0x7F, 0x00, 0x00},  //H
                      {0x00,  0x41, 0x41, 0x7F, 0x41, 0x41, 0x00, 0x00},  //I
                      {0x00,  0x06, 0x01, 0x01, 0x01, 0x7E, 0x00, 0x00},  //J
                      {0x00,  0x7F, 0x08, 0x08, 0x14, 0x63, 0x00, 0x00},  //K
                      {0x00,  0x7F, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00},  //L
                      {0x00,  0x7F, 0x20, 0x10, 0x20, 0x7F, 0x00, 0x00},  //M
                      {0x00,  0x7F, 0x30, 0x08, 0x06, 0x7F, 0x00, 0x00},  //N
                      {0x00,  0x3E, 0x41, 0x41, 0x41, 0x3E, 0x00, 0x00},  //O
                      {0x00,  0x7F, 0x48, 0x48, 0x48, 0x30, 0x00, 0x00},  //P
                      {0x00,  0x3C, 0x42, 0x47, 0x42, 0x3C, 0x00, 0x00},  //Q
                      {0x00,  0x7F, 0x48, 0x48, 0x48, 0x37, 0x00, 0x00},  //R
                      {0x00,  0x32, 0x49, 0x49, 0x49, 0x26, 0x00, 0x00},  //S
                      {0x00,  0x40, 0x40, 0x7F, 0x40, 0x40, 0x00, 0x00},  //T
                      {0x00,  0x7E, 0x01, 0x01, 0x01, 0x7E, 0x00, 0x00},  //U
                      {0x00,  0x78, 0x06, 0x01, 0x06, 0x78, 0x00, 0x00},  //V
                      {0x00,  0x7F, 0x02, 0x04, 0x02, 0x7F, 0x00, 0x00},  //W
                      {0x00,  0x63, 0x14, 0x08, 0x14, 0x63, 0x00, 0x00},  //X
                      {0x00,  0x70, 0x08, 0x07, 0x08, 0x70, 0x00, 0x00},  //Y
                      {0x00,  0x43, 0x45, 0x49, 0x51, 0x61, 0x00, 0x00} //Z
                    };

int resolveChar(char c)
{
  if (c >= 65 && c <= 90)
    return LETTER;
  if (c >= 48 && c <= 57)
    return NUMBER;
  return -1;
}

void writeString(char *str, int offset, int layer, uint8_t r, uint8_t g, uint8_t b)// int bound_h=LENGTH, int bound_l=0
{
  if (layer < 0 || layer >= HEIGHT)
    return;

  
  int LEN = strlen(str);
  if (LEN <= 0)
    return;

  
  int pos;
  for (int c=0; c<LEN; c++)
  {
    char c_ = str[c];
    int type = resolveChar(c_);
    if (type < 0)
      continue;

    for (int i=0; i<CHAR_WIDTH; i++)
    {
      pos = 8*c + i + offset;
      if (pos < 0)
        continue;
      if (pos >= LENGTH)
        return;

      //fbuf[pos][4][2][RED] = 255;
      //continue;

      uint8_t char_slice;
      if (type == LETTER)
        char_slice = CHAR_BUF[c_-65][i];
      else if (type == NUMBER)
        char_slice = NUM_BUF[c_-48][i];
      else
        continue;

        
      for (int j=0; j<8; j++)
      {
        if ((1 << j) & (char_slice))
        {
          fbuf[pos][j][layer][RED] = r;
          fbuf[pos][j][layer][GREEN] = g;
          fbuf[pos][j][layer][BLUE] = b;
        }
      }
    }
  }
}

#endif
