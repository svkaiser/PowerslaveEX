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
#include "playLoop.h"
#include "actor.h"
#include "cmodel.h"
#include "player.h"

#define PMOVE_FRICTION          0.9375f
#define PMOVE_MIN               0.125f
#define PMOVE_SPEED             0.976f
#define PMOVE_SPEED_JUMP        10.25f
#define PMOVE_SPEED_FALL        0.75f
#define PMOVE_MAX_JUMPTICKS     12

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
    this->playerFlags = 0;
    this->jumpTicks = 0;
}

//
// kexPuppet::~kexPuppet
//

kexPuppet::~kexPuppet(void)
{
}

//
// kexPuppet::GroundMove
//

void kexPuppet::GroundMove(kexPlayerCmd *cmd)
{
    kexVec3 forward, right;
    
    yaw += cmd->Angles()[0];
    pitch += cmd->Angles()[1];
    roll -= (cmd->Angles()[0] * 0.25f);
    
    kexMath::Clamp(pitch.an, kexMath::Deg2Rad(-90), kexMath::Deg2Rad(90));
    kexMath::Clamp(roll.an, -0.1f, 0.1f);
    
    kexVec3::ToAxis(&forward, NULL, &right, yaw, 0, 0);

    velocity.x *= PMOVE_FRICTION;
    velocity.y *= PMOVE_FRICTION;

    if(cmd->Buttons() & BC_JUMP)
    {
        if(!(playerFlags & PF_USERJUMPED))
        {
            if(!(playerFlags & PF_JUMPING))
            {
                playerFlags |= PF_JUMPING;
            }

            if(jumpTicks < PMOVE_MAX_JUMPTICKS)
            {
                velocity.z = PMOVE_SPEED_JUMP;
                jumpTicks++;
            }
            else
            {
                playerFlags |= PF_USERJUMPED;
            }
        }
    }
    else
    {
        if(kexMath::Fabs(velocity.z) >= PMOVE_MIN || playerFlags & PF_JUMPING)
        {
            playerFlags |= PF_USERJUMPED;
        }
        else if(playerFlags & PF_USERJUMPED)
        {
            playerFlags &= ~PF_USERJUMPED;
        }
    }
    
    if(origin.z > floorHeight)
    {
        velocity.z -= PMOVE_SPEED_FALL;
    }

    if(kexMath::Fabs(velocity.x) < PMOVE_MIN)
    {
        velocity.x = 0;
    }

    if(kexMath::Fabs(velocity.y) < PMOVE_MIN)
    {
        velocity.y = 0;
    }

    if(cmd->Buttons() & BC_FORWARD)
    {
        velocity.x += forward.x * PMOVE_SPEED;
        velocity.y += forward.y * PMOVE_SPEED;
    }

    if(cmd->Buttons() & BC_BACKWARD)
    {
        velocity.x -= forward.x * PMOVE_SPEED;
        velocity.y -= forward.y * PMOVE_SPEED;
    }

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

    if((origin.z + height) + velocity.z >= ceilingHeight)
    {
        origin.z = ceilingHeight - height;
        velocity.z = -1;
    }
    
    if(origin.z + velocity.z <= floorHeight)
    {
        if(velocity.z < -3.5f)
        {
            owner->LandTime() = velocity.z;
        }

        origin.z = floorHeight;
        velocity.z = 0;
        jumpTicks = 0;
        playerFlags &= ~PF_JUMPING;
    }
    
    if(!kex::cGame->CModel()->MoveActor(this))
    {
        velocity.Clear();
    }
}

//
// kexPuppet::FlyMove
//

void kexPuppet::FlyMove(kexPlayerCmd *cmd)
{
    kexVec3 forward, right;
    
    yaw += cmd->Angles()[0];
    pitch += cmd->Angles()[1];

    kexVec3::ToAxis(&forward, NULL, &right, yaw, pitch, 0);

    velocity *= PMOVE_FRICTION;

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
        velocity += (forward * PMOVE_SPEED);
    }

    if(cmd->Buttons() & BC_BACKWARD)
    {
        velocity -= (forward * PMOVE_SPEED);
    }

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
    
    if(!(playerFlags & PF_NOCLIP))
    {
        if(!kex::cGame->CModel()->MoveActor(this))
        {
            velocity.Clear();
        }
    }
    else
    {
        origin += velocity;
        LinkArea();
    }
}

//
// kexPuppet::Tick
//

void kexPuppet::Tick(void)
{
    kexPlayerCmd *cmd = &owner->Cmd();
    
    roll = (0 - roll) * 0.25f + roll;
    
    if(playerFlags & (PF_NOCLIP|PF_FLY))
    {
        FlyMove(cmd);
    }
    else
    {
        GroundMove(cmd);
    }
}

//
// kexPuppet::Spawn
//

void kexPuppet::Spawn(void)
{
    owner = kex::cGame->Player();
    kex::cGame->Player()->SetActor(this);

    radius      = 96;
    height      = 160;
    stepHeight  = 48;
    health      = 200;
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
    bobTime = 0;
    bobSpeed = 0;
    weaponBob.Clear();
    landTime = 0;

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

//
// kexPlayer::Tick
//

void kexPlayer::Tick(void)
{
    if(cmd.Buttons() & (BC_FORWARD|BC_BACKWARD|BC_STRAFELEFT|BC_STRAFERIGHT))
    {
        bobSpeed = (0.1475f - bobSpeed) * 0.35f + bobSpeed;
        bob = kexMath::Sin(bobTime) * 7.0f;
        bobTime += bobSpeed;
    }
    else
    {
        bobSpeed = 0;
        bobTime = 0;
        bob = (0 - bob) * 0.1f + bob;
    }

    if(landTime < 0)
    {
        landTime += 2;
        if(landTime >= 0)
        {
            landTime = 0;
        }
    }
}
