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
//      Pickup Object
//

#include "kexlib.h"
#include "game.h"

//-----------------------------------------------------------------------------
//
// kexPickup
//
//-----------------------------------------------------------------------------

DECLARE_ABSTRACT_KEX_CLASS(kexPickup, kexActor)

//
// kexPickup::kexPickup
//

kexPickup::kexPickup(void)
{
}

//
// kexPickup::~kexPickup
//

kexPickup::~kexPickup(void)
{
}

//
// kexPickup::Tick
//

void kexPickup::Tick(void)
{
    kexActor::Tick();
}

//
// kexPickup::OnTouch
//

void kexPickup::OnTouch(kexActor *instigator)
{
    kexActor::OnTouch(instigator);
    Remove();
}

//-----------------------------------------------------------------------------
//
// kexWeaponPickup
//
//-----------------------------------------------------------------------------

DECLARE_KEX_CLASS(kexWeaponPickup, kexPickup)

//
// kexWeaponPickup::kexWeaponPickup
//

kexWeaponPickup::kexWeaponPickup(void)
{
    this->weaponSlotToGive = -1;
    this->bRemoveIfOwned = false;
}

//
// kexWeaponPickup::~kexWeaponPickup
//

kexWeaponPickup::~kexWeaponPickup(void)
{
}

//
// kexWeaponPickup::Tick
//

void kexWeaponPickup::Tick(void)
{
    kexPickup::Tick();
}

//
// kexWeaponPickup::OnTouch
//

void kexWeaponPickup::OnTouch(kexActor *instigator)
{
    kexPuppet *puppet;
    kexPlayer *player;
    
    if(!instigator->InstanceOf(&kexPuppet::info))
    {
        return;
    }
    
    puppet = static_cast<kexPuppet*>(instigator);
    player = puppet->Owner();
    
    if(player->GiveWeapon(weaponSlotToGive))
    {
        kexPickup::OnTouch(instigator);
    }
}

//
// kexWeaponPickup::Spawn
//

void kexWeaponPickup::Spawn(void)
{
    if(definition)
    {
        definition->GetInt("weaponSlotToGive", weaponSlotToGive, -1);
        definition->GetBool("removeIfOwned", bRemoveIfOwned, false);
    }
    
    if(bRemoveIfOwned && kexGame::cLocal->Player()->WeaponOwned(weaponSlotToGive))
    {
        Remove();
    }
}
