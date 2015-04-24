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
bool kexRenderScene::bShowPortals = false;
bool kexRenderScene::bShowWaterPortals = false;
bool kexRenderScene::bShowCollision = false;

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
// showportals
//

COMMAND(showportals)
{
    if(kex::cCommands->GetArgc() < 1)
    {
        return;
    }
    
    kexRenderScene::bShowPortals ^= 1;
}

//
// showwaterportals
//

COMMAND(showwaterportals)
{
    if(kex::cCommands->GetArgc() < 1)
    {
        return;
    }
    
    kexRenderScene::bShowWaterPortals ^= 1;
}

//
// showcollision
//

COMMAND(showcollision)
{
    if(kex::cCommands->GetArgc() < 1)
    {
        return;
    }
    
    kexRenderScene::bShowCollision ^= 1;
}

//
// kexRenderScene::kexRenderScene
//

kexRenderScene::kexRenderScene(void)
{
    this->world = NULL;
    this->clipY = 0;
    this->polyList.Init(512);
}

//
// kexRenderScene::~kexRenderScene
//

kexRenderScene::~kexRenderScene(void)
{
}

//
// kexRenderScene::ClipFaceToPlane
//

bool kexRenderScene::ClipFaceToPlane(kexRenderView &view, kexPlane &plane, mapFace_t *face,
                                     float &bx1, float &bx2, float &by1, float &by2)
{
    kexVec3 points[6];
    int numPoints = 0;
    float f, w, h;
    kexVec3 pt;
    kexVec3 v[4];
    float dist[4];
    int sign[4];
    float x1, x2, y1, y2;
    
    w = (float)kex::cSystem->VideoWidth();
    h = (float)kex::cSystem->VideoHeight();
    
    x1 = x2 = y1 = y2 = 0;
    
    for(int i = 0; i < 4; ++i)
    {
        v[i] = world->Vertices()[face->vertexStart+i].origin;
        dist[i] = plane.Distance(v[i]) + plane.d;
        sign[i] = dist[i] > 0;
        
        if(sign[i])
        {
            points[numPoints++] = v[i];
        }
    }
    
    for(int i = 0; i < 4; ++i)
    {
        int j = (i + 1) & 3;

        if(sign[i] != sign[j])
        {
            f = dist[i] / (dist[i] - dist[j]);
            points[numPoints++].Lerp(v[i], v[j], f);
        }
    }
    
    if(numPoints < 3)
    {
        return false;
    }
    
    for(int i = 0; i < numPoints; ++i)
    {
        pt = view.ProjectPoint(points[i]);
        
        if(pt.z <= 0)
        {
            x1 = 0;
            x2 = w;
            y1 = 0;
            y2 = h;
            break;
        }
        
        if(i == 0)
        {
            x1 = x2 = pt.x;
            y1 = y2 = pt.y;
        }
        else
        {
            if(x1 > pt.x) x1 = pt.x;
            if(x2 < pt.x) x2 = pt.x;
            if(y1 > pt.y) y1 = pt.y;
            if(y2 < pt.y) y2 = pt.y;
        }
    }
    
    if(x1 < 0) x1 = 0;
    if(x2 > w) x2 = w;
    if(y1 < 0) y1 = 0;
    if(y2 > h) y2 = h;
    
    if(x1 > bx1) bx1 = x1;
    if(x2 < bx2) bx2 = x2;
    if(y1 > by1) by1 = y1;
    if(y2 < by2) by2 = y2;
    
    return true;
}

//
// kexRenderScene::SetScissorRect
//

