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
