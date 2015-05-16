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
//      Game Object Class. Everything that exists in a world
//      inherits from this object. When having an object
//      referencing other objects, it must increment the ref
//      counter for that object to be targeted.
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
// kexGameObject::GetSoundParameters
//

const bool kexGameObject::GetSoundParameters(float &volume, float &pan)
{
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
            return false;
        }

        volume = 128 - ((dist / SOUND_DISTANCE) * 128);
        ang = (origin - listenOrg).Normalize().ToYaw();
        pan = kexMath::Rad2Deg(ang.Diff(listener->Yaw())) / 1.40625f;
    }

    kexMath::Clamp(volume, 0, 128);
    kexMath::Clamp(pan, -128, 127);

    return true;
}

//
// kexGameObject::PlaySound
//

void kexGameObject::PlaySound(const char *snd)
{
    float volume;
    float pan;

    if(!GetSoundParameters(volume, pan))
    {
        return;
    }

    kex::cSound->Play((void*)snd, (int)volume, (int)pan, this);
}

//
// kexGameObject::StopSound
//

void kexGameObject::StopSound(void)
{
    for(int i = 0; i < kex::cSound->NumSources(); ++i)
    {
        if(kex::cSound->GetRefObject(i) != this)
        {
            continue;
        }

        kex::cSound->Stop(i);
    }
}

//
// kexGameObject::PlayLoopingSound
//

void kexGameObject::PlayLoopingSound(const char *snd)
{
    float volume;
    float pan;

    for(int i = 0; i < kex::cSound->NumSources(); ++i)
    {
        if(kex::cSound->GetRefObject(i) != this)
        {
            continue;
        }

        if(kex::cSound->SourceLooping(i))
        {
            // only one looping source per object allowed
            return;
        }
    }

    if(!GetSoundParameters(volume, pan))
    {
        return;
    }

    kex::cSound->Play((void*)snd, (int)volume, (int)pan, this, true);
}

//
// kexGameObject::StopLoopingSounds
//

void kexGameObject::StopLoopingSounds(void)
{
    for(int i = 0; i < kex::cSound->NumSources(); ++i)
    {
        if(kex::cSound->GetRefObject(i) != this)
        {
            continue;
        }

        if(!kex::cSound->SourceLooping(i))
        {
            continue;
        }

        kex::cSound->Stop(i);
    }
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
