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

#ifndef __AI_H__
#define __AI_H__

#include "actor.h"

typedef enum
{
    AIS_IDLE    = 0,
    AIS_CHASE,
    AIS_PAIN,
    AIS_MELEE,
    AIS_RANGE,
    AIS_DEAD,
    AIS_CUSTOM
} aiState_t;

typedef enum
{
    ADT_NONE        = -1,
    ADT_NORTH       = 0,
    ADT_NORTHEAST,
    ADT_EAST,
    ADT_SOUTHEAST,
    ADT_SOUTH,
    ADT_SOUTHWEST,
    ADT_WEST,
    ADT_NORTHWEST,

    NUMAIDIRTYPES
} aiDirTypes_t;

typedef enum
{
    AIF_TURNING             = BIT(0),
    AIF_LOOKALLAROUND       = BIT(1),
    AIF_ALWAYSRANGEATTACK   = BIT(2),
    AIF_ONFIRE              = BIT(3),
    AIF_FLYING              = BIT(4),
    AIF_RETREATAFTERMELEE   = BIT(5),
    AIF_RETREATTURN         = BIT(6),
    AIF_FLYADJUSTVIEWLEVEL  = BIT(7),
    AIF_NOLAVADAMAGE        = BIT(8),
    AIF_NOINFIGHTING        = BIT(9)
} aiFlags_t;

//-----------------------------------------------------------------------------
//
// kexAI
//
//-----------------------------------------------------------------------------

BEGIN_EXTENDED_KEX_CLASS(kexAI, kexActor);
public:
    kexAI(void);
    ~kexAI(void);

    virtual void                    Tick(void);
    virtual void                    OnDamage(kexActor *instigator);
    virtual void                    OnRemove(void);
    virtual void                    OnDeath(kexActor *instigator);
    virtual void                    UpdateMovement(void);

    void                            Spawn(void);

    void                            FaceTarget(kexActor *targ = NULL);
    void                            Ignite(kexGameObject *igniteTarget);
    void                            Ignite(kexProjectileFlame *instigator);
    void                            Ignite(void);
    void                            ClearBurn(void);

    aiState_t                       &State(void) { return state; }
    unsigned int                    &AIFlags(void) { return aiFlags; }
    float                           &MoveSpeed(void) { return moveSpeed; }
    float                           &TurnSpeed(void) { return turnSpeed; }
    int                             &PainChance(void) { return painChance; }

    static bool                     bNoTargetEnemy;

private:
    float                           GetTargetHeightDifference(void);
    void                            UpdateBurn(void);
    bool                            TrySetDesiredDirection(const int dir);
    void                            SetDesiredDirection(const float angle);
    void                            ChangeStateFromAnim(void);
    bool                            CheckMeleeRange(void);
    bool                            CheckRangeAttack(void);
    bool                            CheckDirection(const kexVec3 &dir);
    void                            ChangeDirection(void);
    void                            StartChasing(void);
    void                            StartPain(void);
    void                            InPain(void);
    void                            LookForTarget(void);
    void                            ChaseTarget(void);
    bool                            IsFacingTarget(kexActor *actor, const float fov);
    bool                            CheckTargetSight(kexActor *actor);
    bool                            CanSeeTarget(kexActor *actor);

    static const float              directionAngles[NUMAIDIRTYPES];
    static const kexVec3            directionVectors[NUMAIDIRTYPES];
    static const int                oppositeDirection[NUMAIDIRTYPES];

    spriteAnim_t                    *chaseAnim;
    spriteAnim_t                    *painAnim;
    spriteAnim_t                    *meleeAnim;
    spriteAnim_t                    *attackAnim;
    kexStr                          painSound;
    kexStr                          sightSound;
    aiState_t                       state;
    unsigned int                    aiFlags;
    float                           thinkTime;
    float                           curThinkTime;
    int                             timeBeforeTurning;
    kexAngle                        desiredYaw;
    float                           turnAmount;
    int                             painChance;
    float                           moveSpeed;
    float                           meleeExtraDist;
    int                             igniteTicks[4];
    kexActor                        *igniteFlames[4];
    float                           sightDistance;
    float                           turnSpeed;
    int                             turnCount;
END_KEX_CLASS();

#endif
