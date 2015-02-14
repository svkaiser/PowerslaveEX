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
//      World/Level logic
//

#include "kexlib.h"
#include "game.h"
#include "renderMain.h"

kexHeapBlock kexWorld::hb_world("world", false, NULL, NULL);

//
// kexWorld::kexWorld
//

kexWorld::kexWorld(void)
{
    this->vertices      = NULL;
    this->sectors       = NULL;
    this->faces         = NULL;
    this->polys         = NULL;
    this->texCoords     = NULL;
    this->events        = NULL;
    this->actors        = NULL;
    this->bMapLoaded    = false;
}

//
// kexWorld::~kexWorld
//

kexWorld::~kexWorld(void)
{
}

//
// kexWorld::ReadTextures
//

void kexWorld::ReadTextures(kexBinFile &mapfile, const unsigned int count)
{
    int len;

    if(count == 0)
    {
        return;
    }

    for(unsigned int i = 0; i < count; ++i)
    {
        kexStr str;
        len = mapfile.Read16();

        if(len > 0)
        {
            for(int j = 0; j < len; ++j)
            {
                char c = (char)mapfile.Read8();
                str += c;
            }

            textures[i] = kexRender::cTextures->Cache(str.c_str(), TC_REPEAT, TF_NEAREST);
        }
        else
        {
            mapfile.Read16();
            textures[i] = NULL;
        }
    }
}

//
// kexWorld::ReadVertices
//

void kexWorld::ReadVertices(kexBinFile &mapfile, const unsigned int count)
{
    if(count == 0)
    {
        kex::cSystem->Error("kexWorld::ReadVertices - No vertices present\n");
        return;
    }

    for(unsigned int i = 0; i < count; ++i)
    {
        vertices[i].origin.x    = (float)mapfile.Read16();
        vertices[i].origin.y    = (float)mapfile.Read16();
        vertices[i].origin.z    = (float)mapfile.Read16();
        vertices[i].rgba[0]     = mapfile.Read8();
        vertices[i].rgba[1]     = mapfile.Read8();
        vertices[i].rgba[2]     = mapfile.Read8();
        vertices[i].rgba[3]     = mapfile.Read8();
    }
}

//
// kexWorld::ReadSectors
//

void kexWorld::ReadSectors(kexBinFile &mapfile, const unsigned int count)
{
    if(count == 0)
    {
        kex::cSystem->Error("kexWorld::ReadSectors - No sectors present\n");
        return;
    }

    for(unsigned int i = 0; i < count; ++i)
    {
        sectors[i].faceStart        = mapfile.Read16();
        sectors[i].faceEnd          = mapfile.Read16();
        sectors[i].lightLevel       = mapfile.Read16();
        sectors[i].ceilingHeight    = mapfile.Read16();
        sectors[i].floorHeight      = mapfile.Read16();
        sectors[i].ceilingSlope     = mapfile.ReadFloat();
        sectors[i].floorSlope       = mapfile.ReadFloat();
        sectors[i].flags            = mapfile.Read16();
        sectors[i].event            = -1;
        sectors[i].validcount       = -1;
        sectors[i].clipCount        = -1;
        sectors[i].floodCount       = 0;
        sectors[i].ceilingFace      = &faces[sectors[i].faceEnd+1];
        sectors[i].floorFace        = &faces[sectors[i].faceEnd+2];

        sectors[i].actorList.Reset();
        
        for(int j = sectors[i].faceStart; j < sectors[i].faceEnd+3; ++j)
        {
            faces[j].sectorOwner = i;
        }
    }
}

//
// kexWorld::ReadFaces
//

