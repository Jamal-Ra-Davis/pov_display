#ifndef SPACE_GAME_LIB
#define SPACE_GAME_LIB

#include "Vector3d.h"
#include "FrameBuffer.h"
#include "Events.h"
#include "Shell.h"

class Bullet
{
    private:
        Vector3d pos;
        Vector3d prev_pos;
        int vel;
        int lifetime;
        static constexpr uint8_t color[3] = {60, 255, 0};
        
    public:
        static const uint8_t LIFE_TIME = 25;
        
        Bullet();
        Bullet(Vector3d pos_, int vel_, int lt);
        void setBullet(Vector3d pos_, int vel_, int lt);
        void reset();

        void update();
        void draw(doubleBuffer *frame_buffer);

        Vector3d getPos() {return pos;}
        int getVel() {return vel;}
        int getLifeTime() {return lifetime;}
        void setLifeTime(int lt) {lifetime = lt;}
        
};
Bullet::Bullet()
{
  lifetime = -1;
}
void Bullet::reset()
{
  lifetime = -1;
}
Bullet::Bullet(Vector3d pos_, int vel_, int lt)
{
  setBullet(pos_, vel_, lt);
}
void Bullet::setBullet(Vector3d pos_, int vel_, int lt)
{
  pos = pos_;
  prev_pos = pos;
  vel = vel_;
  lifetime = lt;
}
void Bullet::update()
{
  if (lifetime < 0)
    return;

  pos.x += vel;
  pos.y--;
  if (pos.x >= LENGTH)
  {
    pos.x %= LENGTH;
  }
  if (pos.x < 0)
  {
    pos.x += LENGTH;
  }

  if (pos.y < 0)
  {
    lifetime = 0;
  }
  
  lifetime--;
}
void Bullet::draw(doubleBuffer *frame_buffer)
{
  if (lifetime < 0)
    return;
  frame_buffer->setColors(pos.x, pos.y, pos.z, color[RED], color[GREEN], color[BLUE]);
}


class Ship
{
    private:
        Vector3d pos, vel;
        bool fire, boost, left, right, up, down;
        int cool_down;
        

        static constexpr uint8_t color[3] = {255, 255, 255};
        static constexpr float BOOST_MULT = 1.5f;
        static const int COOL_DOWN_TIME = 3;
        static const int NUM_BULLETS = 10;

        Bullet bullets[NUM_BULLETS];
        
    public:
        enum BUTTONS{DUMMY, LEFT=DLEFT, FIRE=SQUARE, RIGHT=DRIGHT, UP=LBUMP, DOWN=RBUMP};
        Ship();
        Ship(Vector3d pos_, Vector3d vel_);
        void reset(Vector3d pos_, Vector3d vel_);
        void update();
        void draw(doubleBuffer *frame_buffer);
        void getSerialData();
        void handleEvent(Event e);
        bool checkBlockCollision(Vector3d *block);
        
        void setFire(bool b) {fire = b;}
        void setBoost(bool b) {boost = b;}
        void setLeft(bool b) {left = b;}
        void setRight(bool b) {right = b;}
        void setUp(bool b) {up = b;}
        void setDown(bool b) {down = b;}
};
Ship::Ship()
{
    fire = false;
    boost = false;
    left = false;
    right = false;
    up = false;
    down = false;

    for (int i=0; i<NUM_BULLETS; i++) 
    {
      bullets[i].reset();
    }
}
Ship::Ship(Vector3d pos_, Vector3d vel_)
{
    reset(pos_, vel_);
}
void Ship::reset(Vector3d pos_, Vector3d vel_)
{
    pos = pos_;
    vel = vel_;
    
    fire = false;
    boost = false;
    left = false;
    right = false;
    up = false;
    down = false;
    cool_down = 0;

    for (int i=0; i<NUM_BULLETS; i++) 
    {
      bullets[i].reset();
    }
}
void Ship::update()
{
    if (left)
    {
        pos.y--;
        if (pos.y < 0)
        {
            pos.y = 0;
        }
    }
    else if (right)
    {
        pos.y++;
        if (pos.y >= WIDTH)
        {
            pos.y = WIDTH-1;
        }
    }
    
    if (up)
    {
        pos.z++;
        if (pos.z >= HEIGHT)
        {
            pos.z = HEIGHT-1;
        }
    }
    else if(down)
    {
        pos.z--;
        if (pos.z < 0)
        {
            pos.z = 0;
        }
    }
    

    //Update ship position
    Vector3d boost_vel = vel;
    if (boost)
    {
        boost_vel.x *= BOOST_MULT;
    }
    pos.addVector3d(boost_vel);
    if (pos.x >= LENGTH)
    {
        pos.x %= LENGTH;
    }
    if (pos.x < 0)
    {
        pos.x += LENGTH;
    }
    
    
    //Handle firing
    if (fire /*&& cool_down < 0*/)
    {
        cool_down = COOL_DOWN_TIME;
        int min_lifetime = bullets[0].getLifeTime();
        int min_idx = 0;
        for (int i=1; i<NUM_BULLETS; i++)
        {
          if (bullets[i].getLifeTime() < 0)
          {
            min_idx = i;
            break;
          }
          if (bullets[i].getLifeTime() < bullets[min_idx].getLifeTime())
          {
            min_idx = i;
          }
        }


        int bullet_vel = 2;
        if (vel.x < 0)
          bullet_vel = -2;
        bullets[min_idx].setBullet(pos, bullet_vel, Bullet::LIFE_TIME);
        /*
        Vector3d pos_ = pos;
        if (vel.x > 0)
        {
          pos_.x++;
          if (pos_.x >= LENGTH)
          {
            pos_.x = 0;
          }
          bullets[min_idx].setBullet(pos_, 2, Bullet::LIFE_TIME);
        }
        else
        {
          pos_.x--;
          if (pos_.x < 0)
          {
            pos_.x = LENGTH-1;
          }
          bullets[min_idx].setBullet(pos_, -2, Bullet::LIFE_TIME);
        }
        */
    }
    if (cool_down >= 0)
        cool_down--;

    for (int i=0; i<NUM_BULLETS; i++)
    {
      bullets[i].update();
    }

    
    
    
    //Clear flags
    //up = false;
    //left = false;
    //down = false;
    //right = false;
    fire = false;
    
    //check for collisions
    
    
}
void Ship::draw(doubleBuffer *frame_buffer)
{
  for (int i=0; i<NUM_BULLETS; i++)
  {
    bullets[i].draw(frame_buffer);
  }
  
  if (vel.x > 0)
  {
    for (int i=0; i<3; i++)
    {
      int x_idx = pos.x - i;
      if (x_idx < 0)
      {
        x_idx = LENGTH + x_idx;
      }
      if (i==2)
        frame_buffer->setColors(x_idx, pos.y, pos.z, (rand()%128)+128, (rand()%196)+64, 0);
      else
        frame_buffer->setColors(x_idx, pos.y, pos.z, color[RED], color[GREEN], color[BLUE]);
    }
  }
  else
  {
    for (int i=0; i<4; i++)
    {
      int x_idx = pos.x + i;
      if (x_idx >= LENGTH)
      {
        x_idx %= LENGTH;
      }
      if (i==3)
        frame_buffer->setColors(x_idx, pos.y, pos.z, (rand()%128)+128, (rand()%196)+64, 0);
      else
        frame_buffer->setColors(x_idx, pos.y, pos.z, color[RED], color[GREEN], color[BLUE]);
    }
  }
}
void Ship::handleEvent(Event e)
{    
  switch (e.type)
  {
    case Event::ON_PRESS:
    {
      switch(e.data.button_idx)
      {
        case LEFT:
        {
          setLeft(true);
          break;
        }
        case FIRE:
        {
          setFire(true);
          break;
        }
        case RIGHT:
        {
          setRight(true);
          break;
        }
        case UP:
        {
          setUp(true);
          break;
        }
        case DOWN:
        {
          setDown(true);
          break;
        }
      }
      break;
    }
    case Event::ON_RELEASE:
    {
      switch(e.data.button_idx)
      {
        case LEFT:
        {
          setLeft(false);
          break;
        }
        case FIRE:
        {
          break;
        }
        case RIGHT:
        {
          setRight(false);
          break;
        }
        case UP:
        {
          setUp(false);
          break;
        }
        case DOWN:
        {
          setDown(false);
          break;
        }
        case DX:
        {
          setRight(false);
          setLeft(false);
          break;
        }
      }
      break;
    }
    case Event::TAP:
    {
      switch(e.data.button_idx)
      {
        case LEFT:
        {
          break;
        }
        case FIRE:
        {
          setFire(true);
          break;
        }
        case RIGHT:
        {
          break;
        }
        case UP:
        {
          break;
        }
        case DOWN:
        {
          break;
        }
      }
      break;
    }
  }
}
void Ship::getSerialData()
{
  while (!eventBuffer.isEmpty())
  {
    Event e;
    if (!eventBuffer.pop(e))
    {
      //Handle some error condition
    }
    
    switch (e.type)
    {
      case Event::ON_PRESS:
      {
        switch(e.data.button_idx)
        {
          case LEFT:
          {
            setLeft(true);
            break;
          }
          case FIRE:
          {
            setFire(true);
            break;
          }
          case RIGHT:
          {
            setRight(true);
            break;
          }
          case UP:
          {
            setUp(true);
            break;
          }
          case DOWN:
          {
            setDown(true);
            break;
          }
        }
        break;
      }
      case Event::ON_RELEASE:
      {
        switch(e.data.button_idx)
        {
          case LEFT:
          {
            setLeft(false);
            break;
          }
          case FIRE:
          {
            break;
          }
          case RIGHT:
          {
            setRight(false);
            break;
          }
          case UP:
          {
            setUp(false);
            break;
          }
          case DOWN:
          {
            setDown(false);
            break;
          }
        }
        break;
      }
    }
  }
}

