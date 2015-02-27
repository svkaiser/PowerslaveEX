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

bool kexAI::bNoTargetEnemy = false;

const float kexAI::directionAngles[NUMAIDIRTYPES] =
{
    0,
    kexMath::Deg2Rad(45),
    kexMath::Deg2Rad(90),
    kexMath::Deg2Rad(135),
    kexMath::Deg2Rad(180),
    kexMath::Deg2Rad(-135),
    kexMath::Deg2Rad(-90),
    kexMath::Deg2Rad(-45)
};

const kexVec3 kexAI::directionVectors[NUMAIDIRTYPES] =
{
    kexVec3(0, 1, 0),
    kexVec3(0.5f, 0.5f, 0),
    kexVec3(1, 0, 0),
    kexVec3(0.5f, -0.5f, 0),
    kexVec3(0, -1, 0),
    kexVec3(-0.5f, -0.5f, 0),
    kexVec3(-1, 0, 0),
    kexVec3(-0.5f, 0.5f, 0)
};

//
// noaitarget
//

COMMAND(noaitarget)
{
    kexAI::bNoTargetEnemy ^= 1;
    
    if(kexAI::bNoTargetEnemy)
    {
        kex::cSystem->DPrintf("AI targeting disabled\n");
    }
    else
    {
        kex::cSystem->DPrintf("AI targeting enabled\n");
    }
}

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
    this->painChance = 0xff;
    this->moveSpeed = 8;

    for(int i = 0; i < 4; ++i)
    {
        this->igniteFlames[i] = NULL;
        this->igniteTicks[i] = -1;
    }
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
    
    ChangeStateFromAnim();
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

        case AIS_MELEE:
            FaceTarget();
            break;
                
        default:
            break;
        }

        UpdateBurn();
        curThinkTime = 1;
    }
    
    kexActor::Tick();
}

//
// kexAI::OnRemove
//

void kexAI::OnRemove(void)
{
    kexActor::OnRemove();
}

//
// kexActor::OnDeath
//

void kexAI::OnDeath(kexActor *instigator)
{
    for(int i = 0; i < 4; ++i)
    {
        if(igniteFlames[i] == NULL)
        {
            continue;
        }

        igniteFlames[i]->Remove();
        igniteFlames[i] = NULL;
        igniteTicks[i] = -1;
    }
}

//
// kexAI::OnDamage
//

void kexAI::OnDamage(kexActor *instigator)
{
    if(health <= 0)
    {
        state = AIS_DEAD;
        return;
    }

    if(!target && instigator != this && instigator->Flags() & AF_SHOOTABLE)
    {
        SetTarget(instigator);

        if(state == AIS_IDLE && anim != chaseAnim)
        {
            StartChasing();
        }
    }
    
    if(kexRand::Max(255) < painChance)
    {
        StartPain();
    }
}

//
// kexAI::UpdateBurn
//

void kexAI::UpdateBurn(void)
{
    int cnt = 0;

    if(!(aiFlags & AIF_ONFIRE) || state == AIS_PAIN)
    {
        return;
    }

    for(int i = 0; i < 4; ++i)
    {
        if(igniteTicks[i] <= -1 || igniteFlames[i] == NULL)
        {
            continue;
        }

        if(--igniteTicks[i] < 0)
        {
            igniteFlames[i]->Remove();
            igniteFlames[i] = NULL;
            igniteTicks[i] = -1;
            continue;
        }

        cnt++;

        if(!(kexRand::Int() & 7))
        {
            InflictDamage(static_cast<kexActor*>(igniteFlames[i]->Target()), 9);
        }
    }

    if(cnt == 0)
    {
        aiFlags &= ~AIF_ONFIRE;
    }
}

//
// kexAI::CheckTargetSight
//

bool kexAI::CheckTargetSight(kexActor *actor)
{
    if(bNoTargetEnemy)
    {
        return false;
    }
    
    if(kexMath::Fabs(origin.x - actor->Origin().x) >= 3000) return false;
    if(kexMath::Fabs(origin.y - actor->Origin().y) >= 3000) return false;

    if(!(aiFlags & AIF_LOOKALLAROUND))
    {
        if(kexMath::Fabs(yaw.Diff(kexMath::ATan2(actor->Origin().x - origin.x,
                                                 actor->Origin().y - origin.y))) > 0.785f)
        {
            return false;
        }
    }

    kexVec3 start = origin + kexVec3(0, 0, height * 0.5f);
    kexVec3 end = actor->Origin() + kexVec3(0, 0, actor->Height() * 0.5f);
    
    return !kexGame::cLocal->CModel()->Trace(this, sector, start, end, false);
}

