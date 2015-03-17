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

const int kexAI::oppositeDirection[NUMAIDIRTYPES] =
{
    ADT_SOUTH,
    ADT_SOUTHWEST,
    ADT_WEST,
    ADT_NORTHWEST,
    ADT_NORTH,
    ADT_NORTHEAST,
    ADT_EAST,
    ADT_SOUTHEAST
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
    this->meleeExtraDist = 0;
    this->turnSpeed = 8;
    this->turnCount = 0;

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
        // health is depleted, we're dead now
        state = AIS_DEAD;
        return;
    }

    // we took damage but we're not active yet. target whoever damaged us
    if(!target && instigator != this && (instigator->Flags() & AF_SHOOTABLE ||
        instigator->InstanceOf(&kexProjectile::info)))
    {
        if(instigator && instigator->InstanceOf(&kexProjectile::info))
        {
            SetTarget(instigator->Target());
        }
        else
        {
            SetTarget(instigator);
        }

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
// kexAI::ClearBurn
//

void kexAI::ClearBurn(void)
{
    for(int i = 0; i < 4; ++i)
    {
        if(igniteTicks[i] <= -1 || igniteFlames[i] == NULL)
        {
            continue;
        }

        igniteFlames[i]->Remove();
        igniteFlames[i] = NULL;
        igniteTicks[i] = -1;
    }

    aiFlags &= ~AIF_ONFIRE;
}

//
// kexAI::UpdateBurn
//
// Slowly take damage over time. The more flames
// attached, the more damage it will take
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

        // flame expired?
        if(--igniteTicks[i] < 0)
        {
            igniteFlames[i]->Remove();
            igniteFlames[i] = NULL;
            igniteTicks[i] = -1;
            continue;
        }

        cnt++;

        if((kexRand::Int() & 0xff) <= 7)
        {
            InflictDamage(static_cast<kexActor*>(igniteFlames[i]->Target()), 9);
        }
    }

    if(cnt == 0)
    {
        // all flames have expired
        aiFlags &= ~AIF_ONFIRE;
    }
}

//
// kexAI::CanSeeTarget
//

bool kexAI::CanSeeTarget(kexActor *actor)
{
    kexVec3 start = origin + kexVec3(0, 0, height * 0.5f);
    kexVec3 end = actor->Origin() + kexVec3(0, 0, actor->Height() * 0.5f);
    
    return !kexGame::cLocal->CModel()->Trace(this, sector, start, end, 0, false);
}

//
// kexAI::IsFacingTarget
//

bool kexAI::IsFacingTarget(kexActor *actor, const float fov)
{
    return kexMath::Fabs(yaw.Diff(kexMath::ATan2(actor->Origin().x - origin.x,
                                                 actor->Origin().y - origin.y))) <= fov;
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
        if(!IsFacingTarget(actor, 1.04f))
        {
            // not within FOV
            return false;
        }
    }

    return CanSeeTarget(actor);
}

//
// kexAI::ChangeStateFromAnim
//
// Unlike DOOM, there is no A_Chase codepointer and
// instead, the various states of the AI is based on
// what animation the AI is playing. Should probably
// revisit this...
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
    else if(anim != chaseAnim && anim != spawnAnim &&
            anim != meleeAnim && anim != painAnim)
    {
        state = AIS_CUSTOM;
    }
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
    velocity.Clear();
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
    kexVec3 org;
    kexActor *targ;

    if(!target || !meleeAnim)
    {
        return false;
    }

    targ = static_cast<kexActor*>(target);
    r = (radius + targ->Radius()) + meleeExtraDist;
    org = target->Origin();

    if(aiFlags & AIF_FLYING)
    {
        org.z += stepHeight;
    }

    if(origin.DistanceSq(org) > (r * r))
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
    
    if(!attackAnim || (!CanSeeTarget(targ) && !(aiFlags & AIF_ALWAYSRANGEATTACK)) ||
        RandomDecision(30))
    {
        return false;
    }

    return !((kexRand::Int() & 7) < 4);
}

//
// kexAI::CheckDirection
//
// Traces a path to the direction that it wants to
// change to. Returns false if that direction is blocked
//

