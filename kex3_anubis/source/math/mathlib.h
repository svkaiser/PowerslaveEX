//
// Copyright(C) 2014-2015 Samuel Villarreal
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//

#ifndef __MATHLIB_H__
#define __MATHLIB_H__

#include <math.h>
#include <assert.h>
#include "kexlib.h"

#ifdef M_PI
#undef M_PI
#endif

#define FLOATSIGNBIT(f)  (reinterpret_cast<const unsigned int&>(f) >> 31)

class kexVec3;
class kexVec4;
class kexMatrix;
class kexAngle;
class kexStr;

class kexMath
{
public:
    static const float      pi;
    static const float      rad;
    static const float      deg;
    static const float      infinity;
    
    static float            Sin(float x) { return sinf(x); }
    static float            Cos(float x) { return cosf(x); }
    static float            Tan(float x) { return tanf(x); }
    static float            ATan2(float x, float y) { return atan2f(x, y); }
    static float            ACos(float x) { return acosf(x); }
    static float            Sqrt(float x) { return x * InvSqrt(x); }
    static float            Pow(float x, float y) { return powf(x, y); }
    static float            Log(float x) { return logf(x); }
    static float            Floor(float x) { return floorf(x); }
    static float            Ceil(float x) { return ceilf(x); }
    static float            FMod(float x, float y) { return fmodf(x, y); }
    static float            Deg2Rad(float x) { return x * rad; }
    static float            Rad2Deg(float x) { return x * deg; }
    static float            Sec2MSec(float x) { return x * 1000.0f; }
    static int              Sec2MSec(int x) { return x * 1000; }
    static float            MSec2Sec(float x) { return x * 0.001f; }
    static int              MSec2Sec(int x) { return x / 1000; }
    static float            FrameSec(float x) { assert(x != 0); return 1000.0f / x; }
    static int              FrameSec(int x) { assert(x != 0); return 1000 / x; }

    static int              Abs(int x);
    static float            Fabs(float x);
    static int              RoundPowerOfTwo(int x);
    static float            InvSqrt(float x);
    static float            SinZeroHalfPI(float a);
    static float            ATanPositive(float y, float x);
    static bool             FCmp(float f1, float f2);
    static void             Clamp(float &f, const float min, const float max);
    static void             Clamp(double &f, const double min, const double max);
    static void             Clamp(int &i, const int min, const int max);
    static void             Clamp(short &i, const short min, const short max);
    static void             Clamp(byte &b, const byte min, const byte max);

    static void             CubicCurve(const kexVec3 &start, const kexVec3 &end, const float time,
                                       const kexVec3 &point, kexVec3 *vec);
    static void             QuadraticCurve(const kexVec3 &start, const kexVec3 &end, const float time,
                                           const kexVec3 &pt1, const kexVec3 &pt2, kexVec3 *vec);
};

class kexRand
{
public:
    static void             SetSeed(const int randSeed);
    static int              SysRand(void);
    static int              Int(void);
    static uint8_t          Byte(void);
    static int              Max(const int max);
    static float            Float(void);
    static float            CFloat(void);
    static float            Range(const float r1, const float r2);

private:
    static unsigned int     seed;
};

class kexQuat
{
public:
    kexQuat(void);

    explicit kexQuat(const float angle, const float x, const float y, const float z);
    explicit kexQuat(const float angle, kexVec3 &vector);
    explicit kexQuat(const float angle, const kexVec3 &vector);

    void                    Set(const float x, const float y, const float z, const float w);
    void                    Clear(void);
    float                   Dot(const kexQuat &quat) const;
    float                   UnitSq(void) const;
    float                   Unit(void) const;
    kexQuat                 &Normalize(void);
    kexQuat                 Slerp(const kexQuat &quat, float movement) const;
    kexQuat                 RotateFrom(const kexVec3 &location, const kexVec3 &target, float maxAngle);
    kexQuat                 Inverse(void) const;

    kexQuat                 operator+(const kexQuat &quat);
    kexQuat                 &operator+=(const kexQuat &quat);
    kexQuat                 operator-(const kexQuat &quat);
    kexQuat                 &operator-=(const kexQuat &quat);
    kexQuat                 operator*(const kexQuat &quat);
    kexQuat                 operator*(const float val) const;
    kexQuat                 &operator*=(const kexQuat &quat);
    kexQuat                 &operator*=(const float val);
    kexQuat                 &operator=(const kexQuat &quat);
    kexQuat                 &operator=(const kexVec4 &vec);
    kexQuat                 &operator=(const float *vecs);
    kexVec3                 operator*(const kexVec3 &vector);

    const kexVec3           &ToVec3(void) const;
    kexVec3                 &ToVec3(void);

    float                   x;
    float                   y;
    float                   z;
    float                   w;
};

