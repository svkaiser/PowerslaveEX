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
    if(pickupSound.Length() > 0)
    {
        instigator->PlaySound(pickupSound);
    }

    if(pickupMessage.Length() > 0)
    {
        kexGame::cLocal->PlayLoop()->Print(pickupMessage.c_str());
    }

    kexActor::OnTouch(instigator);

    flags &= ~(AF_SOLID|AF_TOUCHABLE);
    Remove();

    kexGame::cLocal->PlayLoop()->PickupFlash();
}

//
// kexPickup::Spawn
//

void kexPickup::Spawn(void)
{
    if(definition)
    {
        definition->GetString("pickupSound", pickupSound);
        definition->GetString("pickupMessage", pickupMessage);
    }
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

    if(Removing())
    {
        return;
    }
    
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
    this->multiplier = 5;
    this->bFullAmmo = false;
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

    if(Removing())
    {
        return;
    }
    
    if(!instigator->InstanceOf(&kexPuppet::info))
    {
        return;
    }
    
    puppet = static_cast<kexPuppet*>(instigator);
    player = puppet->Owner();

    if(bFullAmmo == true)
    {
        for(int i = 0; i < NUMPLAYERWEAPONS; ++i)
        {
            player->GiveAmmo(i, kexGame::cLocal->WeaponInfo(i)->maxAmmo);
        }

        kexPickup::OnTouch(instigator);
        return;
    }

    if(weaponSlotToGive <= -1)
    {
        weapon = player->PendingWeapon();
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

    if(scale > 1)
    {
        give = (int)kexMath::Floor((float)wpInfo->maxAmmo / (multiplier * scale));
    }
    else
    {
        give = (int)kexMath::Floor((float)wpInfo->maxAmmo / (float)divisor);
    }

    if(give <= 0)
    {
        give = 1;
    }

    player->GiveAmmo(weapon, (int16_t)give);
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
        definition->GetFloat("multiplier", multiplier, 5);
        definition->GetBool("fullAmmo", bFullAmmo);

        if(divisor <= 0) divisor = 1;
        if(multiplier <= 0) multiplier = 1;
    }
}

//-----------------------------------------------------------------------------
//
// kexHealthPickup
//
//-----------------------------------------------------------------------------

DECLARE_KEX_CLASS(kexHealthPickup, kexPickup)

//
// kexHealthPickup::kexHealthPickup
//

kexHealthPickup::kexHealthPickup(void)
{
    this->giveAmount = 20;
    this->multiplier = 1;
}

//
// kexHealthPickup::~kexHealthPickup
//

kexHealthPickup::~kexHealthPickup(void)
{
}

//
// kexHealthPickup::Tick
//

void kexHealthPickup::Tick(void)
{
    kexPickup::Tick();
}

//
// kexHealthPickup::OnTouch
//

void kexHealthPickup::OnTouch(kexActor *instigator)
{
    kexPuppet *puppet;
    kexPlayer *player;
    int giveHealth;

    if(Removing())
    {
        return;
    }
    
    if(!instigator->InstanceOf(&kexPuppet::info))
    {
        return;
    }
    
    puppet = static_cast<kexPuppet*>(instigator);
    player = puppet->Owner();

    giveHealth = giveAmount;

    if(scale > 1)
    {
        giveHealth = (int)((float)giveHealth * multiplier);
    }

    if(!player->GiveHealth(giveHealth))
    {
        return;
    }

    kexPickup::OnTouch(instigator);
}

//
// kexHealthPickup::Spawn
//

void kexHealthPickup::Spawn(void)
{
    if(definition)
    {
        definition->GetInt("giveAmount", giveAmount, 20);
        definition->GetFloat("multiplier", multiplier, 1);
    }
}

//-----------------------------------------------------------------------------
//
// kexMaxHealthPickup
//
//-----------------------------------------------------------------------------

DECLARE_KEX_CLASS(kexMaxHealthPickup, kexPickup)

//
// kexMaxHealthPickup::kexMaxHealthPickup
//

kexMaxHealthPickup::kexMaxHealthPickup(void)
{
    this->bits = 0;
}

//
// kexMaxHealthPickup::~kexMaxHealthPickup
//

kexMaxHealthPickup::~kexMaxHealthPickup(void)
{
}

