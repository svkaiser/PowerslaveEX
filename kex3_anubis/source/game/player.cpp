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
//      Player class
//

#include "kexlib.h"
#include "game.h"
#include "actor.h"
#include "player.h"

const int16_t kexPlayer::maxHealth = 200;
const int kexPlayer::maxAmmo[NUMPLAYERWEAPONS] =
{
    0, 60, 50, 20, 60, 20, 250, 15
};

//-----------------------------------------------------------------------------
//
// kexPuppet
//
//-----------------------------------------------------------------------------

const kexVec3 kexPuppet::accelSpeed(0.1f, 0.1f, 0.1f);
const kexVec3 kexPuppet::deaccelSpeed(0.1f, 0.1f, 0.1f);
const kexVec3 kexPuppet::forwardSpeed(0.2f, 0.2f, 0.2f);
const kexVec3 kexPuppet::backwardSpeed(0.2f, 0.2f, 0.2f);

DECLARE_CLASS(kexPuppet, kexActor)

//
// kexPuppet::kexPuppet
//

kexPuppet::kexPuppet(void)
{
    this->owner = NULL;
}

//
// kexPuppet::~kexPuppet
//

kexPuppet::~kexPuppet(void)
{
}

//
// kexPuppet::Tick
//

void kexPuppet::Tick(void)
{
    kexPlayerCmd *cmd = &owner->Cmd();
    
    yaw += cmd->Angles()[0];
    pitch += cmd->Angles()[1];
}

//
// kexPuppet::Spawn
//

void kexPuppet::Spawn(void)
{
    owner = kex::cGame->Player();
    kex::cGame->Player()->SetActor(this);
}

//
// kexPuppet::Accelerate
//

void kexPuppet::Accelerate(int direction, int axis)
{
    float time = 0;
    float lerp = 0;
    
    switch(axis)
    {
    case 0:
        lerp = acceleration.x;
        break;
    case 1:
        lerp = acceleration.y;
        break;
    case 2:
        lerp = acceleration.z;
        break;
    }
    
    if(direction == 1)
    {
        time = accelSpeed[axis];
        if(time > 1)
        {
            lerp = forwardSpeed[axis];
        }
        else
        {
            lerp = (forwardSpeed[axis] - lerp) * time + lerp;
        }
    }
    else if(direction == -1)
    {
        time = accelSpeed[axis];
        if(time > 1)
        {
            lerp = backwardSpeed[axis];
        }
        else
        {
            lerp = (backwardSpeed[axis] - lerp) * time + lerp;
        }
    }
    else
    {
        time = deaccelSpeed[axis];
        if(time > 1)
        {
            lerp = 0;
        }
        else
        {
            lerp = (0 - lerp) * time + lerp;
        }
    }
    
    switch(axis)
    {
    case 0:
        acceleration.x = lerp;
        break;
    case 1:
        acceleration.y = lerp;
        break;
    case 2:
        acceleration.z = lerp;
        break;
    }
}

//-----------------------------------------------------------------------------
//
// kexPlayer
//
//-----------------------------------------------------------------------------

//
// kexPlayer::kexPlayer
//

kexPlayer::kexPlayer(void)
{
    Reset();
}

//
// kexPlayer::~kexPlayer
//

kexPlayer::~kexPlayer(void)
{
}

//
// kexPlayer::Reset
//

void kexPlayer::Reset(void)
{
    health = maxHealth;
    ankahs = 0;
    actor = NULL;

    cmd.Reset();

    bob = 0;
    weaponBob.Clear();

    currentWeapon = PW_MACHETE;
    weaponState = -1;
    weaponFrame = 0;
    weaponTicks = 0;

    memset(weapons, 0, NUMPLAYERWEAPONS);
    weapons[PW_MACHETE] = true;

    for(int i = 0; i < NUMPLAYERWEAPONS; ++i)
    {
        ammo[i] = maxAmmo[i];
    }

    artifacts = 0;
    keys = 0;
    transmitter = 0;
    teamDolls = 0;
}