void kexWorld::ReadFaces(kexBinFile &mapfile, const unsigned int count)
{
    unsigned int numPortals = 0;

    if(count == 0)
    {
        kex::cSystem->Error("kexWorld::ReadFaces - No faces present\n");
        return;
    }

    for(unsigned int i = 0; i < count; ++i)
    {
        mapFace_t *f = &faces[i];
        
        f->polyStart    = mapfile.Read16();
        f->polyEnd      = mapfile.Read16();
        f->vertexStart  = mapfile.Read16();
        f->sector       = mapfile.Read16();
        f->angle        = mapfile.ReadFloat();
        f->plane.a      = mapfile.ReadFloat();
        f->plane.b      = mapfile.ReadFloat();
        f->plane.c      = mapfile.ReadFloat();
        f->flags        = mapfile.Read16();
        f->tag          = mapfile.Read16();
        f->vertStart    = mapfile.Read16();
        f->vertEnd      = mapfile.Read16();
        f->validcount   = -1;
        f->x1           = 0;
        f->x2           = 0;
        f->y1           = 0;
        f->y2           = 0;
        
        kexAngle::Clamp(f->angle);
        
        for(int j = 0; j < 4; ++j)
        {
            f->edges[j].v1 = &vertices[f->vertexStart+j].origin;
            f->edges[j].v2 = &vertices[f->vertexStart+((j+1)&3)].origin;
            f->edges[j].flags = 0;
        }
        
        UpdateFacePlaneAndBounds(f);

        if(f->flags & FF_PORTAL && f->sector >= 0)
        {
            numPortals++;
        }
    }

    BuildPortals(numPortals);
}

//
// kexWorld::ReadPolys
//

void kexWorld::ReadPolys(kexBinFile &mapfile, const unsigned int count)
{
    if(count == 0)
    {
        kex::cSystem->Error("kexWorld::ReadPolys - No polys present\n");
        return;
    }

    for(unsigned int i = 0; i < count; ++i)
    {
        polys[i].indices[0] = mapfile.Read8();
        polys[i].indices[1] = mapfile.Read8();
        polys[i].indices[2] = mapfile.Read8();
        polys[i].indices[3] = mapfile.Read8();
        polys[i].tcoords[0] = mapfile.Read16();
        polys[i].tcoords[1] = mapfile.Read16();
        polys[i].tcoords[2] = mapfile.Read16();
        polys[i].tcoords[3] = mapfile.Read16();
        polys[i].texture    = mapfile.Read16();
        polys[i].flipped    = mapfile.Read16();
    }
}

//
// kexWorld::ReadTexCoords
//

void kexWorld::ReadTexCoords(kexBinFile &mapfile, const unsigned int count)
{
    if(count == 0)
    {
        kex::cSystem->Error("kexWorld::ReadTexCoords - No texture coordinates present\n");
        return;
    }

    for(unsigned int i = 0; i < count; ++i)
    {
        texCoords[i].uv[0].s = mapfile.ReadFloat();
        texCoords[i].uv[0].t = mapfile.ReadFloat();
        texCoords[i].uv[1].s = mapfile.ReadFloat();
        texCoords[i].uv[1].t = mapfile.ReadFloat();
        texCoords[i].uv[2].s = mapfile.ReadFloat();
        texCoords[i].uv[2].t = mapfile.ReadFloat();
        texCoords[i].uv[3].s = mapfile.ReadFloat();
        texCoords[i].uv[3].t = mapfile.ReadFloat();
    }
}

//
// kexWorld::ReadEvents
//

void kexWorld::ReadEvents(kexBinFile &mapfile, const unsigned int count)
{
    if(count == 0)
    {
        return;
    }

    for(unsigned int i = 0; i < count; ++i)
    {
        events[i].type      = mapfile.Read16();
        events[i].sector    = mapfile.Read16();
        events[i].tag       = mapfile.Read16();
        events[i].unknown   = mapfile.Read16();
        
        if(events[i].sector >= 0)
        {
            sectors[events[i].sector].event = i;
        }
    }
}

//
// kexWorld::ReadActors
//

void kexWorld::ReadActors(kexBinFile &mapfile, const unsigned int count)
{
    if(count == 0)
    {
        kex::cSystem->Error("kexWorld::ReadActors - No actors present; needs at least 1 player actor\n");
        return;
    }

    for(unsigned int i = 0; i < count; ++i)
    {
        actors[i].type      = mapfile.Read16();
        actors[i].sector    = mapfile.Read16();
        actors[i].x         = mapfile.Read16();
        actors[i].y         = mapfile.Read16();
        actors[i].z         = mapfile.Read16();
        actors[i].angle     = mapfile.ReadFloat();
        
        if(actors[i].sector >= 0)
        {
            SpawnMapActor(&actors[i]);
        }
    }
}