bool Ship::checkBlockCollision(Vector3d *block)
{
  bool collide = false;

  for (int i=0; i<NUM_BULLETS; i++)
  {
    if (bullets[i].getLifeTime() < 0)
      continue;
    Vector3d bullet_pos = bullets[i].getPos();
    if (  (bullet_pos.x >= block[0].x && bullet_pos.x <= block[1].x)
        &&(bullet_pos.y >= block[0].y && bullet_pos.y <= block[1].y)
        &&(bullet_pos.z >= block[0].z && bullet_pos.z <= block[1].z))
    {
      collide = true;
      bullets[i].setLifeTime(-1);
    }
  }
  
  return collide;
}

/*
enum FACE_STATES {NEUTRAL_0, ANGRY_1, SAD_2, FLAT_3, NEUTAL_BROW_4, RAISED_BROW_5, OPEN_MOUTH_6, NUM_FACES};
const static uint8_t FACE_BUF[NUM_FACES][6] = {
                              {0x00, 0x24, 0x24, 0x00, 0x3C, 0x00},//NEUTRAL_0
                              {0x00, 0x7E, 0x24, 0x00, 0x3C, 0x42},//ANGRY_1
                              {0x24, 0x00, 0x3C, 0x66, 0x42, 0x00},//SAD_2
                              {0x00, 0x00, 0x66, 0x00, 0x3C, 0x00},//FLAT_3
                              {0x00, 0x66, 0x24, 0x00, 0x3C, 0x00},//NEUTAL_BROW_4
                              {0x60, 0x26, 0x24, 0x00, 0x0C, 0x00},//RAISED_BROW_5
                              {0x66, 0x00, 0x3C, 0x7E, 0x7E, 0x7E},//OPEN_MOUTH_6
                            };
*/
struct sprite {
  uint8_t data[6];
};
//struct sprite test = {
//  .data = {0x0,0x24,0x24,0x0,0x3C,0x0},
//};
#define NUM_FACES 18                            
const static uint8_t FACE_BUF[18][6] = {
                              {0x0,0x24,0x24,0x0,0x3C,0x0},
                              {0x0,0x7E,0x24,0x0,0x3C,0x42},
                              {0x24,0x0,0x3C,0x66,0x42,0x0},
                              {0x60,0x26,0x24,0x0,0xC,0x0},
                              {0x81,0x81,0x0,0x0,0x0,0x7E},
                              {0x0,0x0,0x66,0x0,0x3C,0x0},
                              {0x24,0x24,0x0,0x3C,0x3C,0x3C},
                              {0x24,0x0,0x7E,0x0,0x0,0x0},
                              {0x0,0x0,0xE7,0x0,0x7E,0x0},
                              {0x0,0x66,0x24,0x0,0x3C,0x0},
                              {0x66,0x0,0x3C,0x7E,0x7E,0x7E},
                              {0x0,0x0,0x0,0x24,0x0,0x7E},
                              {0xA0,0xA0,0xA0,0x0,0x0,0xC0},
                              {0x0,0x66,0x24,0x0,0x60,0x60},
                              {0x0,0x66,0x24,0x0,0x10,0x0},
                              {0x0,0x66,0x24,0x0,0x8,0x0},
                              {0x0,0x66,0x24,0x0,0x4,0x0},
                              {0x0,0x66,0x24,0x0,0x2,0x0},
};
const static uint8_t RAW_SPRITE[18*6] = {
                              0x0,0x24,0x24,0x0,0x3C,0x0,
                              0x0,0x7E,0x24,0x0,0x3C,0x42,
                              0x24,0x0,0x3C,0x66,0x42,0x0,
                              0x60,0x26,0x24,0x0,0xC,0x0,
                              0x81,0x81,0x0,0x0,0x0,0x7E,
                              0x0,0x0,0x66,0x0,0x3C,0x0,
                              0x24,0x24,0x0,0x3C,0x3C,0x3C,
                              0x24,0x0,0x7E,0x0,0x0,0x0,
                              0x0,0x0,0xE7,0x0,0x7E,0x0,
                              0x0,0x66,0x24,0x0,0x3C,0x0,
                              0x66,0x0,0x3C,0x7E,0x7E,0x7E,
                              0x0,0x0,0x0,0x24,0x0,0x7E,
                              0xA0,0xA0,0xA0,0x0,0x0,0xC0,
                              0x0,0x66,0x24,0x0,0x60,0x60,//13
                              0x0,0x66,0x24,0x0,0x10,0x0,//14
                              0x0,0x66,0x24,0x0,0x8,0x0,
                              0x0,0x66,0x24,0x0,0x4,0x0,
                              0x0,0x66,0x24,0x0,0x2,0x0,
};

