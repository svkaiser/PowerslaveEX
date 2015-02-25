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
// kexMover::Remove
//

void kexMover::Remove(void)
{
    sector->flags &= ~SF_SPECIAL;
    kexGameObject::Remove();
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
// kexDoor::Tick
//

void kexDoor::Tick(void)
{
    float lastHeight = currentHeight;

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
    
    kexGame::cLocal->World()->MoveSector(sector, true, currentHeight - lastHeight);
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
    case 3:
    case 4:
    case 5:
    case 6:
        waitDelay = 90;
        lip = 0;
        moveSpeed = 4;
        bDirection = true;
        destHeight = (float)sector->ceilingHeight - lip;
        break;

    case 7:
        waitDelay = -1;
        lip = 0;
        moveSpeed = 4;
        bDirection = true;
        destHeight = (float)sector->ceilingHeight - lip;
        break;

    case 8:
        bDirection = false;
        waitDelay = -1;
        lip = 0;
        moveSpeed = 4;
        destHeight = (float)sector->floorHeight;
        break;

    default:
        kex::cSystem->Warning("kexDoor::Spawn: Unknown type (%i)\n", type);
        Remove();
        return;
    }

    sector->flags |= SF_SPECIAL;
    
    state = (bDirection ? DS_UP : DS_DOWN);
    currentTime = 0;
    baseHeight = -kexGame::cLocal->World()->Faces()[sector->faceEnd+1].plane.d;
    currentHeight = baseHeight;
}

//-----------------------------------------------------------------------------
//
// kexFloor
//
//-----------------------------------------------------------------------------

DECLARE_KEX_CLASS(kexFloor, kexMover)

//
// kexFloor::kexFloor
//

kexFloor::kexFloor(void)
{
    this->lip = 0;
    this->moveSpeed = 4;
    this->destHeight = 0;
}

//
// kexFloor::~kexFloor
//

kexFloor::~kexFloor(void)
{
}

//
// kexFloor::Tick
//

void kexFloor::Tick(void)
{
    float lastHeight = currentHeight;

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
    
    kexGame::cLocal->World()->MoveSector(sector, false, currentHeight - lastHeight);
}

//
// kexFloor::Spawn
//

void kexFloor::Spawn(void)
{
    assert(sector != NULL);

    switch(type)
    {
    case 22:
        lip = 0;
        moveSpeed = 4;
        break;
    case 23:
        lip = 0;
        moveSpeed = 2;
        break;

    default:
        kex::cSystem->Warning("kexFloor::Spawn: Unknown type (%i)\n", type);
        Remove();
        return;
    }

    sector->flags |= SF_SPECIAL;
    
    baseHeight = sector->floorFace->plane.d;
    currentHeight = baseHeight;
    destHeight = (float)sector->floorHeight - lip;
}

//-----------------------------------------------------------------------------
//
// kexLift
//
//-----------------------------------------------------------------------------

DECLARE_KEX_CLASS(kexLift, kexMover)

//
// kexLift::kexLift
//

kexLift::kexLift(void)
{
    this->waitDelay = 90;
    this->triggerDelay = 60;
    this->lip = 0;
    this->moveSpeed = 4;
    this->bDirection = true;
    this->destHeight = 0;
    this->state = LS_IDLE;
}

//
// kexLift::~kexLift
//

kexLift::~kexLift(void)
{
}

//
// kexLift::PlayerInside
//

bool kexLift::PlayerInside(void)
{
    for(kexActor *actor = sector->actorList.Next();
        actor != NULL;
        actor = actor->SectorLink().Next())
    {
        if(actor->InstanceOf(&kexPuppet::info))
        {
            return true;
        }
    }

    return false;
}

//
// kexLift::Tick
//

