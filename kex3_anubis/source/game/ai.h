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
    AIS_MELEE
} aiState_t;

typedef enum
{
    AIF_TURNING = BIT(0)
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
    virtual void                    UpdateMovement(void);
    void                            Spawn(void);

private:
    bool                            CheckMeleeRange(void);
    bool                            CheckDirection(const kexVec3 &dir);
    void                            ChangeDirection(void);
    void                            DoMelee(void);
    void                            StartChasing(void);
    void                            StartPain(void);
    void                            InPain(void);
    void                            LookForTarget(void);
    void                            ChaseTarget(void);
    bool                            CheckTargetSight(kexActor *actor);

    spriteAnim_t                    *chaseAnim;
    spriteAnim_t                    *painAnim;
    spriteAnim_t                    *meleeAnim;
    spriteAnim_t                    *attackAnim;
    aiState_t                       state;
    unsigned int                    aiFlags;
    float                           thinkTime;
    float                           curThinkTime;
    int                             timeBeforeTurning;
    kexAngle                        desiredYaw;
    float                           turnAmount;
    int                             painChance;
END_KEX_CLASS();

#endif
