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
#include "dlightObj.h"

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
    this->initialSector = -1;
    this->projectileFlags = 0;
    this->homingTurnAngles = 0.03125f;
    this->homingMaxPitch = 0.125f;
    this->homingMaxSightDistance = 2000;
    this->homingActor = NULL;
}

//
// kexProjectile::~kexProjectile
//

kexProjectile::~kexProjectile(void)
{
    SetHomingTarget(NULL);
}

//
// kexProjectile::Tick
//

void kexProjectile::Tick(void)
{
    kexActor::Tick();

    if(projectileFlags & PF_HOMING)
    {
        HomingThink();
    }
    else if(projectileFlags & PF_AIMONSPAWN)
    {
        AimThink();
    }
}

//
// kexProjectile::OnCollide
//

bool kexProjectile::OnCollide(kexCModel *cmodel)
{
    // exit out of collision immediately
    return false;
}

//
// kexProjectile::SetHomingTarget
//

void kexProjectile::SetHomingTarget(kexActor *actor)
{
    // If there was a target already, decrease its refcount
    if(homingActor)
    {
        homingActor->RemoveRef();
    }

    // Set new target and if non-NULL, increase its counter
    if((homingActor = actor))
    {
        homingActor->AddRef();
    }
}

//
// kexProjectile::HomingThink
//

void kexProjectile::HomingThink(void)
{
    if(homingActor != NULL)
    {
        float speed;

        if(homingActor->Removing() || homingActor->Health() <= 0 ||
            homingActor->Flags() & AF_HIDDEN || !(homingActor->Flags() & AF_SHOOTABLE))
        {
            SetHomingTarget(NULL);
            return;
        }

        kexVec3 vOrigin = homingActor->Origin() + kexVec3(0, 0, homingActor->Height()*0.5f);
        kexVec3 dir = (vOrigin - origin).Normalize();
        kexAngle an1 = dir.ToYaw();

        kexAngle diff = an1 - yaw;

             if(diff.an >  homingTurnAngles) yaw += homingTurnAngles;
        else if(diff.an < -homingTurnAngles) yaw -= homingTurnAngles;
        else yaw = an1;
        
        pitch = -dir.ToPitch();
        kexMath::Clamp(pitch.an, -homingMaxPitch, homingMaxPitch);

        if(!(projectileFlags & PF_NOHOMINGTHRUST))
        {
            speed = velocity.Unit();

            kexVec3::ToAxis(&velocity, 0, 0, yaw, pitch, 0);
            velocity *= speed;
        }
    }
    else if(!RandomDecision(14))
    {
        SeekTargets();
    }
}

//
// kexProjectile::AimThink
//

void kexProjectile::AimThink(void)
{
    if(projectileFlags & PF_AIMING)
    {
        kexVec3 start;
        kexVec3 end;
        kexVec2 v;
        sectorList_t *sectorList;
        kexActor *aimActor;
        float maxDist = kexMath::infinity;
        float dist;

        start = origin + kexVec3(0, 0, height * 0.5f);
        aimActor = NULL;
        sectorList = kexGame::cWorld->FloodFill(start, sector, 1024);

        projectileFlags &= ~PF_AIMING;

        for(unsigned int i = 0; i < sectorList->CurrentLength(); ++i)
        {
            for(kexActor *actor = (*sectorList)[i]->actorList.Next();
                actor != NULL;
                actor = actor->SectorLink().Next())
            {
                if(actor == this || actor == target ||
                    actor->Health() <= 0 || !(actor->Flags() & AF_SHOOTABLE))
                {
                    continue;
                }

                end = actor->Origin() + kexVec3(0, 0, actor->Height() * 0.5f);
                dist = start.DistanceSq(end);

                if(dist > maxDist)
                {
                    continue;
                }

                v.Set(end.x - start.x, end.y - start.y);
                v.Normalize();

                if((kexMath::Sin(yaw) * v.x + kexMath::Cos(yaw) * v.y) < 0.5f)
                {
                    continue;
                }

                if(kexGame::cLocal->CModel()->Trace(this, sector, start, end, 0, false))
                {
                    continue;
                }

                aimActor = actor;
                maxDist = dist;
            }
        }

        SetHomingTarget(aimActor);
    }
    else if(homingActor)
    {
        if(homingActor->Removing() || homingActor->Health() <= 0 ||
            homingActor->Flags() & AF_HIDDEN || !(homingActor->Flags() & AF_SHOOTABLE))
        {
            SetHomingTarget(NULL);
            return;
        }

        kexVec3 dir;
        kexVec3 start = origin + kexVec3(0, 0, height * 0.5f);
        kexVec3 end;

        end = homingActor->Origin() + kexVec3(0, 0, homingActor->Height() * 0.5f);
        dir = (end - start).Normalize();

        yaw = dir.ToYaw();
        pitch = -dir.ToPitch();
    }
    else if(target)
    {
        yaw = target->Yaw();
        pitch = target->Pitch();
    }
}