void kexLift::Tick(void)
{
    float lastHeight = currentHeight;
    float moveAmount;

    switch(state)
    {
    case LS_IDLE:
        if(PlayerInside())
        {
            currentDelay += 0.5f;
            if(currentDelay >= triggerDelay)
            {
                state = (!bDirection ? LS_DOWN : LS_UP);
                currentDelay = 0;
            }
        }
        return;

    case LS_UP:
        if(currentHeight >= destHeight)
        {
            if(!bDirection)
            {
                Remove();
                return;
            }

            state = LS_WAIT;
            currentTime = waitDelay;
        }

        currentHeight += moveSpeed;

        if(currentHeight > destHeight)
        {
            currentHeight = destHeight;
        }
        break;

    case LS_DOWN:
        if(currentHeight <= destHeight)
        {
            if(bDirection)
            {
                Remove();
                return;
            }

            state = LS_WAIT;
            currentTime = waitDelay;
        }

        currentHeight -= moveSpeed;

        if(currentHeight < destHeight)
        {
            currentHeight = destHeight;
        }
        break;

    case LS_WAIT:
        currentTime -= 0.5f;
        if(currentTime <= 0)
        {
            state = (bDirection ? LS_DOWN : LS_UP);
            destHeight = baseHeight;
        }
        return;

    default:
        return;
    }
    
    moveAmount = currentHeight - lastHeight;
    kexGame::cLocal->World()->MoveSector(sector, false, moveAmount);
}

//
// kexLift::Spawn
//

void kexLift::Spawn(void)
{
    assert(sector != NULL);

    switch(type)
    {
    case 21:
    case 22:
        waitDelay = 90;
        triggerDelay = 60;
        lip = 0;
        moveSpeed = 4;
        bDirection = false;
        baseHeight = kexGame::cLocal->World()->Faces()[sector->faceEnd+2].plane.d;
        destHeight = (float)sector->floorHeight - lip;
        break;

    case 24:
        waitDelay = 90;
        triggerDelay = 60;
        lip = 0;
        moveSpeed = 4;
        bDirection = true;
        baseHeight = (float)sector->floorHeight;
        destHeight = kexGame::cLocal->World()->GetHighestSurroundingFloor(sector);
        break;

    default:
        kex::cSystem->Warning("kexLift::Spawn: Unknown type (%i)\n", type);
        Remove();
        return;
    }

    sector->flags |= SF_SPECIAL;
    
    state = InstanceOf(&kexLiftImmediate::info) ? (!bDirection ? LS_DOWN : LS_UP) : LS_IDLE;
    currentTime = 0;
    currentDelay = 0;
    currentHeight = baseHeight;
}

//-----------------------------------------------------------------------------
//
// kexLiftImmediate
//
//-----------------------------------------------------------------------------

DECLARE_KEX_CLASS(kexLiftImmediate, kexLift)

//
// kexLiftImmediate::kexLiftImmediate
//

kexLiftImmediate::kexLiftImmediate(void)
{
}

//
// kexLiftImmediate::~kexLiftImmediate
//

kexLiftImmediate::~kexLiftImmediate(void)
{
}

//-----------------------------------------------------------------------------
//
// kexDropPad
//
//-----------------------------------------------------------------------------

DECLARE_KEX_CLASS(kexDropPad, kexMover)

//
// kexDropPad::kexDropPad
//

kexDropPad::kexDropPad(void)
{
    this->moveSpeed = 4;
    this->destHeight = 0;
    this->triggerDelay = 30;
    this->state = DPS_IDLE;
}

//
// kexDropPad::~kexDropPad
//

kexDropPad::~kexDropPad(void)
{
}

//
// kexDropPad::Tick
//

void kexDropPad::Tick(void)
{
    float lastHeight = currentHeight;
    float moveAmount;
    kexWorld *world = kexGame::cLocal->World();

    switch(state)
    {
    case DPS_IDLE:
        currentDelay += 0.5f;
        if(currentDelay >= triggerDelay)
        {
            state = DPS_FALLING;
            currentDelay = 0;
        }
        return;

    case DPS_FALLING:
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
    }

    moveAmount = currentHeight - lastHeight;
    world->MoveSector(linkedSector, true, moveAmount);
    world->MoveSector(sector, false, moveAmount);
}

//
// kexDropPad::Spawn
//

void kexDropPad::Spawn(void)
{
    assert(sector != NULL);

    switch(type)
    {
    case 48:
        moveSpeed = 4;
        triggerDelay = 30;
        break;

    default:
        kex::cSystem->Warning("kexDropPad::Spawn: Unknown type (%i)\n", type);
        Remove();
        return;
    }

    sector->flags |= SF_SPECIAL;

    linkedSector = &kexGame::cLocal->World()->Sectors()[sector->linkedSector];
    
    baseHeight = -linkedSector->ceilingFace->plane.d;
    destHeight = (float)linkedSector->floorHeight;
    currentDelay = 0;
    currentHeight = baseHeight;
}