const static uint8_t BANANA_SPRITE[64*3] = {
  0,0,0,  0,0,0,  0,0,0,  0,0,0,  209,106,58,   126,37,83,    0,0,0,        0,0,0,
  0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,0,0,        209,106,58,   126,37,83,    0,0,0,
  0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,0,0,        255,236,39,   255,163,0,    0,0,0,
  0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,0,0,        255,236,39,   255,236,39,   255,163,0,
  0,0,0,  0,0,0,  0,0,0,  0,0,0,  255,236,39,   255,236,39,   255,236,39,   255,163,0,
  0,0,0,  0,0,0,  255,236,39,  255,236,39,  255,236,39,   255,236,39,   255,163,0,   255,163,0,
  255,163,0,  255,236,39,  255,236,39,  255,236,39,  255,236,39,  255,163,0, 255,163,0,  0,0,0,
  0,0,0,  255,163,0,  255,163,0,  255,163,0,  255,163,0,  255,163,0,  0,0,0,  0,0,0,  
};
static const uint8_t blue_potion_arr[3*8*8] = {
  0,0,0,  0,0,0,  255,241,232,  255,241,232,  255,241,232,  255,241,232,  0,0,0,  0,0,0,
  0,0,0,  0,0,0,  0,0,0,  194,195,199,  194,195,199,  0,0,0,  0,0,0,  0,0,0,
  0,0,0,  0,0,0,  0,0,0,  255,241,232,  255,241,232,  0,0,0,  0,0,0,  0,0,0,
  0,0,0,  0,0,0,  255,241,232,  131,118,156,  131,118,156,  255,241,232,  0,0,0,  0,0,0,
  0,0,0,  255,241,232,  41,173,255,  41,173,255,  41,173,255,  41,173,255,  255,241,232,  0,0,0,  
  0,0,0,  255,241,232,  41,173,255,  41,173,255,  41,173,255,  41,173,255,  255,241,232,  0,0,0,  
  0,0,0,  194,195,199,  41,173,255,  41,173,255,  41,173,255,  41,173,255,  194,195,199,  0,0,0,  
  0,0,0,  0,0,0,  194,195,199,  194,195,199,  194,195,199,  194,195,199,  0,0,0,  0,0,0,
};
static const uint8_t green_potion_arr[3*8*8] = {
  0,0,0,  0,0,0,  255,241,232,  255,241,232,  255,241,232,  255,241,232,  0,0,0,  0,0,0,
  0,0,0,  0,0,0,  0,0,0,  194,195,199,  194,195,199,  0,0,0,  0,0,0,  0,0,0,
  0,0,0,  0,0,0,  0,0,0,  255,241,232,  255,241,232,  0,0,0,  0,0,0,  0,0,0,
  0,0,0,  0,0,0,  255,241,232,  131,118,156,  131,118,156,  255,241,232,  0,0,0,  0,0,0,
  0,0,0,  255,241,232,  0,228,54,  0,228,54,  0,228,54,  0,228,54,  255,241,232,  0,0,0,
  0,0,0,  255,241,232,  0,228,54,  0,228,54,  0,228,54,  0,228,54,  255,241,232,  0,0,0,
  0,0,0,  194,195,199,  0,228,54,  0,228,54,  0,228,54,  0,228,54,  194,195,199,  0,0,0,
  0,0,0,  0,0,0,  194,195,199,  194,195,199,  194,195,199,  194,195,199,  0,0,0,  0,0,0,
};
static const uint8_t orange_potion_arr[3*8*8] = {
  0,0,0,  0,0,0,  255,241,232,  255,241,232,  255,241,232,  255,241,232,  0,0,0,  0,0,0,
  0,0,0,  0,0,0,  0,0,0,  194,195,199,  194,195,199,  0,0,0,  0,0,0,  0,0,0,
  0,0,0,  0,0,0,  0,0,0,  255,241,232,  255,241,232,  0,0,0,  0,0,0,  0,0,0,
  0,0,0,  0,0,0,  255,241,232,  131,118,156,  131,118,156,  255,241,232,  0,0,0,  0,0,0,
  0,0,0,  255,241,232,  255,163,0,  255,163,0,  255,163,0,  255,163,0,  255,241,232,  0,0,0,      
  0,0,0,  255,241,232,  255,163,0,  255,163,0,  255,163,0,  255,163,0,  255,241,232,  0,0,0,      
  0,0,0,  194,195,199,  255,163,0,  255,163,0,  255,163,0,  255,163,0,  194,195,199,  0,0,0,      
  0,0,0,  0,0,0,  194,195,199,  194,195,199,  194,195,199,  194,195,199,  0,0,0,  0,0,0,
};
static const uint8_t red_potion_arr[3*8*8] = {
  0,0,0,  0,0,0,  255,241,232,  255,241,232,  255,241,232,  255,241,232,  0,0,0,  0,0,0,
  0,0,0,  0,0,0,  0,0,0,  194,195,199,  194,195,199,  0,0,0,  0,0,0,  0,0,0,
  0,0,0,  0,0,0,  0,0,0,  255,241,232,  255,241,232,  0,0,0,  0,0,0,  0,0,0,
  0,0,0,  0,0,0,  255,241,232,  131,118,156,  131,118,156,  255,241,232,  0,0,0,  0,0,0,
  0,0,0,  255,241,232,  255,0,77,  255,0,77,  255,0,77,  255,0,77,  255,241,232,  0,0,0,
  0,0,0,  255,241,232,  255,0,77,  255,0,77,  255,0,77,  255,0,77,  255,241,232,  0,0,0,
  0,0,0,  194,195,199,  255,0,77,  255,0,77,  255,0,77,  255,0,77,  194,195,199,  0,0,0,
  0,0,0,  0,0,0,  194,195,199,  194,195,199,  194,195,199,  194,195,199,  0,0,0,  0,0,0,
};
static const uint8_t blue_gem_arr[3*8*8] = {
  0,0,0,  0,0,0,  0,0,0,  255,241,232,  41,173,255,  0,0,0,  0,0,0,  0,0,0,
  0,0,0,  0,0,0,  255,241,232,  255,241,232,  41,173,255,  41,173,255,  0,0,0,  0,0,0,
  0,0,0,  255,241,232,  255,241,232,  255,241,232,  41,173,255,  41,173,255,  41,173,255,  0,0,0,
  255,241,232,  255,241,232,  255,241,232,  255,241,232,  41,173,255,  41,173,255,  41,173,255,  41,173,255,
  41,173,255,  41,173,255,  41,173,255,  41,173,255,  131,118,156,  131,118,156,  131,118,156,  131,118,156,
  0,0,0,  41,173,255,  41,173,255,  41,173,255,  131,118,156,  131,118,156,  131,118,156,  0,0,0,
  0,0,0,  0,0,0,  41,173,255,  41,173,255,  131,118,156,  131,118,156,  0,0,0,  0,0,0,
  0,0,0,  0,0,0,  0,0,0,  41,173,255,  131,118,156,  0,0,0,  0,0,0,  0,0,0,
};
static const uint8_t green_gem_arr[3*8*8] = {
  0,0,0,  0,0,0,  0,0,0,  255,241,232,  0,228,54,  0,0,0,  0,0,0,  0,0,0,
  0,0,0,  0,0,0,  255,241,232,  255,241,232,  0,228,54,  0,228,54,  0,0,0,  0,0,0,
  0,0,0,  255,241,232,  255,241,232,  255,241,232,  0,228,54,  0,228,54,  0,228,54,  0,0,0,
  255,241,232,  255,241,232,  255,241,232,  255,241,232,  0,228,54,  0,228,54,  0,228,54,  0,228,54,
  0,228,54,  0,228,54,  0,228,54,  0,228,54,  0,135,81,  0,135,81,  0,135,81,  0,135,81,
  0,0,0,  0,228,54,  0,228,54,  0,228,54,  0,135,81,  0,135,81,  0,135,81,  0,0,0,
  0,0,0,  0,0,0,  0,228,54,  0,228,54,  0,135,81,  0,135,81,  0,0,0,  0,0,0,
  0,0,0,  0,0,0,  0,0,0,  0,228,54,  0,135,81,  0,0,0,  0,0,0,  0,0,0,
};
static const uint8_t orange_gem_arr[3*8*8] = {
  0,0,0,  0,0,0,  0,0,0,  255,241,232,  255,163,0,  0,0,0,  0,0,0,  0,0,0,
  0,0,0,  0,0,0,  255,241,232,  255,241,232,  255,163,0,  255,163,0,  0,0,0,  0,0,0,  
  0,0,0,  255,241,232,  255,241,232,  255,241,232,  255,163,0,  255,163,0,  255,163,0,  0,0,0,
  255,241,232,  255,241,232,  255,241,232,  255,241,232,  255,163,0,  255,163,0,  255,163,0,  255,163,0,
  255,163,0,  255,163,0,  255,163,0,  255,163,0,  209,106,58,  209,106,58,  209,106,58,  209,106,58,
  0,0,0,  255,163,0,  255,163,0,  255,163,0,  209,106,58,  209,106,58,  209,106,58,  0,0,0,
  0,0,0,  0,0,0,  255,163,0,  255,163,0,  209,106,58,  209,106,58,  0,0,0,  0,0,0,
  0,0,0,  0,0,0,  0,0,0,  255,163,0,  209,106,58,  0,0,0,  0,0,0,  0,0,0,
};
static const uint8_t red_gem_arr[3*8*8] = {
  0,0,0,  0,0,0,  0,0,0,  255,241,232,  255,0,77,  0,0,0,  0,0,0,  0,0,0,
  0,0,0,  0,0,0,  255,241,232,  255,241,232,  255,0,77,  255,0,77,  0,0,0,  0,0,0,
  0,0,0,  255,241,232,  255,241,232,  255,241,232,  255,0,77,  255,0,77,  255,0,77,  0,0,0,
  255,241,232,  255,241,232,  255,241,232,  255,241,232,  255,0,77,  255,0,77,  255,0,77,  255,0,77,
  255,0,77,  255,0,77,  255,0,77,  255,0,77,  126,37,83,  126,37,83,  126,37,83,  126,37,83,
  0,0,0,  255,0,77,  255,0,77,  255,0,77,  126,37,83,  126,37,83,  126,37,83,  0,0,0,
  0,0,0,  0,0,0,  255,0,77,  255,0,77,  126,37,83,  126,37,83,  0,0,0,  0,0,0,
  0,0,0,  0,0,0,  0,0,0,  255,0,77,  126,37,83,  0,0,0,  0,0,0,  0,0,0,
};
static const uint8_t money_bag_arr[3*8*8] = {
  0,0,0,  0,0,0,  209,106,58,  209,106,58,  209,106,58,  126,37,83,  0,0,0,  0,0,0,
  0,0,0,  0,0,0,  0,0,0,  255,163,0,  255,163,0,  0,0,0,  0,0,0,  0,0,0,
  0,0,0,  0,0,0,  255,163,0,  209,106,58,  209,106,58,  126,37,83,  0,0,0,  0,0,0,
  0,0,0,  209,106,58,  255,163,0,  209,106,58,  209,106,58,  209,106,58,  126,37,83,  0,0,0,
  209,106,58,  209,106,58,  209,106,58,  255,163,0,  209,106,58,  209,106,58,  126,37,83,  126,37,83,
  209,106,58,  209,106,58,  209,106,58,  209,106,58,  209,106,58,  126,37,83,  126,37,83,  126,37,83,
  209,106,58,  209,106,58,  209,106,58,  209,106,58,  126,37,83,  126,37,83,  126,37,83,  126,37,83,
  0,0,0,  126,37,83,  126,37,83,  126,37,83,  126,37,83,  126,37,83,  126,37,83,  0,0,0,
};
static const uint8_t coin_arr[3*8*8] = {
  0,0,0,  0,0,0,  255,236,39,  255,236,39,  255,236,39,  209,106,58,  0,0,0,  0,0,0,
  0,0,0,  255,236,39,  209,106,58,  255,163,0,  255,163,0,  255,236,39,  209,106,58,  0,0,0,
  255,236,39,  209,106,58,  255,163,0,  255,236,39,  209,106,58,  255,163,0,  255,236,39,  209,106,58,
  255,236,39,  209,106,58,  255,236,39,  255,236,39,  255,236,39,  209,106,58,  255,236,39,  209,106,58,
  255,236,39,  209,106,58,  255,236,39,  255,236,39,  255,236,39,  209,106,58,  255,236,39,  209,106,58,
  255,236,39,  209,106,58,  255,163,0,  255,236,39,  209,106,58,  255,163,0,  255,236,39,  209,106,58,
  0,0,0,  255,236,39,  209,106,58,  255,163,0,  255,163,0,  255,236,39,  209,106,58,  0,0,0,
  0,0,0,  0,0,0,  255,236,39,  255,236,39,  255,236,39,  209,106,58,  0,0,0,  0,0,0,
};
static const uint8_t cash_arr[3*8*8] = {
  0,0,0,  0,0,0,  0,135,81,  0,135,81,  0,0,0,  0,0,0,  0,0,0,  0,0,0,
  0,0,0,  0,135,81,  0,228,54,  0,228,54,  0,135,81,  0,0,0,  0,0,0,  0,0,0,
  0,135,81,  0,228,54,  0,228,54,  0,135,81,  0,135,81,  0,135,81,  0,0,0,  0,0,0,
  0,135,81,  0,228,54,  0,228,54,  0,135,81,  0,228,54,  0,228,54,  0,135,81,  0,0,0,
  0,0,0,  0,135,81,  0,228,54,  0,228,54,  0,135,81,  0,228,54,  0,228,54,  0,135,81,
  0,0,0,  0,0,0,  0,135,81,  0,135,81,  0,135,81,  0,228,54,  0,228,54,  0,135,81,
  0,0,0,  0,0,0,  0,0,0,  0,135,81,  0,228,54,  0,228,54,  0,135,81,  0,0,0,
  0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,135,81,  0,135,81,  0,0,0,  0,0,0,
};
static const uint8_t gold_arr[3*8*8] = {
  0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,0,0,
  0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,0,0,
  0,0,0,  0,0,0,  0,0,0,  255,163,0,  209,106,58,  0,0,0,  0,0,0,  0,0,0,
  0,0,0,  0,0,0,  0,0,0,  255,236,39,  255,163,0,  0,0,0,  0,0,0,  0,0,0,
  0,0,0,  209,106,58,  126,37,83,  255,163,0,  209,106,58,  126,37,83,  209,106,58,  0,0,0,
  0,0,0,  255,163,0,  209,106,58,  255,236,39,  255,163,0,  209,106,58,  255,163,0,  0,0,0,
  0,0,0,  209,106,58,  126,37,83,  255,163,0,  209,106,58,  126,37,83,  209,106,58,  0,0,0,
  0,0,0,  255,163,0,  209,106,58,  255,236,39,  255,163,0,  209,106,58,  255,163,0,  0,0,0,
};
static const uint8_t egg_arr[3*8*8] = {
  0,0,0,  0,0,0,  0,0,0,  0,0,0,  255,241,232,  255,241,232,  255,241,232,  0,0,0,
  0,0,0,  255,241,232,  255,241,232,  255,241,232,  255,241,232,  255,241,232,  255,241,232,  255,204,170,
  255,241,232,  255,241,232,  255,236,39,  255,236,39,  255,241,232,  255,241,232,  255,241,232,  255,204,170,
  255,241,232,  255,236,39,  255,236,39,  255,236,39,  255,163,0,  255,241,232,  255,241,232,  255,204,170,
  255,241,232,  255,236,39,  255,236,39,  255,236,39,  255,163,0,  255,241,232,  255,204,170,  0,0,0,
  255,241,232,  255,241,232,  255,163,0,  255,163,0,  255,241,232,  255,241,232,  255,204,170,  0,0,0,
  255,241,232,  255,241,232,  255,241,232,  255,241,232,  255,241,232,  255,241,232,  255,204,170,  0,0,0,
  0,0,0,  0,0,0,  255,204,170,  255,204,170,  255,204,170,  255,204,170,  0,0,0,  0,0,0,
};
static const uint8_t ice_cream_arr[3*8*8] = {
  0,0,0,  0,0,0,  255,241,232,  255,241,232,  255,241,232,  255,204,170,  0,0,0,  0,0,0,
  0,0,0,  255,241,232,  255,241,232,  255,241,232,  255,241,232,  255,241,232,  255,204,170,  0,0,0,
  0,0,0,  255,241,232,  255,241,232,  255,241,232,  255,241,232,  255,241,232,  255,204,170,  0,0,0,
  0,0,0,  255,241,232,  255,241,232,  255,241,232,  255,241,232,  255,204,170,  255,204,170,  0,0,0,
  0,0,0,  0,0,0,  255,204,170,  255,204,170,  255,204,170,  255,204,170,  0,0,0,  0,0,0,
  0,0,0,  0,0,0,  255,163,0,  255,163,0,  255,163,0,  209,106,58,  0,0,0,  0,0,0,
  0,0,0,  0,0,0,  0,0,0,  209,106,58,  209,106,58,  0,0,0,  0,0,0,  0,0,0,
  0,0,0,  0,0,0,  0,0,0,  255,163,0,  209,106,58,  0,0,0,  0,0,0,  0,0,0,
};
static const uint8_t cotton_candy_arr[3*8*8] = {
  0,0,0,  0,0,0,  255,119,168,  255,119,168,  255,119,168,  208,90,161,  0,0,0,  0,0,0,
  0,0,0,  255,119,168,  255,119,168,  255,119,168,  255,119,168,  255,119,168,  208,90,161,  0,0,0,
  0,0,0,  255,119,168,  255,119,168,  255,119,168,  255,119,168,  255,119,168,  208,90,161,  0,0,0,
  0,0,0,  255,119,168,  255,119,168,  255,119,168,  255,119,168,  255,119,168,  208,90,161,  0,0,0,
  0,0,0,  208,90,161,  255,119,168,  255,119,168,  255,119,168,  208,90,161,  208,90,161,  0,0,0,
  0,0,0,  0,0,0,  208,90,161,  208,90,161,  208,90,161,  208,90,161,  0,0,0,  0,0,0,
  0,0,0,  0,0,0,  0,0,0,  194,195,199,  194,195,199,  0,0,0,  0,0,0,  0,0,0,
  0,0,0,  0,0,0,  0,0,0,  0,0,0,  255,241,232,  0,0,0,  0,0,0,  0,0,0,
};
static const uint8_t steak_arr[3*8*8] = {
  0,0,0,  255,241,232,  255,241,232,  255,241,232,  255,241,232,  255,241,232,  0,0,0,  0,0,0,
  255,241,232,  255,119,168,  255,0,77,  255,0,77,  255,0,77,  255,119,168,  255,241,232,  0,0,0,
  255,241,232,  255,0,77,  255,0,77,  255,0,77,  255,0,77,  255,0,77,  255,119,168,  255,204,170,  
  255,241,232,  255,119,168,  255,0,77,  255,0,77,  255,0,77,  255,0,77,  255,119,168,  255,204,170,
  0,0,0,  255,204,170,  255,0,77,  255,0,77,  255,241,232,  255,204,170,  255,0,77,  255,204,170,
  0,0,0,  0,0,0,  255,204,170,  255,0,77,  255,204,170,  255,204,170,  255,0,77,  255,204,170,
  0,0,0,  0,0,0,  255,204,170,  255,119,168,  255,0,77,  255,0,77,  255,119,168,  255,204,170,
  0,0,0,  0,0,0,  0,0,0,  255,204,170,  255,204,170,  255,204,170,  255,204,170,  0,0,0,
};
static const uint8_t corn_arr[3*8*8] = {
  0,0,0,  0,0,0,  255,236,39,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,0,0,
  0,0,0,  255,236,39,  255,163,0,  255,236,39,  0,0,0,  0,0,0,  0,0,0,  0,0,0,
  0,0,0,  255,163,0,  255,236,39,  255,163,0,  255,163,0,  0,228,54,  0,228,54,  0,0,0,
  0,0,0,  255,236,39,  255,163,0,  255,236,39,  255,163,0,  0,228,54,  0,135,81,  0,228,54,
  0,0,0,  0,228,54,  255,236,39,  255,163,0,  0,228,54,  0,135,81,  0,0,0,  0,0,0,
  0,228,54,  0,135,81,  0,228,54,  0,228,54,  0,228,54,  0,135,81,  0,0,0,  0,0,0,
  0,0,0,  0,0,0,  0,135,81,  0,228,54,  0,135,81,  0,135,81,  0,135,81,  0,0,0,
  0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,135,81,  0,135,81,  0,0,0,
};
static const uint8_t apple_arr[3*8*8] = {
  0,0,0,  0,0,0,  0,0,0,  209,106,58,  0,228,54,  0,228,54,  0,0,0,  0,0,0,
  0,0,0,  255,0,77,  255,0,77,  209,106,58,  0,228,54,  255,0,77,  162,50,109,  0,0,0,
  255,0,77,  255,241,232,  255,119,168,  162,50,109,  255,0,77,  255,0,77,  255,0,77,  162,50,109,
  255,0,77,  255,119,168,  255,0,77,  255,0,77,  255,0,77,  255,0,77,  255,0,77,  162,50,109,
  255,0,77,  255,0,77,  255,0,77,  255,0,77,  255,0,77,  255,0,77,  255,0,77,  162,50,109,
  255,0,77,  255,0,77,  255,0,77,  255,0,77,  255,0,77,  255,0,77,  255,0,77,  162,50,109,
  0,0,0,  255,0,77,  255,0,77,  162,50,109,  255,0,77,  255,0,77,  162,50,109,  0,0,0,
  0,0,0,  0,0,0,  162,50,109,  162,50,109,  162,50,109,  162,50,109,  0,0,0,  0,0,0,
};
static const uint8_t carrot_arr[3*8*8] = {
  0,0,0,  0,0,0,  0,0,0,  0,228,54,  0,228,54,  0,0,0,  0,228,54,  0,228,54,
  0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,228,54,  0,135,81,  0,228,54,  0,135,81,
  0,0,0,  0,0,0,  0,0,0,  255,163,0,  255,163,0,  209,106,58,  0,135,81,  0,0,0,
  0,0,0,  0,0,0,  0,0,0,  209,106,58,  255,163,0,  209,106,58,  0,228,54,  0,228,54,
  0,0,0,  0,0,0,  255,163,0,  255,163,0,  209,106,58,  209,106,58,  0,0,0,  0,135,81,
  0,0,0,  0,0,0,  255,163,0,  209,106,58,  209,106,58,  0,0,0,  0,0,0,  0,0,0,
  0,0,0,  255,163,0,  209,106,58,  209,106,58,  0,0,0,  0,0,0,  0,0,0,  0,0,0,
  255,163,0,  209,106,58,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,0,0,
};
static const uint8_t banana_arr[3*8*8] = {
  0,0,0,  0,0,0,  0,0,0,  0,0,0,  209,106,58,  126,37,83,  0,0,0,  0,0,0,
  0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  209,106,58,  126,37,83,  0,0,0,
  0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  255,236,39,  255,163,0,  0,0,0,
  0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  255,236,39,  255,236,39,  255,163,0,
  0,0,0,  0,0,0,  0,0,0,  0,0,0,  255,236,39,  255,236,39,  255,236,39,  255,163,0,
  0,0,0,  0,0,0,  255,236,39,  255,236,39,  255,236,39,  255,236,39,  255,163,0,  255,163,0,
  255,163,0,  255,236,39,  255,236,39,  255,236,39,  255,236,39,  255,163,0,  255,163,0,  0,0,0,
  0,0,0,  255,163,0,  255,163,0,  255,163,0,  255,163,0,  255,163,0,  0,0,0,  0,0,0,
};
static const uint8_t sword_arr[3*8*8] = {
  0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  194,195,199,  255,241,232,
  0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  194,195,199,  255,241,232,  131,118,156,
  0,0,0,  0,0,0,  0,0,0,  0,0,0,  194,195,199,  255,241,232,  131,118,156,  0,0,0,
  255,163,0,  255,163,0,  0,0,0,  194,195,199,  255,241,232,  131,118,156,  0,0,0,  0,0,0,
  0,0,0,  255,163,0,  194,195,199,  255,241,232,  131,118,156,  0,0,0,  0,0,0,  0,0,0,
  0,0,0,  171,82,54,  255,163,0,  131,118,156,  0,0,0,  0,0,0,  0,0,0,  0,0,0,
  171,82,54,  171,82,54,  126,37,83,  255,163,0,  255,163,0,  0,0,0,  0,0,0,  0,0,0,
  126,37,83,  126,37,83,  0,0,0,  0,0,0,  255,163,0,  0,0,0,  0,0,0,  0,0,0,
};
static const uint8_t boomerang_arr[3*8*8] = {
  0,0,0,  255,236,39,  171,82,54,  171,82,54,  255,236,39,  171,82,54,  171,82,54,  0,0,0,
  255,236,39,  255,236,39,  255,236,39,  171,82,54,  255,163,0,  171,82,54,  171,82,54,  171,82,54,
  171,82,54,  255,236,39,  255,163,0,  255,163,0,  126,37,83,  126,37,83,  126,37,83,  126,37,83,
  171,82,54,  171,82,54,  255,163,0,  209,106,58,  0,0,0,  0,0,0,  0,0,0,  0,0,0,
  255,236,39,  255,163,0,  126,37,83,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,0,0,
  171,82,54,  171,82,54,  126,37,83,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,0,0,
  171,82,54,  171,82,54,  126,37,83,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,0,0,
  0,0,0,  171,82,54,  126,37,83,  0,0,0,  0,0,0,  0,0,0,  0,0,0,  0,0,0,
};
static const uint8_t bow_arr[3*8*8] = {
  0,0,0,  0,0,0,  0,0,0,  171,82,54,  171,82,54,  171,82,54,  171,82,54,  0,0,0,
  0,0,0,  0,0,0,  171,82,54,  126,37,83,  0,0,0,  194,195,199,  0,0,0,  0,0,0,
  0,0,0,  171,82,54,  126,37,83,  0,0,0,  0,0,0,  255,241,232,  0,0,0,  0,0,0,
  0,0,0,  171,82,54,  126,37,83,  0,0,0,  0,0,0,  255,241,232,  0,0,0,  0,0,0,
  0,0,0,  171,82,54,  126,37,83,  0,0,0,  0,0,0,  255,241,232,  0,0,0,  0,0,0,
  0,0,0,  171,82,54,  126,37,83,  0,0,0,  0,0,0,  255,241,232,  0,0,0,  0,0,0,
  0,0,0,  0,0,0,  171,82,54,  126,37,83,  0,0,0,  255,241,232,  0,0,0,  0,0,0,
  0,0,0,  0,0,0,  0,0,0,  171,82,54,  171,82,54,  171,82,54,  171,82,54,  0,0,0,
};
static const uint8_t axe_arr[3*8*8] = {
  255,241,232,  194,195,199,  194,195,199,  0,0,0,  171,82,54,  0,0,0,  0,0,0,  0,0,0,
  255,241,232,  194,195,199,  194,195,199,  194,195,199,  255,163,0,  126,37,83,  0,0,0,  255,241,232,
  255,241,232,  194,195,199,  194,195,199,  194,195,199,  255,163,0,  209,106,58,  194,195,199,  255,241,232,
  255,241,232,  194,195,199,  194,195,199,  194,195,199,  255,163,0,  209,106,58,  131,118,156,  255,241,232,
  255,241,232,  194,195,199,  194,195,199,  131,118,156,  171,82,54,  209,106,58,  0,0,0,  194,195,199,
  194,195,199,  131,118,156,  0,0,0,  0,0,0,  255,163,0,  126,37,83,  209,106,58,  0,0,0,
  0,0,0,  0,0,0,  0,0,0,  0,0,0,  171,82,54,  126,37,83,  0,0,0,  0,0,0,
  0,0,0,  0,0,0,  0,0,0,  0,0,0,  171,82,54,  126,37,83,  0,0,0,  0,0,0,
};
static const uint8_t *sprite_buffers[24] = {
  blue_potion_arr,
  green_potion_arr,
  orange_potion_arr,
  red_potion_arr,
  blue_gem_arr,
  green_gem_arr,
  orange_gem_arr,
  red_gem_arr,
  money_bag_arr,
  coin_arr,
  cash_arr,
  gold_arr,
  egg_arr,
  ice_cream_arr,
  cotton_candy_arr,
  steak_arr,
  corn_arr,
  apple_arr,
  carrot_arr,
  banana_arr,
  sword_arr,
  boomerang_arr,
  bow_arr,
  axe_arr,
};
static int sprite_buffer_idx = 0;
struct rgb_pixel {
  uint8_t r;
  uint8_t g;
  uint8_t b;
};