//
// kexProjectile::AdjustAlongFace
//

void kexProjectile::AdjustAlongFace(mapFace_t *face)
{
    if(face->flags & FF_WATER)
    {
        return;
    }

    if(kexMath::Fabs(face->plane.c) <= 0.5f)
    {
        // explode on steep slopes
        OnImpact(NULL);
        return;
    }

    float len = velocity.Unit();

    kexVec3 forward;
    kexVec3::ToAxis(&forward, 0, 0, yaw, 0, 0);

    pitch = face->plane.ToPitch();
    if(face->plane.c < 0)
    {
        pitch += kexMath::pi;
    }

    if(!face->plane.IsFacing(yaw))
    {
        pitch = -pitch;
    }

    velocity = forward * len;
}

//
// kexProjectile::CheckSeekTarget
//

bool kexProjectile::CheckSeekTarget(kexVec3 &start, kexActor *actor)
{
    if(actor == NULL)
    {
        return false;
    }

    if(kexMath::Fabs(origin.x - actor->Origin().x) >= homingMaxSightDistance) return false;
    if(kexMath::Fabs(origin.y - actor->Origin().y) >= homingMaxSightDistance) return false;

    kexVec3 end = actor->Origin() + kexVec3(0, 0, actor->Height() * 0.5f);

    if(kexGame::cLocal->CModel()->Trace(this, sector, start, end, 0, false))
    {
        return false;
    }

    SetHomingTarget(actor);
    return true;
}

//
// kexProjectile::SeekTargets
//

void kexProjectile::SeekTargets(void)
{
    kexVec3 start = origin + kexVec3(0, 0, height * 0.5f);

    if(target && target->InstanceOf(&kexAI::info))
    {
        CheckSeekTarget(start, static_cast<kexActor*>(target->Target()));
    }
    else
    {
        sectorList_t *sectorList;
        sectorList = kexGame::cWorld->FloodFill(start, sector, homingMaxSightDistance);

        for(unsigned int i = 0; i < sectorList->CurrentLength(); ++i)
        {
            for(kexActor *actor = (*sectorList)[i]->actorList.Next();
                actor != NULL;
                actor = actor->SectorLink().Next())
            {
                if(actor == this || actor == target ||
                    actor->Health() <= 0 || !(actor->Flags() & AF_SHOOTABLE) ||
                    !actor->InstanceOf(&kexAI::info))
                {
                    continue;
                }

                if(!CheckSeekTarget(start, actor))
                {
                    continue;
                }

                if((kexRand::Int() & 7) == 3)
                {
                    return;
                }
            }
        }
    }
}

//
// kexProjectile::UpdateVelocity
//

void kexProjectile::UpdateVelocity(void)
{
    // check for drop-offs
    if(origin.z > floorHeight || gravity < 0)
    {
        velocity.z -= gravity;
    }
}

//
// kexProjectile::BumpedCeiling
//

bool kexProjectile::BumpedCeiling(void)
{
    if(sector->ceilingFace->flags & FF_SOLID)
    {
        kexPlane::planeSide_t ceilingSide;
        kexVec3 position = (origin + velocity);
        kexVec3 cPosition = position + kexVec3(0, 0, height);

        ceilingSide = sector->ceilingFace->plane.PointOnSide(cPosition);

        // bump ceiling
        if(ceilingSide == kexPlane::PSIDE_BACK || ceilingSide == kexPlane::PSIDE_ON ||
            cPosition.z >= ceilingHeight)
        {
            return true;
        }
    }

    return false;
}

//
// kexProjectile::BumpedFloor
//

