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
//      Projectile Object
//

#include "kexlib.h"
#include "game.h"

//-----------------------------------------------------------------------------
//
// kexProjectile
//
//-----------------------------------------------------------------------------

DECLARE_KEX_CLASS(kexProjectile, kexActor)

//
// kexProjectile::kexProjectile
//

kexProjectile::kexProjectile(void)
{
    this->damage = 0;
}

//
// kexProjectile::~kexProjectile
//

kexProjectile::~kexProjectile(void)
{
}

//
// kexProjectile::Tick
//

void kexProjectile::Tick(void)
{
    kexActor::Tick();
}

//
// kexProjectile::OnImpact
//

void kexProjectile::OnImpact(kexActor *contactActor)
{
    if(contactActor && damage > 0)
    {
        contactActor->InflictDamage(this, damage);
    }

    ChangeAnim(this->deathAnim);
    flags &= ~(AF_MOVEABLE|AF_SOLID);
}

//
// kexProjectile::Spawn
//

void kexProjectile::Spawn(void)
{
    if(definition)
    {
        definition->GetInt("damage", damage, 0);
    }
}

//-----------------------------------------------------------------------------
//
// kexProjectileFlame
//
//-----------------------------------------------------------------------------

DECLARE_KEX_CLASS(kexProjectileFlame, kexProjectile)

//
// kexProjectileFlame::kexProjectileFlame
//

kexProjectileFlame::kexProjectileFlame(void)
{
    this->fizzleTime = 0;
    this->lifeTime = 50;
}

//
// kexProjectileFlame::~kexProjectileFlame
//

kexProjectileFlame::~kexProjectileFlame(void)
{
}

//
// kexProjectileFlame::Tick
//

void kexProjectileFlame::Tick(void)
{
    if(Removing())
    {
        return;
    }

    kexActor::Tick();

    if(anim == deathAnim)
    {
        return;
    }

    fizzleTime += 0.5f;
    scale += ((5000 - ((fizzleTime * 65536) / 512)) / 128) / 512;

    if(fizzleTime >= (float)lifeTime)
    {
        Remove();
    }
}

//
// kexProjectileFlame::OnImpact
//

void kexProjectileFlame::OnImpact(kexActor *contactActor)
{
    if(contactActor && contactActor->InstanceOf(&kexAI::info))
    {
        static_cast<kexAI*>(contactActor)->Ignite(this);
    }

    kexProjectile::OnImpact(contactActor);
}

//
// kexProjectileFlame::Spawn
//

void kexProjectileFlame::Spawn(void)
{
    if(definition)
    {
        definition->GetInt("lifeTime", lifeTime, 50);
    }

    fizzleTime = 0;
}