class Animation {
  private:
    const uint8_t *sprite_data;
    uint16_t buffer_size;
    uint8_t data_span;
    bool rgb_data;

    uint16_t start_frame;
    uint16_t current_frame;
    uint8_t num_frames;
  
    int loop_number;
    int loop_counter;

    int delay_cycles;
    int delay_cnt;

    bool complete;
    bool played_once;
    bool init;

  public:
    Animation(const uint8_t *data, uint16_t size, uint8_t span, bool rgb_data_);//Called once
    void setAnimation(const uint8_t *data, uint16_t size, uint8_t span, bool rgb_data_);
    void startAnimation(uint8_t frame_index, uint8_t num_frames_, int delay_cycles_, int loop_number_);//Called each time animation changes
    void update();
    bool animationComplete() {return complete;}
    void draw(doubleBuffer *frame_buffer, int x, uint8_t r, uint8_t g, uint8_t b);
    void draw_rgb(doubleBuffer *frame_buffer, int x);
};
Animation::Animation(const uint8_t *data, uint16_t size, uint8_t span, bool rgb_data_=false) {
  sprite_data = data;
  buffer_size = size;
  data_span = span;
  init = false;
  rgb_data = rgb_data_;
}
void Animation::setAnimation(const uint8_t *data, uint16_t size, uint8_t span, bool rgb_data_=false) {
  sprite_data = data;
  buffer_size = size;
  data_span = span;
  init = false;
  rgb_data = rgb_data_;
}
void Animation::startAnimation(uint8_t frame_index, uint8_t num_frames_, int delay_cycles_, int loop_number_=-1) {
  start_frame = frame_index; 
  current_frame = start_frame;
  num_frames = num_frames_;

  loop_number = loop_number_;
  loop_counter = 0;

  delay_cycles = delay_cycles_;
  delay_cnt = 0;

  complete = false;
  played_once = false;
  init = true;
}
void Animation::update() {
  if (!init)
    return;

  if (delay_cnt++ % delay_cycles != 0)
    return;

  uint16_t prev_frame = current_frame;
  //current_frame += span;

  current_frame++;
  int frame_idx = current_frame - start_frame;
  if (frame_idx >= num_frames)
  {
    current_frame = prev_frame;
    if ((loop_number < 0) || (loop_counter < loop_number))
    {
      current_frame = start_frame;
      played_once = true;
      loop_counter++;
    }
    else
    {
      complete = true;
    }
  }
}
void Animation::draw(doubleBuffer *frame_buffer, int x, uint8_t r, uint8_t g, uint8_t b) {
  if (rgb_data) {
    draw_rgb(frame_buffer, x);
    return;
  }
  if (!init)
    return;
  if (sprite_data == NULL)
    return;
  if (current_frame + 6 > buffer_size)
    return;

  for (int k = 0; k<6; k++)
  {
    uint16_t current = current_frame*data_span + k;
    for (int j=0; j<8; j++)
    {
      if (current >= buffer_size)
        return;
      if (sprite_data[current] & (1 << j))
      {
        frame_buffer->setColors(x, j, 5-k, r, g, b);
      }
    }
  }
}
void Animation::draw_rgb(doubleBuffer *frame_buffer, int x) {
  if (!init)
    return;
  if (sprite_data == NULL)
    return;

  static const int SPRITE_WIDTH = 8;
  static const int SPRITE_HEIGHT = 8;
  struct rgb_pixel *sprite_array = (struct rgb_pixel *)sprite_data;
  /*
  for (int i=x; i<SPRITE_WIDTH; i++) {
    for (int j=0; j<SPRITE_HEIGHT; j++) {
      struct rgb_pixel px = sprite_array[i*SPRITE_WIDTH + j];
      int j_idx = WIDTH - 1 - j;
      frame_buffer->setColors(i, j_idx, 0, px.r, px.g, px.b);
    }
  }
  */
  for (int j=0; j<SPRITE_HEIGHT; j++) {
    int j_idx = WIDTH - 1 - j;
    for (int i=x; i<x+SPRITE_WIDTH; i++) {
      struct rgb_pixel px = sprite_array[j*SPRITE_HEIGHT + (i-x)];
      frame_buffer->setColors(i, j_idx, 0, px.r, px.g, px.b);
    }
  }
}

