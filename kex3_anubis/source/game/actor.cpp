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
    this->frameID = 0;
    this->ticks = 0;
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
}

//
// kexActor::Spawn
//

void kexActor::Spawn(void)
{
    if(anim == NULL)
    {
        anim = &kexGame::cLocal->SpriteAnimManager()->defaultAnim;
    }
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
            return true;
        }
    }
    
    return false;
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
    
    if(anim == NULL)
    {
        return;
    }
    
    frame = &anim->frames[frameID];
    ticks += (1.0f / (float)frame->delay) * 0.5f;
    
    // handle advancing to next frame
    if(ticks >= 1)
    {
        ticks = 0;
        
        // reached the end of the frame?
        if(++frameID >= (int16_t)anim->NumFrames())
        {
            // loop back
            frameID = 0;
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
