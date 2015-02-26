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
//      Game Object
//

#include "kexlib.h"
#include "game.h"
#include "gameObject.h"

#define SOUND_DISTANCE  4194304

DECLARE_ABSTRACT_KEX_CLASS(kexGameObject, kexObject)

unsigned int kexGameObject::id = 0;

//
// kexGameObject::kexGameObject
//

kexGameObject::kexGameObject(void)
{
    this->link.SetData(this);
    
    this->refCount      = 0;
    this->target        = NULL;
    this->bStale        = false;
    this->timeStamp     = 0;
    this->objID         = 0;
}

//
// kexGameObject::~kexGameObject
//

kexGameObject::~kexGameObject(void)
{
    SetTarget(NULL);
}

//
// kexGameObject::AddRef
//

int kexGameObject::AddRef(void)
{
    return ++refCount;
}

//
// kexGameObject::RemoveRef
//

int kexGameObject::RemoveRef(void)
{
    return --refCount;
}

//
// kexGameObject::Remove
//

void kexGameObject::Remove(void)
{
    bStale = true;
}

//
// kexGameObject::OnRemove
//

void kexGameObject::OnRemove(void)
{
    link.Remove();
    SetTarget(NULL);
}

//
// kexGameObject::Removing
//

const bool kexGameObject::Removing(void) const
{
    return (bStale && RefCount() <= 0);
}

//
// kexGameObject::PlaySound
//

void kexGameObject::PlaySound(const char *snd)
{
    float volume;
    float pan;

    if(this == kexGame::cLocal->Player()->Actor())
    {
        volume = 128;
        pan = 0;
    }
    else
    {
        kexAngle ang;
        kexRenderView *listener;
        kexVec3 listenOrg;
        float dist;

        listener = &kexGame::cLocal->PlayLoop()->View();
        listenOrg = listener->Origin();

        dist = listenOrg.DistanceSq(origin);
        if(dist > SOUND_DISTANCE)
        {
            return;
        }

        volume = 128 - ((dist / SOUND_DISTANCE) * 128);
        ang = (origin - listenOrg).Normalize().ToYaw();
        pan = kexMath::Rad2Deg(ang.Diff(listener->Yaw())) / 1.40625f;
    }

    volume *= kexSound::cvarVolume.GetFloat();
    kexMath::Clamp(volume, 0, 128);
    kexMath::Clamp(pan, -128, 128);

    kex::cSound->Play((void*)snd, (int)volume, (int)pan, this);
}

//
// kexGameObject::SetTarget
//

void kexGameObject::SetTarget(kexGameObject *targ)
{
    // If there was a target already, decrease its refcount
    if(target)
    {
        target->RemoveRef();
    }

    // Set new target and if non-NULL, increase its counter
    if((target = targ))
    {
        target->AddRef();
    }
}

//
// kexGameObject::Spawn
//

void kexGameObject::Spawn(void)
{
    timeStamp = kex::cSession->GetTime();
    objID = ++kexGameObject::id;
    
    link.Add(kexGame::cLocal->GameObjects());
}