//
// kexAI::ChangeStateFromAnim
//

void kexAI::ChangeStateFromAnim(void)
{
    if(state == AIS_DEAD)
    {
        return;
    }
    
         if(anim == chaseAnim && state != AIS_CHASE)    state = AIS_CHASE;
    else if(anim == spawnAnim && state != AIS_IDLE)     state = AIS_IDLE;
    else if(anim == meleeAnim && state != AIS_MELEE)    state = AIS_MELEE;
    else if(anim == painAnim  && state != AIS_PAIN)     state = AIS_PAIN;
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
    if(painAnim && anim != painAnim)
    {
        ChangeAnim(painAnim);
        state = AIS_PAIN;
    }

    if(painSound.Length() > 0)
    {
        PlaySound(painSound);
    }
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
// kexAI::FaceTarget
//

void kexAI::FaceTarget(kexActor *targ)
{
    kexVec3 org;
    
    if(!targ)
    {
        if(!target)
        {
            return;
        }
        
        org = target->Origin();
    }
    else
    {
        org = targ->Origin();
    }
    
    yaw = kexMath::ATan2(org.x - origin.x, org.y - origin.y);
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
        if(sightSound.Length() > 0)
        {
            PlaySound(sightSound);
        }

        SetTarget(targ);
        StartChasing();
    }
}

//
// CheckMeleeRange
//

bool kexAI::CheckMeleeRange(void)
{
    float r;
    kexActor *targ;

    if(!target || !meleeAnim)
    {
        return false;
    }

    targ = static_cast<kexActor*>(target);
    r = (radius * 0.6f) + targ->Radius();

    if(origin.DistanceSq(target->Origin()) > (r * r))
    {
        return false;
    }

    return CheckTargetSight(targ);
}

//
// kexAI::CheckRangeAttack
//

bool kexAI::CheckRangeAttack(void)
{
    kexActor *targ = static_cast<kexActor*>(target);
    
    if(!attackAnim || (!CheckTargetSight(targ) && !(aiFlags & AIF_ALWAYSRANGEATTACK)) ||
        (kexRand::Int() & 30) != (kexGame::cLocal->PlayLoop()->Ticks() & 30))
    {
        return false;
    }

    return !((kexRand::Int() & 7) < 4);
}

//
// kexAI::CheckDirection
//

bool kexAI::CheckDirection(const kexVec3 &dir)
{
    kexVec3 start;

    start = origin + kexVec3(0, 0, stepHeight);
    
    if(kexGame::cLocal->CModel()->Trace(this, sector, start, start + (dir * (radius * 1.5f))))
    {
        return (kexGame::cLocal->CModel()->ContactActor() == target);
    }
    
    if(flags & AF_NODROPOFF)
    {
        for(int i = sector->faceStart; i <= sector->faceEnd; ++i)
        {
            mapFace_t *face = &kexGame::cLocal->World()->Faces()[i];
            mapSector_t *s;
            
            if(face->sector <= -1)
            {
                continue;
            }
            
            if(kexGame::cLocal->CModel()->PointOnFaceSide(origin, face, radius * 1.125f) > 0)
            {
                continue;
            }

            if(face->plane.IsFacing(yaw))
            {
                continue;
            }
            
            s = &kexGame::cLocal->World()->Sectors()[face->sector];
            if(sector->floorHeight - s->floorHeight > stepHeight ||
               (!(flags & AF_INWATER) && s->flags & SF_WATER))
            {
                return false;
            }
        }
    }

    return true;
}

//
// kexAI::SetDesiredDirection
//

bool kexAI::SetDesiredDirection(const int dir)
{
    if(dir != ADT_NONE)
    {
        if(CheckDirection(directionVectors[dir]))
        {
            desiredYaw = directionAngles[dir];
            turnAmount = desiredYaw.Diff(yaw) / 8;
            return true;
        }
    }

    return false;
}

//
// kexAI::ChangeDirection
//

