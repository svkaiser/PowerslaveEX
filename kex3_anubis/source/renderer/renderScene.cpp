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

bool kexRenderScene::bPrintStats = false;

//
// statscene
//

COMMAND(statscene)
{
    if(kex::cCommands->GetArgc() < 1)
    {
        return;
    }
    
    kexRenderScene::bPrintStats ^= 1;
}

//
// kexRenderScene::kexRenderScene
//

kexRenderScene::kexRenderScene(void)
{
    this->world = NULL;
    this->view = NULL;
    this->clipY = 0;
}

//
// kexRenderScene::~kexRenderScene
//

kexRenderScene::~kexRenderScene(void)
{
}

//
// kexRenderScene::DrawSky
//

void kexRenderScene::DrawSky(void)
{
    kexCpuVertList *vl = kexRender::cVertList;
    float s, c;
    float x = view->Origin().x;
    float y = view->Origin().y;
    float z = view->Origin().z + 1280;
    int t = 0;
    int u = 0;
    float radius = 8192;
    float height = 6144;
    float px[10], py[10], pz[10];
    float lx[10], ly[10], lz[10];
    int tris = 0;
    float ang = 0;
    
    kexRender::cTextures->whiteTexture->Bind();
    
    for(unsigned int i = 0; i < world->VisibleSkyFaces().CurrentLength(); ++i)
    {
        mapFace_t *face = &world->Faces()[world->VisibleSkyFaces()[i]];
        mapVertex_t *v = &world->Vertices()[face->vertexStart];

        vl->AddVertex(v[0].origin, 0, 0);
        vl->AddVertex(v[1].origin, 0, 0);
        vl->AddVertex(v[2].origin, 0, 0);
        vl->AddVertex(v[3].origin, 0, 0);
        
        vl->AddTriangle(tris+0, tris+2, tris+1);
        vl->AddTriangle(tris+0, tris+3, tris+2);
        tris += 4;
    }
    
    if(tris == 0)
    {
        return;
    }
    
    kexRender::cBackend->SetColorMask(0);
    vl->DrawElements();
    
    // TEMP
    kexTexture *skyTexture = kexRender::cTextures->Cache("textures/skies/sky_sunny.png",
                                                         TC_REPEAT, TF_NEAREST);
    skyTexture->Bind();
    tris = 0;
    
    kexRender::cBackend->SetColorMask(1);
    kexRender::cBackend->SetDepth(GLFUNC_GEQUAL);
    
    for(int i = 0; i < 17; i++)
    {
        float z1;
        float ang2;
        
        s = kexMath::Sin(kexMath::Deg2Rad(ang));
        c = kexMath::Cos(kexMath::Deg2Rad(ang));
        
        z1 = z + height * c;
        ang += 22.5f;
        
        if(!(i % 8))
        {
            px[0] = x;
            py[0] = y + radius * s;
            pz[0] = z1;
            
            if(i != 0)
            {
                for(int j = 0; j < 8; j++)
                {
                    vl->AddVertex(px[0], py[0], pz[0], 0, 0);
                    vl->AddVertex(lx[j], ly[j], lz[j], 0, 0);
                    vl->AddVertex(lx[1+j], ly[1+j], lz[1+j], 0, 0);
                    if(i == 8)
                    {
                        vl->AddTriangle(tris+2, tris+1, tris+0);
                        tris += 3;
                    }
                    else
                    {
                        vl->AddTriangle(tris+0, tris+1, tris+2);
                        tris += 3;
                    }
                }
            }
            
            continue;
        }
        
        ang2 = 45;
        
        for(int j = 0; j < 9; j++)
        {
            float x2, y2, z2;
            
            x2 = x + kexMath::Sin(kexMath::Deg2Rad(ang2)) * radius * s;
            y2 = y + kexMath::Cos(kexMath::Deg2Rad(ang2)) * radius * s;
            z2 = z1;
            
            ang2 += 22.5f;
            
            px[1+j] = x2;
            py[1+j] = y2;
            pz[1+j] = z2;
        }
        
        if(i == 1 || i == 9)
        {
            for(int j = 0; j < 8; j++)
            {
                vl->AddVertex(px[0], py[0], pz[0], 0, 0);
                vl->AddVertex(px[1+j], py[1+j], pz[1+j], 0, 0);
                vl->AddVertex(px[2+j], py[2+j], pz[2+j], 0, 0);
                
                if(i >= 9)
                {
                    vl->AddTriangle(tris+2, tris+1, tris+0);
                    tris += 3;
                }
                else
                {
                    vl->AddTriangle(tris+0, tris+1, tris+2);
                    tris += 3;
                }
            }
        }
        else
        {
            float tv1 = (float)t / 6;
            float tv2 = ((float)t + 1) / 6;
            
            for(int j = 0; j < 8; j++)
            {
                float tu1 = (float)u / 4;
                float tu2 = ((float)u + 1) / 4;
                
                if(i >= 9)
                {
                    vl->AddVertex(lx[1+j], ly[1+j], lz[1+j], tu2, 1.0f - tv1);
                    vl->AddVertex(lx[j], ly[j], lz[j], tu1, 1.0f - tv1);
                    vl->AddVertex(px[2+j], py[2+j], pz[2+j], tu2, 1.0f - tv2);
                    vl->AddVertex(px[1+j], py[1+j], pz[1+j], tu1, 1.0f - tv2);
                }
                else
                {
                    vl->AddVertex(lx[j], ly[j], lz[j], tu1, tv1);
                    vl->AddVertex(lx[1+j], ly[1+j], lz[1+j], tu2, tv1);
                    vl->AddVertex(px[1+j], py[1+j], pz[1+j], tu1, tv2);
                    vl->AddVertex(px[2+j], py[2+j], pz[2+j], tu2, tv2);
                }
                
                vl->AddTriangle(tris+0, tris+2, tris+1);
                vl->AddTriangle(tris+1, tris+2, tris+3);
                tris += 4;
                
                u++;
            }
            
            t = (t + 1) % 6;
        }
        
        for(int j = 0; j < 9; j++)
        {
            lx[j] = px[1+j];
            ly[j] = py[1+j];
            lz[j] = pz[1+j];
        }
    }
    
    vertCount += vl->VertexCount();
    triCount += vl->IndiceCount();
    
    vl->DrawElements();
    kexRender::cBackend->ClearBuffer(GLCB_DEPTH);
    kexRender::cBackend->SetDepth(GLFUNC_LEQUAL);
}

