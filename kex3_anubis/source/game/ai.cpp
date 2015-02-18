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
//      AI Object
//

#include "kexlib.h"
#include "game.h"

//-----------------------------------------------------------------------------
//
// kexAI
//
//-----------------------------------------------------------------------------

DECLARE_KEX_CLASS(kexAI, kexActor)

//
// kexAI::kexProjectile
//

kexAI::kexAI(void)
{
    this->chaseAnim = NULL;
    this->painAnim = NULL;
    this->meleeAnim = NULL;
    this->attackAnim = NULL;
}

//
// kexAI::~kexAI
//

kexAI::~kexAI(void)
{
}

//
// kexAI::Tick
//

void kexAI::Tick(void)
{
    kexActor::Tick();
}

//
// kexAI::OnDamage
//

void kexAI::OnDamage(kexActor *instigator)
{
    if(painAnim)
    {
        ChangeAnim(painAnim);
    }
}

//
// kexAI::UpdateMovement
//

void kexAI::UpdateMovement(void)
{
}

//
// kexAI::Spawn
//

void kexAI::Spawn(void)
{
    if(definition)
    {
        kexStr animName;
        
        if(definition->GetString("painAnim", animName))
        {
            painAnim = kexGame::cLocal->SpriteAnimManager()->Get(animName);
        }
    }
}
