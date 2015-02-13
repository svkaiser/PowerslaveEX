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

//-----------------------------------------------------------------------------
//
// kexAmmoPickup
//
//-----------------------------------------------------------------------------

DECLARE_KEX_CLASS(kexAmmoPickup, kexPickup)

//
// kexAmmoPickup::kexAmmoPickup
//

kexAmmoPickup::kexAmmoPickup(void)
{
    this->weaponSlotToGive = -1;
    this->divisor = 1;
    this->multiplier = 1;
}

//
// kexAmmoPickup::~kexAmmoPickup
//

kexAmmoPickup::~kexAmmoPickup(void)
{
}

//
// kexAmmoPickup::Tick
//

void kexAmmoPickup::Tick(void)
{
    kexPickup::Tick();
}

//
// kexAmmoPickup::OnTouch
//

void kexAmmoPickup::OnTouch(kexActor *instigator)
{
    kexPuppet *puppet;
    kexPlayer *player;
    int weapon, give;
    
    if(!instigator->InstanceOf(&kexPuppet::info))
    {
        return;
    }
    
    puppet = static_cast<kexPuppet*>(instigator);
    player = puppet->Owner();

    if(weaponSlotToGive <= -1)
    {
        weapon = player->CurrentWeapon();
    }
    else
    {
        weapon = weaponSlotToGive;
    }

    const kexGameLocal::weaponInfo_t *wpInfo = kexGame::cLocal->WeaponInfo(weapon);
    int ammo = player->GetAmmo(weapon);

    if(ammo >= wpInfo->maxAmmo)
    {
        return;
    }

    give = divisor;

    if(scale > 1)
    {
        give /= multiplier;
    }

    if(give <= 0)
    {
        return;
    }

    player->GiveAmmo(weapon, (int16_t)(wpInfo->maxAmmo / give));
    kexPickup::OnTouch(instigator);
}

//
// kexAmmoPickup::Spawn
//

void kexAmmoPickup::Spawn(void)
{
    if(definition)
    {
        definition->GetInt("weaponSlotToGive", weaponSlotToGive, -1);
        definition->GetInt("divisor", divisor, 1);
        definition->GetInt("multiplier", multiplier, 1);

        if(divisor <= 0) divisor = 1;
        if(multiplier <= 0) multiplier = 1;
    }
}
