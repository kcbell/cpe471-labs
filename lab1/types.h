#ifndef __TYPES_H__
#define __TYPES_H__

#include <math.h>
#include <vector>

/* Color struct */
typedef struct color_struct {
   double r;
   double g;
   double b;
   double f; // "filter" or "alpha"
} color_t;

typedef struct vertex_struct {
   int x;
   int y;
   int z;
} vertex_t;

typedef struct bary_struct {
   double alpha;
   double beta;
   double gamma;
} bary_t;

typedef struct triangle_struct {
   vertex_t a;
   vertex_t b;
   vertex_t c;
   color_t aC;
   color_t bC;
   color_t cC;
} triangle_t;

class SColor
{

public:

    float Red, Green, Blue;

    SColor()
        : Red(0.7f), Green(0.7f), Blue(0.7f)
    {}

};

class SVector3
{

public:

   float X, Y, Z;

   SVector3()
      : X(0), Y(0), Z(0)
   {}

   SVector3(float in)
      : X(in), Y(in), Z(in)
   {}

   SVector3(float in_x, float in_y, float in_z)
      : X(in_x), Y(in_y), Z(in_z)
   {}

   SVector3 crossProduct(SVector3 const & v) const
   {
      return SVector3(Y*v.Z - v.Y*Z, v.X*Z - X*v.Z, X*v.Y - v.X*Y);
   }

   float dotProduct(SVector3 const & v) const
   {
      return X*v.X + Y*v.Y + Z*v.Z;
   }

   float length() const
   {
      return sqrtf(X*X + Y*Y + Z*Z);
   }

   SVector3 operator + (SVector3 const & v) const
   {
      return SVector3(X+v.X, Y+v.Y, Z+v.Z);
   }

   SVector3 & operator += (SVector3 const & v)
   {
      X += v.X;
      Y += v.Y;
      Z += v.Z;

      return * this;
   }

   SVector3 operator - (SVector3 const & v) const
   {
      return SVector3(X-v.X, Y-v.Y, Z-v.Z);
   }

   SVector3 & operator -= (SVector3 const & v)
   {
      X -= v.X;
      Y -= v.Y;
      Z -= v.Z;

      return * this;
   }

   SVector3 operator * (SVector3 const & v) const
   {
      return SVector3(X*v.X, Y*v.Y, Z*v.Z);
   }

   SVector3 & operator *= (SVector3 const & v)
   {
      X *= v.X;
      Y *= v.Y;
      Z *= v.Z;

      return * this;
   }

   SVector3 operator / (SVector3 const & v) const
   {
      return SVector3(X/v.X, Y/v.Y, Z/v.Z);
   }

   SVector3 & operator /= (SVector3 const & v)
   {
      X /= v.X;
      Y /= v.Y;
      Z /= v.Z;

      return * this;
   }

   SVector3 operator * (float const s) const
   {
      return SVector3(X*s, Y*s, Z*s);
   }

   SVector3 & operator *= (float const s)
   {
      X *= s;
      Y *= s;
      Z *= s;

      return * this;
   }

   SVector3 operator / (float const s) const
   {
      return SVector3(X/s, Y/s, Z/s);
   }

   SVector3 & operator /= (float const s)
   {
      X /= s;
      Y /= s;
      Z /= s;

      return * this;
   }

};

class SVertex
{

public:

    SVector3 Position;
    SColor Color;

};

class CMesh
{

    friend class CVertexBufferObject;
    friend class CMeshLoader;

    struct STriangle
    {
        unsigned int VertexIndex1, VertexIndex2, VertexIndex3;
      SColor Color;
    };

    std::vector<SVertex> Vertices;
    std::vector<STriangle> Triangles;

    CMesh();

public:

    ~CMesh();

   void centerMeshByAverage(SVector3 const & CenterLocation);
   void centerMeshByExtents(SVector3 const & CenterLocation);

   void resizeMesh(SVector3 const & Scale);

};

#endif
