#ifndef VECTOR3D_LIB
#define VECTOR3D_LIB

class Vector3d
{
  public:
    int x, y, z;
    Vector3d(){}
    Vector3d(int x_, int y_, int z_) {x = x_; y=y_; z=z_;}

    void setVector3d(int x_, int y_, int z_) {x = x_;y=y_;z=z_;}
    void addVector3d(int x_, int y_, int z_);
    void subVector3d(Vector3d v);
    void addVector3d(Vector3d v);

    static Vector3d addVector3d(Vector3d v0, Vector3d v1);
    static Vector3d subVector3d(Vector3d v0, Vector3d v1);
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
void Vector3d::subVector3d(Vector3d v)
{
  x -= v.x;
  y -= v.y;
  z -= v.z;
}

Vector3d Vector3d::addVector3d(Vector3d v0, Vector3d v1)
{
  return Vector3d(v0.x+v1.x, v0.y+v1.y, v0.z+v1.z);
}
Vector3d Vector3d::subVector3d(Vector3d vf, Vector3d vi)
{
  return Vector3d(vf.x - vi.x, vf.y - vi.y, vf.z - vi.z);
}
#endif