//
// kexRenderScene::DrawSector
//

void kexRenderScene::DrawSector(mapSector_t *sector)
{
    int start = sector->faceStart;
    int end = sector->faceEnd;
    int rectY;
    
    if(!view->TestBoundingBox(sector->bounds))
    {
        return;
    }
    
    rectY = (int)sector->y2;
    
    if(rectY > clipY)
    {
        rectY = clipY;
    }
    
    kexRender::cBackend->SetScissorRect((int)sector->x1, (int)sector->y1,
                                        (int)sector->x2, rectY);
    
    if(sector->flags & SF_DEBUG)
    {
        sector->flags &= ~SF_DEBUG;
    }
    
    for(int j = start; j < end+3; ++j)
    {
        DrawFace(sector, j);
    }
}

//
// kexRenderScene::DrawFace
//

void kexRenderScene::DrawFace(mapSector_t *sector, int faceID)
{
    mapFace_t *face = &world->Faces()[faceID];
    
    if(face->flags & FF_OCCLUDED && face->sector <= -1)
    {
        if(faceID <= sector->faceEnd)
        {
            face->flags &= ~FF_OCCLUDED;
            return;
        }
    }
    
    face->validcount = 0;
    
    if(face->polyStart == -1 || face->polyEnd == -1)
    {
        return;
    }
    
    if(!view->TestBoundingBox(face->bounds))
    {
        return;
    }
    
    if(faceID <= sector->faceEnd && !face->InFront(view->Origin()))
    {
        return;
    }
    
    for(int k = face->polyStart; k <= face->polyEnd; ++k)
    {
        DrawPolygon(face, &world->Polys()[k]);
    }
}

//
// kexRenderScene::DrawPolygon
//

void kexRenderScene::DrawPolygon(mapFace_t *face, mapPoly_t *poly)
{
    int tris = 0;
    kexCpuVertList *vl = kexRender::cVertList;
    mapTexCoords_t *tcoord = world->TexCoords();
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
        if(poly->indices[idx] == 0xff || poly->tcoords[idx] == -1)
        {
            continue;
        }
        
        indices[curIdx] = poly->indices[idx];
        tcoords[curIdx] = poly->tcoords[idx];
        curIdx++;
    }

    if(curIdx <= 2)
    {
        return;
    }
    
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
    
    vl->AddTriangle(tris+0, tris+2, tris+1);
    if(curIdx == 4)
    {
        vl->AddTriangle(tris+0, tris+3, tris+2);
    }
    
    tris += curIdx;
    
    vertCount += vl->VertexCount();
    triCount += vl->IndiceCount();
    
    vl->DrawElements();
}

//
// kexRenderScene::DrawActors
//

