#ifndef SPACE_GAME_LIB
#define SPACE_GAME_LIB

#include "Vector3d.h"
#include "FrameBuffer.h"

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
  if (pos.x >= LENGTH)
  {
    pos.x %= LENGTH;
  }
  if (pos.x < 0)
  {
    pos.x += LENGTH;
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
        Ship();
        Ship(Vector3d pos_, Vector3d vel_);
        void reset(Vector3d pos_, Vector3d vel_);
        void update();
        void draw(doubleBuffer *frame_buffer);
        void getSerialData();
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
void Ship::getSerialData()
{
  while (Serial1.available())
  {
    if (Serial1.available() > 1)
    {
      char c0 = Serial1.read();
      char c1 = Serial1.read();
      switch (c0)
      {
        case '1':
        {
          if (c1 == 'p')
          {
            setLeft(true);
          }
          else if(c1 == 'r')
          {
            setLeft(false);
          }
          break;
        }
        case '2':
        {
          if (c1 == 'p')
          {
            setFire(true);
          }
          else if(c1 == 'r')
          {

          }
          break;
        }
        case '3':
        {
          if (c1 == 'p')
          {
            setRight(true);
          }
          else if(c1 == 'r')
          {
            setRight(false);
          }
          break;
        }
        case '4':
        {
          if (c1 == 'p')
          {
            setUp(true);
          }
          else if(c1 == 'r')
          {
            setUp(false);
          }
          break;
        }
        case '5':
        {
          if (c1 == 'p')
          {
            setDown(true);
          }
          else if(c1 == 'r')
          {
            setDown(false);
          }
          break;
        }
      }
    }
    else
    {
      Serial1.read();
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
  
  int i=0;
  while(1)
  {
    /*
    i++;
    if (i >= 10)
    {
      ship.setFire(true);
      i =0;
      
    }
    */
    ship.getSerialData();

    
    ship.update();
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