//
// kexWorld::BuildPortals
//

void kexWorld::BuildPortals(unsigned int count)
{
    if(count == 0)
    {
        return;
    }

    pvsSize = ((numSectors + 63) & ~63) >> 3;
    pvsMask = (byte*)Mem_Calloc(pvsSize, hb_world);
}

//
// kexWorld::BuildAreaNodes
//

void kexWorld::BuildAreaNodes(void)
{
    kexBBox rootBounds;
    kexVec3 point;
    
    areaNodes.Init(8);
    
    for(unsigned int i = 0; i < numVertices; ++i)
    {
        point.Set(vertices[i].origin.x, vertices[i].origin.y, 0);
        rootBounds.AddPoint(point);
    }
    
    areaNodes.AddBoxToRoot(rootBounds);
    areaNodes.BuildNodes();
}

//
// kexWorld::UpdateSectorBounds
//

void kexWorld::UpdateSectorBounds(mapSector_t *sector)
{
    int end = sector->faceEnd;
    
    mapFace_t *f1 = &faces[end+1];
    mapFace_t *f2 = &faces[end+2];
    
    int vs1 = f1->vertexStart;
    int vs2 = f2->vertexStart;
    
    sector->bounds.Clear();
    
    sector->bounds.AddPoint(vertices[vs1+0].origin);
    sector->bounds.AddPoint(vertices[vs1+1].origin);
    sector->bounds.AddPoint(vertices[vs1+2].origin);
    sector->bounds.AddPoint(vertices[vs1+3].origin);
    sector->bounds.AddPoint(vertices[vs2+0].origin);
    sector->bounds.AddPoint(vertices[vs2+1].origin);
    sector->bounds.AddPoint(vertices[vs2+2].origin);
    sector->bounds.AddPoint(vertices[vs2+3].origin);
}

//
// kexWorld::UpdateFacePlaneAndBounds
//

void kexWorld::UpdateFacePlaneAndBounds(mapFace_t *face)
{
    face->bounds.Clear();
    
    for(int j = 0; j < 4; ++j)
    {
        face->bounds.AddPoint(vertices[face->vertexStart+j].origin);
    }
    
    face->plane.SetDistance(vertices[face->vertexStart].origin);
}

//
// kexWorld::BuildSectorBounds
//

void kexWorld::BuildSectorBounds(void)
{
    for(unsigned int i = 0; i < numSectors; ++i)
    {
        mapSector_t *sector = &sectors[i];
        UpdateSectorBounds(sector);
    }
}

//
// kexWorld::SetupEdges
//

void kexWorld::SetupEdges(void)
{
    for(unsigned int i = 0; i < numSectors; ++i)
    {
        mapSector_t *sector = &sectors[i];
        int start = sector->faceStart;
        int end = sector->faceEnd;
        
        for(int j = start; j < end+1; ++j)
        {
            mapFace_t *face = &faces[j];
            
            if(face->polyStart == -1 || face->polyEnd == -1)
            {
                mapEdge_t *l1 = face->LeftEdge();
                mapEdge_t *r1 = face->RightEdge();
                
                for(int k = start; k < end+1; ++k)
                {
                    if(k == j)
                    {
                        continue;
                    }
                    
                    mapFace_t *f = &faces[k];
                    
                    if(f->polyStart == -1 || f->polyEnd == -1)
                    {
                        continue;
                    }
                    
                    mapEdge_t *l2 = f->LeftEdge();
                    mapEdge_t *r2 = f->RightEdge();
                    
                    if(kexMath::Fabs(r2->v2->z - r1->v1->z) <= 0.001f &&
                       kexMath::Fabs(l2->v1->z - l1->v2->z) <= 0.001f)
                    {
                        f->BottomEdge()->flags |= EGF_TOPSTEP;
                    }
                    
                    if(kexMath::Fabs(r2->v1->z - r1->v2->z) <= 0.001f &&
                       kexMath::Fabs(l2->v2->z - l1->v1->z) <= 0.001f)
                    {
                        f->TopEdge()->flags |= EGF_BOTTOMSTEP;
                    }
                }
            }
        }
    }
}

