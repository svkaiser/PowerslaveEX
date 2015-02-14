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
//      Mover Object
//

#include "kexlib.h"
#include "game.h"
#include "mover.h"

//-----------------------------------------------------------------------------
//
// kexMover
//
//-----------------------------------------------------------------------------

DECLARE_ABSTRACT_KEX_CLASS(kexMover, kexGameObject)

//
// kexMover::kexMover
//

kexMover::kexMover(void)
{
    this->type = -1;
    this->sector = NULL;
}

//
// kexMover::~kexMover
//

kexMover::~kexMover(void)
{
}

//
// kexMover::Tick
//

void kexMover::Tick(void)
{
}

//
// kexMover::SetSector
//

void kexMover::SetSector(mapSector_t *s)
{
    sector = s;
}

//-----------------------------------------------------------------------------
//
// kexDoor
//
//-----------------------------------------------------------------------------

DECLARE_KEX_CLASS(kexDoor, kexMover)

//
// kexDoor::kexDoor
//

kexDoor::kexDoor(void)
{
    this->waitDelay = 90;
    this->lip = 0;
    this->moveSpeed = 4;
    this->bDirection = true;
    this->destHeight = 0;
    this->state = DS_IDLE;
}

//
// kexDoor::~kexDoor
//

kexDoor::~kexDoor(void)
{
}

//
// kexDoor::Remove
//

void kexDoor::Remove(void)
{
    sector->flags &= ~SF_SPECIAL;
    kexGameObject::Remove();
}

//
// kexDoor::Tick
//

void kexDoor::Tick(void)
{
    mapVertex_t *vertices = kexGame::cLocal->World()->Vertices();
    float lastHeight = currentHeight;
    float moveAmount;

    switch(state)
    {
    case DS_UP:
        if(currentHeight >= destHeight)
        {
            if(waitDelay <= -1)
            {
                Remove();
                return;
            }

            state = DS_WAIT;
            currentTime = waitDelay;
            return;
        }

        currentHeight += moveSpeed;

        if(currentHeight > destHeight)
        {
            currentHeight = destHeight;
        }
        break;

    case DS_DOWN:
        if(currentHeight <= destHeight)
        {
            Remove();
            return;
        }

        currentHeight -= moveSpeed;

        if(currentHeight < destHeight)
        {
            currentHeight = destHeight;
        }
        break;

    case DS_WAIT:
        currentTime -= 0.5f;
        if(currentTime <= 0)
        {
            state = DS_DOWN;
            destHeight = baseHeight;
        }
        return;

    default:
        return;
    }

    moveAmount = currentHeight - lastHeight;
    
    for(int i = sector->faceStart; i < sector->faceEnd+3; ++i)
    {
        mapFace_t *face = &kexGame::cLocal->World()->Faces()[i];
        
        if(face->flags & FF_PORTAL && face->sector >= 0)
        {
            mapSector_t *s = &kexGame::cLocal->World()->Sectors()[face->sector];
            
            for(int j = s->faceStart; j < s->faceEnd+3; ++j)
            {
                mapFace_t *f = &kexGame::cLocal->World()->Faces()[j];
                
                if(f->tag != sector->event)
                {
                    continue;
                }
                
                if(f->flags & FF_PORTAL)
                {
                    continue;
                }
                
                for(int k = f->vertStart; k <= f->vertEnd; ++k)
                {
                    mapVertex_t *v = &vertices[k];
                    v->origin.z += moveAmount;
                }
                
                vertices[f->vertexStart+0].origin.z += moveAmount;
                vertices[f->vertexStart+1].origin.z += moveAmount;
                vertices[f->vertexStart+2].origin.z += moveAmount;
                vertices[f->vertexStart+3].origin.z += moveAmount;
            }
            
            continue;
        }
        
        if(i != sector->faceEnd+1)
        {
            continue;
        }
        
        for(int j = face->vertStart; j <= face->vertEnd; ++j)
        {
            mapVertex_t *v = &vertices[j];
            v->origin.z += moveAmount;
        }
        
        vertices[face->vertexStart+0].origin.z += moveAmount;
        vertices[face->vertexStart+1].origin.z += moveAmount;
        vertices[face->vertexStart+2].origin.z += moveAmount;
        vertices[face->vertexStart+3].origin.z += moveAmount;
        
        kexGame::cLocal->World()->UpdateFacePlaneAndBounds(face);
    }
    
    kexGame::cLocal->World()->UpdateSectorBounds(sector);
}

//
// kexDoor::Spawn
//

void kexDoor::Spawn(void)
{
    assert(sector != NULL);

    switch(type)
    {
    case 1:
        waitDelay = 90;
        lip = 0;
        moveSpeed = 4;
        bDirection = true;
        break;

    case 7:
        waitDelay = -1;
        lip = 0;
        moveSpeed = 4;
        bDirection = true;
        break;

    default:
        break;
    }

    sector->flags |= SF_SPECIAL;
    
    state = DS_UP;
    currentTime = 0;
    baseHeight = (float)sector->floorHeight;
    currentHeight = baseHeight;
    destHeight = (float)sector->ceilingHeight - lip;
}
