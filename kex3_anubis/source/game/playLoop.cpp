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
#include "viewBounds.h"
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
        return;
    }
    
    kexGame::cLocal->Player()->Ready();
}

//
// kexPlayLoop::Stop
//

void kexPlayLoop::Stop(void)
{
    kexGame::cLocal->World()->UnloadMap();
}

void GetEdgeProjectedPoints(kexRenderView &renderView, kexVec3 &proj1, kexVec3 &proj2, mapEdge_t *edge, bool bDir)
{
    kexVec3 p1, p2;
    kexPlane *top, *bottom, *left, *right, *near;
    float d1, d2, dn1, dn2;
    float td, bd, nd, ld, rd;
    
    top = &renderView.Frustum().Top();
    bottom = &renderView.Frustum().Bottom();
    right = &renderView.Frustum().Right();
    left = &renderView.Frustum().Left();
    near = &renderView.Frustum().Near();
    
    if(bDir)
    {
        p1 = *edge->v2;
        p2 = *edge->v1;
    }
    else
    {
        p1 = *edge->v1;
        p2 = *edge->v2;
    }
    
    kexVec3 lp1, lp2;
    bool bn, bt, bb, bl, br;
    
    d1 = top->Distance(p1) + top->d;
    d2 = top->Distance(p2) + top->d;
    td = d1 / (d1 - d2);
    
    d1 = bottom->Distance(p1) + bottom->d;
    d2 = bottom->Distance(p2) + bottom->d;
    bd = d1 / (d1 - d2);
    
    bt = (td >= 0 && td <= 1) && (dn2 >= 0);
    bb = (bd >= 0 && bd <= 1) && (dn1 >= 0);
    
    if(bt) p1.Lerp(p2, td);
    if(bb) p2.Lerp(p1, 1.0f - bd);
    
    d1 = near->Distance(p1) + near->d;
    d2 = near->Distance(p2) + near->d;
    dn1 = d1;
    dn2 = d2;
    nd = d1 / (d1 - d2);
    
    bn = (nd >= 0 && nd <= 1);
    
    if(bn)
    {
        if(dn1 > dn2)
        {
            p2.Lerp(p1, 1.0f - nd);
        }
        else
        {
            p1.Lerp(p2, nd);
        }
    }
    
    d1 = left->Distance(p1) + left->d;
    d2 = left->Distance(p2) + left->d;
    ld = d1 / (d1 - d2);
    
    d1 = right->Distance(p1) + right->d;
    d2 = right->Distance(p2) + right->d;
    rd = d1 / (d1 - d2);
    
    bl = (ld >= 0 && ld <= 1) && (dn2 >= 0);
    br = (rd >= 0 && rd <= 1) && (dn1 >= 0);
    
    if(bl) p1.Lerp(p2, ld);
    if(br) p2.Lerp(p1, 1.0f - rd);
    
    //kex::cSystem->Printf("%i %i %i %i %i\n", bn, bt, bb, bl, br);
    
    //proj1 = renderView.ProjectPoint(lp1);
    //proj2 = renderView.ProjectPoint(lp2);
    proj1 = p1;
    proj2 = p2;
}

//
// kexPlayLoop::Draw
//

