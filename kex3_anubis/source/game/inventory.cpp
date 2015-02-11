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
//      Inventory Object
//

#include "kexlib.h"
#include "game.h"
#include "inventory.h"

DECLARE_KEX_CLASS(kexInventory, kexActor)

//
// kexInventory::kexInventory
//

kexInventory::kexInventory(void)
{
    this->amount = 1;
    this->maxAmount = 1;
}

//
// kexInventory::~kexInventory
//

kexInventory::~kexInventory(void)
{
}

//
// kexInventory::Tick
//

void kexInventory::Tick(void)
{
    kexActor::Tick();
}

//
// kexInventory::OnTouch
//

void kexInventory::OnTouch(kexActor *instigator)
{
    Remove();
}

//
// kexInventory::Spawn
//

void kexInventory::Spawn(void)
{
    if(definition)
    {
        definition->GetInt("amount", amount, 1);
        definition->GetInt("maxAmount", maxAmount, 1);
    }
}
