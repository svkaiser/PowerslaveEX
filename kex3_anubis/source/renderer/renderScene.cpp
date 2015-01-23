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
// kexRenderScene::SetInitialScissorRect
//

void kexRenderScene::SetInitialScissorRect(void)
{
    int clipY = (int)((float)kex::cSystem->VideoHeight() / (240.0f / 24.0f));
    
    kexRender::cBackend->SetState(GLSTATE_SCISSOR, true);
    kexRender::cBackend->SetScissorRect(0, clipY, kex::cSystem->VideoWidth(), kex::cSystem->VideoHeight());
}

//
// kexRenderScene::RecursiveSectorPortals
//

void kexRenderScene::RecursiveSectorPortals(mapSector_t *sector, kexViewBounds *vb)
{
    mapFace_t *face;
    mapSector_t *next;
    kexViewBounds viewBounds;
    int start, end;
    
    if(!view->Frustum().TestBoundingBox(sector->bounds))
    {
        return;
    }
    
    if(sector->floodCount == 1)
    {
        return;
    }
    
    sector->floodCount = 1;
    
    start = sector->faceStart;
    end = sector->faceEnd;
    
    for(int i = start; i < end+3; ++i)
    {
        face = &world->Faces()[i];
        
        if(!(face->flags & FF_PORTAL) || face->sector <= -1)
        {
            continue;
        }

        if(world->Sectors()[face->sector].floodCount == 1)
        {
            continue;
        }

        if(!kexGame::cLocal->CModel()->PointOnFaceSide(view->Origin(), face))
        {
            continue;
        }
        
        if(!view->Frustum().TestBoundingBox(face->bounds))
        {
            continue;
        }
        
        viewBounds.AddVector(view, *face->TopEdge()->v1);
        viewBounds.AddVector(view, *face->RightEdge()->v1);
        viewBounds.AddVector(view, *face->BottomEdge()->v1);
        viewBounds.AddVector(view, *face->LeftEdge()->v1);
        
        if(viewBounds.IsClosed())
        {
            continue;
        }
        
        if(!viewBounds.ViewBoundInside(*vb))
        {
            continue;
        }
        
        next = &world->Sectors()[face->sector];
        
        visibleSectors.Set(next);
        visiblePortals.Set(viewBounds);
        
        RecursiveSectorPortals(next, &viewBounds);
    }
}

//
// kexRenderScene::FindVisibleSectors
//

void kexRenderScene::FindVisibleSectors(void)
{
    mapSector_t *startSector = kexGame::cLocal->Player()->Actor()->Sector();
    kexViewBounds viewBounds;
    
    viewBounds.Fill();
    
    visibleSectors.Reset();
    visiblePortals.Reset();
    visibleSectors.Set(startSector);
    
    RecursiveSectorPortals(startSector, &viewBounds);
    validcount++;
}

//
// kexRenderScene::Draw
//

void kexRenderScene::Draw(void)
{
    kexRender::cBackend->LoadProjectionMatrix(view->ProjectionView());
    kexRender::cBackend->LoadModelViewMatrix(view->ModelView());
    
    SetInitialScissorRect();
    FindVisibleSectors();
}
