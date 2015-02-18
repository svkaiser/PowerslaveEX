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
//      AI Object
//

#include "kexlib.h"
#include "game.h"

//-----------------------------------------------------------------------------
//
// kexAI
//
//-----------------------------------------------------------------------------

DECLARE_KEX_CLASS(kexAI, kexActor)

//
// kexAI::kexProjectile
//

kexAI::kexAI(void)
{
    this->chaseAnim = NULL;
    this->painAnim = NULL;
    this->meleeAnim = NULL;
    this->attackAnim = NULL;
    this->state = AIS_IDLE;
    this->thinkTime = 0.5f;
    this->curThinkTime = 1;
    this->timeBeforeTurning = 0;
    this->aiFlags = 0;
}

//
// kexAI::~kexAI
//

kexAI::~kexAI(void)
{
}

//
// kexAI::Tick
//

void kexAI::Tick(void)
{
    if(Removing())
    {
        return;
    }
    
    curThinkTime -= thinkTime;
    
    if(curThinkTime <= 0)
    {
        switch(state)
        {
        case AIS_IDLE:
            LookForTarget();
            break;
                
        case AIS_CHASE:
            ChaseTarget();
            break;
                
        case AIS_PAIN:
            InPain();
            break;
        }
        
        curThinkTime = 1;
    }
    
    kexActor::Tick();
}

//
// kexAI::OnDamage
//

void kexAI::OnDamage(kexActor *instigator)
{
    StartPain();
}

//
// kexAI::CheckTargetSight
//

bool kexAI::CheckTargetSight(kexActor *actor)
{
    kexVec3 start = origin + kexVec3(0, 0, height * 0.5f);
    kexVec3 end = actor->Origin() + kexVec3(0, 0, actor->Height() * 0.5f);
    
    return !kexGame::cLocal->CModel()->Trace(this, sector, start, end, false);
}

//
// kexAI::StartChasing
//

void kexAI::StartChasing(void)
{
    if(chaseAnim)
    {
        ChangeAnim(chaseAnim);
    }
    
    state = AIS_CHASE;
    timeBeforeTurning = kexRand::Max(8);
}

//
// kexAI::StartPain
//

void kexAI::StartPain(void)
{
    if(painAnim)
    {
        ChangeAnim(painAnim);
    }
    
    state = AIS_PAIN;
}

//
// kexAI::InPain
//

void kexAI::InPain(void)
{
    if(anim == chaseAnim)
    {
        state = AIS_CHASE;
    }
}

//
// kexAI::LookForTarget
//

void kexAI::LookForTarget(void)
{
    if(target != NULL)
    {
        return;
    }
    
    kexActor *targ = kexGame::cLocal->Player()->Actor();
    
    if(CheckTargetSight(targ))
    {
        SetTarget(targ);
        StartChasing();
    }
}

//
// kexAI::ChaseTarget
//

void kexAI::ChaseTarget(void)
{
    kexVec3 forward;
    
    if(!(aiFlags & AIF_TURNING))
    {
        if(--timeBeforeTurning <= 0)
        {
            float yawAmount;
            float yawAmountFabs;
            
            aiFlags |= AIF_TURNING;
            desiredYaw = kexMath::ATan2(target->Origin().x - origin.x, target->Origin().y - origin.y);
            yawAmount = desiredYaw.Diff(yaw);
            yawAmountFabs = kexMath::Fabs(yawAmount);
            
            if((kexRand::Int() & 1) && yawAmountFabs < kexMath::Deg2Rad(22.5f))
            {
                desiredYaw += kexMath::Deg2Rad(kexRand::Range(-45.0f, 45.0f));
                yawAmount = desiredYaw.Diff(yaw);
                yawAmountFabs = kexMath::Fabs(yawAmount);
            }
            
            if(yawAmountFabs < kexMath::Deg2Rad(22.5f))
            {
                turnAmount = yawAmount / 16.0f;
            }
            else
            {
                turnAmount = yawAmount / 8.0f;
            }
        }
    }
    else
    {
        yaw += turnAmount;
        if(kexMath::Fabs(yaw.Diff(desiredYaw)) < 0.001f)
        {
            aiFlags &= ~AIF_TURNING;
            timeBeforeTurning = 8 + kexRand::Max(8);
        }
    }
    
    kexVec3::ToAxis(&forward, 0, 0, yaw, 0, 0);
    movement = forward * 8;
}

//
// kexAI::UpdateMovement
//

void kexAI::UpdateMovement(void)
{
    UpdateVelocity();
    CheckFloorAndCeilings();
    
    movement += velocity;
    
    if(movement.UnitSq() > 0)
    {
        if(!kexGame::cLocal->CModel()->MoveActor(this))
        {
            velocity.Clear();
        }
        
        movement.Clear();
    }
}

//
// kexAI::Spawn
//

void kexAI::Spawn(void)
{
    if(definition)
    {
        kexStr animName;
        
        if(definition->GetString("chaseAnim", animName))
        {
            chaseAnim = kexGame::cLocal->SpriteAnimManager()->Get(animName);
        }
        
        if(definition->GetString("painAnim", animName))
        {
            painAnim = kexGame::cLocal->SpriteAnimManager()->Get(animName);
        }
    }
}
