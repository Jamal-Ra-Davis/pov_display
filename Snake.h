#ifndef SNAKE_LIB
#define SNAKE_LIB
#include <Arduino.h>
#include "Events.h"
#include "FrameBuffer.h"
#define GAME_LENGTH (LENGTH-5)
#define GAME_WIDTH WIDTH
#define GAME_HEIGHT HEIGHT
#define WRAP false

class SnakeNode
{
  public:
    int8_t x, y, z;
    uint8_t r, g, b;

    SnakeNode();
    SnakeNode(int8_t x_, int8_t y_, int8_t z_, int r_, int g_, int b_);
    static SnakeNode randNode();
    static void randNode(SnakeNode *out);
};
SnakeNode::SnakeNode()
{
  //SnakeNode node = randNode();
  x = rand() % GAME_LENGTH;
  y = rand() % GAME_WIDTH;
  z = rand() % GAME_HEIGHT;
  doubleBuffer::randColor(&r, &g, &b);
}
SnakeNode::SnakeNode(int8_t x_, int8_t y_, int8_t z_, int r_, int g_, int b_)
{
  x = x_;
  y = y_;
  z = z_;
  r = r_;
  g = g_;
  b = b_;
}
SnakeNode SnakeNode::randNode()
{
  SnakeNode out;
  out.x = rand() % GAME_LENGTH;
  out.y = rand() % GAME_WIDTH;
  out.z = rand() % GAME_HEIGHT;
  doubleBuffer::randColor(&(out.r), &(out.g), &(out.b));
  return out;
}
void SnakeNode::randNode(SnakeNode *out)
{
  out->x = rand() % GAME_LENGTH;
  out->y = rand() % GAME_WIDTH;
  out->z = rand() % GAME_HEIGHT;
  //SerialUSB.println(out->x);
  //SerialUSB.println(random(256));
  doubleBuffer::randColor(&(out->r), &(out->g), &(out->b));
}

class Snake
{
  private:
    const static int MAX_LEN = 20;
    int len = 3;
    SnakeNode body[MAX_LEN];
    SnakeNode dot; 
    int dir;
    bool auto_play;
    bool end_game;


  public:
    enum DIRECTION{CW, CCW, IN, OUT, UP, DOWN, NUM_DIR};

    Snake();
    void reset();

    void update();
    void draw(doubleBuffer *frame_buffer);
    bool game_over() {return end_game;}
    int handleOnPress(Event e, int dir_);
    int handleOnRelease(Event e, int dir_);
    
};


Snake::Snake()
{
  //Serial.println("Enter construct");
  //reset();
  //Serial.println("Exit construct");
  auto_play = false;
}
void Snake::reset()
{
  SerialUSB.println("Enter Reset");
  len = 3;
  dir = CW;
  end_game = false;
  
  for (int i=0; i<len; i++)
  {
    body[i].x = (len-1)-i;
    body[i].y = 0;
    body[i].z = 0;
    body[i].r = 255;
    body[i].g = 255;
    body[i].b = 255;
  }
  while(1)
  {
    SnakeNode node;
    SnakeNode::randNode(&node);
    bool pass = true;
    //SerialUSB.println(node.x);
    //SerialUSB.println(node.y);
    //SerialUSB.println(node.z);
    for (int i=0; i<len; i++)
    {
      //Check if rand dot location is at same location as snake body
      if (body[i].x == node.x && body[i].y == node.y && body[i].z == node.z)
      {
        pass = false;
        break;
      }
    }
    if (pass)
    {
      dot = node;
      break;
    }
  }

  SerialUSB.println("Exit Reset");
}

