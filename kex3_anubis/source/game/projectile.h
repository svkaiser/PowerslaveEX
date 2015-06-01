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
    PF_IMPACTWALLSONLY      = BIT(0),
    PF_HOMING               = BIT(1),
    PF_IMPACTED             = BIT(2),
    PF_AIMONSPAWN           = BIT(3),
    PF_AIMING               = BIT(4),
    PF_NOHOMINGTHRUST       = BIT(5),
    PF_NOCLIPINITIALSECTOR  = BIT(6),
    PF_STUNTARGET           = BIT(7)
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
    virtual bool                    OnCollide(kexCModel *cmodel);

    void                            Spawn(void);

    int                             &Damage(void) { return damage; }
    unsigned int                    &ProjectileFlags(void) { return projectileFlags; }
    void                            SetHomingTarget(kexActor *actor);

    bool                            BumpedCeiling(void);
    bool                            BumpedFloor(void);

    int                             &InitialSector(void) { return initialSector; }

private:
    virtual void                    UpdateVelocity(void);
    virtual void                    CheckFloorAndCeilings(void);
    virtual void                    UpdateMovement(void);

    void                            HomingThink(void);
    void                            AimThink(void);
    void                            AdjustAlongFace(mapFace_t *face);
    void                            SeekTargets(void);
    bool                            CheckSeekTarget(kexVec3 &start, kexActor *actor);

    int                             damage;
    int                             initialSector;
    unsigned int                    projectileFlags;
    float                           homingTurnAngles;
    float                           homingMaxPitch;
    float                           homingMaxSightDistance;
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

//-----------------------------------------------------------------------------
//
// kexFireballSpawner
//
//-----------------------------------------------------------------------------

BEGIN_EXTENDED_KEX_CLASS(kexFireballSpawner, kexActor);
friend class kexFireballFactory;
public:
    kexFireballSpawner(void);
    ~kexFireballSpawner(void);

    virtual void                    Tick(void);
    virtual void                    OnActivate(kexActor *instigator);
    virtual void                    OnDeactivate(kexActor *instigator);

    void                            Spawn(void);

    void                            SpawnFireball(mapFace_t *face, mapPoly_t *poly);

private:
    float                           fireDelay;
    bool                            bEnabled;
END_KEX_CLASS();

//-----------------------------------------------------------------------------
//
// kexFireballFactory
//
//-----------------------------------------------------------------------------

BEGIN_EXTENDED_KEX_CLASS(kexFireballFactory, kexGameObject);
public:
    kexFireballFactory(void);
    ~kexFireballFactory(void);

    virtual void                    Tick(void);
    virtual void                    Remove();
    void                            Spawn(void);

    mapSector_t                     *Sector(void) { return sector; }
    void                            SetSector(mapSector_t *s);
    float                           &Intervals(void) { return intervals; }
    int                             &FireballType(void) { return fireballType; }
    float                           &ExtraDelay(void) { return extraDelay; }

private:
    float                           intervals;
    float                           currentTime;
    float                           extraDelay;
    mapSector_t                     *sector;
    int                             fireballType;
END_KEX_CLASS();

#endif