bool kexProjectile::BumpedFloor(void)
{
    if(sector->floorFace->flags & FF_SOLID)
    {
        kexPlane::planeSide_t floorSide;
        kexVec3 position = (origin + velocity);
        floorSide = sector->floorFace->plane.PointOnSide(position);

        // bump floor
        if(floorSide == kexPlane::PSIDE_BACK || floorSide == kexPlane::PSIDE_ON ||
            position.z <= floorHeight)
        {
            return true;
        }
    }

    return false;
}

//
// kexProjectile::CheckFloorAndCeilings
//

void kexProjectile::CheckFloorAndCeilings(void)
{
    if(BumpedCeiling())
    {
        if(flags & AF_BOUNCY)
        {
            velocity.Project(sector->ceilingFace->plane.Normal(), 2);
        }
        else if(projectileFlags & PF_IMPACTWALLSONLY)
        {
            AdjustAlongFace(sector->ceilingFace);
        }
        else
        {
            OnImpact(NULL);
        }
        return;
    }
    
    if(BumpedFloor())
    {
        if(flags & AF_BOUNCY)
        {
            if(flags & AF_EXPIRES)
            {
                if(--health == 0)
                {
                    OnImpact(NULL);
                    return;
                }
            }
            velocity.Project(sector->floorFace->plane.Normal(), 2);
        }
        else if(projectileFlags & PF_IMPACTWALLSONLY)
        {
            AdjustAlongFace(sector->floorFace);
        }
        else
        {
            OnImpact(NULL);
        }
        return;
    }

    kexActor::CheckFloorAndCeilings();
}

//
// kexProjectile::UpdateMovement
//

void kexProjectile::UpdateMovement(void)
{
    kexCModel *cm = kexGame::cLocal->CModel();

    kexActor::UpdateMovement();

    if(cm->Fraction() != 1)
    {
        mapFace_t *face = cm->ContactFace();

        if(flags & AF_BOUNCY && face)
        {
            velocity.Project(face->plane.Normal(), 2.0f);
            return;
        }

        if(initialSector >= 0)
        {
            if(face && velocity.Dot(face->plane.Normal()) >= 0)
            {
                return;
            }
            else
            {
                projectileFlags &= ~PF_NOCLIPINITIALSECTOR;
            }
        }

        OnImpact(cm->ContactActor());
        return;
    }
    else
    {
        float dist;
        float r;
        
        // make sure we explode if we're stuck inside an actor
        for(kexActor *actor = sector->actorList.Next(); actor != NULL; actor = actor->SectorLink().Next())
        {
            if(actor == this || actor == target || !(actor->Flags() & AF_SOLID))
            {
                continue;
            }

            if(!(bounds + origin).IntersectingBox(actor->Bounds() + actor->Origin()))
            {
                continue;
            }
            
            dist = origin.DistanceSq(actor->Origin());
            r = actor->Radius() + 1.024f;
            
            if(dist <= (r * r))
            {
                OnImpact(actor);
                return;
            }
        }
    }

    if(flags & AF_EXPIRES && !(flags & AF_BOUNCY))
    {
        if(--health == 0)
        {
            OnImpact(NULL);
            return;
        }
    }

    if(velocity.UnitSq() <= 0.05f || (origin - prevOrigin).UnitSq() <= 0.05f)
    {
        projectileFlags &= ~PF_NOCLIPINITIALSECTOR;
        OnImpact(NULL);
        return;
    }

    if(initialSector >= 0 && projectileFlags & PF_NOCLIPINITIALSECTOR &&
        SectorIndex() != initialSector)
    {
        projectileFlags &= ~PF_NOCLIPINITIALSECTOR;
    }

    if(!cm->PointWithinSectorEdges(origin, sector))
    {
        kex::cSystem->Warning("Projectile (type %i) out of sector bounds\n", type);
        OnImpact(NULL);
    }

}

//
// kexProjectile::OnImpact
//

void kexProjectile::OnImpact(kexActor *contactActor)
{
    if(projectileFlags & PF_IMPACTED)
    {
        return;
    }

    if(!contactActor && (projectileFlags & PF_NOCLIPINITIALSECTOR))
    {
        return;
    }

    if(contactActor && damage > 0)
    {
        contactActor->InflictDamage(this, damage);
    }

    if(flags & AF_INWATER && deathWaterAnim)
    {
        ChangeAnim(deathWaterAnim);
    }
    else
    {
        if(deathAnim)
        {
            ChangeAnim(deathAnim);
        }
        else
        {
            Remove();
        }
    }

    SetHomingTarget(NULL);

    flags &= ~(AF_MOVEABLE|AF_SOLID);
    projectileFlags |= PF_IMPACTED;
}