class kexVec3;

class kexVec2
{
public:
    kexVec2(void);
    explicit kexVec2(const float x, const float y);

    void                    Set(const float x, const float y);
    void                    Clear(void);
    float                   Dot(const kexVec2 &vec) const;
    static float            Dot(const kexVec2 &vec1, const kexVec2 &vec2);
    float                   CrossScalar(const kexVec2 &vec) const;
    kexVec2                 Cross(const kexVec2 &vec) const;
    kexVec2                 &Cross(const kexVec2 &vec1, const kexVec2 &vec2);
    float                   Dot(const kexVec3 &vec) const;
    static float            Dot(const kexVec3 &vec1, const kexVec3 &vec2);
    kexVec2                 Cross(const kexVec3 &vec) const;
    kexVec2                 &Cross(const kexVec3 &vec1, const kexVec3 &vec2);
    float                   UnitSq(void) const;
    float                   Unit(void) const;
    float                   DistanceSq(const kexVec2 &vec) const;
    float                   Distance(const kexVec2 &vec) const;
    kexVec2                 &Normalize(void);
    kexVec2                 &Project(const kexVec2 &normal, const float amount);
    kexVec2                 Lerp(const kexVec2 &next, float movement) const;
    kexVec2                 &Lerp(const kexVec2 &next, const float movement);
    kexVec2                 &Lerp(const kexVec2 &start, const kexVec2 &next, float movement);
    kexStr                  ToString(void) const;
    float                   ToYaw(void) const;
    float                   *ToFloatPtr(void);
    kexVec3                 ToVec3(void);

    kexVec2                 operator+(const kexVec2 &vec);
    kexVec2                 operator+(const kexVec2 &vec) const;
    kexVec2                 operator+(kexVec2 &vec);
    kexVec2                 operator-(void) const;
    kexVec2                 operator-(const kexVec2 &vec) const;
    kexVec2                 operator*(const kexVec2 &vec);
    kexVec2                 operator*(const float val);
    kexVec2                 operator*(const float val) const;
    kexVec2                 operator/(const kexVec2 &vec);
    kexVec2                 operator/(const float val);
    kexVec2                 &operator=(kexVec3 &vec);
    kexVec2                 &operator=(const kexVec2 &vec);
    kexVec2                 &operator=(const kexVec3 &vec);
    kexVec2                 &operator=(const float *vecs);
    kexVec2                 &operator+=(const kexVec2 &vec);
    kexVec2                 &operator-=(const kexVec2 &vec);
    kexVec2                 &operator*=(const kexVec2 &vec);
    kexVec2                 &operator*=(const float val);
    kexVec2                 &operator/=(const kexVec2 &vec);
    kexVec2                 &operator/=(const float val);
    kexVec2                 operator*(const kexMatrix &mtx);
    kexVec2                 operator*(const kexMatrix &mtx) const;
    kexVec2                 &operator*=(const kexMatrix &mtx);
    float                   operator[](int index) const;
    float                   &operator[](int index);
    bool                    operator==(const kexVec2 &vec);

    operator                float *(void) { return reinterpret_cast<float*>(&x); }

    static kexVec2          vecZero;
    static const kexVec2    vecRight;
    static const kexVec2    vecUp;

    float                   x;
    float                   y;
};

class kexVec3
{
public:
    kexVec3(void);
    explicit kexVec3(const float x, const float y, const float z);
    explicit kexVec3(const kexVec2 &vec, const float z);

    void                    Set(const float x, const float y, const float z);
    void                    Set(const float val);
    void                    Clear(void);
    float                   Dot(const kexVec3 &vec) const;
    static float            Dot(const kexVec3 &vec1, const kexVec3 &vec2);
    kexVec3                 Cross(const kexVec3 &vec) const;
    kexVec3                 &Cross(const kexVec3 &vec1, const kexVec3 &vec2);
    float                   UnitSq(void) const;
    float                   Unit(void) const;
    float                   DistanceSq(const kexVec3 &vec) const;
    float                   Distance(const kexVec3 &vec) const;
    kexVec3                 &Normalize(void);
    kexVec3                 Lerp(const kexVec3 &next, const float movement) const;
    kexVec3                 &Lerp(const kexVec3 &next, const float movement);
    kexVec3                 &Lerp(const kexVec3 &start, const kexVec3 &next, const float movement);
    kexVec3                 &Project(const kexVec3 &normal, const float amount);
    kexQuat                 ToQuat(void);
    float                   ToYaw(void) const;
    float                   ToPitch(void) const;
    static void             ToAxis(kexVec3 *forward, kexVec3 *up, kexVec3 *right,
                                   const float yaw, const float pitch, const float roll);
    kexStr                  ToString(void) const;
    float                   *ToFloatPtr(void);
    kexVec2                 ToVec2(void);
    kexVec2                 ToVec2(void) const;