void kexPlayLoop::Draw(void)
{
    kexPlayer *p = kexGame::cLocal->Player();
    kexWorld *world = kexGame::cLocal->World();
    
    renderView.SetupFromPlayer(p);
    //renderScene.Draw(&renderView);

    // TEMP
    kexRender::cBackend->LoadProjectionMatrix(renderView.ProjectionView());
    kexRender::cBackend->LoadModelViewMatrix(renderView.ModelView());
    
    kexCpuVertList *vl = kexRender::cVertList;
    
    //kexRender::cTextures->defaultTexture->Bind();
    kexRender::cBackend->SetState(GLSTATE_DEPTHTEST, true);
    kexRender::cBackend->SetState(GLSTATE_ALPHATEST, true);
    kexRender::cBackend->SetState(GLSTATE_BLEND, true);
    kexRender::cBackend->SetState(GLSTATE_SCISSOR, true);

    int clipY = (int)((float)kex::cSystem->VideoHeight() / (240.0f / 24.0f));
    kexRender::cBackend->SetScissorRect(0, clipY, kex::cSystem->VideoWidth(), kex::cSystem->VideoHeight());
    
    if(world->MapLoaded())
    {
        kexVec3 proj1, proj2, proj3, proj4;
        kexViewBounds viewBounds;
        
        {
            mapFace_t *face = &world->Faces()[1900];
            GetEdgeProjectedPoints(renderView, proj1, proj2, face->TopEdge(), false);
            GetEdgeProjectedPoints(renderView, proj3, proj4, face->BottomEdge(), true);
            viewBounds.AddVector(&renderView, proj1);
            viewBounds.AddVector(&renderView, proj2);
            viewBounds.AddVector(&renderView, proj3);
            viewBounds.AddVector(&renderView, proj4);
        }
        
        for(unsigned int i = 0; i < world->NumSectors(); ++i)
        //for(unsigned int i = 0; i < renderScene.VisibleSectors().CurrentLength(); ++i)
        {
            //mapSector_t *sector = renderScene.VisibleSectors()[i];
            mapSector_t *sector = &world->Sectors()[i];
            
            int start = sector->faceStart;
            int end = sector->faceEnd;
            bool inSector = false;
            
            sector->floodCount = 0;

            if(!renderView.Frustum().TestBoundingBox(sector->bounds))
            {
                continue;
            }

            if(sector->flags & SF_DEBUG)
            {
                kexRender::cUtils->DrawBoundingBox(sector->bounds, 255, 128, 0);
                sector->flags &= ~SF_DEBUG;
                inSector = true;
            }
            
            for(int j = start; j < end+3; ++j)
            {
                mapFace_t *face = &world->Faces()[j];
                
                if(inSector && face->sector != -1)
                {
                    //kex::cSystem->Printf("%i\n", j);
                }
                
                if(face->polyStart == -1 || face->polyEnd == -1)
                {
                    continue;
                }
                
                if(!renderView.Frustum().TestBoundingBox(face->bounds))
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
                
                //kexRender::cUtils->DrawLine(*face->BottomEdge()->v1, *face->BottomEdge()->v2, 0, 255, 0);
                //kexRender::cUtils->DrawLine(*face->TopEdge()->v1, *face->TopEdge()->v2, 0, 255, 0);
                //kexRender::cUtils->DrawLine(*face->LeftEdge()->v1, *face->LeftEdge()->v2, 0, 255, 0);
                //kexRender::cUtils->DrawLine(*face->RightEdge()->v1, *face->RightEdge()->v2, 0, 255, 0);
                
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
            static spriteAnim_t *anim = p->Weapon().Anim();
            const kexGameLocal::weaponInfo_t *weaponInfo = kexGame::cLocal->WeaponInfo(p->CurrentWeapon());
            
            kexRender::cScreen->SetOrtho();
            kexRender::cBackend->SetState(GLSTATE_DEPTHTEST, false);
            kexRender::cBackend->SetBlend(GLSRC_SRC_ALPHA, GLDST_ONE_MINUS_SRC_ALPHA);
            
            kexCpuVertList *vl = kexRender::cVertList;

            if(anim)
            {
                spriteFrame_t *frame = p->Weapon().Frame();
                spriteSet_t *spriteSet;
                kexSprite *sprite;
                spriteInfo_t *info;

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
                    byte c = 0xff;

                    float u1, u2, v1, v2;
                    
                    u1 = info->u[0 ^ spriteSet->bFlipped];
                    u2 = info->u[1 ^ spriteSet->bFlipped];
                    v1 = info->v[0];
                    v2 = info->v[1];

                    kexRender::cScreen->SetAspectDimentions(x, y, w, h);

                    sprite->Texture()->Bind();

                    x += p->Weapon().BobX() + weaponInfo->offsetX;
                    y += p->Weapon().BobY() + weaponInfo->offsetY;

                    if(!(frame->flags & SFF_FULLBRIGHT))
                    {
                        c = (byte)(p->Actor()->Sector()->lightLevel << 1);
                    }

                    vl->AddQuad(x, y + 8, 0, w, h, u1, v1, u2, v2, c, c, c, 255);
                    vl->DrawElements();
                }
            }
        }
        
        kexRender::cBackend->SetState(GLSTATE_SCISSOR, false);
        vl->BindDrawPointers();
        
        kexTexture *gfx = kexRender::cTextures->Cache("gfx/hud.png", TC_CLAMP, TF_NEAREST);
        gfx->Bind();

        vl->AddQuad(0, 192, 0, 64, 64, 0, 0, 0.25f, 1, 255, 255, 255, 255);
        vl->AddQuad(64, 216, 0, 96, 24, 0.25f, 0, 0.625f, 0.375f, 255, 255, 255, 255);
        vl->AddQuad(160, 216, 0, 96, 24, 0.25f, 0.375f, 0.625f, 0.75f, 255, 255, 255, 255);
        vl->AddQuad(256, 192, 0, 64, 64, 0.625f, 0, 0.875f, 1, 255, 255, 255, 255);
        vl->DrawElements();
        
        kexRender::cBackend->SetOrtho();
        //kexRender::cUtils->DrawLine(renderView.ProjectPoint(proj1),
        //                            renderView.ProjectPoint(proj2), 255, 0, 255);
        viewBounds.DebugDraw();
        
        for(unsigned int i = 0; i < renderScene.VisiblePortals().CurrentLength(); ++i)
        {
            kexViewBounds viewBounds = renderScene.VisiblePortals()[i];
            viewBounds.DebugDraw();
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
