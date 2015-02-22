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
//      Actor Object
//

#include "kexlib.h"
#include "game.h"

#define AMOVE_MIN   0.125f

DECLARE_KEX_CLASS(kexActor, kexGameObject)

//
// kexActor::kexActor
//

kexActor::kexActor(void)
{
    this->bounds.min.Set(-32, -32, -32);
    this->bounds.max.Set(32, 32, 32);
    this->type = AT_INVALID;
    this->health = 100;
    this->radius = 16;
    this->height = 32;
    this->scale = 1;
    this->stepHeight = 16;
    this->flags = 0;
    this->anim = NULL;
    this->spawnAnim = NULL;
    this->deathAnim = NULL;
    this->frameID = 0;
    this->ticks = 0;
    this->animSpeed = 0.5f;
    this->flashTicks = 0;
    this->sectorLink.SetData(this);
    this->areaLink.link.SetData(this);
    this->areaLink.node = NULL;
    this->friction = 0.5f;
    this->gravity = 0.75f;
    this->floorOffset = 0;
}

//
// kexActor::~kexActor
//

kexActor::~kexActor(void)
{
}

//
// kexActor::Tick
//

void kexActor::Tick(void)
{
    if(Removing())
    {
        return;
    }

    UpdateSprite();
    
    if(flags & AF_MOVEABLE)
    {
        UpdateMovement();
    }
    
    if(flags & AF_FLASH)
    {
        if(flashTicks-- <= 0)
        {
            flashTicks = 0;
            flags &= ~AF_FLASH;
        }
    }
}

//
// kexActor::OnTouch
//

void kexActor::OnTouch(kexActor *instigator)
{
}

//
// kexActor::Remove
//

void kexActor::Remove(void)
{
    animSpeed = 0;
    flags &= ~(AF_SOLID|AF_SHOOTABLE|AF_MOVEABLE|AF_TOUCHABLE|AF_BOUNCY);
    flags |= AF_NOADVANCEFRAMES;
    
    kexGameObject::Remove();
}

//
// kexActor::OnRemove
//

void kexActor::OnRemove(void)
{
    UnlinkArea();
    UnlinkSector();
    
    kexGameObject::OnRemove();
}

//
// kexActor::Spawn
//

void kexActor::Spawn(void)
{
    float r, h;

    if(definition)
    {
        kexStr animName;
        
        definition->GetFloat("scale", scale, 1);
        definition->GetFloat("radius", radius, 16);
        definition->GetFloat("height", height, 32);
        definition->GetFloat("stepHeight", stepHeight, 16);
        definition->GetFloat("animSpeed", animSpeed, 0.5f);
        definition->GetFloat("friction", friction, 0.5f);
        definition->GetFloat("gravity", gravity, 0.75f);
        definition->GetFloat("floorOffset", floorOffset, 0);
        definition->GetInt("health", health, 100);

        if(definition->GetBool("noAdvanceFrames"))  flags |= AF_NOADVANCEFRAMES;
        if(definition->GetBool("randomizeFrames"))  flags |= AF_RANDOMIZATION;
        if(definition->GetBool("solid"))            flags |= AF_SOLID;
        if(definition->GetBool("shootable"))        flags |= AF_SHOOTABLE;
        if(definition->GetBool("fullbright"))       flags |= AF_FULLBRIGHT;
        if(definition->GetBool("moveable"))         flags |= AF_MOVEABLE;
        if(definition->GetBool("touchable"))        flags |= AF_TOUCHABLE;
        if(definition->GetBool("bouncy"))           flags |= AF_BOUNCY;
        if(definition->GetBool("noDropOff"))        flags |= AF_NODROPOFF;
        if(definition->GetBool("expires"))          flags |= AF_EXPIRES;

        if(flags & AF_BOUNCY)
        {
            definition->GetString("bounceSound_1", bounceSounds[0]);
            definition->GetString("bounceSound_2", bounceSounds[1]);
            definition->GetString("bounceSound_3", bounceSounds[2]);
        }
        
        if(definition->GetString("spawnAnim", animName))
        {
            spawnAnim = kexGame::cLocal->SpriteAnimManager()->Get(animName);
            ChangeAnim(spawnAnim);
        }
        if(definition->GetString("deathAnim", animName))
        {
            deathAnim = kexGame::cLocal->SpriteAnimManager()->Get(animName);
        }
        
        definition->GetInt("initialFrame", frameID, 0);
    }
    
    if(sector)
    {
        floorHeight = kexGame::cLocal->CModel()->GetFloorHeight(origin, sector);
        ceilingHeight = kexGame::cLocal->CModel()->GetCeilingHeight(origin, sector);
    }
    
    if(anim == NULL)
    {
        anim = &kexGame::cLocal->SpriteAnimManager()->defaultAnim;
    }
    
    velocity.Clear();

    r = (radius * scale) * 0.5f;
    h = (height * scale) * 0.5f;
    
    bounds.min.Set(-r, -r, -h);
    bounds.max.Set(r, r, h);

    if(flags & AF_RANDOMIZATION)
    {
        if(anim->NumFrames() > 0)
        {
            frameID = kexRand::SysRand() % anim->NumFrames();
        }
    }
    
    LinkArea();
}