//
// kexWorld::SpawnMapActor
//

void kexWorld::SpawnMapActor(mapActor_t *mapActor)
{
    float x, y, z;
    float an;
    kexActor *actor;
    
    if(mapActor->type <= -1 || mapActor->type >= NUMACTORTYPES)
    {
        return;
    }
    
    x   = (float)mapActor->x;
    y   = (float)mapActor->y;
    z   = (float)mapActor->z;
    an  = kexMath::Deg2Rad((360 - (float)mapActor->angle) + 90);
    
    actor = kexGame::cLocal->SpawnActor(mapActor->type, x, y, z, an, mapActor->sector);
    actor->SetMapActor(mapActor);
}

//
// kexWorld::LoadMap
//

bool kexWorld::LoadMap(const char *mapname)
{
    kexBinFile mapfile;

    bMapLoaded = false;

    if(!mapfile.Open(mapname))
    {
        kex::cSystem->Warning("kexWorld::LoadMap - %s not found\n", mapname);
        return false;
    }

    numTextures     = mapfile.Read32();
    numVertices     = mapfile.Read32();
    numSectors      = mapfile.Read32();
    numFaces        = mapfile.Read32();
    numPolys        = mapfile.Read32();
    numTCoords      = mapfile.Read32();
    numEvents       = mapfile.Read32();
    numActors       = mapfile.Read32();

    if(numTextures  > 0) textures  = (kexTexture**)   Mem_Malloc(sizeof(kexTexture*) * numTextures, hb_world);
    if(numVertices  > 0) vertices  = (mapVertex_t*)   Mem_Malloc(sizeof(mapVertex_t) * numVertices, hb_world);
    if(numSectors   > 0) sectors   = (mapSector_t*)   Mem_Malloc(sizeof(mapSector_t) * numSectors, hb_world);
    if(numFaces     > 0) faces     = (mapFace_t*)     Mem_Malloc(sizeof(mapFace_t) * numFaces, hb_world);
    if(numPolys     > 0) polys     = (mapPoly_t*)     Mem_Malloc(sizeof(mapPoly_t) * numPolys, hb_world);
    if(numTCoords   > 0) texCoords = (mapTexCoords_t*)Mem_Malloc(sizeof(mapTexCoords_t) * numTCoords, hb_world);
    if(numEvents    > 0) events    = (mapEvent_t*)    Mem_Malloc(sizeof(mapEvent_t) * numEvents, hb_world);
    if(numActors    > 0) actors    = (mapActor_t*)    Mem_Malloc(sizeof(mapActor_t) * numActors, hb_world);

    ReadTextures(mapfile, numTextures);
    ReadVertices(mapfile, numVertices);
    ReadSectors(mapfile, numSectors);
    ReadFaces(mapfile, numFaces);
    ReadPolys(mapfile, numPolys);
    ReadTexCoords(mapfile, numTCoords);
    ReadEvents(mapfile, numEvents);
    
    BuildAreaNodes();
    BuildSectorBounds();
    SetupEdges();
    kexGame::cLocal->CModel()->Setup(this);
    
    ReadActors(mapfile, numActors);

    bMapLoaded = true;
    return true;
}

//
// kexWorld::UnloadMap
//

void kexWorld::UnloadMap(void)
{
    if(bMapLoaded)
    {
        kexGame::cLocal->RemoveAllGameObjects();
        kexGame::cLocal->Player()->ClearActor();
        areaNodes.Destroy();
    }
    
    kexGame::cLocal->CModel()->Reset();
    Mem_Purge(hb_world);
}

//
// kexWorld::UseSectorSpecial
//

