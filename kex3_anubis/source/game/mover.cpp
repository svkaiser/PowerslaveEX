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
//      Mover Objects. Movers can manipulate sectors and faces
//      to achieve effects such as opening doors, lifts, etc
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

//
// kexMover::UpdateFloorOrigin
//

void kexMover::UpdateFloorOrigin(void)
{
    mapVertex_t *v = kexGame::cLocal->World()->Vertices();
    
    origin = (v[sector->floorFace->vertexStart+0].origin +
              v[sector->floorFace->vertexStart+1].origin +
              v[sector->floorFace->vertexStart+2].origin +
              v[sector->floorFace->vertexStart+3].origin) / 4;
}

//
// kexMover::UpdateCeilingOrigin
//

void kexMover::UpdateCeilingOrigin(void)
{
    mapVertex_t *v = kexGame::cLocal->World()->Vertices();
    
    origin = (v[sector->ceilingFace->vertexStart+0].origin +
              v[sector->ceilingFace->vertexStart+1].origin +
              v[sector->ceilingFace->vertexStart+2].origin +
              v[sector->ceilingFace->vertexStart+3].origin) / 4;
}

//
// kexMover::PlayerInside
//

bool kexMover::PlayerInside(void)
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
// kexMover::CheckActorHeight
//

bool kexMover::CheckActorHeight(const float height)
{
    for(kexActor *actor = sector->actorList.Next();
        actor != NULL;
        actor = actor->SectorLink().Next())
    {
        if(!(actor->Flags() & AF_SOLID))
        {
            continue;
        }
        
        if(actor->Height() > height)
        {
            return false;
        }
    }
    
    return true;
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
    kexWorld *world = kexGame::cLocal->World();

    if(IsStale())
    {
        return;
    }

    switch(state)
    {
    case DS_UP:
        if(currentHeight >= destHeight)
        {
            StopLoopingSounds();

            if(waitDelay <= -1)
            {
                PlaySound("sounds/stonestop.wav");
                Remove();
                return;
            }

            state = DS_WAIT;
            currentTime = waitDelay;
            return;
        }
        else
        {
            PlayLoopingSound("sounds/stonemov.wav");
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
            if(type == 2)
            {
                world->ResetWallSwitchFromTag(world->Events()[sector->event].tag);
                sector->objectThinker = NULL;
            }

            StopLoopingSounds();
            PlaySound("sounds/stonestop.wav");

            Remove();
            return;
        }
        else
        {
            PlayLoopingSound("sounds/stonemov.wav");
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
    
    world->MoveSector(sector, true, currentHeight - lastHeight);
    UpdateCeilingOrigin();
    
    if(bDirection && state == DS_DOWN &&
       !CheckActorHeight(currentHeight - destHeight))
    {
        state = DS_UP;
        destHeight = raiseHeight;
    }
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
        raiseHeight = destHeight;
        break;

    case 2:
        waitDelay = 90;
        lip = 0;
        moveSpeed = 4;
        bDirection = true;
        destHeight = (float)sector->ceilingHeight - lip;
        sector->objectThinker = this;
        raiseHeight = destHeight;
        break;

    case 7:
        waitDelay = -1;
        lip = 0;
        moveSpeed = 4;
        bDirection = true;
        destHeight = (float)sector->ceilingHeight - lip;
        raiseHeight = destHeight;
        break;

    case 8:
    case 9:
        bDirection = false;
        waitDelay = -1;
        lip = 0;
        moveSpeed = 4;
        destHeight = (float)sector->floorHeight;
        raiseHeight = destHeight;
        break;

    default:
        kex::cSystem->Warning("kexDoor::Spawn: Unknown type (%i)\n", type);
        Remove();
        return;
    }

    sector->flags |= SF_SPECIAL;
    
    state = (bDirection ? DS_UP : DS_DOWN);
    currentTime = 0;
    baseHeight = -sector->ceilingFace->plane.d;
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

    if(IsStale())
    {
        return;
    }

    if(currentHeight <= destHeight)
    {
        StopLoopingSounds();
        PlaySound("sounds/stonestop.wav");
        Remove();
        return;
    }
    else
    {
        PlayLoopingSound("sounds/platstart.wav");
    }

    currentHeight -= moveSpeed;

    if(currentHeight < destHeight)
    {
        currentHeight = destHeight;
    }
    
    kexGame::cLocal->World()->MoveSector(sector, false, currentHeight - lastHeight);
    UpdateFloorOrigin();
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
// kexLift::Tick
//

void kexLift::Tick(void)
{
    float lastHeight = currentHeight;
    float moveAmount;

    if(IsStale())
    {
        return;
    }

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
            StopLoopingSounds();

            if(!bDirection)
            {
                PlaySound("sounds/stonestop.wav");

                Remove();
                return;
            }

            state = LS_WAIT;
            currentTime = waitDelay;
        }
        else
        {
            PlayLoopingSound("sounds/platstart.wav");
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
            StopLoopingSounds();

            if(bDirection)
            {
                PlaySound("sounds/stonestop.wav");

                Remove();
                return;
            }

            state = LS_WAIT;
            currentTime = waitDelay;
        }
        else
        {
            PlayLoopingSound("sounds/platstart.wav");
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
    UpdateFloorOrigin();
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
        baseHeight = sector->floorFace->plane.d;
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
    this->speedAccel = 0;
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
    case DPS_COUNTDOWN:
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
            PlaySound("sounds/platfall.wav");
            state = DPS_DOWN;
            sector->flags &= ~SF_SPECIAL;
            world->ResetWallSwitchFromTag(world->Events()[sector->event].tag);
            return;
        }

        currentHeight -= (moveSpeed + speedAccel);
        speedAccel += 1;

        if(currentHeight < destHeight)
        {
            currentHeight = destHeight;
        }
        break;
            
    case DPS_RAISE:
        if(currentHeight >= destHeight)
        {
            state = DPS_IDLE;
            sector->flags &= ~SF_SPECIAL;
            return;
        }
        
        currentHeight += moveSpeed;
        
        if(currentHeight > destHeight)
        {
            currentHeight = destHeight;
        }
        break;

    case DPS_IDLE:
        if(PlayerInside())
        {
            Start();
        }
        return;
            
    default:
        return;
    }

    moveAmount = currentHeight - lastHeight;
    
    world->MoveSector(linkedSector, true, moveAmount);
    world->MoveSector(sector, false, moveAmount);
    
    UpdateFloorOrigin();
}

