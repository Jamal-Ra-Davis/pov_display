#ifndef VECTOR3D_LIB
#define VECTOR3D_LIB

class Vector3d
{
  public:
    int x, y, z;
    Vector3d();
    Vector3d(int x_, int y_, int z_) {x = x_;y=y_;z=z_;}

    void setVector3d(int x_, int y_, int z_) {x = x_;y=y_;z=z_;}
    void addVector3d(int x_, int y_, int z_);
    void addVector3d(Vector3d v);
};

void Vector3d::addVector3d(int x_, int y_, int z_)
{
  x += x_;
  y += y_;
  z += z_;
}
void Vector3d::addVector3d(Vector3d v)
{
  x += v.x;
  y += v.y;
  z += v.z;
}

#endif
