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
//      Player Weapon Behavior
//

#include "kexlib.h"
#include "game.h"

//
// kexPlayerWeapon::kexPlayerWeapon
//

kexPlayerWeapon::kexPlayerWeapon(void)
{
    this->owner = NULL;
    this->anim = NULL;
    this->state = WS_IDLE;
    this->bob_x = 0;
    this->bob_y = 0;
}

//
// kexPlayerWeapon::~kexPlayerWeapon
//

kexPlayerWeapon::~kexPlayerWeapon(void)
{
}

//
// kexPlayerWeapon::Update
//

void kexPlayerWeapon::Update(void)
{
    UpdateBob();
    UpdateSprite();
}

//
// kexPlayerWeapon::ChangeAnim
//

void kexPlayerWeapon::ChangeAnim(spriteAnim_t *changeAnim)
{
    if(changeAnim == NULL)
    {
        return;
    }
    
    anim = changeAnim;
    frameID = 0;
    ticks = 0;

    if(anim == kexGame::cLocal->WeaponInfo(owner->CurrentWeapon())->idle)
    {
        state = WS_IDLE;
    }

    for(unsigned int i = 0; i < anim->frames[0].actions.Length(); ++i)
    {
        anim->frames[0].actions[i]->Execute(owner->Actor());
    }
}

//
// kexPlayerWeapon::ChangeAnim
//

void kexPlayerWeapon::ChangeAnim(const char *animName)
{
    ChangeAnim(kexGame::cLocal->SpriteAnimManager()->Get(animName));
}

//
// kexPlayerWeapon::ChangeAnim
//

void kexPlayerWeapon::ChangeAnim(const weaponState_t changeState)
{
    switch(changeState)
    {
    case WS_IDLE:
        ChangeAnim(kexGame::cLocal->WeaponInfo(owner->CurrentWeapon())->idle);
        state = WS_IDLE;
        break;

    case WS_RAISE:
        ChangeAnim(kexGame::cLocal->WeaponInfo(owner->CurrentWeapon())->raise);
        state = WS_RAISE;
        break;

    case WS_LOWER:
        ChangeAnim(kexGame::cLocal->WeaponInfo(owner->CurrentWeapon())->lower);
        state = WS_LOWER;
        break;

    case WS_FIRE:
        ChangeAnim(kexGame::cLocal->WeaponInfo(owner->CurrentWeapon())->fire);
        state = WS_FIRE;
        break;

    default:
        break;
    }
}

//
// kexPlayerWeapon::UpdateBob
//

void kexPlayerWeapon::UpdateBob(void)
{
    kexActor *actor = owner->Actor();
    float movement = actor->Velocity().ToVec2().Unit();

    if((movement > 0 || owner->LandTime() < 0) &&
        actor->Origin().z <= actor->FloorHeight() &&
        state != WS_FIRE)
    {
        float t = (float)bobTime;
        float x, y;

        movement = movement / 16.0f;
        kexMath::Clamp(movement, 0, 1);

        x = kexMath::Sin(t * 0.075f) * (16*movement);
        y = kexMath::Fabs(kexMath::Sin(t * 0.075f) * (16*movement)) - owner->LandTime();

        bob_x = (x - bob_x) * 0.25f + bob_x;
        bob_y = (y - bob_y) * 0.25f + bob_y;

        bobTime++;
    }
    else
    {
        bobTime = 0;
        bob_x = (0 - bob_x) * 0.25f + bob_x;
        bob_y = (0 - bob_y) * 0.25f + bob_y;
    }
}

//
// kexPlayerWeapon::UpdateSprite
//

void kexPlayerWeapon::UpdateSprite(void)
{
    spriteFrame_t *frame;
    const kexGameLocal::weaponInfo_t *weaponInfo;

    if(anim == NULL)
    {
        return;
    }

    frame = &anim->frames[frameID];
    ticks += (1.0f / (float)frame->delay) * 0.5f;
    
    // handle advancing to next frame
    if(ticks >= 1)
    {
        ticks = 0;
        
        // reached the end of the frame?
        if(++frameID >= (int16_t)anim->NumFrames())
        {
            // loop back
            frameID = 0;

            // if lowering weapon, then switch to pending weapon
            if(state == WS_LOWER)
            {
                owner->ChangeWeapon();
                ChangeAnim(WS_RAISE);
                return;
            }
        }

        for(unsigned int i = 0; i < anim->frames[frameID].actions.Length(); ++i)
        {
            anim->frames[frameID].actions[i]->Execute(owner->Actor());
        }
    }

    weaponInfo = kexGame::cLocal->WeaponInfo(owner->CurrentWeapon());

    // handle re-fire
    if(frame->HasRefireFrame() && owner->Cmd().Buttons() & BC_ATTACK)
    {
        ChangeAnim(frame->refireFrame);
    }
    // handle goto jumps
    else if(frame->HasNextFrame())
    {
        ChangeAnim(frame->nextFrame);
    }

    switch(state)
    {
    case WS_IDLE:
        if(owner->PendingWeapon() != owner->CurrentWeapon())
        {
            ChangeAnim(WS_LOWER);
        }
        if(owner->Cmd().Buttons() & BC_ATTACK)
        {
            ChangeAnim(WS_FIRE);
        }
        break;

    default:
        break;
    }
}
