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

#ifndef __PROJECTILE_H__
#define __PROJECTILE_H__

#include "actor.h"

typedef enum
{
    PF_IMPACTWALLSONLY  = BIT(0),
    PF_HOMING           = BIT(1)
} projectileFlags_t;

//-----------------------------------------------------------------------------
//
// kexProjectile
//
//-----------------------------------------------------------------------------

BEGIN_EXTENDED_KEX_CLASS(kexProjectile, kexActor);
public:
    kexProjectile(void);
    ~kexProjectile(void);

    virtual void                    Tick(void);
    virtual void                    OnImpact(kexActor *contactActor);

    void                            Spawn(void);

    int                             &Damage(void) { return damage; }
    unsigned int                    &ProjectileFlags(void) { return projectileFlags; }
    void                            SetHomingTarget(kexActor *actor);

private:
    virtual void                    CheckFloorAndCeilings(void);
    virtual void                    UpdateMovement(void);

    void                            AdjustAlongFace(mapFace_t *face);
    void                            SeekTargets(void);

    int                             damage;
    unsigned int                    projectileFlags;
    kexActor                        *homingActor;
END_KEX_CLASS();

//-----------------------------------------------------------------------------
//
// kexProjectileFlame
//
//-----------------------------------------------------------------------------

BEGIN_EXTENDED_KEX_CLASS(kexProjectileFlame, kexProjectile);
public:
    kexProjectileFlame(void);
    ~kexProjectileFlame(void);

    virtual void                    Tick(void);
    virtual void                    OnImpact(kexActor *contactActor);

    void                            Spawn(void);

private:
    float                           fizzleTime;
    int                             lifeTime;
END_KEX_CLASS();

#endif
