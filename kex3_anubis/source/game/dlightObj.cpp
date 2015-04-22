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
//      Dynamic Light Object
//

#include "kexlib.h"
#include "game.h"
#include "dlightObj.h"

DECLARE_KEX_CLASS(kexDLight, kexGameObject)

//
// kexDLight::kexDLight
//

kexDLight::kexDLight(void)
{
    this->radius = 512.0f;
    this->rgb[0] = 255;
    this->rgb[1] = 255;
    this->rgb[2] = 255;
    this->fadeTime = -1;
    this->passes = 2;
}

//
// kexDLight::~kexDLight
//

kexDLight::~kexDLight(void)
{
}

//
// kexDLight::Tick
//

void kexDLight::Tick(void)
{
    kexActor *actor;

    if(target == NULL || !target->InstanceOf(&kexActor::info) || target->IsStale())
    {
        Remove();
        return;
    }

    if(initialTime > 0)
    {
        if(fadeTime <= 0)
        {
            Remove();
            return;
        }

        fadeTime -= 0.5f;
    }

    actor = static_cast<kexActor*>(target);

    float r = radius * 0.5f;

    bounds.min.Set(-r, -r, -r);
    bounds.max.Set( r,  r,  r);

    origin = actor->Origin();
    origin.z += (actor->Height() * 0.5f);

    bounds.min += origin;
    bounds.max += origin;

    sector = actor->Sector();
    kexGame::cLocal->PlayLoop()->RenderScene().DLights().AddLight(this);
}

//
// kexDLight::Spawn
//

void kexDLight::Spawn(void)
{
    initialTime = fadeTime;
}