//
// kexMaxHealthPickup::Tick
//

void kexMaxHealthPickup::Tick(void)
{
    kexPickup::Tick();
}

//
// kexMaxHealthPickup::OnTouch
//

void kexMaxHealthPickup::OnTouch(kexActor *instigator)
{
    kexPuppet *puppet;
    kexPlayer *player;

    if(Removing())
    {
        return;
    }

    if(!instigator->InstanceOf(&kexPuppet::info))
    {
        return;
    }

    puppet = static_cast<kexPuppet*>(instigator);
    player = puppet->Owner();

    if(!player->IncreaseMaxHealth(BIT(bits)))
    {
        return;
    }

    kexPickup::OnTouch(instigator);
}

//
// kexMaxHealthPickup::Spawn
//

void kexMaxHealthPickup::Spawn(void)
{
    if(definition)
    {
        definition->GetInt("bits", bits, 0);
    }

    if(kexGame::cLocal->Player()->AnkahFlags() & BIT(bits))
    {
        Remove();
    }
}

//-----------------------------------------------------------------------------
//
// kexKeyPickup
//
//-----------------------------------------------------------------------------

DECLARE_KEX_CLASS(kexKeyPickup, kexPickup)

//
// kexKeyPickup::kexKeyPickup
//

kexKeyPickup::kexKeyPickup(void)
{
    this->bits = 0;
}

//
// kexKeyPickup::~kexKeyPickup
//

kexKeyPickup::~kexKeyPickup(void)
{
}

//
// kexKeyPickup::Tick
//

void kexKeyPickup::Tick(void)
{
    kexPickup::Tick();
}

//
// kexKeyPickup::OnTouch
//

void kexKeyPickup::OnTouch(kexActor *instigator)
{
    if(Removing())
    {
        return;
    }

    if(!instigator->InstanceOf(&kexPuppet::info) || bits < 0)
    {
        return;
    }

    if(static_cast<kexPuppet*>(instigator)->Owner()->GiveKey(bits))
    {
        kexPickup::OnTouch(instigator);
    }
}

//
// kexKeyPickup::Spawn
//

void kexKeyPickup::Spawn(void)
{
    if(definition)
    {
        definition->GetInt("keyType", bits, 0);
    }
}

//-----------------------------------------------------------------------------
//
// kexArtifactPickup
//
//-----------------------------------------------------------------------------

DECLARE_KEX_CLASS(kexArtifactPickup, kexPickup)

//
// kexArtifactPickup::kexArtifactPickup
//

kexArtifactPickup::kexArtifactPickup(void)
{
    this->bits = 0;
}

//
// kexArtifactPickup::~kexArtifactPickup
//

kexArtifactPickup::~kexArtifactPickup(void)
{
}

//
// kexArtifactPickup::Tick
//

void kexArtifactPickup::Tick(void)
{
    kexPickup::Tick();
}

//
// kexArtifactPickup::OnTouch
//

void kexArtifactPickup::OnTouch(kexActor *instigator)
{
    kexPlayer *player;

    if(Removing())
    {
        return;
    }

    if(!instigator->InstanceOf(&kexPuppet::info) || bits < 0)
    {
        return;
    }

    player = static_cast<kexPuppet*>(instigator)->Owner();

    if(!(player->Artifacts() & BIT(bits)))
    {
        player->Artifacts() |= BIT(bits);
        kexGame::cLocal->PlayLoop()->InventoryMenu().ShowArtifact(bits, false);

        kexPickup::OnTouch(instigator);
        kexGame::cWorld->FireRemoteEventFromTag(1000 + bits);
    }
}

//
// kexArtifactPickup::Spawn
//

void kexArtifactPickup::Spawn(void)
{
    if(definition)
    {
        definition->GetInt("type", bits, 0);
    }

    if(kexGame::cLocal->Player()->Artifacts() & BIT(bits))
    {
        kexGame::cWorld->FireRemoteEventFromTag(1000 + bits);
        Remove();
    }
}

//-----------------------------------------------------------------------------
//
// kexQuestPickup
//
//-----------------------------------------------------------------------------

DECLARE_KEX_CLASS(kexQuestPickup, kexPickup)

//
// kexQuestPickup::kexQuestPickup
//

kexQuestPickup::kexQuestPickup(void)
{
    this->bits = 0;
}