bool kexRenderScene::SetScissorRect(kexRenderView &view, mapFace_t *face)
{
#if 1
    float bx1, bx2, by1, by2;
    float w, h;
    
    w = (float)kex::cSystem->VideoWidth();
    h = (float)kex::cSystem->VideoHeight();
    
    bx1 = 0;
    bx2 = w;
    by1 = 0;
    by2 = h;

    if( view.TestPointNearPlane(world->Vertices()[face->vertexStart+0].origin) &&
        view.TestPointNearPlane(world->Vertices()[face->vertexStart+1].origin) &&
        view.TestPointNearPlane(world->Vertices()[face->vertexStart+2].origin) &&
        view.TestPointNearPlane(world->Vertices()[face->vertexStart+3].origin))
    {
        kexVec3 pt;
        float x1, x2, y1, y2;

        x1 = x2 = y1 = y2 = 0;

        for(int i = 0; i < 4; ++i)
        {
            pt = view.ProjectPoint(world->Vertices()[face->vertexStart+i].origin);

            if(pt.z <= 0)
            {
                x1 = 0;
                x2 = w;
                y1 = 0;
                y2 = h;
                break;
            }
            
            if(i == 0)
            {
                x1 = x2 = pt.x;
                y1 = y2 = pt.y;
            }
            else
            {
                if(x1 > pt.x) x1 = pt.x;
                if(x2 < pt.x) x2 = pt.x;
                if(y1 > pt.y) y1 = pt.y;
                if(y2 < pt.y) y2 = pt.y;
            }
        }
    
        if(x1 > bx1) bx1 = x1;
        if(x2 < bx2) bx2 = x2;
        if(y1 > by1) by1 = y1;
        if(y2 < by2) by2 = y2;
    }
    else
    {
        ClipFaceToPlane(view, view.NearPlane(), face, bx1, bx2, by1, by2);
        ClipFaceToPlane(view, view.TopPlane(), face, bx1, bx2, by1, by2);
        ClipFaceToPlane(view, view.RightPlane(), face, bx1, bx2, by1, by2);
        ClipFaceToPlane(view, view.LeftPlane(), face, bx1, bx2, by1, by2);
        ClipFaceToPlane(view, view.BottomPlane(), face, bx1, bx2, by1, by2);
    }
    
    if(bx1 > bx2) bx1 = 0;
    if(bx2 < bx1) bx2 = w;
    if(by1 > by2) by1 = 0;
    if(by2 < by1) by2 = h;
    
    face->x1 = bx1;
    face->x2 = bx2;
    face->y1 = by1;
    face->y2 = by2;
#else
    float x1, x2, x3, x4, y1, y2, y3, y4;
    float fx1, fx2, fy1, fy2;
    float w, h;
    
    kexVec3 p1 = view.ProjectPoint(world->Vertices()[face->vertexStart+2].origin);
    kexVec3 p2 = view.ProjectPoint(world->Vertices()[face->vertexStart+3].origin);
    kexVec3 p3 = view.ProjectPoint(world->Vertices()[face->vertexStart+1].origin);
    kexVec3 p4 = view.ProjectPoint(world->Vertices()[face->vertexStart+0].origin);

    w = (float)kex::cSystem->VideoWidth();
    h = (float)kex::cSystem->VideoHeight();

    x1 = p4.x;
    x2 = p2.x;
    x3 = p1.x;
    x4 = p3.x;
    y1 = p2.y;
    y2 = p4.y;
    y3 = p3.y;
    y4 = p1.y;

    fx1 = (x2 < x1) ? x2 : x1;
    if(x3 < fx1) fx1 = x3;
    if(x4 < fx1) fx1 = x4;

    fx2 = (x1 < x2) ? x2 : x1;
    if(fx2 < x3) fx2 = x3;
    if(fx2 < x4) fx2 = x4;
    
    fy1 = (y2 < y1) ? y2 : y1;
    if(y3 < fy1) fy1 = y3;
    if(y4 < fy1) fy1 = y4;

    fy2 = (y1 < y2) ? y2 : y1;
    if(fy2 < y3) fy2 = y3;
    if(fy2 < y4) fy2 = y4;

    if(fx1 < 0) fx1 = 0;
    if(fx2 > w) fx2 = w;
    if(fy1 < 0) fy1 = 0;
    if(fy2 > h) fy2 = h;

    face->x1 = fx1;
    face->x2 = fx2;
    face->y1 = fy1;
    face->y2 = fy2;

    if(p2.z <= 0 || p4.z <= 0)
    {
        face->x1 = 0;
        face->y1 = 0;
        face->y2 = h;
    }

    if(p1.z <= 0 || p3.z <= 0)
    {
        face->x2 = w;
        face->y1 = 0;
        face->y2 = h;
    }
#endif
    return true;
}