bool kexAI::CheckDirection(const kexVec3 &dir)
{
    kexVec3 start, end;

    start = origin + kexVec3(0, 0, stepHeight);
    end = start + (dir * (radius * 1.5f));
    
    if(kexGame::cLocal->CModel()->Trace(this, sector, start, end))
    {
        return (kexGame::cLocal->CModel()->ContactActor() == target);
    }
    
    // check for ledges
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
            
            if(kexGame::cLocal->CModel()->PointOnFaceSide(end, face, radius * 1.125f) > 0)
            {
                continue;
            }
            
            s = &kexGame::cLocal->World()->Sectors()[face->sector];
            if(sector->floorHeight - s->floorHeight > stepHeight ||
               (!(flags & AF_INWATER) && s->flags & SF_WATER))
            {
                // sector the ai is on is too high, so avoid this edge
                return false;
            }
        }
    }

    return true;
}

//
// kexAI::SetDesiredDirection
//

void kexAI::SetDesiredDirection(const float angle)
{
    desiredYaw = angle;
    turnAmount = desiredYaw.Diff(yaw) / turnSpeed;
    turnCount = 0;
}

//
// kexAI::SetDesiredDirection
//

bool kexAI::TrySetDesiredDirection(const int dir)
{
    if(dir != ADT_NONE)
    {
        if(CheckDirection(directionVectors[dir]))
        {
            SetDesiredDirection(directionAngles[dir]);
            return true;
        }
    }

    return false;
}

//
// kexAI::GetTargetHeightDifference
//

float kexAI::GetTargetHeightDifference(void)
{
    float heightLevel;

    if(target == NULL || !target->InstanceOf(&kexActor::info))
    {
        return 0;
    }

    float targHeight = static_cast<kexActor*>(target)->Height();
    heightLevel = ((origin.z + height) - target->Origin().z);

    if(aiFlags & AIF_FLYADJUSTVIEWLEVEL)
    {
        return heightLevel - ((targHeight * 0.5f) + stepHeight);
    }

    return heightLevel - targHeight;
}

//
// kexAI::ChangeDirection
//
// Picks a new direction to move to
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
    
    // determine direction to change towards the target
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

                    if(!CheckDirection(directionVectors[moveDir]))
                    {
                        moveDir = ADT_EAST;
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

                    if(!CheckDirection(directionVectors[moveDir]))
                    {
                        moveDir = ADT_WEST;
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

                    if(!CheckDirection(directionVectors[moveDir]))
                    {
                        moveDir = ADT_NORTH;
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

                    if(!CheckDirection(directionVectors[moveDir]))
                    {
                        moveDir = ADT_SOUTH;
                    }
                }
                else
                {
                    moveDir = ADT_SOUTH;
                }
            }
        }

        if(TrySetDesiredDirection(moveDir))
        {
            bAdjust = false;
        }
    }
    
    if(!bAdjust)
    {
        return;
    }

    // direction was blocked, so pick another direction
    if(kexRand::Float() >= 0.5f)
    {
        for(int i = 1; i < 8; ++i)
        {
            newYaw = yaw + kexMath::Deg2Rad(25.71f * (float)i);
            kexVec3::ToAxis(&forward, 0, 0, newYaw, 0, 0);
            
            if(CheckDirection(forward))
            {
                SetDesiredDirection(newYaw);
                return;
            }
        }
    }
    else
    {
        for(int i = 1; i < 8; ++i)
        {
            newYaw = yaw - kexMath::Deg2Rad(25.71f * (float)i);
            kexVec3::ToAxis(&forward, 0, 0, newYaw, 0, 0);
            
            if(CheckDirection(forward))
            {
                SetDesiredDirection(newYaw);
                return;
            }
        }
    }

    // no idea what to do now. randomly pick a direction and hope for the best
    SetDesiredDirection(yaw + (kexMath::Deg2Rad(135) + (kexRand::Float() * kexMath::Deg2Rad(90))));
}

//
// kexAI::Ignite
//
// Actor is on fire and will start taking damage
// from the attached flames. IgniteTarget is
// whatever that caused the ai to ignite
//

void kexAI::Ignite(kexGameObject *igniteTarget)
{
    aiFlags |= AIF_ONFIRE;
    
    // spawn flames
    for(int i = 0; i < 4; ++i)
    {
        if(igniteTicks[i] <= -1 && igniteFlames[i] == NULL)
        {
            float x, y, z;
            kexActor *ignite;
            
            igniteTicks[i] = kexRand::Max(64) + 64;
            
            x = origin.x + (kexRand::Float() * (radius*0.5f));
            y = origin.y + (kexRand::Float() * (radius*0.5f));
            z = origin.z + (kexRand::Float() * (height*0.5f));

            if(z < origin.z + stepHeight && stepHeight < height)
            {
                z = origin.z + stepHeight;
            }
            
            ignite = kexGame::cActorFactory->Spawn(AT_IGNITEFLAME, x, y, z, 0, SectorIndex());
            ignite->SetTarget(igniteTarget);
            
            igniteFlames[i] = ignite;
            break;
        }
    }
}