//
// kexQuestPickup::~kexQuestPickup
//

kexQuestPickup::~kexQuestPickup(void)
{
}

//
// kexQuestPickup::Tick
//

void kexQuestPickup::Tick(void)
{
    kexPickup::Tick();
}

//
// kexQuestPickup::OnTouch
//

void kexQuestPickup::OnTouch(kexActor *instigator)
{
    kexPlayer *player;

    if(Removing())
    {
        return;
    }

    if(!instigator->InstanceOf(&kexPuppet::info) || bits < 0)
    {
        return;
    }

    player = static_cast<kexPuppet*>(instigator)->Owner();

    if(!(player->QuestItems() & BIT(bits)))
    {
        player->QuestItems() |= BIT(bits);
        kexGame::cLocal->PlayLoop()->InventoryMenu().ShowTransmitter(bits);

        kexPickup::OnTouch(instigator);
    }
}

//
// kexQuestPickup::Spawn
//

void kexQuestPickup::Spawn(void)
{
    if(definition)
    {
        definition->GetInt("type", bits, 0);
    }

    if(kexGame::cLocal->Player()->QuestItems() & BIT(bits))
    {
        Remove();
    }
}

//-----------------------------------------------------------------------------
//
// kexTeamDollPickup
//
//-----------------------------------------------------------------------------

DECLARE_KEX_CLASS(kexTeamDollPickup, kexPickup)

//
// kexTeamDollPickup::kexTeamDollPickup
//

kexTeamDollPickup::kexTeamDollPickup(void)
{
    this->bits = 0;
}

//
// kexTeamDollPickup::~kexTeamDollPickup
//

kexTeamDollPickup::~kexTeamDollPickup(void)
{
}

//
// kexTeamDollPickup::Tick
//

void kexTeamDollPickup::Tick(void)
{
    kexPickup::Tick();
}

//
// kexTeamDollPickup::OnTouch
//

void kexTeamDollPickup::OnTouch(kexActor *instigator)
{
    kexPlayer *player;

    if(Removing())
    {
        return;
    }

    if(!instigator->InstanceOf(&kexPuppet::info) || bits < 0)
    {
        return;
    }

    player = static_cast<kexPuppet*>(instigator)->Owner();

    if(!(player->TeamDolls() & BIT(bits)))
    {
        int dollsCollected = 0;

        player->TeamDolls() |= BIT(bits);
        kexGame::cLocal->PlayLoop()->InventoryMenu().ShowArtifact(-1, pickupSound.Length() != 0);

        for(int i = 0; i < 32; ++i)
        {
            if(player->TeamDolls() & BIT(i))
            {
                dollsCollected++;
            }
        }

        if(dollsCollected >= 14)
        {
            player->Abilities() |= PAB_VULTURE;
        }
        else if(dollsCollected >= 10)
        {
            player->Abilities() |= PAB_DOLPHIN;
        }

        kexPickup::OnTouch(instigator);
    }
}

//
// kexTeamDollPickup::Spawn
//

void kexTeamDollPickup::Spawn(void)
{
    if(definition)
    {
        definition->GetInt("type", bits, 0);
    }

    if(kexGame::cLocal->Player()->TeamDolls() & BIT(bits))
    {
        Remove();
    }
}

//-----------------------------------------------------------------------------
//
// kexMapPickup
//
//-----------------------------------------------------------------------------

DECLARE_KEX_CLASS(kexMapPickup, kexPickup)

//
// kexMapPickup::kexMapPickup
//

kexMapPickup::kexMapPickup(void)
{
}

//
// kexMapPickup::~kexMapPickup
//

kexMapPickup::~kexMapPickup(void)
{
}

//
// kexMapPickup::Tick
//

void kexMapPickup::Tick(void)
{
    kexPickup::Tick();
}

//
// kexMapPickup::OnTouch
//

void kexMapPickup::OnTouch(kexActor *instigator)
{
    if(Removing())
    {
        return;
    }

    if(!instigator->InstanceOf(&kexPuppet::info))
    {
        return;
    }

    kexGame::cLocal->PlayLoop()->ToggleMapAll(true);
    kexPickup::OnTouch(instigator);
}

//
// kexMapPickup::Spawn
//

void kexMapPickup::Spawn(void)
{
}
