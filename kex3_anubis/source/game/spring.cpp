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
//      Spring physics
//

#include "kexlib.h"
#include "game.h"
#include "spring.h"

//
// kexSpring::kexSpring
//

kexSpring::kexSpring(void)
{
    this->constant = 0.05f;
    this->damping = 0.9f;
    this->origin = NULL;
}

//
// kexSpring::~kexSpring
//

kexSpring::~kexSpring(void)
{
}

//
// kexSpring::SetSpring
//

void kexSpring::SetSpring(const kexVec3 &force)
{
    velocity += force;
}

//
// kexSpring::Update
//

void kexSpring::Update(void)
{
    if(origin == NULL)
    {
        return;
    }
    
    kexVec3 x = (*origin - initialOrigin) + velocity;
    kexVec3 accel = x * -constant;
    
    *origin += (velocity * damping);
    velocity += accel;
}
