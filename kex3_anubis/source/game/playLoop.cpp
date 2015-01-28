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
#include "clipper.h"

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
    
    renderScene.SetView(&renderView);
    renderScene.SetWorld(kexGame::cLocal->World());

    kexGame::cLocal->Player()->Ready();
}

//
// kexPlayLoop::Stop
//

void kexPlayLoop::Stop(void)
{
    kexGame::cLocal->World()->UnloadMap();
}

void ProjectFacePoints(kexRenderView &view, mapFace_t *face)
{
    kexVec3 p1 = *face->BottomEdge()->v2;
    kexVec3 p2 = *face->BottomEdge()->v1;
    kexVec3 p3 = *face->BottomEdge()->v2;
    kexVec3 p4 = *face->TopEdge()->v1;

    face->h[0] = face->h[2] = 0;
    face->h[1] = face->h[3] = 320;
    face->v[0] = face->v[2] = 0;
    face->v[1] = face->v[3] = 240;

    p1 = *face->TopEdge()->v1 * view.ModelView();
    p2 = *face->TopEdge()->v2 * view.ModelView();
    p3 = *face->BottomEdge()->v2 * view.ModelView();
    p4 = *face->BottomEdge()->v1 * view.ModelView();

    float d = face->plane.Distance(view.Origin()) - face->plane.d;

    if(d > 96.0f)
    {
        face->h[0] = 320 - (((p1.x / p1.z) * 160) + 160);
        face->h[1] = 320 - (((p2.x / p2.z) * 160) + 160);
        face->h[2] = 320 - (((p3.x / p3.z) * 160) + 160);
        face->h[3] = 320 - (((p4.x / p4.z) * 160) + 160);

        face->v[0] = ((p1.y / p1.z) * 120) + 120;
        face->v[1] = ((p2.y / p2.z) * 120) + 120;
        face->v[2] = ((p3.y / p3.z) * 120) + 120;
        face->v[3] = ((p4.y / p4.z) * 120) + 120;

        for(int i = 0; i < 4; i++)
        {
            kexMath::Clamp(face->h[i], 0, 320);
            kexMath::Clamp(face->v[i], 0, 240);
        }

        if(!view.Frustum().TestSphere(*face->TopEdge()->v1, 0.1f))
        {
            face->h[0] = (p1.x >= 0) ? 320.0f : 0;
        }
        if(!view.Frustum().TestSphere(*face->TopEdge()->v2, 0.1f))
        {
            face->h[1] = (p2.x >= 0) ? 320.0f : 0;
        }
        if(!view.Frustum().TestSphere(*face->BottomEdge()->v2, 0.1f))
        {
            face->h[2] = (p3.x >= 0) ? 320.0f : 0;
        }
        if(!view.Frustum().TestSphere(*face->BottomEdge()->v1, 0.1f))
        {
            face->h[3] = (p4.x >= 0) ? 320.0f : 0;
        }

        if(face->v[0] >= face->v[2]) face->v[0] = 0;
        if(face->v[1] >= face->v[3]) face->v[1] = 0;
        if(face->v[2] <= face->v[0]) face->v[2] = 240;
        if(face->v[3] <= face->v[1]) face->v[3] = 240;
    }
}

bool TestProjectedFace(mapFace_t *src, mapFace_t *dst)
{
    bool bh = (dst->h[0] < src->h[0] && dst->h[2] < src->h[2] && dst->h[1] < src->h[0] && dst->h[3] < src->h[2]) ||
              (dst->h[0] > src->h[1] && dst->h[2] > src->h[3] && dst->h[1] > src->h[1] && dst->h[3] > src->h[3]);

    bool bv = (dst->v[0] < src->v[0] && dst->v[1] < src->v[1] && dst->v[2] < src->v[0] && dst->v[3] < src->v[2]) ||
              (dst->v[0] > src->v[2] && dst->v[1] > src->v[3] && dst->v[2] > src->v[2] && dst->v[3] > src->v[3]);

    if(bh || bv)
    {
        return false;
    }

    return true;
}

