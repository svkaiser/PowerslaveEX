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
//      Play loop (in-game) logic
//

#include "kexlib.h"
#include "renderMain.h"
#include "renderView.h"
#include "game.h"
#include "actor.h"
#include "player.h"
#include "playLoop.h"
#include "cmodel.h"

//
// kexPlayLoop::kexPlayLoop
//

kexPlayLoop::kexPlayLoop(void)
{
}

//
// kexPlayLoop::~kexPlayLoop
//

kexPlayLoop::~kexPlayLoop(void)
{
}

//
// kexPlayLoop::Init
//

void kexPlayLoop::Init(void)
{
}

//
// kexPlayLoop::Start
//

void kexPlayLoop::Start(void)
{
    ticks = 0;
    
    if(kex::cGame->Player()->Actor() == NULL)
    {
        kex::cSystem->Warning("No player starts present\n");
        kex::cGame->SetGameState(GS_TITLE);
    }
}

//
// kexPlayLoop::Stop
//

void kexPlayLoop::Stop(void)
{
    kex::cGame->World()->UnloadMap();
}

//
// kexPlayLoop::Draw
//

void kexPlayLoop::Draw(void)
{
    // TEMP
    kex::cGame->RenderView()->Yaw() = kex::cGame->Player()->Actor()->Yaw();
    kex::cGame->RenderView()->Pitch() = kex::cGame->Player()->Actor()->Pitch();
    kex::cGame->RenderView()->Roll() = kex::cGame->Player()->Actor()->Roll();
    kex::cGame->RenderView()->Origin() = kex::cGame->Player()->Actor()->Origin();
    
    kex::cGame->RenderView()->Origin().z += 64 + kex::cGame->Player()->Bob();
    
    kex::cGame->RenderView()->Setup();
    kexRender::cBackend->LoadProjectionMatrix(kex::cGame->RenderView()->ProjectionView());
    kexRender::cBackend->LoadModelViewMatrix(kex::cGame->RenderView()->ModelView());
    kexRender::cUtils->DrawBoundingBox(kexBBox(
        kexVec3(-64, -128, -32),
        kexVec3(64, 128, 32)), 255, 0, 0);
    
    kexCpuVertList *vl = kexRender::cVertList;
    kexWorld *world = kex::cGame->World();
    
    //kexRender::cTextures->defaultTexture->Bind();
    kexRender::cBackend->SetState(GLSTATE_DEPTHTEST, true);
    kexRender::cBackend->SetState(GLSTATE_ALPHATEST, true);
    kexRender::cBackend->SetState(GLSTATE_BLEND, true);
    
    if(world->MapLoaded())
    {
        for(unsigned int i = 0; i < world->NumSectors(); ++i)
        {
            mapSector_t *sector = &world->Sectors()[i];
            
            int start = sector->faceStart;
            int end = sector->faceEnd;

            if(!kex::cGame->RenderView()->Frustum().TestBoundingBox(sector->bounds))
            {
                continue;
            }

            if(sector->flags & SF_DEBUG)
            {
                kexRender::cUtils->DrawBoundingBox(sector->bounds, 255, 128, 0);
                sector->flags &= ~SF_DEBUG;
            }
            
            for(int j = start; j < end+3; ++j)
            {
                mapFace_t *face = &world->Faces()[j];
                
                if(face->polyStart == -1 || face->polyEnd == -1)
                {
                    continue;
                }
                
                if(!kex::cGame->RenderView()->Frustum().TestBoundingBox(face->bounds))
                {
                    continue;
                }
                
                if(face->BottomEdge()->flags & EGF_TOPSTEP)
                {
                    kexRender::cUtils->DrawLine(*face->BottomEdge()->v1, *face->BottomEdge()->v2, 0, 255, 0);
                }
                if(face->TopEdge()->flags & EGF_BOTTOMSTEP)
                {
                    kexRender::cUtils->DrawLine(*face->TopEdge()->v1, *face->TopEdge()->v2, 255, 0, 0);
                }

                /*if(j <= end)
                {
                    kexRender::cUtils->DrawBoundingBox(face->bounds, 8, 255, 32);
                }
                else
                {
                    kexRender::cUtils->DrawBoundingBox(face->bounds, 128, 64, 255);
                }*/
                
                for(int k = face->polyStart; k <= face->polyEnd; ++k)
                {
                    int tris = 0;

                    mapPoly_t *poly = &world->Polys()[k];
                    mapTexCoords_t *tcoord = &world->TexCoords()[poly->tcoord];
                    mapVertex_t *vertex;
                    
                    int indices[4] = { 0, 0, 0, 0 };
                    int tcoords[4] = { 0, 0, 0, 0 };
                    int curIdx = 0;

                    if(world->Textures()[poly->texture])
                    {
                        world->Textures()[poly->texture]->Bind();
                    }
                    
                    for(int idx = 0; idx < 4; idx++)
                    {
                        if(poly->indices[idx] == poly->indices[(idx+1)&3])
                        {
                            continue;
                        }
                        
                        indices[curIdx] = poly->indices[idx];
                        tcoords[curIdx] = idx;
                        curIdx++;
                    }
                    
                    if(poly->flipped == 0)
                    {
                        for(int idx = 0; idx < curIdx; idx++)
                        {
                            vertex = &world->Vertices()[face->vertStart + indices[idx]];
                            
                            vl->AddVertex(vertex->origin,
                                          tcoord->uv[tcoords[idx]].s, 1.0f - tcoord->uv[tcoords[idx]].t,
                                          vertex->rgba[0],
                                          vertex->rgba[1],
                                          vertex->rgba[2],
                                          255);
                        }
                    }
                    else
                    {
                        for(int idx = (curIdx-1); idx >= 0; idx--)
                        {
                            vertex = &world->Vertices()[face->vertStart + indices[idx]];
                            
                            vl->AddVertex(vertex->origin,
                                          tcoord->uv[tcoords[idx]].s, 1.0f - tcoord->uv[tcoords[idx]].t,
                                          vertex->rgba[0],
                                          vertex->rgba[1],
                                          vertex->rgba[2],
                                          255);
                        }
                    }
                    
                    vl->AddTriangle(tris+0, tris+2, tris+1);
                    if(curIdx == 4)
                    {
                        vl->AddTriangle(tris+0, tris+3, tris+2);
                    }
                    
                    tris += curIdx;

                    vl->DrawElements();
                }
            }
        }
    }
}

//
// kexPlayLoop::Tick
//

void kexPlayLoop::Tick(void)
{
    if(ticks > 4)
    {
        kex::cGame->UpdateActors();
        kex::cGame->Player()->Tick();
    }
    
    ticks++;
}

//
// kexPlayLoop::ProcessInput
//

bool kexPlayLoop::ProcessInput(inputEvent_t *ev)
{
    return false;
}
