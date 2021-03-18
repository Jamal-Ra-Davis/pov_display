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
        const uint8_t color[3] = {60, 255, 0};
        
    public:
        static const uint8_t LIFE_TIME = 25;
        
        Bullet();
        Bullet(Vector3d pos_, int vel_, int lt);
        void setBullet(Vector3d pos_, int vel_, int lt);

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
        

        const uint8_t color[3] = {255, 255, 255};
        const float BOOST_MULT = 1.5;
        const int COOL_DOWN_TIME = 3;
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
      switch(e.button_idx)
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
      switch(e.button_idx)
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
        switch(e.button_idx)
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
        switch(e.button_idx)
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

class SpaceGame
{
  private:
    Ship ship;
    Vector3d block[2];
    bool block_collide;
    uint8_t hits;
    uint8_t cnt;
    bool pause;

    static constexpr int block_color[3] = {70, 100, 70};
    static const uint8_t delay_cnt = 10;
    static const uint8_t MAX_HITS = 5;
    
  public:
    SpaceGame();
    void reset();
    void update();
    void draw(doubleBuffer *frame_buffer);
};
SpaceGame::SpaceGame()
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
  pause = false;
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
    if (e.type == Event::ON_PRESS && e.button_idx == OPTIONS)
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
