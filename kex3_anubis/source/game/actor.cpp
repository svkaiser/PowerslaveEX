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

#define AMOVE_FRICTION          0.5f
#define AMOVE_MIN               0.125f
#define AMOVE_SPEED_FALL        0.75f

DECLARE_KEX_CLASS(kexActor, kexGameObject)

//
// kexActor::kexActor
//

kexActor::kexActor(void)
{
    this->link.SetData(this);
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
        definition->GetInt("health", health, 100);

        if(definition->GetBool("noAdvanceFrames"))  flags |= AF_NOADVANCEFRAMES;
        if(definition->GetBool("randomizeFrames"))  flags |= AF_RANDOMIZATION;
        if(definition->GetBool("solid"))            flags |= AF_SOLID;
        if(definition->GetBool("shootable"))        flags |= AF_SHOOTABLE;
        if(definition->GetBool("fullbright"))       flags |= AF_FULLBRIGHT;
        if(definition->GetBool("moveable"))         flags |= AF_MOVEABLE;
        if(definition->GetBool("touchable"))        flags |= AF_TOUCHABLE;
        if(definition->GetBool("bouncy"))           flags |= AF_BOUNCY;
        
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
    
    r = (radius * scale) * 0.5f;
    h = (height * scale) * 0.5f;
    
    bounds.min.Set(-r, -r, -h);
    bounds.max.Set(r, r, h);

    if(flags & AF_RANDOMIZATION)
    {
        if(anim->NumFrames() > 0 && !(flags & AF_NOADVANCEFRAMES))
        {
            frameID = kexRand::Max(anim->NumFrames());
        }
    }
    
    link.Add(kexGame::cLocal->Actors());
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
    ChangeAnim(kexGame::cLocal->SpriteAnimManager()->Get(animName));
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

    // handle goto jumps
    if(frame->HasNextFrame())
    {
        ChangeAnim(frame->nextFrame);
    }
}

//
// kexActor::InflictDamage
//

void kexActor::InflictDamage(kexActor *inflictor, const int amount)
{
    flags |= AF_FLASH;
    flashTicks = 1;

    if(health > 0)
    {
        health -= amount;

        if(health <= 0)
        {
            ChangeAnim(deathAnim);
        }
    }
}

//
// kexActor::UpdateMovement
//

void kexActor::UpdateMovement(void)
{
    // check for drop-offs
    if(origin.z > floorHeight)
    {
        velocity.z -= AMOVE_SPEED_FALL;
    }
    else
    {
        // apply friction
        velocity.x *= AMOVE_FRICTION;
        velocity.y *= AMOVE_FRICTION;
    }
    
    if(kexMath::Fabs(velocity.x) < AMOVE_MIN)
    {
        velocity.x = 0;
    }
    
    if(kexMath::Fabs(velocity.y) < AMOVE_MIN)
    {
        velocity.y = 0;
    }
    
    // bump floor
    if(origin.z + velocity.z <= floorHeight)
    {
        origin.z = floorHeight;
        
        if(flags & AF_BOUNCY && (kexMath::Fabs(velocity.z) * 0.75f) > AMOVE_SPEED_FALL)
        {
            velocity.z = (floorHeight - (origin.z + velocity.z)) * (1.2f - AMOVE_SPEED_FALL);
            velocity.x *= AMOVE_FRICTION;
            velocity.y *= AMOVE_FRICTION;
        }
        else
        {
            velocity.z = 0;
        }
    }
    
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