class SpaceGame
{
  private:
    Ship ship;
    Vector3d block[2];
    bool block_collide;
    uint8_t hits;
    uint8_t cnt;
    int draw_cnt;
    bool pause;
    Animation face_animation;
    Animation banana;
    Animation sprites;

    static constexpr int block_color[3] = {70, 100, 70};
    static const uint8_t delay_cnt = 6;
    static const uint8_t MAX_HITS = 5;
    
  public:
    SpaceGame();
    void reset();
    void update();
    void draw(doubleBuffer *frame_buffer);
};
SpaceGame::SpaceGame() : face_animation(RAW_SPRITE, 18*6, 6), banana(BANANA_SPRITE, 64*3, 64*3, true), sprites(sprite_buffers[0], 64*3, 64*3, true)
{
  reset();
}
void SpaceGame::reset()
{
  ship.reset(Vector3d(0, 4, 3), Vector3d(1, 0, 0));
  block[0].setVector3d(rand()%LENGTH - 1, rand()%WIDTH - 1, rand()%HEIGHT - 1);
  block[1].setVector3d(block[0].x+1, block[0].y+1, block[0].z+1);

  block_collide = false;
  hits = MAX_HITS;
  cnt = 0;
  draw_cnt = 0;
  pause = false;


  face_animation.setAnimation(RAW_SPRITE, 18*6, 6);
  banana.setAnimation(BANANA_SPRITE, 64*3, 64*3, true);
  sprites.setAnimation(sprite_buffers[0], 64*3, 64*3, true);


  face_animation.startAnimation(13, 5, 3, 10);
  banana.startAnimation(0, 1, 1);
  sprites.startAnimation(0, 1, 1);
}
void SpaceGame::update()
{
  if (cnt++ % delay_cnt != 0)
  {
    return;
  }

  //Events already loaded into event buffer
  uint8_t num_events = eventBuffer.size();
  for (int i=0; i<num_events; i++)
  {
    Event e;
    eventBuffer.pop(e);
    if (e.type == Event::ON_PRESS && e.data.button_idx == OPTIONS)
    {
      pause = (pause) ? false : true;
    }
    if (!pause)
    {
      ship.handleEvent(e);
    }
  }
  if (pause)
  {
    return;
  }

  ship.update();
  face_animation.update();
  block_collide = ship.checkBlockCollision(block);
  if (block_collide)
  {
    hits--;
    if (hits <= 0)
    {
      hits = MAX_HITS;
      block[0].setVector3d(rand()%(LENGTH - 1), rand()%(WIDTH - 1), rand()%(HEIGHT - 1));
      block[1].setVector3d(block[0].x+1, block[0].y+1, block[0].z+1);
      block_collide = false;
    }
  }
}
void SpaceGame::draw(doubleBuffer *frame_buffer)
{
  //static int cnt = 0;
  int idx = (draw_cnt/100) % NUM_FACES;
  draw_cnt++;

  face_animation.draw(frame_buffer, 10, 255, 128, 0);
  banana.draw_rgb(frame_buffer, 0);

  sprites.setAnimation(sprite_buffers[(draw_cnt/350)%24], 64*3, 64*3, true);
  sprites.startAnimation(0, 1, 1);
  sprites.draw_rgb(frame_buffer, 65);

  for (int k = 0; k<6; k++)
  {
    for (int j=0; j<8; j++)
    {
      if (FACE_BUF[idx][k] & (1 << j))
      {
        frame_buffer->setColors(60, j, 5-k, 0, 0, 255);
      }
    }
  }

  if (!block_collide)
  {
    frame_buffer->drawBlock(block[0], block[1], block_color[RED], block_color[GREEN], block_color[BLUE]);
  }
  else
  {
    frame_buffer->drawBlock(block[0], block[1], 255, 0, 0);
  }
  ship.draw(frame_buffer);
}

