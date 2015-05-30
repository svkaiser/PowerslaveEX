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
// DESCRIPTION:
//      Plane operations
//

#include "mathlib.h"

//
// kexPlane::kexPlane
//

kexPlane::kexPlane(void)
{
    this->a = 0;
    this->b = 0;
    this->c = 0;
    this->d = 0;
}

//
// kexPlane::kexPlane
//

kexPlane::kexPlane(const float a, const float b, const float c, const float d)
{
    this->a = a;
    this->b = b;
    this->c = c;
    this->d = d;
}

//
// kexPlane::kexPlane
//

kexPlane::kexPlane(const kexVec3 &pt1, const kexVec3 &pt2, const kexVec3 &pt3)
{
    SetNormal(pt1, pt2, pt3);
    this->d = kexVec3::Dot(pt1, Normal());
}

//
// kexPlane::kexPlane
//

kexPlane::kexPlane(const kexVec3 &normal, const kexVec3 &point)
{
    this->a = normal.x;
    this->b = normal.y;
    this->c = normal.z;
    this->d = point.Dot(normal);
}

//
// kexPlane::kexPlane
//

kexPlane::kexPlane(const kexPlane &plane)
{
    this->a = plane.a;
    this->b = plane.b;
    this->c = plane.c;
    this->d = plane.d;
}

//
// kexPlane::SetNormal
//

kexPlane &kexPlane::SetNormal(const kexVec3 &normal)
{
    Normal() = normal;
    return *this;
}

//
// kexPlane::SetNormal
//

kexPlane &kexPlane::SetNormal(const kexVec3 &pt1, const kexVec3 &pt2, const kexVec3 &pt3)
{
    Normal() = (pt2 - pt1).Cross(pt3 - pt2).Normalize();
    return *this;
}

//
// kexPlane::Normal
//

kexVec3 const &kexPlane::Normal(void) const
{
    return *reinterpret_cast<const kexVec3*>(&a);
}

//
// kexPlane::Normal
//

kexVec3 &kexPlane::Normal(void)
{
    return *reinterpret_cast<kexVec3*>(&a);
}

//
// kexPlane::Dot
//

float kexPlane::Dot(const kexVec3 &point)
{
    return point.Dot(Normal());
}

//
// kexPlane::SetDistance
//

kexPlane &kexPlane::SetDistance(const kexVec3 &point)
{
    this->d = point.Dot(Normal());
    return *this;
}

//
// kexPlane::IsFacing
//

bool kexPlane::IsFacing(const float yaw)
{
    return -kexMath::Sin(yaw) * a + -kexMath::Cos(yaw) * b < 0;
}

//
// kexPlane::ToYaw
//

float kexPlane::ToYaw(void)
{
    float d = Normal().Unit();

    if(d != 0)
    {
        float phi;
        phi = kexMath::ACos(b / d);
        if(a <= 0)
        {
            phi = -phi;
        }

        return phi;
    }

    return 0;
}

//
// kexPlane::ToPitch
//

float kexPlane::ToPitch(void)
{
    return kexMath::ACos(kexVec3::vecUp.Dot(Normal()));
}

//
// kexPlane::ToQuat
//

kexQuat kexPlane::ToQuat(void)
{
    kexVec3 cross = kexVec3::vecUp.Cross(Normal()).Normalize();
    return kexQuat(kexMath::ACos(kexVec3::vecUp.Dot(Normal())), cross);
}

//
// kexPlane::ToVec4
//

kexVec4 const &kexPlane::ToVec4(void) const
{
    return *reinterpret_cast<const kexVec4*>(&a);
}

//
// kexPlane::ToVec4
//

kexVec4 &kexPlane::ToVec4(void)
{
    return *reinterpret_cast<kexVec4*>(&a);
}

//
// kexPlane::BestAxis
//

const kexPlane::planeAxis_t kexPlane::BestAxis(void) const
{
    float na = kexMath::Fabs(a);
    float nb = kexMath::Fabs(b);
    float nc = kexMath::Fabs(c);

    // figure out what axis the plane lies on
    if(na >= nb && na >= nc)
    {
        return AXIS_YZ;
    }
    else if(nb >= na && nb >= nc)
    {
        return AXIS_XZ;
    }

    return AXIS_XY;
}

//
// kexPlane::PointOnSide
//

const kexPlane::planeSide_t kexPlane::PointOnSide(const kexVec3 &point)
{
    float pd = Distance(point);

    if(pd > 0) return PSIDE_FRONT;
    if(pd < 0) return PSIDE_BACK;

    return PSIDE_ON;
}

//
// kexPlane::Distance
//

float kexPlane::Distance(const kexVec3 &point)
{
    return Dot(point) - d;
}

//
// kexPlane::operator[]
//

float kexPlane::operator[](int index) const
{
    assert(index >= 0 && index < 4);
    return (&a)[index];
}

//
// kexPlane::operator[]
//

float &kexPlane::operator[](int index)
{
    assert(index >= 0 && index < 4);
    return (&a)[index];
}

//
// kexPlane::operator=
//

kexPlane &kexPlane::operator=(const kexPlane &p)
{
    a = p.a;
    b = p.b;
    c = p.c;
    d = p.d;
    
    return *this;
}