void Snake::update()
{
  SerialUSB.println("Enter Update");
  SnakeNode next = body[0];


  /*
  if (button_states[0] == ON_PRESS || button_states[0] == ON_RELEASE)
  {
    //Turn left
    dir--;
    if (dir < 0)
      dir = NUM_DIR - 1;
  }
  else if (button_states[1] == ON_PRESS || button_states[1] == ON_RELEASE)
  {
    //Turn right
    dir++;
    if (dir >= NUM_DIR)
      dir = 0;
  }
  resetButtonStates();
  */
  Event::SerialParser();
  
  int dir_ = dir;
  while (!eventBuffer.isEmpty())
  {
    Event e;
    if (!eventBuffer.lockedPop(e))
    {
      //Handle some error condition
    }
    
    switch (e.type)
    {
      case Event::ON_PRESS:
      {
        SerialUSB.println("Button press");
        handleOnPress(e, dir_);
        break;
      }
      case Event::ON_RELEASE:
      {
        SerialUSB.println("Button release");
        handleOnRelease(e, dir_);
        break;
      }
    }
  }

  if (auto_play)
  {
    switch(dir)
    {
      case CW:
      {
        if (body[0].x == dot.x)
        {
          //Move up/down or in/out
          if (body[0].y < dot.y)
          {
            dir = OUT;
          }
          else if (body[0].y > dot.y)
          {
            dir = IN;
          }
          else if (body[0].z > dot.z)
          {
            dir = UP;
          }
          else if(body[0].z > dot.z)
          {
            dir = DOWN;
          }
        }
        break;
      }
      case CCW:
      {
        next.x--;
        break;
      }
      case IN:
      {
        next.y--;
        break;
      }
      case OUT:
      {
        next.y++;
        break;
      }
      case UP:
      {
        next.z++;
        break;
      }
      case DOWN:
      {
        next.z--;
        break;
      }
    }
  }
  
  switch(dir)
  {
    case CW:
    {
      next.x++;
      break;
    }
    case CCW:
    {
      next.x--;
      break;
    }
    case IN:
    {
      next.y--;
      break;
    }
    case OUT:
    {
      next.y++;
      break;
    }
    case UP:
    {
      next.z++;
      break;
    }
    case DOWN:
    {
      next.z--;
      break;
    }
  }

  //Check for game edge collision
  if ((next.x >= GAME_LENGTH || next.y >= GAME_WIDTH || next.z >= GAME_HEIGHT || next.x < 0 || next.y <0 || next.z < 0) && !WRAP)
  {
    end_game = true;
    return;
  }

  //Update snake body
  for (int i=len-1; i>=1; i--)
  {
    body[i].x = body[i-1].x;
    body[i].y = body[i-1].y;
    body[i].z = body[i-1].z;
  }
  body[0] = next;

  for (int i=1; i<len; i++)
  {
    if (next.x == body[i].x && next.y == body[i].y && next.z == body[i].z)
    {
      end_game = true;
      return;
    }
  }

  //Check for dot collision
  if (body[0].x == dot.x && body[0].y == dot.y && body[0].z == dot.z)
  {
    if (len < MAX_LEN)
    {
      body[len] = dot;
      body[len].x = body[len-1].x;
      body[len].y = body[len-1].y;
      body[len].z = body[len-1].z;
      len++;
    }
    while(1)
    {
      SnakeNode dot_ = SnakeNode::randNode();
      bool pass = true;
      for (int i=0; i<len; i++)
      {
        if (dot_.x == body[i].x && dot_.y == body[i].y && dot_.z == body[i].z)
        {
          pass = false;
          break;
        }
      }
      if (pass)
      {
        dot = dot_;
        break;
      }
    }
  }


  

  SerialUSB.println("Exit update");
}

void Snake::draw(doubleBuffer *frame_buffer)
{
  //Serial.println("Enter draw");
  frame_buffer->clear();
  for (int i=0; i<len; i++)
  {
    
    frame_buffer->setColors(body[i].x, body[i].y, body[i].z, body[i].r, body[i].g, body[i].b);   
  }
  frame_buffer->setColors(dot.x, dot.y, dot.z, dot.r, dot.g, dot.b);
  
  for (int i=GAME_LENGTH; i<LENGTH; i++)
  {
    for (int j=0; i<WIDTH; j++)
    {
      for (int k=0; k<HEIGHT; k++)
      {
        frame_buffer->setColors(i, j, k, 255, 0, 0);
      }
    }
  }
  frame_buffer->update();
  //Serial.println("Exit Draw");
}

int Snake::handleOnPress(Event e, int dir_)
{
  switch(e.button_idx)
  {
    case 1://left (CCW)
    {
      if (dir_ == UP || dir == DOWN || dir_ == IN || dir_ == OUT)
      {
        dir = CCW;
      }
      else if(dir_ == CCW)
      {
        dir = IN;
      }
      else if (dir_ == CW)
      {
        dir = OUT;
      }
      break;
    }
    case 2://fire(toggle auto_play)
    {
      if (!auto_play)
        auto_play = true;
      else
        auto_play = false;
      break;
    }
    case 3://right
    {
      if (dir_ == UP || dir == DOWN || dir_ == IN || dir_ == OUT)
      {
        dir = CW;
      }
      else if(dir_ == CCW)
      {
        dir = OUT;
      }
      else if (dir_ == CW)
      {
        dir = IN;
      }
      break;
    }
    case 4://up
    {
      if (dir_ == CW || dir == CCW || dir_ == IN || dir_ == OUT)
      {
        dir = UP;
      }
      else if(dir_ == UP)
      {
        dir = CW;
      }
      else if (dir_ == DOWN)
      {
        dir = CCW;
      }
      
      break;
    }
    case 5://down
    {
      if (dir_ == CW || dir == CCW || dir_ == IN || dir_ == OUT)
      {
        dir = DOWN;
      }
      else if(dir_ == UP)
      {
        dir = CCW;
      }
      else if (dir_ == DOWN)
      {
        dir = CW;
      }
      break;
    }
  }

  /*
  if (e.button_idx == 0)
  {
    dir = dir_-1;
    if (dir < 0)
      dir = NUM_DIR - 1;
  }
  else if (e.button_idx == 1)
  {
    dir = dir_ + 1;
    if (dir >= NUM_DIR)
      dir = 0;
  }
  */
  return 0;
}
int Snake::handleOnRelease(Event e, int dir_)
{
  return 0;
}

#endif