//
// kexRenderScene::SetFaceDistance
//

void kexRenderScene::SetFaceDistance(kexRenderView &view, mapFace_t *face)
{
    kexVec3 p[4];
    float max_x = kexMath::infinity;
    int inc_x = 0;
    float max_y = kexMath::infinity;
    int inc_y = 0;
    float max_z = kexMath::infinity;
    int inc_z = 0;
    float d;
    float best;
    kexMatrix mtx(view.Pitch(), 1);

    mtx = mtx * kexMatrix(-view.Yaw()-kexMath::pi, 2);

    p[0] = world->Vertices()[face->vertexStart+0].origin-view.Origin();
    p[1] = world->Vertices()[face->vertexStart+1].origin-view.Origin();
    p[2] = world->Vertices()[face->vertexStart+2].origin-view.Origin();
    p[3] = world->Vertices()[face->vertexStart+3].origin-view.Origin();

    p[0] *= mtx;
    p[1] *= mtx;
    p[2] *= mtx;
    p[3] *= mtx;

    for(int i = 0; i < 4; ++i)
    {
        d = p[i].x;
        if(d < 0)
        {
            d = -d;
        }
        else
        {
            inc_x++;
        }

        if(d < max_x)
        {
            max_x = d;
        }

        d = p[i].y;
        if(d < 0)
        {
            d = -d;
        }
        else
        {
            inc_y++;
        }

        if(d < max_y)
        {
            max_y = d;
        }

        d = p[i].z;
        if(d < 0)
        {
            d = -d;
        }
        else
        {
            inc_z++;
        }

        if(d < max_z)
        {
            max_z = d;
        }
    }

    if(!(inc_x == 0 || inc_x == 4)) max_x = 0;
    if(!(inc_y == 0 || inc_y == 4)) max_y = 0;
    if(!(inc_z == 0 || inc_z == 4)) max_z = 0;

    if(max_x < 0) max_x = -max_x;
    if(max_y < 0) max_y = -max_y;
    if(max_z < 0) max_z = -max_z;

    if(max_x < max_y)
    {
        best = max_x;
    }
    else
    {
        best = max_y;
    }

    if(max_z < best)
    {
        best = max_z;
    }

    face->dist = (max_x + max_y + max_z) - (best * 0.5f);
}

//
// kexRenderScene::FindVisibleSectors
//

