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
//      Dynamic Light Manager
//

#include "renderMain.h"
#include "game.h"
#include "dlightObj.h"
#include "renderScene.h"

//
// kexRenderDLight::kexRenderDLight
//

kexRenderDLight::kexRenderDLight(void)
{
    this->lightMarks = NULL;
    this->numDLights = 0;

    memset(dLightList, 0, sizeof(dLightList));
}

//
// kexRenderDLight::Init
//

void kexRenderDLight::Init(void)
{
    uint numSectors = kexGame::cWorld->NumSectors();

    if(numSectors == 0)
    {
        return;
    }

    lightMarks = (uint*)Mem_Calloc(sizeof(uint) * numSectors, kexWorld::hb_world);

    Clear();
}

//
// kexRenderDLight::Clear
//

void kexRenderDLight::Clear(void)
{
    numDLights = 0;
    memset(lightMarks, 0, sizeof(uint) * kexGame::cWorld->NumSectors());
}

//
// kexRenderDLight::AddLight
//

void kexRenderDLight::AddLight(kexDLight *light)
{
    uint sectorCount = 0;
    kexWorld *w = kexGame::cWorld;
    mapFace_t *faces;
    mapSector_t *sectors;
    mapSector_t *startSector;
    sectorList_t *sectorList;

    if(light->Sector() == NULL || numDLights >= MAX_DLIGHTS)
    {
        light->Remove();
        return;
    }

    sectorList = &w->ScanSectors();
    sectorList->Reset();

    faces = w->Faces();
    sectors = w->Sectors();
    
    startSector = light->Sector();
    
    sectorList->Set(startSector);
    lightMarks[startSector - sectors] |= BIT(numDLights);
    
    do
    {
        mapSector_t *sec = (*sectorList)[sectorCount++];
        
        for(int i = sec->faceStart; i < sec->faceEnd+3; ++i)
        {
            mapFace_t *face = &faces[i];
            mapSector_t *s;
            
            if(face->sector <= -1)
            {
                continue;
            }
            
            s = &sectors[face->sector];
            
            if(s == startSector || s == sec)
            {
                continue;
            }
            
            if(lightMarks[s - sectors] & BIT(numDLights))
            {
                // we already checked this sector
                continue;
            }
            
            // is this linked sector reachable?
            if(light->Bounds().IntersectingBox(s->bounds))
            {
                sectorList->Set(s);
                lightMarks[s - sectors] |= BIT(numDLights);
            }
        }
        
    } while(sectorCount < sectorList->CurrentLength());

    dLightList[numDLights++] = light;
}

//
// kexRenderDLight::RenderLitPolygon
//

void kexRenderDLight::RenderLitPolygon(mapPoly_t *poly, int &tris)
{
    kexWorld *w = kexGame::cWorld;
    kexCpuVertList *vl = kexRender::cVertList;
    mapFace_t *face = &w->Faces()[poly->faceRef];
    mapSector_t *sector = &w->Sectors()[face->sectorOwner];
    int xi, yi;
    float n1, n2, v1, v2;
    float radius, rHalf;
    int passes;
    byte *rgb;
    mapVertex_t *verts;
    kexVec3 lightOrg;

    sector->floodCount = 0;
    verts = w->Vertices();

    for(int j = 0; j < MAX_DLIGHTS; ++j)
    {
        int indices[4] = { 0, 0, 0, 0 };
        int curIdx = 0;
        kexVec3 vPoint;
        kexVec3 vStart;
        float dist;
        float px, py;
        int r, g, b, a;
        mapVertex_t *vertex;
        kexDLight *light;

        if(!(lightMarks[sector - w->Sectors()] & BIT(j)))
        {
            continue;
        }

        if(dLightList[j] == NULL)
        {
            continue;
        }

        light = dLightList[j];

        lightOrg = light->Origin();
        radius = light->Radius() * 4;
        passes = light->Passes();
        rgb = light->Color();

        rHalf = radius * 0.5f;

        dist = face->plane.Distance(lightOrg);

        if(dist > radius || dist < 0)
        {
            continue;
        }

        dist = (radius / (dist * dist)) * 16;
        kexMath::Clamp(dist, 0, 1);

        for(int idx = 0; idx < 4; idx++)
        {
            if(poly->indices[idx] == 0xff || poly->tcoords[idx] == -1)
            {
                continue;
            }
            
            indices[curIdx] = poly->indices[idx];
            curIdx++;
        }

        if(curIdx == 0)
        {
            continue;
        }

        vStart = verts[face->vertStart + indices[(curIdx-1)]].origin;

        switch(face->plane.BestAxis())
        {
        case kexPlane::AXIS_XY:
            xi = 0;
            yi = 1;
            break;

        case kexPlane::AXIS_XZ:
            xi = 0;
            yi = 2;
            break;

        case kexPlane::AXIS_YZ:
            xi = 1;
            yi = 2;
            break;
        }

        n1 = lightOrg[xi];
        n2 = lightOrg[yi];
        v1 = vStart[xi];
        v2 = vStart[yi];

        px = ((v1 - n1) / rHalf) + 0.5f;
        py = ((v2 - n2) / rHalf) + 0.5f;

        r = (byte)(rgb[0] * dist);
        g = (byte)(rgb[1] * dist);
        b = (byte)(rgb[2] * dist);
        a = 255;

        if(light->FadeTime() > -1)
        {
            r = (byte)(r * light->FadeFrac());
            g = (byte)(g * light->FadeFrac());
            b = (byte)(b * light->FadeFrac());
        }

        for(int idx = (curIdx-1); idx >= 0; idx--)
        {
            vertex = &verts[face->vertStart + indices[idx]];
            vPoint = vertex->origin;

            float tu = px + ((vPoint[xi] - v1) / rHalf);
            float tv = py + ((vPoint[yi] - v2) / rHalf);

            vl->AddVertex(vPoint, tu, tv, r, g, b, a);
        }
        
        for(int p = 0; p < passes+1; ++p)
        {
            vl->AddTriangle(tris+0, tris+2, tris+1);
            if(curIdx == 4)
            {
                vl->AddTriangle(tris+0, tris+3, tris+2);
            }
        }
        
        tris += curIdx;
    }
}

//
// kexRenderDLight::Draw
//

void kexRenderDLight::Draw(kexRenderScene *rScene)
{
    kexWorld *w = kexGame::cWorld;
    kexCpuVertList *vl = kexRender::cVertList;
    int tris;
    uint count;

    kexRender::cBackend->SetDepth(GLFUNC_EQUAL);
    kexRender::cBackend->SetBlend(GLSRC_DST_COLOR, GLDST_ONE);
    kexRender::cBackend->SetState(GLSTATE_SCISSOR, false);
    kexRender::cTextures->lightTexture->Bind();

    tris = 0;
    count = rScene->polyList.CurrentLength();

    for(uint i = 0; i < count; ++i)
    {
        mapPoly_t *poly = &w->Polys()[rScene->polyList[i]];

        if(lightMarks[w->Faces()[poly->faceRef].sectorOwner] == 0)
        {
            continue;
        }

        RenderLitPolygon(poly, tris);
    }

    if(tris > 0)
    {
        vl->DrawElements();
    }

    kexRender::cBackend->SetBlend(GLSRC_SRC_ALPHA, GLDST_ONE_MINUS_SRC_ALPHA);
    kexRender::cBackend->SetDepth(GLFUNC_LEQUAL);
    kexRender::cBackend->SetState(GLSTATE_SCISSOR, true);
}