//void getSerialData(Ship *ship);
void ship_loop(doubleBuffer *frame_buffer)
{

  Ship ship(Vector3d(0, 4, 3), Vector3d(1, 0, 0));
  Vector3d block[2];
  block[0].setVector3d(rand()%LENGTH - 1, rand()%WIDTH - 1, rand()%HEIGHT - 1);
  block[1].setVector3d(block[0].x+1, block[0].y+1, block[0].z+1);
  uint8_t block_color[3] = {70, 100, 70};
  bool block_collide = false;
  int hits = 5;
  int block_vel = 1;
  int block_update = 0;
  
  int i=0;
  while(1)
  {
    SerialUSB.println("Ship Loop");
    /*
    i++;
    if (i >= 10)
    {
      ship.setFire(true);
      i =0;
      
    }
    */
    Event::SerialParser();
    ship.getSerialData();

    
    ship.update();
    /*
    if (block_update == 2)
    {
      if (vel > 0)
      {
        block[0].z++;
        if (block[0].z >= HEIGHT)
        {
          vel *= -1;
          block[0].z = HEIGHT - 2;
        }
      }
      else
      {
        
      }
      block_update = 0;
    }
    block_update++;
    */
    block_collide = ship.checkBlockCollision(block);
    if (block_collide)
    {
      hits--;
      if (hits <= 0)
      {
        hits = 5;
        block[0].setVector3d(rand()%(LENGTH - 1), rand()%(WIDTH - 1), rand()%(HEIGHT - 1));
        block[1].setVector3d(block[0].x+1, block[0].y+1, block[0].z+1);
        block_collide = false;
      }
    }
    
    frame_buffer->clear();
    if (!block_collide)
      frame_buffer->drawBlock(block[0], block[1], block_color[RED], block_color[GREEN], block_color[BLUE]);
    else
      frame_buffer->drawBlock(block[0], block[1], 255, 0, 0);
    ship.draw(frame_buffer);
    frame_buffer->update();
    
    delay(35);
  }
}

#endif