void kexRenderScene::FindVisibleSectors(kexRenderView &view, mapSector_t *sector)
{
    static int clipCount = 0;
    sectorList_t *scanSectors;

    int secnum;
    int start, end;
    kexVec3 origin;
    unsigned int scanCount;
    float w, h;
    
    if(bPrintStats)
    {
        floodFillTime = kex::cTimer->GetPerformanceCounter();
    }

    secnum = sector - world->Sectors();

    clipCount++;

    w = (float)kex::cSystem->VideoWidth();
    h = (float)kex::cSystem->VideoHeight();

    for(unsigned int i = 0; i < world->NumSectors(); ++i)
    {
        world->Sectors()[i].x1 = w;
        world->Sectors()[i].x2 = 0;
        world->Sectors()[i].y1 = h;
        world->Sectors()[i].y2 = 0;

        world->Sectors()[i].floodCount = 0;
        world->Sectors()[i].flags &= ~SF_CLIPPED;
    }

    origin = view.Origin();

    scanSectors = &world->ScanSectors();

    scanSectors->Reset();
    scanSectors->Set(sector);

    visibleSkyFaces.Reset();
    visibleSectors.Reset();
    visibleSectors.Set(secnum);

    world->ClearSectorPVS();
    world->MarkSectorInPVS(secnum);

    sector->floodCount = 1;
    sector->x1 = 0;
    sector->x2 = w;
    sector->y1 = 0;
    sector->y2 = h;

    scanCount = 0;
    
    do
    {
        mapSector_t *s = (*scanSectors)[scanCount++];
        
        if(s->floodCount == 0)
        {
            continue;
        }
        
        s->floodCount = 0;

        start = s->faceStart;
        end = s->faceEnd;
        
        if(s->clipCount != clipCount)
        {
            float dist = 0;

            s->clipCount = clipCount;

            for(int i = start; i < end+3; ++i)
            {
                mapFace_t *face = &world->Faces()[i];

                dist = face->plane.Distance(origin) - face->plane.d;

                if(face->flags & FF_PORTAL)
                {
                    if(i < end+1 && dist <= 0)
                    {
                        continue;
                    }
                }

                // compute the scissor rect if the portal is either:
                // * not a water surface
                // * render view is not 'on' the plane
                // * render view is 64 units away from a ceiling/floor portal
                
                if(face->flags & FF_WATER || dist <= 0.5f || (i >= end+1 && dist < 64))
                {
                    face->x1 = 0;
                    face->x2 = w;
                    face->y1 = 0;
                    face->y2 = h;
                }
                else
                {
                    SetScissorRect(view, &world->Faces()[i]);
                }
            }
        }
        
        for(int i = start; i < end+3; ++i)
        {
            mapFace_t *face = &world->Faces()[i];

            if(i < end+1 && !view.TestBoundingBox(face->bounds))
            {
                continue;
            }
            
            if(face->sector >= 0)
            {
                mapSector_t *next = &world->Sectors()[face->sector];
                bool bInside = false;

                if(next->bounds.max.z <= next->bounds.min.z)
                {
                    continue;
                }

                if(face->x2 < s->x1) continue;
                if(face->x1 > s->x2) continue;
                if(face->y2 < s->y1) continue;
                if(face->y1 > s->y2) continue;

                float tx1 = face->x1;
                float tx2 = face->x2;
                float ty1 = face->y1;
                float ty2 = face->y2;
                
                if(tx1 < s->x1) tx1 = s->x1;
                if(tx2 > s->x2) tx2 = s->x2;
                if(ty1 < s->y1) ty1 = s->y1;
                if(ty2 > s->y2) ty2 = s->y2;

                if(tx1 < next->x1) { next->x1 = tx1; bInside = true; }
                if(tx2 > next->x2) { next->x2 = tx2; bInside = true; }
                if(ty1 < next->y1) { next->y1 = ty1; bInside = true; }
                if(ty2 > next->y2) { next->y2 = ty2; bInside = true; }

                if(!bInside)
                {
                    next->flags |= SF_CLIPPED;
                    continue;
                }
                
                if(!world->SectorInPVS(face->sector))
                {
                    world->MarkSectorInPVS(face->sector);
                    visibleSectors.Set(face->sector);
                }

                scanSectors->Set(next);
                next->floodCount = 1;
            }
            else
            {
                face->flags |= FF_OCCLUDED;

                if(face->x2 < s->x1) continue;
                if(face->x1 > s->x2) continue;
                if(face->y2 < s->y1) continue;
                if(face->y1 > s->y2) continue;

                face->flags &= ~FF_OCCLUDED;

                if((face->polyStart == -1 || face->polyEnd == -1))
                {
                    visibleSkyFaces.Set(i);
                }
            }
        }
        
    } while(scanCount < scanSectors->CurrentLength());
    
    if(bPrintStats)
    {
        floodFillTime = kex::cTimer->GetPerformanceCounter() - floodFillTime;
    }
}

//
// kexRenderScene::DrawSky
//