//
// kexActor::FindSector
//

bool kexActor::FindSector(const kexVec3 &pos)
{
    mapSector_t *sector;
    
    for(unsigned int i = 0; i < kexGame::cLocal->World()->NumSectors(); ++i)
    {
        sector = &kexGame::cLocal->World()->Sectors()[i];
        
        if(kexGame::cLocal->CModel()->PointWithinSectorEdges(pos, sector))
        {
            SetSector(sector);
            floorHeight = kexGame::cLocal->CModel()->GetFloorHeight(origin, sector);
            ceilingHeight = kexGame::cLocal->CModel()->GetCeilingHeight(origin, sector);
            return true;
        }
    }
    
    return false;
}

//
// kexActor::SetSector
//

void kexActor::SetSector(mapSector_t *s)
{
    sector = s;
    LinkSector();
}

//
// kexActor::SectorIndex
//

const int kexActor::SectorIndex(void)
{
    return (sector - kexGame::cLocal->World()->Sectors());
}

//
// kexActor::ChangeAnim
//

void kexActor::ChangeAnim(spriteAnim_t *changeAnim)
{
    if(changeAnim == NULL)
    {
        return;
    }
    
    anim = changeAnim;
    frameID = 0;
    ticks = 0;

    for(unsigned int i = 0; i < anim->frames[0].actions.Length(); ++i)
    {
        anim->frames[0].actions[i]->Execute(this);
    }
}

//
// kexActor::ChangeAnim
//

void kexActor::ChangeAnim(const char *animName)
{
    spriteAnim_t *sprAnim = kexGame::cLocal->SpriteAnimManager()->Get(animName);
    
    if(sprAnim == NULL)
    {
        kex::cSystem->Warning("kexActor::ChangeAnim - %s not found\n", animName);
        return;
    }
    
    ChangeAnim(sprAnim);
}

//
// kexActor::ChangeAnim
//

void kexActor::ChangeAnim(const kexStr &str)
{
    spriteAnim_t *sprAnim = kexGame::cLocal->SpriteAnimManager()->Get(str.c_str());
    
    if(sprAnim == NULL)
    {
        kex::cSystem->Warning("kexActor::ChangeAnim - %s not found\n", str.c_str());
        return;
    }
    
    ChangeAnim(sprAnim);
}

//
// kexActor::UpdateSprite
//

void kexActor::UpdateSprite(void)
{
    spriteFrame_t *frame;

    if(Removing())
    {
        return;
    }
    
    if(anim == NULL)
    {
        return;
    }
    
    frame = &anim->frames[frameID];
    ticks += (1.0f / (float)frame->delay) * animSpeed;
    
    // handle advancing to next frame
    if(ticks >= 1)
    {
        ticks = 0;

        // handle goto jumps
        if(frame->HasNextFrame())
        {
            ChangeAnim(frame->nextFrame);
        }
        
        if(!(flags & AF_NOADVANCEFRAMES))
        {
            // reached the end of the frame?
            if(++frameID >= (int16_t)anim->NumFrames())
            {
                // loop back
                frameID = 0;
            }
        }

        for(unsigned int i = 0; i < anim->frames[frameID].actions.Length(); ++i)
        {
            anim->frames[frameID].actions[i]->Execute(this);
        }
    }
}

//
// kexActor::OnDamage
//

void kexActor::OnDamage(kexActor *instigator)
{
}

//
// kexActor::InflictDamage
//

void kexActor::InflictDamage(kexActor *inflictor, const int amount)
{
    flags |= AF_FLASH;
    flashTicks = 1;
    
    if(anim == deathAnim)
        return;

    if(health > 0)
    {
        int oldHealth = health;
        
        health -= amount;
        OnDamage(inflictor);

        if(health <= 0 && oldHealth > 0)
        {
            flags &= AF_SHOOTABLE;
            ChangeAnim(deathAnim);
        }
    }
}

