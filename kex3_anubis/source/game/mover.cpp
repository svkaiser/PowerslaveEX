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
//      Mover Object
//

#include "kexlib.h"
#include "game.h"
#include "mover.h"

//-----------------------------------------------------------------------------
//
// kexMover
//
//-----------------------------------------------------------------------------

DECLARE_ABSTRACT_KEX_CLASS(kexMover, kexGameObject)

//
// kexMover::kexMover
//

kexMover::kexMover(void)
{
    this->definition = NULL;
}

//
// kexMover::~kexMover
//

kexMover::~kexMover(void)
{
}

//
// kexMover::Tick
//

void kexMover::Tick(void)
{
}

//-----------------------------------------------------------------------------
//
// kexDoor
//
//-----------------------------------------------------------------------------

DECLARE_KEX_CLASS(kexDoor, kexMover)

//
// kexDoor::kexDoor
//

kexDoor::kexDoor(void)
{
    this->waitDelay = 90;
    this->lip = 0;
    this->moveSpeed = 50;
    this->bDirection = true;
}

//
// kexDoor::~kexDoor
//

kexDoor::~kexDoor(void)
{
}

//
// kexDoor::Tick
//

void kexDoor::Tick(void)
{
}

//
// kexDoor::Spawn
//

void kexDoor::Spawn(void)
{
    if(definition)
    {
        definition->GetFloat("waitDelay", waitDelay, 90);
        definition->GetFloat("lip", lip, 0);
        definition->GetFloat("moveSpeed", moveSpeed, 50);
        definition->GetBool("direction", bDirection, true);
    }
}