void kexWorld::UseWallSpecial(mapFace_t *face)
{
    mapEvent_t *ev;

    if(face->tag <= -1)
    {
        return;
    }

    ev = &events[face->tag];

    if(ev->sector >= 0)
    {
        if(sectors[ev->sector].flags & SF_SPECIAL)
        {
            return;
        }
    }

    switch(ev->type)
    {
    case 1:
        kexGame::cLocal->SpawnMover("kexDoor", ev->type, ev->sector);
        break;

    case 7:
        break;

    default:
        break;
    }
}

//
// kexWorld::EnterSectorSpecial
//

void kexWorld::EnterSectorSpecial(mapSector_t *sector)
{
    mapEvent_t *ev;

    if(sector->event <= -1)
    {
        return;
    }

    if(sector->flags & SF_SPECIAL)
    {
        return;
    }

    ev = &events[sector->event];

    switch(ev->type)
    {
    case 50:
        SendRemoteTrigger(ev);
        sector->event = -1;
        break;

    default:
        break;
    }
}

//
// kexWorld::SendRemoteTrigger
//

void kexWorld::SendRemoteTrigger(mapEvent_t *event)
{
    for(unsigned int i = 0; i < numEvents; ++i)
    {
        mapEvent_t *ev = &events[i];

        if(ev == event)
        {
            continue;
        }

        if(ev->tag == event->tag)
        {
            if(ev->sector >= 0)
            {
                if(sectors[ev->sector].flags & SF_SPECIAL)
                {
                    continue;
                }
            }

            if(ev->type == event->type)
            {
                ev->tag = -1;
                continue;
            }

            switch(ev->type)
            {
            case 7:
                kexGame::cLocal->SpawnMover("kexDoor", ev->type, ev->sector);
                break;

            default:
                break;
            }
        }
    }
}

//
// kexWorld::RadialDamage
//

void kexWorld::RadialDamage(kexActor *source, const float radius, const int damage)
{
    if(source->Sector() == NULL)
    {
        return;
    }
    
    unsigned int scanCount = 0;
    kexVec3 start = source->Origin() + kexVec3(0, 0, source->Height() * 0.5f);
    kexVec3 end;
    kexBBox bounds;
    float dist;
    float dmgAmount;
    
    bounds.min.Set(-radius);
    bounds.max.Set( radius);
    bounds.min += start;
    bounds.max += start;
    
    scanSectors.Reset();
    scanSectors.Set(source->Sector());
    source->Sector()->floodCount = 1;
    
    memset(pvsMask, 0, pvsSize);
    MarkSectorInPVS(source->Sector() - sectors);
    
    do
    {
        mapSector_t *sector = scanSectors[scanCount++];
        
        if(sector->floodCount == 0)
        {
            continue;
        }
        
        sector->floodCount = 0;
        
        for(int i = sector->faceStart; i < sector->faceEnd+3; ++i)
        {
            mapFace_t *face = &faces[i];
            
            if(face->flags & FF_PORTAL && face->sector >= 0)
            {
                mapSector_t *next = &sectors[face->sector];
                
                if(SectorInPVS(face->sector))
                {
                    continue;
                }
                
                if(bounds.IntersectingBox(next->bounds))
                {
                    next->floodCount = 1;
                    scanSectors.Set(next);
                    MarkSectorInPVS(face->sector);
                }
            }
        }
        
        for(kexActor *actor = sector->actorList.Next();
            actor != NULL;
            actor = actor->SectorLink().Next())
        {
            if(actor == source)
            {
                // don't check self
                continue;
            }
            
            if(!(actor->Flags() & AF_SOLID) || !(actor->Flags() & AF_SHOOTABLE))
            {
                continue;
            }
            
            end = actor->Origin() + kexVec3(0, 0, actor->Height() * 0.5f);
            dist = start.DistanceSq(end);
            
            if(dist > (radius * radius))
            {
                continue;
            }
            
            if(!kexGame::cLocal->CModel()->Trace(source, source->Sector(), start, end))
            {
                continue;
            }
            
            if(kexGame::cLocal->CModel()->ContactActor() != actor)
            {
                continue;
            }
            
            dmgAmount = (1.0f - (dist / (radius * radius))) * (float)damage;
            actor->InflictDamage(source, (int)dmgAmount);
        }
        
    } while(scanCount < scanSectors.CurrentLength());
}