void kexRenderScene::DrawActors(mapSector_t *sector)
{
    kexCpuVertList *vl = kexRender::cVertList;

    for(kexActor *actor = sector->actorList.Next();
        actor != NULL;
        actor = actor->SectorLink().Next())
    {
        if(actor->Anim() == NULL || actor == kexGame::cLocal->Player()->Actor())
        {
            continue;
        }
        
        if(!view->TestBoundingBox(actor->Bounds() + actor->Origin()))
        {
            continue;
        }
        
        kexVec3 org = actor->Origin();
        kexMatrix scale(actor->Scale(), actor->Scale(), actor->Scale());
        spriteFrame_t *frame;
        spriteSet_t *spriteSet;
        kexSprite *sprite;
        spriteInfo_t *info;

        frame = actor->Frame();

        for(unsigned int i = 0; i < frame->spriteSet.Length(); ++i)
        {
            spriteSet = &frame->spriteSet[i];
            sprite = spriteSet->sprite;
            info = &sprite->InfoList()[spriteSet->index];

            float x = (float)spriteSet->x;
            float y = (float)spriteSet->y;
            float w = (float)info->atlas.w;
            float h = (float)info->atlas.h;

            float u1, u2, v1, v2;
            
            u1 = info->u[0 ^ spriteSet->bFlipped];
            u2 = info->u[1 ^ spriteSet->bFlipped];
            v1 = info->v[0];
            v2 = info->v[1];

            sprite->Texture()->Bind();

            kexVec3 p1 = kexVec3(x, 0, y);
            kexVec3 p2 = kexVec3(x+w, 0, y);
            kexVec3 p3 = kexVec3(x, 0, y+h);
            kexVec3 p4 = kexVec3(x+w, 0, y+h);

            p1 *= spriteMatrix;
            p2 *= spriteMatrix;
            p3 *= spriteMatrix;
            p4 *= spriteMatrix;

            p1 *= scale;
            p2 *= scale;
            p3 *= scale;
            p4 *= scale;

            p1 += org;
            p2 += org;
            p3 += org;
            p4 += org;

            vl->AddVertex(p1, u1, v1, 255, 255, 255, 255);
            vl->AddVertex(p2, u2, v1, 255, 255, 255, 255);
            vl->AddVertex(p3, u1, v2, 255, 255, 255, 255);
            vl->AddVertex(p4, u2, v2, 255, 255, 255, 255);

            vl->AddTriangle(0, 2, 1);
            vl->AddTriangle(1, 2, 3);

            if(actor->Flags() & AF_FLASH)
            {
                vl->DrawElements(false);
                
                kexRender::cTextures->whiteTexture->Bind();
                kexRender::cBackend->SetDepth(GLFUNC_EQUAL);
                kexRender::cBackend->SetBlend(GLSRC_ONE, GLDST_ONE);
                
                vl->DrawElements();
                
                kexRender::cBackend->SetBlend(GLSRC_SRC_ALPHA, GLDST_ONE_MINUS_SRC_ALPHA);
                kexRender::cBackend->SetDepth(GLFUNC_LEQUAL);
            }
            else
            {
                vl->DrawElements();
            }
        }
    }
}

//
// kexRenderScene::PrintStats
//

void kexRenderScene::PrintStats(void)
{
    if(!bPrintStats)
    {
        return;
    }
    
    kexRender::cUtils->PrintStatsText("Vertices Drawn", "%i", vertCount);
    kexRender::cUtils->PrintStatsText("Triangles Drawn", "%i", triCount/3);
}

//
// kexRenderScene::Draw
//

void kexRenderScene::Draw(void)
{
    int w, h;
    
    vertCount = 0;
    triCount = 0;
    
    kexRender::cBackend->LoadProjectionMatrix(view->ProjectionView());
    kexRender::cBackend->LoadModelViewMatrix(view->ModelView());
    
    kexRender::cBackend->SetState(GLSTATE_DEPTHTEST, true);
    kexRender::cBackend->SetState(GLSTATE_ALPHATEST, true);
    kexRender::cBackend->SetState(GLSTATE_BLEND, true);
    kexRender::cBackend->SetState(GLSTATE_SCISSOR, true);
    
    w = kex::cSystem->VideoWidth();
    h = kex::cSystem->VideoHeight();
    
    clipY = h - (int)((float)h / (240.0f / 24.0f));
    
    if(!world->MapLoaded())
    {
        return;
    }
    
    kexRender::cVertList->BindDrawPointers();
    
    DrawSky();

    spriteMatrix = kexMatrix(-view->Pitch(), 1) * kexMatrix(view->Yaw(), 2);
    spriteMatrix.RotateX(kexMath::pi);
    
    for(unsigned int i = 0; i < world->VisibleSectors().CurrentLength(); ++i)
    {
        DrawSector(&world->Sectors()[world->VisibleSectors()[i]]);
    }

    kexRender::cBackend->SetScissorRect(0, 0, w, clipY);

    for(unsigned int i = 0; i < world->VisibleSectors().CurrentLength(); ++i)
    {
        DrawActors(&world->Sectors()[world->VisibleSectors()[i]]);
    }
    
    PrintStats();
}