void kexAI::ChangeDirection(void)
{
    kexVec3 forward;
    bool bAdjust;
    kexAngle newYaw;
    float dx, dy;
    float fx, fy;
    aiDirTypes_t moveDir = ADT_NONE;
    
    aiFlags |= AIF_TURNING;
    bAdjust = (timeBeforeTurning > 0);
    
    if(timeBeforeTurning <= 0)
    {
        dx = target->Origin().x - origin.x;
        dy = target->Origin().y - origin.y;

        fx = kexMath::Fabs(dx);
        fy = kexMath::Fabs(dy);

        if(fx > fy)
        {
            if(dx > radius)
            {
                if(kexRand::Int() & 1 && (fx + fy) * 0.5f > radius-1)
                {
                    if(dy >= 0)
                    {
                        moveDir = ADT_NORTHEAST;
                    }
                    else
                    {
                        moveDir = ADT_SOUTHEAST;
                    }
                }
                else
                {
                    moveDir = ADT_EAST;
                }
            }
            else if(dx < -radius)
            {
                if(kexRand::Int() & 1 && (fx + fy) * 0.5f > radius-1)
                {
                    if(dy >= 0)
                    {
                        moveDir = ADT_NORTHWEST;
                    }
                    else
                    {
                        moveDir = ADT_SOUTHWEST;
                    }
                }
                else
                {
                    moveDir = ADT_WEST;
                }
            }
        }
        else if(fy > fx)
        {
            if(dy > radius)
            {
                if(kexRand::Int() & 1 && (fx + fy) * 0.5f > radius-1)
                {
                    if(dx >= 0)
                    {
                        moveDir = ADT_NORTHEAST;
                    }
                    else
                    {
                        moveDir = ADT_NORTHWEST;
                    }
                }
                else
                {
                    moveDir = ADT_NORTH;
                }
            }
            else if(dy < -radius)
            {
                if(kexRand::Int() & 1 && (fx + fy) * 0.5f > radius-1)
                {
                    if(dx >= 0)
                    {
                        moveDir = ADT_SOUTHEAST;
                    }
                    else
                    {
                        moveDir = ADT_SOUTHWEST;
                    }
                }
                else
                {
                    moveDir = ADT_SOUTH;
                }
            }
        }

        if(SetDesiredDirection(moveDir))
        {
            bAdjust = false;
        }
    }
    
    if(!bAdjust)
    {
        return;
    }

    for(int i = 1; i < 8; ++i)
    {
        newYaw = yaw + kexMath::Deg2Rad(25.71f * (float)i);
        kexVec3::ToAxis(&forward, 0, 0, newYaw, 0, 0);
        
        if(CheckDirection(forward))
        {
            desiredYaw = newYaw;
            turnAmount = desiredYaw.Diff(yaw) / 8;
            return;
        }
        
        newYaw = yaw - kexMath::Deg2Rad(25.71f * (float)i);
        kexVec3::ToAxis(&forward, 0, 0, newYaw, 0, 0);
        
        if(CheckDirection(forward))
        {
            desiredYaw = newYaw;
            turnAmount = desiredYaw.Diff(yaw) / 8;
            return;
        }
    }

    desiredYaw = yaw + (kexMath::Deg2Rad(135) + (kexRand::Float() * kexMath::Deg2Rad(90)));
    turnAmount = desiredYaw.Diff(yaw) / 8;
}

//
// kexAI::Ignite
//

void kexAI::Ignite(void)
{
    if(aiFlags & AIF_ONFIRE)
    {
        if((kexRand::Int() & 3) != 0)
        {
            return;
        }
    }

    if(!target)
    {
        SetTarget(kexGame::cLocal->Player()->Actor());
    }

    aiFlags |= AIF_ONFIRE;

    for(int i = 0; i < 4; ++i)
    {
        if(igniteTicks[i] <= -1 && igniteFlames[i] == NULL)
        {
            float x, y, z;
            kexActor *ignite;

            igniteTicks[i] = kexRand::Max(64) + 8;

            x = origin.x + (kexRand::Float() * (radius*0.5f));
            y = origin.y + (kexRand::Float() * (radius*0.5f));
            z = origin.z + stepHeight + (kexRand::Float() * (height*0.35f));

            ignite = kexGame::cLocal->SpawnActor(AT_IGNITEFLAME, x, y, z, 0, SectorIndex());
            ignite->SetTarget(target);

            igniteFlames[i] = ignite;
            break;
        }
    }
}

