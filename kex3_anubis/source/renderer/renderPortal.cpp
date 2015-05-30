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
//      Sector scanning and visibility determination
//

#include "kexlib.h"
#include "game.h"
#include "renderMain.h"
#include "renderView.h"
#include "renderScene.h"

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
        dist[i] = plane.Dot(v[i]) + plane.d;
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
    uint scanCount;
    float w, h;
    
    if(bPrintStats)
    {
        floodFillTime = kex::cTimer->GetPerformanceCounter();
    }

    secnum = sector - world->Sectors();

    clipCount++;

    w = (float)kex::cSystem->VideoWidth();
    h = (float)kex::cSystem->VideoHeight();

    for(uint i = 0; i < world->NumSectors(); ++i)
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

                dist = face->plane.Distance(origin);

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
            
            if(face->sector >= 0 && face->flags & FF_PORTAL)
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