//
// kexAI::Ignite
//
// AI has ignited from an unknown source,
// most likely from lava
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
        // caught fire from lava? whatever, its the player's fault
        SetTarget(kexGame::cLocal->Player()->Actor());
    }
    
    Ignite(target);
}

//
// kexAI::Ignite
//
// AI has ignited from an flame-based projectile
//

void kexAI::Ignite(kexProjectileFlame *instigator)
{
    if(!(kexRand::Int() & 1))
    {
        return;
    }

    if(!target)
    {
        // target whoever fired that projectile
        SetTarget(instigator->Target());
    }
    
    Ignite(instigator->Target());
}

//
// kexAI::ChaseTarget
//

void kexAI::ChaseTarget(void)
{
    kexVec3 forward;

    if(!target)
    {
        kex::cSystem->Warning("kexAI::ChaseTarget - Called without a target\n");
        SetTarget(kexGame::cLocal->Player()->Actor());
    }

    if(aiFlags & AIF_RETREATTURN)
    {
        velocity.z = 0;
        yaw += turnAmount;

        if(!RandomDecision(30))
        {
            if((kexRand::Int() & 0xff) <= 64)
            {
                aiFlags &= ~AIF_RETREATTURN;
            }
        }

        kexVec3::ToAxis(&forward, 0, 0, yaw, 0, 0);
        movement = forward * moveSpeed;
        return;
    }

    // not currently turning?
    if(!(aiFlags & AIF_TURNING))
    {
        kexVec3::ToAxis(&forward, 0, 0, yaw, 0, 0);

        // ready to change direction?
        if(--timeBeforeTurning <= 0 || !CheckDirection(forward))
        {
            ChangeDirection();
        }
    }
    else
    {
        // turn towards new direction
        yaw += turnAmount;
        kexVec3::ToAxis(&forward, 0, 0, yaw, 0, 0);

        if(++turnCount >= turnSpeed)
        {
            aiFlags &= ~AIF_TURNING;
            timeBeforeTurning = 8 + kexRand::Max(16);
        }
    }

    if(aiFlags & AIF_FLYING)
    {
        float viewZ = GetTargetHeightDifference();

             if(viewZ < 0) velocity.z =  3;
        else if(viewZ > 0) velocity.z = -3;
        else velocity.z = 0;
    }

    if(CheckMeleeRange())
    {
        aiFlags &= ~AIF_TURNING;
        FaceTarget();
        ChangeAnim(meleeAnim);
        state = AIS_MELEE;
        if(aiFlags & AIF_FLYING && GetTargetHeightDifference() <= 0)
        {
            velocity.z = 0;
        }
        if(aiFlags & AIF_RETREATAFTERMELEE)
        {
            aiFlags |= AIF_RETREATTURN;

            if((kexRand::Int() & 0xff) <= 128)
            {
                turnAmount = kexMath::Deg2Rad(1);
            }
            else
            {
                turnAmount = kexMath::Deg2Rad(-1);
            }
        }
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

    if(!(aiFlags & AIF_NOLAVADAMAGE) && sector->floorFace->flags & FF_LAVA &&
        kexGame::cLocal->CModel()->PointOnFaceSide(origin, sector->floorFace) <= 0.1f)
    {
        // dumbass fell into lava
        Ignite();
    }
    
    if(movement.UnitSq() > 0)
    {
        if(!kexGame::cLocal->CModel()->MoveActor(this))
        {
            velocity.Clear();
        }
        
        // update attached flame positions
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
        definition->GetFloat("meleeExtraDist", meleeExtraDist);
        definition->GetFloat("turnSpeed", turnSpeed, 8);
        definition->GetString("painSound", painSound);
        definition->GetString("sightSound", sightSound);

        if(definition->GetBool("lookAllAround"))        aiFlags |= AIF_LOOKALLAROUND;
        if(definition->GetBool("alwaysRangeAttack"))    aiFlags |= AIF_ALWAYSRANGEATTACK;
        if(definition->GetBool("flying"))               aiFlags |= AIF_FLYING;
        if(definition->GetBool("retreatAfterMelee"))    aiFlags |= AIF_RETREATAFTERMELEE;
        if(definition->GetBool("flyAdjustViewLevel"))   aiFlags |= AIF_FLYADJUSTVIEWLEVEL;
        if(definition->GetBool("noLavaDamage"))         aiFlags |= AIF_NOLAVADAMAGE;
        
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

    if(turnSpeed <= 0)
    {
        turnSpeed = 1;
    }
}