    kexVec3                 operator+(const kexVec3 &vec);
    kexVec3                 operator+(const kexVec3 &vec) const;
    kexVec3                 operator+(kexVec3 &vec);
    kexVec3                 operator-(void) const;
    kexVec3                 operator-(const kexVec3 &vec) const;
    kexVec3                 operator*(const kexVec3 &vec);
    kexVec3                 operator*(const float val);
    kexVec3                 operator*(const float val) const;
    kexVec3                 operator/(const kexVec3 &vec);
    kexVec3                 operator/(const float val);
    kexVec3                 operator*(const kexQuat &quat);
    kexVec3                 operator*(const kexMatrix &mtx);
    kexVec3                 operator*(const kexMatrix &mtx) const;
    kexVec3                 &operator=(const kexVec3 &vec);
    kexVec3                 &operator=(const float *vecs);
    kexVec3                 &operator+=(const kexVec3 &vec);
    kexVec3                 &operator-=(const kexVec3 &vec);
    kexVec3                 &operator*=(const kexVec3 &vec);
    kexVec3                 &operator*=(const float val);
    kexVec3                 &operator/=(const kexVec3 &vec);
    kexVec3                 &operator/=(const float val);
    kexVec3                 &operator*=(const kexQuat &quat);
    kexVec3                 &operator*=(const kexMatrix &mtx);
    float                   operator[](int index) const;
    float                   &operator[](int index);

    operator                float *(void) { return reinterpret_cast<float*>(&x); }

    static const kexVec3    vecForward;
    static const kexVec3    vecUp;
    static const kexVec3    vecRight;
    static const kexVec3    vecZero;

    float                   x;
    float                   y;
    float                   z;
};

class kexVec4
{
public:
    kexVec4(void);
    explicit kexVec4(const float x, const float y, const float z, const float w);
    explicit kexVec4(const kexVec3 &vector, const float w);

    void                    Set(const float x, const float y, const float z, const float w);
    void                    Clear(void);
    float                   *ToFloatPtr(void);

    const kexVec3           &ToVec3(void) const;
    kexVec3                 &ToVec3(void);
    kexVec4                 operator|(const kexMatrix &mtx);
    kexVec4                 &operator|=(const kexMatrix &mtx);
    kexVec4                 operator+(const kexVec4 &vec);
    kexVec4                 operator+(const kexVec4 &vec) const;
    kexVec4                 operator+(kexVec4 &vec);
    kexVec4                 operator*(const kexMatrix &mtx);
    kexVec4                 &operator*=(const kexMatrix &mtx);
    float                   operator[](int index) const;
    float                   &operator[](int index);

    float                   x;
    float                   y;
    float                   z;
    float                   w;
};

class kexMatrix
{
public:
    kexMatrix(void);
    kexMatrix(const kexMatrix &mtx);
    kexMatrix(const float x, const float y, const float z);
    kexMatrix(const kexQuat &quat);
    kexMatrix(const float angle, const int axis);

    kexMatrix               &Identity(void);
    kexMatrix               &Identity(const float x, const float y, const float z);
    kexMatrix               &SetTranslation(const float x, const float y, const float z);
    kexMatrix               &SetTranslation(const kexVec3 &vector);
    kexMatrix               &AddTranslation(const float x, const float y, const float z);
    kexMatrix               &AddTranslation(const kexVec3 &vector);
    void                    RotateX(float angle);
    void                    RotateY(float angle);
    void                    RotateZ(float angle);
    kexMatrix               &Scale(const float x, const float y, const float z);
    kexMatrix               &Scale(const kexVec3 &vector);
    static kexMatrix        Scale(const kexMatrix &mtx, const float x, const float y, const float z);
    kexMatrix               &Transpose(void);
    static kexMatrix        Transpose(const kexMatrix &mtx);
    static kexMatrix        Invert(kexMatrix &mtx);
    kexQuat                 ToQuat(void);
    float                   *ToFloatPtr(void);
    void                    SetViewProjection(float aspect, float fov, float zNear, float zFar);
    void                    SetOrtho(float left, float right,
                                     float bottom, float top,
                                     float zNear, float zFar);

    kexMatrix               operator*(const kexVec3 &vector);
    kexMatrix               &operator*=(const kexVec3 &vector);
    kexMatrix               operator*(const kexMatrix &matrix);
    kexMatrix               &operator*=(const kexMatrix &matrix);
    friend kexMatrix        operator*(const kexMatrix &m1, const kexMatrix &m2);
    kexMatrix               &operator=(const kexMatrix &matrix);
    kexMatrix               &operator=(const float *m);
    kexMatrix               operator|(const kexMatrix &matrix);

    kexVec4                 vectors[4];
};