//
// kexActor::UpdateVelocity
//

void kexActor::UpdateVelocity(void)
{
    // check for drop-offs
    if(origin.z > floorHeight)
    {
        velocity.z -= gravity;
    }
    else
    {
        // apply friction
        velocity.x *= friction;
        velocity.y *= friction;
    }
    
    if(kexMath::Fabs(velocity.x) < AMOVE_MIN)
    {
        velocity.x = 0;
    }
    
    if(kexMath::Fabs(velocity.y) < AMOVE_MIN)
    {
        velocity.y = 0;
    }
}

//
// kexActor::CheckFloorAndCeilings
//

void kexActor::CheckFloorAndCeilings(void)
{
    float floorz = floorHeight + floorOffset;
    
    // bump ceiling
    if((origin.z + height) + velocity.z >= ceilingHeight)
    {
        if(InstanceOf(&kexProjectile::info))
        {
            kexProjectile *proj = static_cast<kexProjectile*>(this);
            proj->OnImpact(NULL);
            return;
        }
    }
    
    // bump floor
    if(origin.z + velocity.z <= floorz)
    {
        origin.z = floorz;
        
        if(flags & AF_BOUNCY && (kexMath::Fabs(kexMath::Floor(velocity.z)) * 0.75f) > gravity)
        {
            int r = kex::cSession->GetTime() % 3;
            
            velocity.z = (floorz - (origin.z + velocity.z)) * (1.2f - gravity);
            velocity.x *= friction;
            velocity.y *= friction;
            
            if(bounceSounds[r].Length() > 0)
            {
                PlaySound(bounceSounds[r].c_str());
            }
        }
        else
        {
            velocity.z = 0;
        }
        
        if(InstanceOf(&kexProjectile::info))
        {
            kexProjectile *proj = static_cast<kexProjectile*>(this);
            proj->OnImpact(NULL);
            return;
        }
    }
}

//
// kexActor::UpdateMovement
//

void kexActor::UpdateMovement(void)
{
    mapSector_t *oldSector;

    UpdateVelocity();
    CheckFloorAndCeilings();
    
    oldSector = sector;
    movement = velocity;
    
    if(movement.UnitSq() > 0)
    {
        if(!kexGame::cLocal->CModel()->MoveActor(this))
        {
            velocity.Clear();
            movement.Clear();
        }
        else
        {
            velocity = movement;
            if(kexGame::cLocal->CModel()->Fraction() != 1)
            {
                if(InstanceOf(&kexProjectile::info))
                {
                    kexProjectile *proj = static_cast<kexProjectile*>(this);
                    proj->OnImpact(kexGame::cLocal->CModel()->ContactActor());
                    return;
                }
            }
        }

        if(!(oldSector->flags & SF_WATER) && sector->flags & SF_WATER)
        {
            PlaySound("sounds/splash01.wav");
            kexGame::cLocal->SpawnActor(AT_WATERSPLASH,
                origin.x,
                origin.y,
                kexGame::cLocal->CModel()->GetCeilingHeight(origin, sector), 0, SectorIndex());

            flags |= AF_INWATER;
        }
        else if(!(sector->flags & SF_WATER))
        {
            flags &= ~AF_INWATER;
        }
    }
    else if(flags & AF_EXPIRES)
    {
        if(--health <= 0)
        {
            if(health == 0)
            {
                expireAmount = scale / 32;
            }
            
            scale -= expireAmount;
            
            if(scale <= 0)
            {
                Remove();
            }
        }
    }
}

//
// kexActor::LinkArea
//

void kexActor::LinkArea(void)
{
    kexBBox box;

    if(IsStale())
    {
        return;
    }

    box.min.Set(-radius, -radius, 0);
    box.max.Set(radius, radius, 0);
    box.min += origin;
    box.max += origin;

    areaLink.Link(kexGame::cLocal->World()->AreaNodes(), box);
}

//
// kexActor::UnlinkArea
//

void kexActor::UnlinkArea(void)
{
    areaLink.UnLink();
}

//
// kexActor::LinkSector
//

void kexActor::LinkSector(void)
{
    UnlinkSector();
    sectorLink.AddBefore(sector->actorList);
}

//
// kexActor::UnlinkSector
//

void kexActor::UnlinkSector(void)
{
    sectorLink.Remove();
}
