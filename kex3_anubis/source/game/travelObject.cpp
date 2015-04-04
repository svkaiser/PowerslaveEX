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
//      Travel/Level Warp Object
//

#include "kexlib.h"
#include "game.h"

DECLARE_KEX_CLASS(kexTravelObject, kexActor)

kexTravelObject *kexTravelObject::currentObject = NULL;

//
// kexTravelObject::kexTravelObject
//

kexTravelObject::kexTravelObject(void)
{
    this->reTriggerTime = 0;
}

//
// kexTravelObject::~kexTravelObject
//

kexTravelObject::~kexTravelObject(void)
{
}

//
// kexTravelObject::Tick
//

void kexTravelObject::Tick(void)
{
    kexActor::Tick();

    if(reTriggerTime)
    {
        reTriggerTime--;
    }
}

//
// kexTravelObject::OnTouch
//

void kexTravelObject::OnTouch(kexActor *instigator)
{
    if(reTriggerTime > 0)
    {
        return;
    }

    reTriggerTime = 120;
    currentObject = this;
    kexGame::cLocal->SetMenu(MENU_TRAVEL);

    if(warpSounds.Length() == 0)
    {
        return;
    }

    int r = kexRand::Max(warpSounds.Length());
    
    PlaySound(warpSounds[r].c_str());
}

//
// kexTravelObject::Spawn
//

void kexTravelObject::Spawn(void)
{
    int i = 1;

    while(1)
    {
        kexStr str;

        if(!definition->GetString(kexStr::Format("warpSound_%i", i), str))
        {
            break;
        }

        warpSounds.Push(str);
        i++;
    }
}