//
// kexPlayLoop::Draw
//

kexCvar cvarTest("portaltest", CVF_INT, "0", " ");
kexCvar cvarTest2("testvar", CVF_INT, "0", " ");

void kexPlayLoop::Draw(void)
{
    kexPlayer *p = kexGame::cLocal->Player();
    kexWorld *world = kexGame::cLocal->World();

    static kexClipper clipper;
    static kexClipper clipper2;
    
    renderView.SetupFromPlayer(p);
    world->FindVisibleSectors(renderView, p->Actor()->Sector());
    //renderScene.FindVisibleSectors(p->Actor()->Sector());

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
    kexRender::cBackend->SetScissorRect(0, 0, kex::cSystem->VideoWidth(), kex::cSystem->VideoHeight() - clipY);
    
    if(world->MapLoaded())
    {
        clipper.Clear();
        clipper2.Clear();
        mapFace_t *testFace;
        kexViewBounds viewBound;

        if(0)
        {
            /*mapFace_t *face = &world->Faces()[372];
            kexVec3 p1 = *face->BottomEdge()->v2;
            kexVec3 p2 = *face->BottomEdge()->v1;

            float an1 = (renderView.Origin() - p1).ToYaw();
            float an2 = (renderView.Origin() - p2).ToYaw();
            float an3 = (renderView.Origin() - p1).ToPitch();
            float an4 = (renderView.Origin() - p2).ToPitch();

            kex::cSystem->Printf("%f %f\n", kexMath::Rad2Deg(an3), kexMath::Rad2Deg(an4));*/

            testFace = &world->Faces()[cvarTest2.GetInt()];
            
            if(testFace->InFront(renderView.Origin()))
            {
                //ProjectFacePoints(renderView, testFace);

                kexVec3 org = renderView.Origin();
                kexVec3 p1 = *testFace->BottomEdge()->v1 - org;
                kexVec3 p2 = *testFace->BottomEdge()->v2 - org;

                float s = kexMath::Sin(renderView.Yaw());
                float c = kexMath::Cos(renderView.Yaw());

                float p1x = (p1.y * s) - (p1.x * c);
                float p1y = (p1.x * s) + (p1.y * c);         
                float p2x = (p2.y * s) - (p2.x * c);
                float p2y = (p2.x * s) + (p2.y * c);
                
                kexMatrix mtx = renderView.RotationMatrix();
                mtx.AddTranslation(-renderView.Origin() * mtx);
                
                kexVec3 p3 = *testFace->BottomEdge()->v1 * mtx;
                kexVec3 p4 = *testFace->BottomEdge()->v2 * mtx;
                
                //kex::cSystem->Printf("%f %f\n", p1x*p2y, p2x*p1y);
                //kex::cSystem->Printf("%f %f\n\n", p3.x*p4.y, p4.x*p3.y);

                if(p1x*p2y < p2x*p1y)
                {
                    float dxb1, dxb2;

                    if(p1y >= 0.999f)
                    {
                        dxb1 = 320 - (p1x*160/p1y+160);
                    }
                    else
                    {
                        dxb1 = -kexMath::infinity;
                    }
                    if(p2y >= 0.999f)
                    {
                        dxb2 = 320 - (p2x*160/p2y+160);
                    }
                    else
                    {
                        dxb2 = kexMath::infinity;
                    }

                    kex::cSystem->Printf("%f %f\n", dxb1, dxb2);
                    kex::cSystem->Printf("%f %f\n\n", (p3.x*160/p3.y+160), (p4.x*160/p4.y+160));
                }

                //kex::cSystem->Printf("%f %f %f %f\n", testFace->h[0], testFace->h[2], testFace->h[1], testFace->h[3]);
                //kex::cSystem->Printf("%f %f %f %f\n\n", testFace->v[0], testFace->v[2], testFace->v[1], testFace->v[3]);
                //kex::cSystem->Printf("%f %f %f %f\n\n", cv[0], cv[1], cv[2], cv[3]);
                
                //kexRender::cUtils->DrawLine(p1, p2, 0, 255, 0);
                //kexRender::cUtils->DrawLine(p1+kexVec3(0, 0, 256), p2+kexVec3(0, 0, 256), 0, 255, 0);
                //kexRender::cUtils->DrawLine(p1+kexVec3(0, 0, 256), p1, 0, 255, 0);
                //kexRender::cUtils->DrawLine(p2+kexVec3(0, 0, 256), p2, 0, 255, 0);
            }
        }

        unsigned int max = cvarTest.GetInt();

        if(max == 0 || max > world->VisibleSectors().CurrentLength())
        {
            max = world->VisibleSectors().CurrentLength();
        }

        //for(unsigned int i = 0; i < world->NumSectors(); ++i)
        for(unsigned int i = 0; i < max; ++i)
        {
            mapSector_t *sector = &world->Sectors()[world->VisibleSectors()[i]];
            //mapSector_t *sector = &world->Sectors()[i];
            
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

                if(face->validcount != 1 && !(p->Actor()->PlayerFlags() & PF_NOCLIP))
                {
                    if(j <= end)
                    {
                        continue;
                    }
                }

                face->validcount = 0;
                
                if(0 && /*inSector && */face->sector != -1)
                {
                    kexRender::cUtils->DrawLine(*face->BottomEdge()->v1, *face->BottomEdge()->v2, 255, 0, 255);
                    kexRender::cUtils->DrawLine(*face->TopEdge()->v1, *face->TopEdge()->v2, 255, 0, 255);
                    kexRender::cUtils->DrawLine(*face->LeftEdge()->v1, *face->LeftEdge()->v2, 255, 0, 255);
                    kexRender::cUtils->DrawLine(*face->RightEdge()->v1, *face->RightEdge()->v2, 255, 0, 255);
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
                
                if(j <= end && !face->InFront(renderView.Origin()))
                {
                    continue;
                }

                if(0)
                {
                    if(j <= end && j != 372)
                    {
                        ProjectFacePoints(renderView, face);

#if 1
                        bool bh = (face->h[0] < testFace->h[0] && face->h[2] < testFace->h[2] && face->h[1] < testFace->h[0] && face->h[3] < testFace->h[2]) ||
                                  (face->h[0] > testFace->h[1] && face->h[2] > testFace->h[3] && face->h[1] > testFace->h[1] && face->h[3] > testFace->h[3]);

                        bool bv = (face->v[0] < testFace->v[0] && face->v[1] < testFace->v[1] && face->v[2] < testFace->v[0] && face->v[3] < testFace->v[2]) ||
                                  (face->v[0] > testFace->v[2] && face->v[1] > testFace->v[3] && face->v[2] > testFace->v[2] && face->v[3] > testFace->v[3]);

                        if(bh || bv)
                        {
                            continue;
                        }

#else
                        bool bh = (face->h[0] >= testFace->h[0] && face->h[2] >= testFace->h[2] && face->h[1] <= testFace->h[1] && face->h[3] <= testFace->h[3]);
                        bool bv = (face->v[0] >= testFace->v[0] && face->v[1] >= testFace->v[1] && face->v[2] <= testFace->v[2] && face->v[3] <= testFace->v[3]);

                        if(bh && bv)
                        {
                            continue;
                        }
#endif
                    }
                }

                if(1 && j <= end)
                {
                    kexRender::cUtils->DrawLine(*face->BottomEdge()->v1, *face->BottomEdge()->v2, 0, 255, 0);
                    kexRender::cUtils->DrawLine(*face->TopEdge()->v1, *face->TopEdge()->v2, 0, 255, 0);
                    kexRender::cUtils->DrawLine(*face->LeftEdge()->v1, *face->LeftEdge()->v2, 0, 255, 0);
                    kexRender::cUtils->DrawLine(*face->RightEdge()->v1, *face->RightEdge()->v2, 0, 255, 0);
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
        
        if(0)
        {
            kexRender::cBackend->SetOrtho();
            //kexRender::cUtils->DrawLine(kexVec3(testFace->h[0], testFace->v[0], 0), kexVec3(testFace->h[1], testFace->v[1], 0), 255, 0, 0);
            //kexRender::cUtils->DrawLine(kexVec3(testFace->h[2], testFace->v[2], 0), kexVec3(testFace->h[3], testFace->v[3], 0), 255, 0, 0);
            //kexRender::cUtils->DrawLine(kexVec3(testFace->h[0], testFace->v[0], 0), kexVec3(testFace->h[2], testFace->v[2], 0), 255, 0, 0);
            //kexRender::cUtils->DrawLine(kexVec3(testFace->h[1], testFace->v[1], 0), kexVec3(testFace->h[3], testFace->v[3], 0), 255, 0, 0);

            kexVec4 t1, t2, t3, t4;
            kexVec3 proj1 = renderView.ProjectPoint(*testFace->BottomEdge()->v2, &t1);
            kexVec3 proj2 = renderView.ProjectPoint(*testFace->BottomEdge()->v1, &t2);
            kexVec3 proj3 = renderView.ProjectPoint(*testFace->TopEdge()->v1, &t3);
            kexVec3 proj4 = renderView.ProjectPoint(*testFace->TopEdge()->v2, &t4);

            if(testFace->plane.Distance(renderView.Origin()) - testFace->plane.d <= 128.0f ||
                testFace->plane.IsFacing(renderView.Yaw()))
            {
                proj1.x = proj3.x = (float)kex::cSystem->VideoWidth();
                proj1.y = proj2.y = (float)kex::cSystem->VideoHeight();
                proj2.x = proj4.x = 0;
                proj3.y = proj4.y = 0;
            }
            else
            {
                if(proj2.x <= proj1.x)
                {
                    proj2.x = (float)kex::cSystem->VideoWidth();
                }
                if(proj4.x <= proj3.x)
                {
                    proj4.x = (float)kex::cSystem->VideoWidth();
                }
                if(proj2.y <= proj3.y || proj2.y <= proj4.y)
                {
                    proj2.y = (float)kex::cSystem->VideoHeight();
                }
                if(proj1.y <= proj3.y || proj1.y <= proj4.y)
                {
                    proj1.y = (float)kex::cSystem->VideoHeight();
                }

                if(t1.x <= -1) proj1.x = 0;
                if(t1.x >=  1) proj1.x = (float)kex::cSystem->VideoWidth();
                if(t2.x <= -1) proj2.x = 0;
                if(t2.x >=  1) proj2.x = (float)kex::cSystem->VideoWidth();
                if(t3.x <= -1) proj3.x = 0;
                if(t3.x >=  1) proj3.x = (float)kex::cSystem->VideoWidth();
                if(t4.x <= -1) proj4.x = 0;
                if(t4.x >=  1) proj4.x = (float)kex::cSystem->VideoWidth();
                if(t1.y >=  1) proj1.y = 0;
                if(t1.y <= -1) proj1.y = (float)kex::cSystem->VideoHeight();
                if(t2.y >=  1) proj2.y = 0;
                if(t2.y <= -1) proj2.y = (float)kex::cSystem->VideoHeight();
                if(t3.y >=  1) proj3.y = 0;
                if(t3.y <= -1) proj3.y = (float)kex::cSystem->VideoHeight();
                if(t4.y >=  1) proj4.y = 0;
                if(t4.y <= -1) proj4.y = (float)kex::cSystem->VideoHeight();
            }

            kexRender::cUtils->DrawLine(proj1, proj2, 255, 0, 0);
            kexRender::cUtils->DrawLine(proj2, proj4, 255, 0, 0);
            kexRender::cUtils->DrawLine(proj1, proj3, 255, 0, 0);
            kexRender::cUtils->DrawLine(proj3, proj4, 255, 0, 0);
            
            //kex::cSystem->Printf("%s\n", proj1.ToString().c_str());
            //kex::cSystem->Printf("%s\n", proj2.ToString().c_str());
            //kex::cSystem->Printf("%s\n", proj3.ToString().c_str());
            //kex::cSystem->Printf("%s\n\n", proj4.ToString().c_str());
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