class kexPluecker
{
public:
    kexPluecker(void);
    kexPluecker(const kexVec3 &start, const kexVec3 &end, bool bRay = false);

    void                    Clear(void);
    void                    SetLine(const kexVec3 &start, const kexVec3 &end);
    void                    SetRay(const kexVec3 &start, const kexVec3 &dir);
    float                   InnerProduct(const kexPluecker &pluecker) const;

    float                   p[6];
};

class kexPlane
{
public:
    kexPlane(void);
    kexPlane(const float a, const float b, const float c, const float d);
    kexPlane(const kexVec3 &pt1, const kexVec3 &pt2, const kexVec3 &pt3);
    kexPlane(const kexVec3 &normal, const kexVec3 &point);
    kexPlane(const kexPlane &plane);

    typedef enum
    {
        AXIS_YZ             = 0,
        AXIS_XZ,
        AXIS_XY
    } planeAxis_t;

    typedef enum
    {
        PSIDE_FRONT         = 0,
        PSIDE_BACK,
        PSIDE_ON
    } planeSide_t;

    const kexVec3           &Normal(void) const;
    kexVec3                 &Normal(void);
    kexPlane                &SetNormal(const kexVec3 &normal);
    kexPlane                &SetNormal(const kexVec3 &pt1, const kexVec3 &pt2, const kexVec3 &pt3);
    float                   Dot(const kexVec3 &point);
    float                   Distance(const kexVec3 &point);
    kexPlane                &SetDistance(const kexVec3 &point);
    bool                    IsFacing(const float yaw);
    const planeAxis_t       BestAxis(void) const;
    const planeSide_t       PointOnSide(const kexVec3 &point);
    float                   ToYaw(void);
    float                   ToPitch(void);
    kexQuat                 ToQuat(void);
    const kexVec4           &ToVec4(void) const;
    kexVec4                 &ToVec4(void);

    float                   operator[](int index) const;
    float                   &operator[](int index);
    kexPlane                &operator=(const kexPlane &p);

    float                   a;
    float                   b;
    float                   c;
    float                   d;
};

class kexAngle
{
public:
    kexAngle(void);
    kexAngle(const float f);

    void                    Clamp(void);
    static void             Clamp(float &f);
    static void             Clamp360(float &f);
    float                   Diff(const float f);
    float                   Diff(const kexAngle &ang);

    operator                float (void) { return an; }

    kexAngle                operator+(const float f) const;
    kexAngle                &operator+=(const float f);
    kexAngle                operator-(const float f) const;
    kexAngle                &operator-=(const float f);
    kexAngle                operator+(const kexAngle &ang) const;
    kexAngle                &operator+=(const kexAngle &ang);
    kexAngle                operator-(const kexAngle &ang) const;
    kexAngle                &operator-=(const kexAngle &ang);
    kexAngle                &operator=(const float f);
    kexAngle                &operator=(const kexAngle &ang);
    kexAngle                operator-(void) const;

    float                   an;
};

class kexBBox
{
public:
    kexBBox(void);
    explicit kexBBox(const kexVec3 &vMin, const kexVec3 &vMax);

    void                    Clear(void);
    kexVec3                 Center(void) const;
    float                   Radius(void) const;
    float                   Radius2D(void) const;
    void                    AddPoint(const kexVec3 &vec);
    bool                    PointInside(const kexVec3 &vec) const;
    bool                    IntersectingBox(const kexBBox &box) const;
    bool                    IntersectingBox2D(const kexBBox &box) const;
    float                   DistanceToPlane(kexPlane &plane);
    bool                    LineIntersect(const kexVec3 &start, const kexVec3 &end);
    void                    ToPoints(float *points) const;
    void                    ToVectors(kexVec3 *vectors) const;

    kexBBox                 operator+(const float radius) const;
    kexBBox                 &operator+=(const float radius);
    kexBBox                 operator+(const kexVec3 &vec) const;
    kexBBox                 &operator+=(const kexVec3 &vec);
    kexBBox                 operator-(const float radius) const;
    kexBBox                 operator-(const kexVec3 &vec) const;
    kexBBox                 &operator-=(const kexVec3 &vec);
    kexBBox                 &operator-=(const float radius);
    kexBBox                 operator*(const kexMatrix &matrix) const;
    kexBBox                 &operator*=(const kexMatrix &matrix);
    kexBBox                 operator*(const kexVec3 &vec) const;
    kexBBox                 &operator*=(const kexVec3 &vec);
    kexBBox                 operator*(const kexVec2 &vec) const;
    kexBBox                 &operator*=(const kexVec2 &vec);
    kexBBox                 &operator=(const kexBBox &bbox);
    kexVec3                 operator[](int index) const;
    kexVec3                 &operator[](int index);

    kexVec3                 min;
    kexVec3                 max;
};

#endif

