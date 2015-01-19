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
    
    if(kexGame::cLocal->Player()->Actor() == NULL)
    {
        kex::cSystem->Warning("No player starts present\n");
        kexGame::cLocal->SetGameState(GS_TITLE);
    }
}

//
// kexPlayLoop::Stop
//

void kexPlayLoop::Stop(void)
{
    kexGame::cLocal->World()->UnloadMap();
}

//
// kexPlayLoop::Draw
//

void kexPlayLoop::Draw(void)
{
    kexPlayer *p = kexGame::cLocal->Player();

    // TEMP
    kexGame::cLocal->RenderView()->Yaw() = p->Actor()->Yaw();
    kexGame::cLocal->RenderView()->Pitch() = p->Actor()->Pitch();
    kexGame::cLocal->RenderView()->Roll() = p->Actor()->Roll();
    kexGame::cLocal->RenderView()->Origin() = p->Actor()->Origin();
    
    kexGame::cLocal->RenderView()->Origin().z += 64 + p->Bob() + p->LandTime() + p->StepViewZ();
    
    kexGame::cLocal->RenderView()->Setup();
    kexRender::cBackend->LoadProjectionMatrix(kexGame::cLocal->RenderView()->ProjectionView());
    kexRender::cBackend->LoadModelViewMatrix(kexGame::cLocal->RenderView()->ModelView());
    kexRender::cUtils->DrawBoundingBox(kexBBox(
        kexVec3(-64, -128, -32),
        kexVec3(64, 128, 32)), 255, 0, 0);
    
    kexCpuVertList *vl = kexRender::cVertList;
    kexWorld *world = kexGame::cLocal->World();
    
    //kexRender::cTextures->defaultTexture->Bind();
    kexRender::cBackend->SetState(GLSTATE_DEPTHTEST, true);
    kexRender::cBackend->SetState(GLSTATE_ALPHATEST, true);
    kexRender::cBackend->SetState(GLSTATE_BLEND, true);
    kexRender::cBackend->SetState(GLSTATE_SCISSOR, true);

    int clipY = (int)((float)kex::cSystem->VideoHeight() / (240.0f / 24.0f));
    kexRender::cBackend->SetScissorRect(0, clipY, kex::cSystem->VideoWidth(), kex::cSystem->VideoHeight());
    
    if(world->MapLoaded())
    {
        for(kexActor *actor = kexGame::cLocal->Actors().Next(); actor != NULL; actor = actor->Link().Next())
        {
            kexRender::cUtils->DrawOrigin(actor->Origin().x, actor->Origin().y, actor->Origin().z, 16);
        }

        for(unsigned int i = 0; i < world->NumSectors(); ++i)
        {
            mapSector_t *sector = &world->Sectors()[i];
            
            int start = sector->faceStart;
            int end = sector->faceEnd;

            if(!kexGame::cLocal->RenderView()->Frustum().TestBoundingBox(sector->bounds))
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
                
                if(!kexGame::cLocal->RenderView()->Frustum().TestBoundingBox(face->bounds))
                {
                    continue;
                }
                
                if(face->BottomEdge()->flags & EGF_TOPSTEP)
                {
                    //kexRender::cUtils->DrawLine(*face->BottomEdge()->v1, *face->BottomEdge()->v2, 0, 255, 0);
                }
                if(face->TopEdge()->flags & EGF_BOTTOMSTEP)
                {
                    //kexRender::cUtils->DrawLine(*face->TopEdge()->v1, *face->TopEdge()->v2, 255, 0, 0);
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

        {
            static spriteAnim_t *anim = p->WeaponAnim();
            const kexGameLocal::weaponInfo_t *weaponInfo = kexGame::cLocal->WeaponInfo(p->CurrentWeapon());

            if(anim)
            {
                spriteFrame_t *frame = p->WeaponFrame();
                spriteSet_t *spriteSet;
                kexSprite *sprite;
                spriteInfo_t *info;

                kexRender::cScreen->SetOrtho();
                kexRender::cBackend->SetState(GLSTATE_DEPTHTEST, false);
                kexRender::cBackend->SetBlend(GLSRC_SRC_ALPHA, GLDST_ONE_MINUS_SRC_ALPHA);

                kexCpuVertList *vl = kexRender::cVertList;
                vl->BindDrawPointers();

                for(unsigned int i = 0; i < frame->spriteSet.Length(); ++i)
                {
                    spriteSet = &frame->spriteSet[i];
                    sprite = spriteSet->sprite;
                    info = &sprite->InfoList()[spriteSet->index];

                    float x = (float)spriteSet->x;
                    float y = (float)spriteSet->y;
                    float w = (float)info->atlas.w;
                    float h = (float)info->atlas.h;
                    byte c;

                    float u1, u2, v1, v2;
                    
                    u1 = info->u[0 ^ spriteSet->bFlipped];
                    u2 = info->u[1 ^ spriteSet->bFlipped];
                    v1 = info->v[0];
                    v2 = info->v[1];

                    kexRender::cScreen->SetAspectDimentions(x, y, w, h);

                    sprite->Texture()->Bind();

                    x += p->WeaponBobX() + weaponInfo->offsetX;
                    y += p->WeaponBobY() + weaponInfo->offsetY;

                    c = (byte)(p->Actor()->Sector()->lightLevel << 1);

                    vl->AddQuad(x, y + 8, 0, w, h, u1, v1, u2, v2, c, c, c, 255);
                    vl->DrawElements();
                }
            }
        }
    }

    kexRender::cBackend->SetState(GLSTATE_SCISSOR, false);
}

//
// kexPlayLoop::Tick
//

void kexPlayLoop::Tick(void)
{
    if(ticks > 4)
    {
        kexGame::cLocal->UpdateActors();
        kexGame::cLocal->Player()->Tick();
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
