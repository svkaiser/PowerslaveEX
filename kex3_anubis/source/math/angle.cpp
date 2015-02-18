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
//      Angle operations
//

#include "mathlib.h"

#define FULLCIRCLE  (kexMath::pi * 2)

//
// kexAngle::kexAngle
//

kexAngle::kexAngle(void)
{
    an = 0;
}

//
// kexAngle::kexAngle
//

kexAngle::kexAngle(const float f)
{
    an = f;
    Clamp();
}

//
// kexAngle::Clamp
//

void kexAngle::Clamp(void)
{
    if(an < -kexMath::pi) for(; an < -kexMath::pi; an = an + FULLCIRCLE);
    if(an >  kexMath::pi) for(; an >  kexMath::pi; an = an - FULLCIRCLE);
}

//
// kexAngle::Clamp
//

void kexAngle::Clamp(float &f)
{
    if(f < -kexMath::pi) for(; f < -kexMath::pi; f = f + FULLCIRCLE);
    if(f >  kexMath::pi) for(; f >  kexMath::pi; f = f - FULLCIRCLE);
}

//
// kexAngle::Clamp360
//

void kexAngle::Clamp360(float &f)
{
    Clamp(f);
    f = (f > 0 ? f : (FULLCIRCLE + f)) * 360 / FULLCIRCLE;
}

//
// kexAngle::operator+
//

kexAngle kexAngle::operator+(const float f) const
{
    return kexAngle(an + f);
}

//
// kexAngle::operator+
//

kexAngle &kexAngle::operator+=(const float f)
{
    float fa = f;
    Clamp(fa);
    an += fa;
    Clamp();
    return *this;
}

//
// kexAngle::operator-
//

kexAngle kexAngle::operator-(const float f) const
{
    float fa = f;
    Clamp(fa);
    return kexAngle(an - fa);
}

//
// kexAngle::operator-
//

kexAngle &kexAngle::operator-=(const float f)
{
    float fa = f;
    Clamp(fa);
    an -= fa;
    Clamp();
    return *this;
}

//
// kexAngle::operator+
//

kexAngle kexAngle::operator+(const kexAngle &ang) const
{
    return kexAngle(an + ang.an);
}

//
// kexAngle::operator+
//

kexAngle &kexAngle::operator+=(const kexAngle &ang)
{
    an += ang.an;
    Clamp();
    return *this;
}

//
// kexAngle::operator-
//

kexAngle kexAngle::operator-(const kexAngle &ang) const
{
    return kexAngle(an - ang.an);
}

//
// kexAngle::operator-
//

kexAngle &kexAngle::operator-=(const kexAngle &ang)
{
    an -= ang.an;
    Clamp();
    return *this;
}

//
// kexAngle::Diff
//

float kexAngle::Diff(const float f)
{
    float an1;
    float an2;
    float fa = f;

    Clamp(fa);

    an2 = 0.0f;

    if(an <= fa)
    {
        an1 = fa + FULLCIRCLE;
        if(an - fa > an1 - an)
        {
            an2 = an - an1;
        }
        else
        {
            an2 = an - fa;
        }
    }
    else
    {
        an1 = fa - FULLCIRCLE;
        if(fa - an <= an - an1)
        {
            an2 = an - fa;
        }
        else
        {
            an2 = an - an1;
        }
    }

    Clamp(an2);
    return an2;
}

//
// kexAngle::Diff
//

float kexAngle::Diff(const kexAngle &ang)
{
    return Diff(ang.an);
}

//
// kexAngle::operator=
//

kexAngle &kexAngle::operator=(const float f)
{
    an = f;
    Clamp();
    return *this;
}

//
// kexAngle::operator=
//

kexAngle &kexAngle::operator=(const kexAngle &ang)
{
    an = ang.an;
    return *this;
}

//
// kexAngle::operator-
//

kexAngle kexAngle::operator-(void) const
{
    return kexAngle(-an);
}
