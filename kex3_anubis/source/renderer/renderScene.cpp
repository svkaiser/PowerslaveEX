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
//      Scene Rendering
//

#include "kexlib.h"
#include "game.h"
#include "renderMain.h"
#include "renderView.h"
#include "renderScene.h"
#include "viewBounds.h"

//
// kexRenderScene::kexRenderScene
//

kexRenderScene::kexRenderScene(void)
{
    this->world = NULL;
    this->view = NULL;
}

//
// kexRenderScene::~kexRenderScene
//

kexRenderScene::~kexRenderScene(void)
{
}

//
// kexRenderScene::FloodPortalView
//

void kexRenderScene::FloodPortalView(portal_t *portal, portal_t *prevPortal)
{
    kexVec3 p1, p2;
    kexVec3 rvO;
    float an1, an2;
    float an3, an4, an5, an6;

    portal->hClipSpan.Clear();
    portal->vClipSpan[0].Clear();
    portal->vClipSpan[1].Clear();

    p1 = *portal->face->BottomEdge()->v2;
    p2 = *portal->face->BottomEdge()->v1;
                
    an1 = (view->Origin() - p1).ToYaw();
    an2 = (view->Origin() - p2).ToYaw();
                
    rvO = view->Origin() + kexVec3(0, 0, 32);
    an3 = (*portal->face->LeftEdge()->v1 - rvO).ToPitch();
    an4 = (*portal->face->LeftEdge()->v2 - rvO).ToPitch();
    an5 = (*portal->face->RightEdge()->v2 - rvO).ToPitch();
    an6 = (*portal->face->RightEdge()->v1 - rvO).ToPitch();

    portal->hClipSpan.AddRangeSpan(an2, an1);
    portal->vClipSpan[0].AddRangeSpan(an4, an3);
    portal->vClipSpan[1].AddRangeSpan(an6, an5);

    portal->face->leftSpan = an1;
    portal->face->rightSpan = an2;
    portal->face->bottomSpan = an3;
    portal->face->topSpan = an4;

    RecursiveSectorPortals(portal);
}

//
// kexRenderScene::FaceInPortalView
//

bool kexRenderScene::FaceInPortalView(portal_t *portal, mapFace_t *face)
{
    kexVec3 rvO;
    float an1, an2;
    float an3, an4;
    float an5, an6;

    rvO = view->Origin() + kexVec3(0, 0, 32);

    an1 = (view->Origin() - *face->BottomEdge()->v2).ToYaw();
    an2 = (view->Origin() - *face->BottomEdge()->v1).ToYaw();
    an3 = (*face->LeftEdge()->v1 - rvO).ToPitch();
    an4 = (*face->LeftEdge()->v2 - rvO).ToPitch();
    an5 = (*face->RightEdge()->v2 - rvO).ToPitch();
    an6 = (*face->RightEdge()->v1 - rvO).ToPitch();
    
    if(!portal->hClipSpan.CheckRange(an1, an2))
    {
        return false;
    }

    /*if(!portal->face->plane.IsFacing(view->Yaw()))
    {
        if(!portal->vClipSpan[0].CheckRange(an3, an4) &&
           !portal->vClipSpan[1].CheckRange(an5, an6))
        {
            return false;
        }
    }*/

    return true;
}

//
// kexRenderScene::RecursiveSectorPortals
//

void kexRenderScene::RecursiveSectorPortals(portal_t *portal)
{
    mapFace_t *face;
    int start, end;
    mapSector_t *sector;

    sector = &world->Sectors()[portal->face->sector];
    
    if(sector->floodCount == validcount)
    {
        return;
    }
    
    sector->floodCount = validcount;

    if(sector->validcount != validcount)
    {
        visibleSectors.Set(sector);
        sector->validcount = validcount;
    }
    
    start = sector->faceStart;
    end = sector->faceEnd;

    for(int i = start; i < end+1; ++i)
    {
        mapSector_t *s;

        face = &world->Faces()[i];

        if(!face->portal)
        {
            continue;
        }

        s = &world->Sectors()[face->sector];

        if(s->validcount != validcount)
        {
            visibleSectors.Set(s);
            s->validcount = validcount;
        }
    }
    
    for(int i = start; i < end+3; ++i)
    {
        face = &world->Faces()[i];

        if(face->validcount == 1)
        {
            continue;
        }
        
        if(i <= end && !face->InFront(view->Origin()))
        {
            continue;
        }
        
        if(!view->Frustum().TestBoundingBox(face->bounds))
        {
            continue;
        }

        if(i <= end && !FaceInPortalView(portal, face))
        {
            if(sector->validcount == validcount)
            {
                sector->floodCount = 0;
            }
            continue;
        }

        face->validcount = 1;
        
        if(i >= end+1)
        {
            continue;
        }

        if(!(face->flags & FF_PORTAL) || face->sector <= -1)
        {
            continue;
        }

        if(face->portal == NULL)
        {
            continue;
        }
        
        FloodPortalView(face->portal, portal);
    }

    
}

//
// kexRenderScene::FindVisibleSectors
//

void kexRenderScene::FindVisibleSectors(mapSector_t *startSector)
{
    mapFace_t *face;
    int start, end;

    visibleSectors.Reset();
    visibleSectors.Set(startSector);

    start = startSector->faceStart;
    end = startSector->faceEnd;

    for(int i = start; i < end+1; ++i)
    {
        mapSector_t *s;

        face = &world->Faces()[i];

        if(!face->portal)
        {
            continue;
        }

        s = &world->Sectors()[face->sector];

        if(s->validcount != validcount)
        {
            visibleSectors.Set(s);
            s->validcount = validcount;
        }
    }

    for(int i = start; i < end+3; ++i)
    {
        face = &world->Faces()[i];

        if(i <= end && !face->InFront(view->Origin()))
        {
            continue;
        }
        
        if(!view->Frustum().TestBoundingBox(face->bounds))
        {
            continue;
        }

        face->validcount = 1;
        
        if(i >= end+1)
        {
            continue;
        }

        if(!(face->flags & FF_PORTAL) || face->sector <= -1)
        {
            continue;
        }

        if(face->portal == NULL)
        {
            continue;
        }
        
        FloodPortalView(face->portal, NULL);
    }

    validcount++;
}

//
// kexRenderScene::Draw
//

void kexRenderScene::Draw(void)
{
}
