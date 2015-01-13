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
#include "cmodel.h"
#include "player.h"

#define PMOVE_FRICTION  0.9375f
#define PMOVE_MIN       0.125f
#define PMOVE_SPEED     0.976f

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
    kexVec3 forward, right;
    
    yaw += cmd->Angles()[0];
    pitch += cmd->Angles()[1];

    kexVec3::ToAxis(&forward, NULL, &right, yaw, pitch, 0);

    velocity.x *= PMOVE_FRICTION;
    velocity.y *= PMOVE_FRICTION;
    velocity.z *= PMOVE_FRICTION;

    if(kexMath::Fabs(velocity.x) < PMOVE_MIN)
    {
        velocity.x = 0;
    }

    if(kexMath::Fabs(velocity.y) < PMOVE_MIN)
    {
        velocity.y = 0;
    }

    if(kexMath::Fabs(velocity.z) < PMOVE_MIN)
    {
        velocity.z = 0;
    }

    if(cmd->Buttons() & BC_FORWARD)
    {
        velocity.x += forward.x * PMOVE_SPEED;
        velocity.y += forward.y * PMOVE_SPEED;
        velocity.z += forward.z * PMOVE_SPEED;
    }

    if(cmd->Buttons() & BC_BACKWARD)
    {
        velocity.x -= forward.x * PMOVE_SPEED;
        velocity.y -= forward.y * PMOVE_SPEED;
        velocity.z -= forward.z * PMOVE_SPEED;
    }

    //velocity.z -= 1 * PMOVE_SPEED;

    if(cmd->Buttons() & BC_STRAFELEFT)
    {
        velocity.x -= right.x * PMOVE_SPEED;
        velocity.y -= right.y * PMOVE_SPEED;
    }

    if(cmd->Buttons() & BC_STRAFERIGHT)
    {
        velocity.x += right.x * PMOVE_SPEED;
        velocity.y += right.y * PMOVE_SPEED;
    }
    
    if(!kex::cGame->CModel()->MoveActor(this))
    {
        velocity.Clear();
    }
    
    LinkArea();
}

//
// kexPuppet::Spawn
//

void kexPuppet::Spawn(void)
{
    owner = kex::cGame->Player();
    kex::cGame->Player()->SetActor(this);

    radius = 96;
    height = 96;
    health = 200;
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
