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
//      Actor Object
//

#include "kexlib.h"
#include "actor.h"
#include "world.h"
#include "cmodel.h"

DECLARE_CLASS(kexActor, kexGameObject)

//
// kexActor::kexActor
//

kexActor::kexActor(void)
{
    this->link.SetData(this);
    this->bounds.min.Set(-32, -32, -32);
    this->bounds.max.Set(32, 32, 32);
    this->type = AT_INVALID;
    this->health = 100;
    this->radius = 16;
    this->height = 32;
    this->stepHeight = 16;
    this->flags = 0;
    this->areaLink.link.SetData(this);
    this->areaLink.node = NULL;
}

//
// kexActor::~kexActor
//

kexActor::~kexActor(void)
{
}

//
// kexActor::Tick
//

void kexActor::Tick(void)
{
}

//
// kexActor::Spawn
//

void kexActor::Spawn(void)
{
}

//
// kexActor::FindSector
//

bool kexActor::FindSector(const kexVec3 &pos)
{
    mapSector_t *sector;
    
    for(unsigned int i = 0; i < kex::cGame->World()->NumSectors(); ++i)
    {
        sector = &kex::cGame->World()->Sectors()[i];
        
        if(kex::cGame->CModel()->PointWithinSectorEdges(pos, sector))
        {
            SetSector(sector);
            return true;
        }
    }
    
    return false;
}

//
// kexActor::LinkArea
//

void kexActor::LinkArea(void)
{
    kexBBox box;

    if(IsStale())
    {
        return;
    }

    box.min.Set(-radius, -radius, 0);
    box.max.Set(radius, radius, 0);
    box.min += origin;
    box.max += origin;

    areaLink.Link(kex::cGame->World()->AreaNodes(), box);
}

//
// kexActor::UnlinkArea
//

void kexActor::UnlinkArea(void)
{
    areaLink.UnLink();
}