//
// kexProjectile::Spawn
//

void kexProjectile::Spawn(void)
{
    float r, h;

    if(definition)
    {
        definition->GetInt("damage", damage, 0);
        definition->GetFloat("homingTurnAngles", homingTurnAngles, 0.03125f);
        definition->GetFloat("homingMaxPitch", homingMaxPitch, 0.25f);
        definition->GetFloat("homingMaxSightDistance", homingMaxSightDistance, 2000);

        if(definition->GetBool("impactWallsOnly"))  projectileFlags |= PF_IMPACTWALLSONLY;
        if(definition->GetBool("homing"))           projectileFlags |= PF_HOMING;
        if(definition->GetBool("aimOnSpawn"))       projectileFlags |= PF_AIMONSPAWN;
        if(definition->GetBool("noHomingThrust"))   projectileFlags |= PF_NOHOMINGTHRUST;
        if(definition->GetBool("stunTarget"))       projectileFlags |= PF_STUNTARGET;

        if(definition->GetBool("spawnLight"))
        {
            float lightRadius;
            int lightPasses;
            kexVec3 lightColor;

            definition->GetVector("lightColor", lightColor);
            definition->GetFloat("lightRadius", lightRadius);
            definition->GetInt("lightPasses", lightPasses);

            kexGame::cLocal->SpawnDynamicLight(this, lightRadius, lightColor, -1, lightPasses);
        }
    }

    if(projectileFlags & PF_AIMONSPAWN)
    {
        projectileFlags |= PF_AIMING;
    }

    r = (radius * 0.5f) * scale;
    h = (height * 0.5f) * scale;
    
    bounds.min.Set(-r, -r, -h);
    bounds.max.Set(r, r, h);
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
    float lt = (float)lifeTime;

    if(Removing())
    {
        return;
    }
    
    if(flags & AF_INWATER)
    {
        Remove();
        return;
    }

    kexProjectile::Tick();

    if(anim == deathAnim)
    {
        return;
    }

    fizzleTime += 0.5f;
    scale += ((5000 - ((fizzleTime * 65536) / 512)) / 128) / 512;

    if(fizzleTime < lt * 0.5f)
    {
        color.Lerp(kexVec3(1.0f, 1.0f, 0.25f), 1.0f / (lt * 0.5f));
    }
    else
    {
        color.Lerp(kexVec3(1.0f, 0.25f, 0.0625f), 1.0f / (lt * 0.5f));
    }

    if(fizzleTime >= lt)
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

//-----------------------------------------------------------------------------
//
// kexFireballSpawner
//
//-----------------------------------------------------------------------------

DECLARE_KEX_CLASS(kexFireballSpawner, kexActor)

//
// kexProjectile::kexProjectile
//

kexFireballSpawner::kexFireballSpawner(void)
{
    this->fireDelay = 50;
    this->bEnabled = false;
}

//
// kexFireballSpawner::~kexFireballSpawner
//

kexFireballSpawner::~kexFireballSpawner(void)
{
}

//
// kexFireballSpawner::Tick
//

void kexFireballSpawner::Tick(void)
{
    if(Removing())
    {
        return;
    }

    if(mapActor == NULL)
    {
        Remove();
        return;
    }

    kexActor::Tick();

    if(!bEnabled)
    {
        return;
    }

    fireDelay -= 0.5f;

    if(fireDelay <= 0)
    {
        bEnabled = false;

        mapFace_t *face = &kexGame::cWorld->Faces()[mapActor->params2];
        mapPoly_t *poly = &kexGame::cWorld->Polys()[mapActor->params1];
        SpawnFireball(face, poly);
    }
}

//
// kexFireballSpawner::SpawnFireball
//

void kexFireballSpawner::SpawnFireball(mapFace_t *face, mapPoly_t *poly)
{
    mapVertex_t *v = kexGame::cWorld->Vertices();
    int secID, projType;
    float speed;
    kexVec3 vOrigin;
    kexProjectile *proj;

    secID = sector - kexGame::cWorld->Sectors();
    
    vOrigin = (v[face->vertStart + poly->indices[0]].origin +
               v[face->vertStart + poly->indices[1]].origin +
               v[face->vertStart + poly->indices[2]].origin);

    if(poly->indices[3] != 0xff)
    {
        vOrigin += v[face->vertStart + poly->indices[3]].origin;
        vOrigin /= 4;
    }
    else
    {
        vOrigin /= 3;
    }

    vOrigin += (face->plane.Normal() * 32);

    switch(type)
    {
    case AT_FIREBALLSPAWNER:
        kexGame::cActorFactory->Spawn(AT_FIREBALLPUFF, vOrigin.x, vOrigin.y, vOrigin.z, 0, secID);
        projType = AT_FIREBALL;
        speed = 12;
        break;

    case AT_LASERSPAWNER:
        projType = AT_LASER;
        speed = 16;
        break;

    default:
        return;
    }

    proj = static_cast<kexProjectile*>(kexGame::cActorFactory->Spawn(projType,
                                       vOrigin.x, vOrigin.y, vOrigin.z,
                                       face->plane.Normal().ToYaw(),
                                       secID));

    if(proj == NULL)
    {
        return;
    }

    proj->SetTarget(this);
    proj->Velocity() = (face->plane.Normal() * speed);
    proj->PlaySound("sounds/fballshoot.wav");
    proj->ProjectileFlags() |= PF_NOCLIPINITIALSECTOR;
    proj->InitialSector() = secID;
}

//
// kexFireballSpawner::OnActivate
//

void kexFireballSpawner::OnActivate(kexActor *instigator)
{
    bEnabled = true;
}

//
// kexFireballSpawner::OnDeactivate
//

void kexFireballSpawner::OnDeactivate(kexActor *instigator)
{
    bEnabled = false;
}

//
// kexFireballSpawner::Spawn
//

void kexFireballSpawner::Spawn(void)
{
    flags |= AF_HIDDEN;
}

//-----------------------------------------------------------------------------
//
// kexFireballFactory
//
//-----------------------------------------------------------------------------

DECLARE_KEX_CLASS(kexFireballFactory, kexGameObject)

//
// kexProjectile::kexProjectile
//

kexFireballFactory::kexFireballFactory(void)
{
    this->intervals = 60;
    this->currentTime = 0;
    this->fireballType = -1;
    this->extraDelay = 0;
    this->sector = NULL;
}

//
// kexFireballFactory::~kexFireballFactory
//

kexFireballFactory::~kexFireballFactory(void)
{
}

//
// kexFireballFactory::Remove
//

void kexFireballFactory::Remove(void)
{
    for(kexActor *actor = sector->actorList.Next();
        actor != NULL;
        actor = actor->SectorLink().Next())
    {
        if(actor->MapActor() == NULL)
        {
            continue;
        }

        if(actor->MapActor()->type != fireballType)
        {
            continue;
        }

        actor->OnDeactivate(NULL);
    }

    sector->flags &= ~SF_SPECIAL;
    sector->objectThinker = NULL;
    kexGameObject::Remove();
}

//
// kexFireballFactory::Tick
//

void kexFireballFactory::Tick(void)
{
    float fireDelay = extraDelay;

    if(sector == NULL)
    {
        Remove();
        return;
    }

    currentTime += 0.5f;

    if(currentTime >= intervals)
    {
        currentTime = 0;

        for(kexActor *actor = sector->actorList.Next();
            actor != NULL;
            actor = actor->SectorLink().Next())
        {
            if(actor->MapActor() == NULL)
            {
                continue;
            }

            if(actor->MapActor()->type != fireballType)
            {
                continue;
            }

            actor->OnActivate(NULL);
            if(actor->InstanceOf(&kexFireballSpawner::info))
            {
                if(fireDelay > intervals)
                {
                    fireDelay = (fireDelay - intervals);
                }

                static_cast<kexFireballSpawner*>(actor)->fireDelay = fireDelay;
                fireDelay += 8;
            }
        }
    }
}

//
// kexFireballFactory::Spawn
//

void kexFireballFactory::Spawn(void)
{
    switch(fireballType)
    {
    case AT_FIREBALLSPAWNER:
        intervals = 60;
        break;

    case AT_LASERSPAWNER:
        intervals = 10;
        break;

    default:
        break;
    }

    currentTime = intervals;
}

//
// kexFireballFactory::SetSector
//

void kexFireballFactory::SetSector(mapSector_t *s)
{
    sector = s;
    sector->flags |= SF_SPECIAL;
    sector->objectThinker = this;
}
