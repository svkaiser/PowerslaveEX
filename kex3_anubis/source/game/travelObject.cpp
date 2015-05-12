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
#include "overWorld.h"

DECLARE_KEX_CLASS(kexTravelObject, kexActor)

kexTravelObject *kexTravelObject::currentObject = NULL;

//
// kexTravelObject::kexTravelObject
//

kexTravelObject::kexTravelObject(void)
{
    this->reTriggerTime = 0;
    this->mapDestination = -1;
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
    if(reTriggerTime > 0 || !instigator->InstanceOf(&kexPuppet::info))
    {
        return;
    }

    if(warpSounds.Length() != 0)
    {
        PlaySound(warpSounds[kexRand::Max(warpSounds.Length())].c_str());
    }

    if(mapDestination >= 0)
    {
        kexGame::cLocal->OverWorld()->SelectedMap() = mapDestination;
        kexGame::cLocal->PlayLoop()->RequestExit(kexGame::cLocal->MapInfoList()[mapDestination].map);
        static_cast<kexPuppet*>(instigator)->ScheduleWarpForNextMap(destinationPosition);
        reTriggerTime = 10000;
    }
    else if(mapDestination == -2)
    {
        kexGame::cLocal->OverWorld()->SelectedMap() = 0;
        kexGame::cLocal->PlayLoop()->RequestExit(GS_ENDING_GOOD);
    }
    else if(mapDestination == -3)
    {
        kexGame::cLocal->OverWorld()->SelectedMap() = 0;
        kexGame::cLocal->PlayLoop()->RequestExit(GS_ENDING_BAD);
    }
    else
    {
        reTriggerTime = 120;
        currentObject = this;
        kexGame::cLocal->SetMenu(MENU_TRAVEL);
    }

    if(mapDestination != -1)
    {
        kexGame::cLocal->Player()->Artifacts() |= giveArtifacts;

        if(bRequestSave)
        {
            kexGame::cLocal->SaveGame();
        }
    }
}

//
// kexTravelObject::Spawn
//

void kexTravelObject::Spawn(void)
{
    int i = 1;
    
    if(!definition)
    {
        return;
    }
    
    definition->GetInt("mapDestination", mapDestination, -1);
    definition->GetInt("giveArtifacts", giveArtifacts, 0);
    definition->GetVector("destinationPosition", destinationPosition);
    definition->GetBool("requestSave", bRequestSave);
    
    if(kexGame::cLocal->MapInfoList().Length() != 0)
    {
        kexMath::Clamp(mapDestination, -3, kexGame::cLocal->MapInfoList().Length());
    }
    else
    {
        mapDestination = -1;
    }

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
