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

#define PMOVE_FRICTION          0.9375f
#define PMOVE_MIN               0.125f
#define PMOVE_SPEED             0.976f
#define PMOVE_SPEED_JUMP        10.25f
#define PMOVE_SPEED_FALL        0.75f
#define PMOVE_MAX_JUMPTICKS     13

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
    mapSector_t *oldSector;
    
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

    oldSector = sector;
    
    if(!kexGame::cLocal->CModel()->MoveActor(this))
    {
        velocity.Clear();
    }

    if(oldSector == sector && velocity.z <= 0 && sector->floorFace->plane.IsFacing(velocity.ToYaw()))
    {
        float diff = origin.z - floorHeight;

        if(diff > 0 && diff <= 8)
        {
            origin.z = floorHeight;
        }
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
        if(!kexGame::cLocal->CModel()->MoveActor(this))
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
    
    roll = (0 - roll) * 0.35f + roll;
    
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
    owner = kexGame::cLocal->Player();
    kexGame::cLocal->Player()->SetActor(this);

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
    weaponAnim = NULL;

    cmd.Reset();

    bob = 0;
    bobTime = 0;
    bobSpeed = 0;
    weaponBob_x = 0;
    weaponBob_y = 0;
    weaponBobTime = 0;
    landTime = 0;
    stepViewZ = 0;

    currentWeapon = PW_MACHETE;
    pendingWeapon = currentWeapon;
    weaponState = WS_RAISE;
    weaponFrame = 0;
    weaponTicks = 0;

    memset(weapons, 0, NUMPLAYERWEAPONS);
    weapons[PW_MACHETE] = true;

    // TEMP
    weapons[PW_PISTOL] = true;
    weapons[PW_M60] = true;
    weapons[PW_BOMBS] = true;

    for(int i = 0; i < NUMPLAYERWEAPONS; ++i)
    {
        ammo[i] = maxAmmo[i];
    }

    artifacts = 0;
    keys = 0;
    transmitter = 0;
    teamDolls = 0;

    weaponAnim = kexGame::cLocal->WeaponInfo(currentWeapon)->raise;
}

//
// kexPlayer::UpdateWeaponBob
//

void kexPlayer::UpdateWeaponBob(void)
{
    float movement = actor->Velocity().ToVec2().Unit();

    if((movement > 0 || landTime < 0) && actor->Origin().z <= actor->FloorHeight())
    {
        float t = (float)weaponBobTime;
        float x, y;

        movement = movement / 16.0f;
        kexMath::Clamp(movement, 0, 1);

        x = kexMath::Sin(t * 0.075f) * (16*movement);
        y = kexMath::Fabs(kexMath::Sin(t * 0.075f) * (16*movement)) - landTime;

        weaponBob_x = (x - weaponBob_x) * 0.25f + weaponBob_x;
        weaponBob_y = (y - weaponBob_y) * 0.25f + weaponBob_y;

        weaponBobTime++;
    }
    else
    {
        weaponBobTime = 0;
        weaponBob_x = (0 - weaponBob_x) * 0.25f + weaponBob_x;
        weaponBob_y = (0 - weaponBob_y) * 0.25f + weaponBob_y;
    }
}

//
// kexPlayer::UpdateViewBob
//

void kexPlayer::UpdateViewBob(void)
{
    if(actor->Origin().z > actor->FloorHeight())
    {
        bob = 0;
        bobTime = 0;
        bobSpeed = 0;
    }
    else
    {
        if(cmd.Buttons() & (BC_FORWARD|BC_BACKWARD|BC_STRAFELEFT|BC_STRAFERIGHT))
        {
            bobSpeed = (0.148f - bobSpeed) * 0.35f + bobSpeed;
            bob = kexMath::Sin(bobTime) * 7.0f;
            bobTime += bobSpeed;
        }
        else
        {
            bobSpeed = 0;
            bobTime = 0;
            bob = (0 - bob) * 0.1f + bob;
        }
    }

    if(landTime < 0)
    {
        landTime += 2;
        if(landTime >= 0)
        {
            landTime = 0;
        }
    }

    if(stepViewZ < 0)
    {
        stepViewZ += 8;
        if(stepViewZ >= 0)
        {
            stepViewZ = 0;
        }
    }
}

//
// kexPlayer::CycleNextWeapon
//

void kexPlayer::CycleNextWeapon(void)
{
    int setWeapon = pendingWeapon;

    do
    {
        if(++setWeapon >= NUMPLAYERWEAPONS)
        {
            setWeapon = PW_MACHETE;
        }

        if(weapons[setWeapon] == true)
        {
            pendingWeapon = static_cast<playerWeapons_t>(setWeapon);
            break;
        }

    } while(setWeapon != currentWeapon);
}

//
// kexPlayer::CyclePrevWeapon
//

void kexPlayer::CyclePrevWeapon(void)
{
    int setWeapon = pendingWeapon;

    do
    {
        if(--setWeapon < 0)
        {
            setWeapon = NUMPLAYERWEAPONS-1;
        }

        if(weapons[setWeapon] == true)
        {
            pendingWeapon = static_cast<playerWeapons_t>(setWeapon);
            break;
        }

    } while(setWeapon != currentWeapon);
}

//
// kexPlayer::UpdateWeaponSprite
//

void kexPlayer::UpdateWeaponSprite(void)
{
    spriteFrame_t *frame;
    const kexGameLocal::weaponInfo_t *weaponInfo;

    if(weaponAnim == NULL)
    {
        return;
    }

    if(cmd.Buttons() & BC_WEAPONLEFT)
    {
        CyclePrevWeapon();
    }
    if(cmd.Buttons() & BC_WEAPONRIGHT)
    {
        CycleNextWeapon();
    }

    frame = &weaponAnim->frames[weaponFrame];

    weaponTicks += (1.0f / (float)frame->delay) * 0.5f;
    if(weaponTicks >= 1)
    {
        weaponTicks = 0;
        
        if(++weaponFrame >= (int16_t)weaponAnim->NumFrames())
        {
            weaponFrame = 0;

            if(weaponState == WS_LOWER)
            {
                currentWeapon = pendingWeapon;
                weaponState = WS_RAISE;
                weaponAnim = kexGame::cLocal->WeaponInfo(currentWeapon)->raise;
                return;
            }
        }
    }

    weaponInfo = kexGame::cLocal->WeaponInfo(currentWeapon);

    if(frame->HasNextFrame())
    {
        weaponAnim = kexGame::cLocal->SpriteAnimManager()->Get(frame->nextFrame);
        weaponFrame = 0;
        weaponTicks = 0;

        if(weaponAnim == weaponInfo->idle)
        {
            weaponState = WS_IDLE;
        }
    }

    switch(weaponState)
    {
    case WS_IDLE:
        if(pendingWeapon != currentWeapon)
        {
            weaponState = WS_LOWER;
            weaponAnim = weaponInfo->lower;
        }
        if(cmd.Buttons() & BC_ATTACK)
        {
            weaponState = WS_FIRE;
            weaponAnim = weaponInfo->fire;
        }
        break;

    default:
        break;
    }
}

//
// kexPlayer::Tick
//

void kexPlayer::Tick(void)
{
    UpdateViewBob();
    UpdateWeaponBob();
    UpdateWeaponSprite();
}