//
// kexDropPad::Start
//

void kexDropPad::Start(void)
{
    if(state != DPS_IDLE)
    {
        return;
    }
    
    baseHeight = -linkedSector->ceilingFace->plane.d;
    destHeight = (float)linkedSector->floorHeight;
    currentDelay = 0;
    speedAccel = 0;
    currentHeight = baseHeight;
    
    state = DPS_COUNTDOWN;
    sector->flags |= SF_SPECIAL;
}

//
// kexDropPad::Reset
//

void kexDropPad::Reset(void)
{
    if(state != DPS_DOWN)
    {
        return;
    }
    
    baseHeight = -linkedSector->ceilingFace->plane.d;
    destHeight = (float)linkedSector->ceilingHeight;
    currentDelay = 0;
    speedAccel = 0;
    currentHeight = baseHeight;
    
    state = DPS_RAISE;
    sector->flags |= SF_SPECIAL;
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

    state = DPS_IDLE;

    linkedSector = &kexGame::cLocal->World()->Sectors()[sector->linkedSector];
    
    baseHeight = -linkedSector->ceilingFace->plane.d;
    destHeight = (float)linkedSector->floorHeight;
    currentDelay = 0;
    speedAccel = 0;
    currentHeight = baseHeight;
}

//-----------------------------------------------------------------------------
//
// kexFloatingPlatform
//
//-----------------------------------------------------------------------------

DECLARE_KEX_CLASS(kexFloatingPlatform, kexMover)

//
// kexFloatingPlatform::kexFloatingPlatform
//

kexFloatingPlatform::kexFloatingPlatform(void)
{
    this->moveSpeed = 2;
    this->moveHeight = 128;
    this->time = 0;
    this->angOffset = 0;
    this->currentHeight = 0;
}

//
// kexFloatingPlatform::~kexFloatingPlatform
//

kexFloatingPlatform::~kexFloatingPlatform(void)
{
}

//
// kexFloatingPlatform::Tick
//

void kexFloatingPlatform::Tick(void)
{
    kexWorld *world = kexGame::cLocal->World();
    float a = (float)time * (moveSpeed * kexMath::Deg2Rad(1));
    float amt = (kexMath::Cos(a+angOffset+kexMath::pi) * (moveHeight*0.5f));
    float move = amt - currentHeight;
    
    currentHeight = amt;
    
    world->MoveSector(linkedSector, true, move);
    world->MoveSector(sector, false, move);
    
    UpdateFloorOrigin();
    time++;
}

//
// kexFloatingPlatform::Spawn
//

void kexFloatingPlatform::Spawn(void)
{
    kexWorld *world = kexGame::cLocal->World();

    assert(sector != NULL);
    assert(sector->linkedSector >= 0);

    linkedSector = &world->Sectors()[sector->linkedSector];

    switch(type)
    {
    case 41:
        moveSpeed = 2;
        moveHeight = 128;
        break;

    case 43:
        moveSpeed = 2;
        moveHeight = 128;
        angOffset = kexMath::Deg2Rad(60);
        break;

    case 44:
        moveSpeed = 2;
        moveHeight = 128;
        angOffset = kexMath::Deg2Rad(120);
        break;

    case 45:
        moveSpeed = 2;
        moveHeight = 128;
        angOffset = kexMath::pi;
        break;

    case 46:
        moveSpeed = 2;
        moveHeight = 128;
        angOffset = kexMath::Deg2Rad(-60);
        break;

    case 47:
        moveSpeed = 2;
        moveHeight = 128;
        angOffset = kexMath::Deg2Rad(-120);
        break;

    case 60:
        moveSpeed = 2;
        moveHeight = 64;
        angOffset = kexMath::Deg2Rad(world->Events()[sector->event].params);
        break;

    case 65:
        moveSpeed = 2;
        moveHeight = 128;
        angOffset = kexMath::Deg2Rad(kexRand::CFloat() * 256);
        break;

    case 68:
        moveSpeed = 2;
        moveHeight = 512;
        angOffset = kexMath::Deg2Rad(world->Events()[sector->event].params);
        break;

    default:
        kex::cSystem->Warning("kexFloatingPlatform::Spawn: Unknown type (%i)\n", type);
        Remove();
        return;
    }

    baseHeight = -linkedSector->ceilingFace->plane.d;
}