//
// kexWorld::MarkSectorInPVS
//

void kexWorld::MarkSectorInPVS(const int secnum)
{
    if(secnum <= -1)
    {
        return;
    }

    pvsMask[secnum >> 3] |= (1 << (secnum & 7));
}

//
// kexWorld::SectorInPVS
//

bool kexWorld::SectorInPVS(const int secnum)
{
    if(secnum <= -1)
    {
        return false;
    }

    return (pvsMask[secnum >> 3] & (1 << (secnum & 7))) != 0;
}

//
// kexWorld::SetFaceSpans
//

bool kexWorld::SetFaceSpans(kexRenderView &view, mapFace_t *face)
{
    float x1, x2, x3, x4, y1, y2, y3, y4;
    float fx1, fx2, fy1, fy2;
    float w, h;
    
    kexVec3 p1 = view.ProjectPoint(vertices[face->vertexStart+2].origin);
    kexVec3 p2 = view.ProjectPoint(vertices[face->vertexStart+3].origin);
    kexVec3 p3 = view.ProjectPoint(vertices[face->vertexStart+1].origin);
    kexVec3 p4 = view.ProjectPoint(vertices[face->vertexStart+0].origin);

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

    return true;
}

//
// kexWorld::FindVisibleSectors
//

void kexWorld::FindVisibleSectors(kexRenderView &view, mapSector_t *sector)
{
    static int clipCount = 0;
    int secnum = sector - sectors;
    int start, end;
    kexVec3 origin;
    unsigned int scanCount;
    float w, h;

    portalsPassed = 0;
    clipCount++;

    w = (float)kex::cSystem->VideoWidth();
    h = (float)kex::cSystem->VideoHeight();

    for(unsigned int i = 0; i < numSectors; ++i)
    {
        sectors[i].x1 = w;
        sectors[i].x2 = 0;
        sectors[i].y1 = h;
        sectors[i].y2 = 0;

        sectors[i].floodCount = 0;
        sectors[i].flags &= ~SF_CLIPPED;
    }

    origin = view.Origin();

    scanSectors.Reset();
    scanSectors.Set(sector);

    visibleSkyFaces.Reset();
    visibleSectors.Reset();
    visibleSectors.Set(secnum);

    memset(pvsMask, 0, pvsSize);
    MarkSectorInPVS(secnum);

    sector->floodCount = 1;
    sector->x1 = 0;
    sector->x2 = w;
    sector->y1 = 0;
    sector->y2 = h;

    scanCount = 0;
    
    do
    {
        mapSector_t *s = scanSectors[scanCount++];
        
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
                if(faces[i].flags & FF_PORTAL)
                {
                    dist = faces[i].plane.Distance(origin) - faces[i].plane.d;

                    if(dist <= 0)
                    {
                        continue;
                    }

                    SetFaceSpans(view, &faces[i]);

                    if(dist <= 96)
                    {
                        faces[i].x1 = 0;
                        faces[i].x2 = w;
                        faces[i].y1 = 0;
                        faces[i].y2 = h;
                    }
                }
                else
                {
                    SetFaceSpans(view, &faces[i]);
                }
            }
        }

        if(!view.TestBoundingBox(s->bounds))
        {
            continue;
        }
        
        for(int i = start; i < end+3; ++i)
        {
            mapFace_t *face = &faces[i];

            if(!view.TestBoundingBox(face->bounds))
            {
                continue;
            }
            
            if(face->sector >= 0)
            {
                mapSector_t *next = &sectors[face->sector];
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
                
                portalsPassed++;
                
                if(!SectorInPVS(face->sector))
                {
                    MarkSectorInPVS(face->sector);
                    visibleSectors.Set(face->sector);
                }

                scanSectors.Set(next);
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
        
    } while(scanCount < scanSectors.CurrentLength());
}