void kexRenderScene::DrawSky(kexRenderView &view)
{
    kexCpuVertList *vl = kexRender::cVertList;
    float s, c;
    float x = view.Origin().x;
    float y = view.Origin().y;
    float z = view.Origin().z + 1280;
    int t = 0;
    int u = 0;
    float radius = 8192;
    float height = 6144;
    float px[10], py[10], pz[10];
    float lx[10], ly[10], lz[10];
    int tris = 0;
    float ang = 0;
    
    kexRender::cTextures->whiteTexture->Bind();
    
    for(unsigned int i = 0; i < visibleSkyFaces.CurrentLength(); ++i)
    {
        mapFace_t *face = &world->Faces()[visibleSkyFaces[i]];
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
    
    kexTexture *skyTexture;

    if(world->SkyTexture() == NULL)
    {
        skyTexture = kexRender::cTextures->defaultTexture;
    }
    else
    {
        skyTexture = world->SkyTexture();
    }

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

void kexRenderScene::DrawSector(kexRenderView &view, mapSector_t *sector)
{
    int start = sector->faceStart;
    int end = sector->faceEnd;
    
    if(!view.TestBoundingBox(sector->bounds))
    {
        return;
    }
    
    if(sector->flags & SF_DEBUG)
    {
        sector->flags &= ~SF_DEBUG;
    }
    
    for(int j = start; j < end+3; ++j)
    {
        DrawFace(view, sector, j);
    }
}

//
// kexRenderScene::DrawPortal
//

void kexRenderScene::DrawPortal(kexRenderView &view, mapFace_t *face, byte r, byte g, byte b)
{
    mapVertex_t *v = &world->Vertices()[face->vertexStart];
    mapSector_t *sector = &world->Sectors()[face->sectorOwner];

    kexRender::cBackend->SetScissorRect((int)sector->x1, (int)sector->y1,
                                        (int)sector->x2, (int)sector->y2);
    
    kexRender::cUtils->DrawLine(v[0].origin, v[1].origin, r, g, b);
    kexRender::cUtils->DrawLine(v[1].origin, v[2].origin, r, g, b);
    kexRender::cUtils->DrawLine(v[2].origin, v[3].origin, r, g, b);
    kexRender::cUtils->DrawLine(v[3].origin, v[0].origin, r, g, b);
}

//
// kexRenderScene::DrawFace
//

void kexRenderScene::DrawFace(kexRenderView &view, mapSector_t *sector, int faceID)
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

    if(face->flags & FF_WATER && bShowWaterPortals)
    {
        DrawPortal(view, face, 0, 0, 255);
    }
    
    if(face->flags & FF_PORTAL && bShowPortals)
    {
        DrawPortal(view, face, 255, 0, 255);
    }
    
    if(face->polyStart == -1 || face->polyEnd == -1)
    {
        return;
    }
    
    if(!view.TestBoundingBox(face->bounds))
    {
        return;
    }
    
    if(faceID <= sector->faceEnd && face->plane.PointOnSide(view.Origin()) == kexPlane::PSIDE_BACK)
    {
        return;
    }
    
    if(face->flags & FF_WATER)
    {
        waterFaces.Set(faceID);
        return;
    }
    
    face->flags |= FF_MAPPED;
    face->flags &= ~FF_HIDDEN;
    
    for(int k = face->polyStart; k <= face->polyEnd; ++k)
    {
        polyList.Set(k);
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
        kexVec3 vPoint;
        int r, g, b;
        vertex = &world->Vertices()[face->vertStart + indices[idx]];

        vPoint = vertex->origin;
        r = vertex->rgba[0];
        g = vertex->rgba[1];
        b = vertex->rgba[2];

        if(world->Sectors()[face->sectorOwner].flags & SF_WATER || face->flags & FF_WATER)
        {
            int v = kexGame::cLocal->PlayLoop()->GetWaterVelocityPoint(vPoint.x + vPoint.z, vPoint.y + vPoint.z);
            float max = (((float)r + (float)g + (float)b) / 3) / 3;
            float c = ((float)v / (float)kexGame::cLocal->PlayLoop()->MaxWaterMagnitude()) * max;

            kexMath::Clamp(c, -max, max);

            r += (int)c;
            g += (int)c;
            b += (int)c;

            kexMath::Clamp(r, 0, 255);
            kexMath::Clamp(g, 0, 255);
            kexMath::Clamp(b, 0, 255);
        }

        if(face->flags & FF_WATER && face->sector >= 0)
        {
            int v = kexGame::cLocal->PlayLoop()->GetWaterVelocityPoint(vPoint.x, vPoint.y);
            vPoint.z += (float)v / 32768.0f;
            
        }
        
        vl->AddVertex(vPoint,
                      tcoord->uv[tcoords[idx]].s, 1.0f - tcoord->uv[tcoords[idx]].t,
                      r, g, b,
                      (face->flags & FF_WATER) ? 128 : 255);
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
// kexRenderScene::DrawWater
//

void kexRenderScene::DrawWater(kexRenderView &view)
{
    kexRender::cBackend->SetBlend(GLSRC_SRC_ALPHA, GLDST_ONE_MINUS_SRC_ALPHA);
    kexRender::cBackend->SetState(GLSTATE_CULL, true);
    kexRender::cBackend->SetDepthMask(0);
    
    for(unsigned int i = 0; i < waterFaces.CurrentLength(); ++i)
    {
        mapFace_t *face = &world->Faces()[waterFaces[i]];
        
        for(int k = face->polyStart; k <= face->polyEnd; ++k)
        {
            DrawPolygon(face, &world->Polys()[k]);
        }
    }
    
    kexRender::cBackend->SetDepthMask(1);
}

//
// kexRenderScene::DrawSprite
//

void kexRenderScene::DrawSprite(kexRenderView &view, mapSector_t *sector, kexActor *actor)
{
    kexCpuVertList  *vl = kexRender::cVertList;
    spriteFrame_t   *frame;
    spriteSet_t     *spriteSet;
    kexSprite       *sprite;
    spriteInfo_t    *info;
    int             rotation;
    kexMatrix       scale;
    kexVec3         org;

    org = actor->Origin();
    scale.Identity(actor->Scale(), actor->Scale(), actor->Scale());

    frame = actor->Frame();
    rotation = 0;

    if(frame->flags & SFF_HASROTATIONS)
    {
        float an = actor->Yaw() - kexMath::ATan2(actor->Origin().x - view.Origin().x,
                                                 actor->Origin().y - view.Origin().y);
        
        kexAngle::Clamp360(an);
        rotation = (int)((an + ((45 / 2) * 9)) / 45);

        if(rotation >= 8) rotation -= 8;
        if(rotation <  0) rotation += 8;
    }

    for(unsigned int i = 0; i < frame->spriteSet[rotation].Length(); ++i)
    {
        int c = 0xff;
        int r, g, b;

        spriteSet = &frame->spriteSet[rotation][i];
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

        if(!(actor->Flags() & AF_FULLBRIGHT))
        {
            c = (sector->lightLevel << 1) + 32;

            if(c > 255)
            {
                c = 255;
            }
        }

        r = (int)((float)c * actor->Color().x * 2);
        g = (int)((float)c * actor->Color().y * 2);
        b = (int)((float)c * actor->Color().z * 2);

        kexMath::Clamp(r, 0, 255);
        kexMath::Clamp(g, 0, 255);
        kexMath::Clamp(b, 0, 255);

        vl->AddVertex(p1, u1, v1, r, g, b, 255);
        vl->AddVertex(p2, u2, v1, r, g, b, 255);
        vl->AddVertex(p3, u1, v2, r, g, b, 255);
        vl->AddVertex(p4, u2, v2, r, g, b, 255);

        vl->AddTriangle(0, 2, 1);
        vl->AddTriangle(1, 2, 3);

        if(actor->Flags() & AF_FLASH)
        {
            vl->DrawElements(false);
            
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

    if(bShowCollision && actor->Flags() & AF_SOLID)
    {
        kexRender::cUtils->DrawRadius(org.x, org.y, org.z - (actor->Height()*0.5f),
                                      actor->Radius(), actor->Height() + actor->StepHeight(),
                                      255, 128, 64);
        kexRender::cUtils->DrawSphere(org.x, org.y, org.z + actor->StepHeight(),
                                      actor->Radius(), 255, 32, 32);
    }
}

//
// kexRenderScene::DrawStretchSprite
//

void kexRenderScene::DrawStretchSprite(kexRenderView &view, mapSector_t *sector, kexActor *actor)
{
    kexMatrix       mtx, scale;
    kexVec3         org;
    int             count, tris;
    spriteFrame_t   *frame;
    spriteSet_t     *spriteSet;
    kexSprite       *sprite;
    spriteInfo_t    *info;
    kexCpuVertList  *vl = kexRender::cVertList;

    if(!actor->GetTaggedActor())
    {
        return;
    }

    frame = actor->Frame();

    if(frame->spriteSet[0].Length() == 0)
    {
        return;
    }

    spriteSet = &frame->spriteSet[0][0];
    sprite = spriteSet->sprite;
    info = &sprite->InfoList()[spriteSet->index];

    scale.Identity(actor->Scale(), actor->Scale(), actor->Scale());
    count = -1;

    for(kexActor *child = actor->GetTaggedActor(); ; child = static_cast<kexActor*>(child->Target()))
    {
        int c = 0xff;
        int r, g, b;

        org = child->Origin();

        mtx = kexMatrix(-child->Pitch(), 1) * kexMatrix(child->Yaw() + 1.57f, 2);
        mtx.RotateX(kexMath::pi);

        float x = (float)spriteSet->x;
        float y = (float)spriteSet->y;
        float h = (float)info->atlas.h;

        float u1, u2, v1, v2;
        
        u1 = info->u[0];
        u2 = info->u[1];
        v1 = info->v[0];
        v2 = info->v[1];

        kexVec3 p1 = kexVec3(x, 0, y);
        kexVec3 p2 = kexVec3(x, 0, y+h);

        p1 *= (mtx * scale);
        p2 *= (mtx * scale);

        p1 += org;
        p2 += org;

        if(!(child->Flags() & AF_FULLBRIGHT))
        {
            c = (sector->lightLevel << 1) + 32;

            if(c > 255)
            {
                c = 255;
            }
        }

        r = (int)((float)c * child->Color().x * 2);
        g = (int)((float)c * child->Color().y * 2);
        b = (int)((float)c * child->Color().z * 2);

        kexMath::Clamp(r, 0, 255);
        kexMath::Clamp(g, 0, 255);
        kexMath::Clamp(b, 0, 255);

        vl->AddVertex(p1, u1, v1, r, g, b, 255);
        vl->AddVertex(p2, u1, v2, r, g, b, 255);

        count++;

        if(child == actor)
        {
            break;
        }
    }

    tris = 0;

    for(int i = 0; i < count; ++i)
    {
        vl->AddTriangle(tris+0, tris+2, tris+1);
        vl->AddTriangle(tris+1, tris+2, tris+3);
        tris += 2;
    }

    sprite->Texture()->Bind();
    vl->DrawElements();
}

//
// kexRenderScene::DrawActorList
//

void kexRenderScene::DrawActorList(kexRenderView &view, mapSector_t *sector)
{
    for(kexActor *actor = sector->actorList.Next();
        actor != NULL;
        actor = actor->SectorLink().Next())
    {
        if(actor->Anim() == NULL || actor == kexGame::cLocal->Player()->Actor())
        {
            continue;
        }

        if(actor->Flags() & AF_HIDDEN)
        {
            continue;
        }
        
        if(!view.TestBoundingBox(actor->Bounds() + actor->Origin()))
        {
            continue;
        }
        
        kexRender::cBackend->SetState(GLSTATE_CULL, !(actor->Flags() & AF_STRETCHY));

        if(actor->Flags() & AF_STRETCHY)
        {
            DrawStretchSprite(view, sector, actor);
        }
        else
        {
            DrawSprite(view, sector, actor);
        }
    }
}

//
// kexRenderScene::SortPolys
//

int kexRenderScene::SortPolys(const int *p1, const int *p2)
{
    mapPoly_t *poly1 = &kexGame::cLocal->World()->Polys()[*p1];
    mapPoly_t *poly2 = &kexGame::cLocal->World()->Polys()[*p2];

    if(poly1->texture < poly2->texture) return  1;
    if(poly1->texture > poly2->texture) return -1;

    return 0;
}

//
// kexRenderScene::DrawSectors
//

void kexRenderScene::DrawSectors(kexRenderView &view)
{
    mapSector_t *prevSector = NULL;
    
    if(bPrintStats)
    {
        drawSectorTime = kex::cTimer->GetPerformanceCounter();
    }

    polyList.Reset();

    for(unsigned int i = 0; i < visibleSectors.CurrentLength(); ++i)
    {
        DrawSector(view, &world->Sectors()[visibleSectors[i]]);
    }

    if(bPrintStats)
    {
        polySortTime = kex::cTimer->GetPerformanceCounter();
    }
    
    polyList.Sort(kexRenderScene::SortPolys);
    
    if(bPrintStats)
    {
        polySortTime = kex::cTimer->GetPerformanceCounter() - polySortTime;
    }

    for(uint i = 0; i < polyList.CurrentLength(); ++i)
    {
        mapPoly_t *poly = &world->Polys()[polyList[i]];
        mapFace_t *face = &world->Faces()[poly->faceRef];
        mapSector_t *sector = &world->Sectors()[face->sectorOwner];

        if(sector != prevSector)
        {
            int rectY = (int)sector->y2;
        
            if(rectY > clipY)
            {
                rectY = clipY;
            }

            prevSector = sector;
            
            // backend accepts the values as integers so there's precision loss when
            // converting from float to ints. add -/+1 to the rect to avoid getting
            // tiny seams between walls
            kexRender::cBackend->SetScissorRect((int)sector->x1-1, (int)sector->y1-1,
                                                (int)sector->x2+1, rectY+1);
        }

        DrawPolygon(face, poly);
    }
    
    if(bPrintStats)
    {
        drawSectorTime = kex::cTimer->GetPerformanceCounter() - drawSectorTime;
    }
}

//
// kexRenderScene::DrawActors
//

void kexRenderScene::DrawActors(kexRenderView &view)
{
    if(bPrintStats)
    {
        drawActorTime = kex::cTimer->GetPerformanceCounter();
    }
    
    spriteMatrix = kexMatrix(-view.Pitch(), 1) * kexMatrix(view.Yaw(), 2);
    spriteMatrix.RotateX(kexMath::pi);
    
    kexRender::cBackend->SetScissorRect(0, 0, kex::cSystem->VideoWidth(), clipY);
    
    for(int i = (int)visibleSectors.CurrentLength()-1; i >= 0; i--)
    {
        DrawActorList(view, &world->Sectors()[visibleSectors[i]]);
    }
    
    if(bPrintStats)
    {
        drawActorTime = kex::cTimer->GetPerformanceCounter() - drawActorTime;
    }
}

//
// kexRenderScene::Prepare
//

void kexRenderScene::Prepare(kexRenderView &view)
{
    int h;
    
    vertCount = 0;
    triCount = 0;
    
    kexRender::cBackend->LoadProjectionMatrix(view.ProjectionView());
    kexRender::cBackend->LoadModelViewMatrix(view.ModelView());
    
    kexRender::cBackend->ClearBuffer(GLCB_STENCIL);
    
    kexRender::cBackend->SetState(GLSTATE_DEPTHTEST, true);
    kexRender::cBackend->SetState(GLSTATE_ALPHATEST, true);
    kexRender::cBackend->SetState(GLSTATE_BLEND, true);
    kexRender::cBackend->SetState(GLSTATE_SCISSOR, true);
    
    h = kex::cSystem->VideoHeight();
    
    clipY = h - (int)((float)h / (240.0f / 24.0f));
    
    kexRender::cVertList->BindDrawPointers();
    waterFaces.Reset();
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
    kexRender::cUtils->PrintStatsText("Visible Sectors", "%i", visibleSectors.CurrentLength());
    kexRender::cUtils->PrintStatsText("Visible Polygons", "%i", polyList.CurrentLength());
    
    kexRender::cUtils->AddDebugLineSpacing();
    
    kexRender::cUtils->PrintStatsText("Flood Fill Time", "%fms",
                                      kex::cTimer->MeasurePerformance(floodFillTime));
    kexRender::cUtils->PrintStatsText("Draw Sector Time", "%fms",
                                      kex::cTimer->MeasurePerformance(drawSectorTime));
    kexRender::cUtils->PrintStatsText("Draw Actor Time", "%fms",
                                      kex::cTimer->MeasurePerformance(drawActorTime));
    kexRender::cUtils->PrintStatsText("Polygon Sort Time", "%fms",
                                      kex::cTimer->MeasurePerformance(polySortTime));
    
    kexRender::cUtils->AddDebugLineSpacing();
}

//
// kexRenderScene::DrawView
//

void kexRenderScene::DrawView(kexRenderView &view, mapSector_t *sector)
{
    if(!world->MapLoaded())
    {
        return;
    }
    
    FindVisibleSectors(view, sector);
    
    Prepare(view);
    
    DrawSky(view);
    
    DrawSectors(view);

    dLights.Draw(this, polyList);
    
    DrawActors(view);
    
    DrawWater(view);
    
    PrintStats();
}