//
// kexAI::Ignite
//

void kexAI::Ignite(kexProjectileFlame *instigator)
{
    int i;

    if(!(kexRand::Int() & 1))
    {
        return;
    }

    if(!target)
    {
        SetTarget(instigator->Target());
    }

    aiFlags |= AIF_ONFIRE;

    for(i = 0; i < 4; ++i)
    {
        if(igniteTicks[i] <= -1 && igniteFlames[i] == NULL)
        {
            float x, y, z;
            kexActor *ignite;

            igniteTicks[i] = kexRand::Max(64) + 8;

            x = origin.x + (kexRand::Float() * (radius*0.5f));
            y = origin.y + (kexRand::Float() * (radius*0.5f));
            z = origin.z + stepHeight + (kexRand::Float() * (height*0.35f));

            ignite = kexGame::cLocal->SpawnActor(AT_IGNITEFLAME, x, y, z, 0, SectorIndex());
            ignite->SetTarget(instigator->Target());

            igniteFlames[i] = ignite;
            break;
        }
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
        kexVec3::ToAxis(&forward, 0, 0, yaw, 0, 0);

        if(--timeBeforeTurning <= 0 || !CheckDirection(forward))
        {
            if(!target)
            {
                kex::cSystem->Warning("kexAI::ChaseTarget - Called without a target\n");
                SetTarget(kexGame::cLocal->Player()->Actor());
            }

            ChangeDirection();
        }
    }
    else
    {
        yaw += turnAmount;
        kexVec3::ToAxis(&forward, 0, 0, yaw, 0, 0);

        if(kexMath::Fabs(yaw.Diff(desiredYaw)) < kexMath::Deg2Rad(10.0f))
        {
            aiFlags &= ~AIF_TURNING;
            timeBeforeTurning = 8 + kexRand::Max(16);
        }
    }

    if(CheckMeleeRange())
    {
        aiFlags &= ~AIF_TURNING;
        FaceTarget();
        ChangeAnim(meleeAnim);
        state = AIS_MELEE;
        return;
    }
    
    if(CheckRangeAttack())
    {
        aiFlags &= ~AIF_TURNING;
        FaceTarget();
        ChangeAnim(attackAnim);
        state = AIS_RANGE;
        timeBeforeTurning = kexRand::Max(8) + kexRand::Max(8);
        return;
    }
    
    movement = forward * moveSpeed;
}

//
// kexAI::UpdateMovement
//

void kexAI::UpdateMovement(void)
{
    UpdateVelocity();
    CheckFloorAndCeilings();
    
    movement += velocity;

    if(sector->floorFace->flags & FF_LAVA &&
        kexGame::cLocal->CModel()->PointOnFaceSide(origin, sector->floorFace) <= 0.1f)
    {
        Ignite();
    }
    
    if(movement.UnitSq() > 0)
    {
        if(!kexGame::cLocal->CModel()->MoveActor(this))
        {
            velocity.Clear();
        }
        
        if(aiFlags & AIF_ONFIRE)
        {
            for(int i = 0; i < 4; ++i)
            {
                if(igniteFlames[i] == NULL)
                {
                    continue;
                }

                igniteFlames[i]->Origin() += movement;
                igniteFlames[i]->SetSector(sector);
            }
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

        definition->GetInt("painChance", painChance, 0xff);
        definition->GetFloat("moveSpeed", moveSpeed, 8);
        definition->GetString("painSound", painSound);
        definition->GetString("sightSound", sightSound);

        if(definition->GetBool("lookAllAround"))        aiFlags |= AIF_LOOKALLAROUND;
        if(definition->GetBool("alwaysRangeAttack"))    aiFlags |= AIF_ALWAYSRANGEATTACK;
        
        if(definition->GetString("chaseAnim", animName))
        {
            chaseAnim = kexGame::cLocal->SpriteAnimManager()->Get(animName);
        }
        
        if(definition->GetString("painAnim", animName))
        {
            painAnim = kexGame::cLocal->SpriteAnimManager()->Get(animName);
        }

        if(definition->GetString("meleeAnim", animName))
        {
            meleeAnim = kexGame::cLocal->SpriteAnimManager()->Get(animName);
        }
        
        if(definition->GetString("rangeAttackAnim", animName))
        {
            attackAnim = kexGame::cLocal->SpriteAnimManager()->Get(animName);
        }
    }
}
