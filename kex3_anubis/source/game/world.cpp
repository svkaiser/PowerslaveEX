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
        sectors[i].validcount       = -1;
        sectors[i].floodCount       = 0;
        sectors[i].ceilingFace      = &faces[sectors[i].faceEnd+1];
        sectors[i].floorFace        = &faces[sectors[i].faceEnd+2];
        
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
        f->portal       = NULL;
        
        f->bounds.Clear();
        kexAngle::Clamp(f->angle);
        
        for(int j = 0; j < 4; ++j)
        {
            f->bounds.AddPoint(vertices[f->vertexStart+j].origin);
            f->edges[j].v1 = &vertices[f->vertexStart+j].origin;
            f->edges[j].v2 = &vertices[f->vertexStart+((j+1)&3)].origin;
            f->edges[j].flags = 0;
        }
        
        f->plane.SetDistance(vertices[f->vertexStart].origin);

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
        polys[i].texture    = mapfile.Read16();
        polys[i].flipped    = mapfile.Read16();
        polys[i].tcoord     = mapfile.Read32();
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
        
        SpawnMapActor(&actors[i]);
    }
}

//
// kexWorld::BuildPortals
//

void kexWorld::BuildPortals(unsigned int count)
{
    unsigned int idx = 0;

    if(count == 0)
    {
        return;
    }

    pvsSize = ((numSectors + 63) & ~63) >> 3;
    pvsMask = (byte*)Mem_Calloc(pvsSize, hb_world);

    numPortals = count;
    portals = (portal_t*)Mem_Calloc(sizeof(portal_t) * numPortals, hb_world);

    for(unsigned int i = 0; i < numFaces; ++i)
    {
        mapFace_t *f = &faces[i];
        portal_t *p;

        if(!(f->flags & FF_PORTAL) || f->sector == -1)
        {
            continue;
        }

        assert(idx < numPortals);

        p = &portals[idx++];
        p->face = f;
        p->sector = &sectors[f->sectorOwner];
        f->portal = p;
    }

    assert(idx == numPortals);
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
// kexWorld::BuildSectorBounds
//

void kexWorld::BuildSectorBounds(void)
{
    for(unsigned int i = 0; i < numSectors; ++i)
    {
        mapSector_t *sector = &sectors[i];
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
    
    actor = kexGame::cLocal->SpawnActor(mapActor->type, x, y, z, an);
    actor->SetMapActor(mapActor);
    actor->SetSector(&sectors[mapActor->sector]);
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
    
    ReadActors(mapfile, numActors);
    kexGame::cLocal->CModel()->Setup(this);

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
        kexGame::cLocal->RemoveAllActors();
        kexGame::cLocal->Player()->ClearActor();
        areaNodes.Destroy();
    }
    
    kexGame::cLocal->CModel()->Reset();
    Mem_Purge(hb_world);
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

void kexWorld::SetFaceSpans(kexRenderView &view, mapFace_t *face)
{
    kexMatrix mtx = view.RotationMatrix();
    
    kexVec3 p1 = *face->BottomEdge()->v1 * mtx;
    kexVec3 p2 = *face->BottomEdge()->v2 * mtx;
    kexVec3 p3 = *face->TopEdge()->v2 * mtx;
    kexVec3 p4 = *face->TopEdge()->v1 * mtx;
    
    kexVec2 c1p(p1.x, p1.y);
    kexVec2 c2p(p2.x, p2.y);
    kexVec2 c3p(p3.x, p3.y);
    kexVec2 c4p(p4.x, p4.y);
    kexVec2 c5p(p2.z, p2.y);
    kexVec2 c6p(p4.z, p4.y);
    kexVec2 c7p(p1.z, p1.y);
    kexVec2 c8p(p3.z, p3.y);

    face->leftSpan[0] = face->leftSpan[1] = -kexMath::infinity;
    face->topSpan[0] = face->topSpan[1] = -kexMath::infinity;
    face->rightSpan[0] = face->rightSpan[1] = kexMath::infinity;
    face->bottomSpan[0] = face->bottomSpan[1] = kexMath::infinity;

    if(c2p.CrossScalar(c1p) >= 0)
    {
        if(p1.y >= 0.999f) face->rightSpan[0] = (p1.x * 160 / p1.y + 160);
        if(p2.y >= 0.999f) face->leftSpan[0] = (p2.x * 160 / p2.y + 160);
    }
    
    if(c4p.CrossScalar(c3p) >= 0)
    {
        if(p3.y >= 0.999f) face->rightSpan[1] = (p3.x * 160 / p3.y + 160);
        if(p4.y >= 0.999f) face->leftSpan[1] = (p4.x * 160 / p4.y + 160);
    }

    if(c5p.CrossScalar(c6p) >= 0)
    {
        if(p4.y >= 0.999f) face->topSpan[0] = 240 - (p4.z * 120 / p4.y + 120);
        if(p2.y >= 0.999f) face->bottomSpan[0] = 240 - (p2.z * 120 / p2.y + 120);
    }

    if(c7p.CrossScalar(c8p) >= 0)
    {
        if(p3.y >= 0.999f) face->topSpan[1] = 240 - (p3.z * 120 / p3.y + 120);
        if(p1.y >= 0.999f) face->bottomSpan[1] = 240 - (p1.z * 120 / p1.y + 120);
    }
}

//
// kexWorld::FaceInPortalView
//
static float left[2];
static float right[2];
static float top[2];
static float bottom[2];
bool kexWorld::FaceInPortalView(kexRenderView &view, mapSector_t *sector, mapFace_t *face)
{
    float pls1 = left[0];
    float pls2 = left[1];
    float prs1 = right[0];
    float prs2 = right[1];
    float pts1 = top[0];
    float pts2 = top[1];
    float pbs1 = bottom[0];
    float pbs2 = bottom[1];
    float ls1 = face->leftSpan[0];
    float ls2 = face->leftSpan[1];
    float rs1 = face->rightSpan[0];
    float rs2 = face->rightSpan[1];
    float ts1 = face->topSpan[0];
    float ts2 = face->topSpan[1];
    float bs1 = face->bottomSpan[0];
    float bs2 = face->bottomSpan[1];
    
    int sidebit1 = 0;
    int sidebit2 = 0;
    int sidebit3 = 0;
    int sidebit4 = 0;
    
    float t;
    
    t = ls1-pls1; sidebit1 |= (FLOATSIGNBIT(t) << 0);
    t = ls1-pls2; sidebit1 |= (FLOATSIGNBIT(t) << 1);
    t = ls2-pls2; sidebit1 |= (FLOATSIGNBIT(t) << 2);
    t = ls2-pls1; sidebit1 |= (FLOATSIGNBIT(t) << 3);
    t = rs1-pls1; sidebit1 |= (FLOATSIGNBIT(t) << 4);
    t = rs1-pls2; sidebit1 |= (FLOATSIGNBIT(t) << 5);
    t = rs2-pls2; sidebit1 |= (FLOATSIGNBIT(t) << 6);
    t = rs2-pls1; sidebit1 |= (FLOATSIGNBIT(t) << 7);
    
    t = prs1-rs1; sidebit2 |= (FLOATSIGNBIT(t) << 0);
    t = prs2-rs2; sidebit2 |= (FLOATSIGNBIT(t) << 1);
    t = prs1-rs2; sidebit2 |= (FLOATSIGNBIT(t) << 2);
    t = prs2-rs1; sidebit2 |= (FLOATSIGNBIT(t) << 3);
    t = prs1-ls1; sidebit2 |= (FLOATSIGNBIT(t) << 4);
    t = prs2-ls2; sidebit2 |= (FLOATSIGNBIT(t) << 5);
    t = prs1-ls2; sidebit2 |= (FLOATSIGNBIT(t) << 6);
    t = prs2-ls1; sidebit2 |= (FLOATSIGNBIT(t) << 7);
    
    t = ts1-pts1; sidebit3 |= (FLOATSIGNBIT(t) << 0);
    t = ts1-pts2; sidebit3 |= (FLOATSIGNBIT(t) << 1);
    t = ts2-pts1; sidebit3 |= (FLOATSIGNBIT(t) << 2);
    t = ts2-pts2; sidebit3 |= (FLOATSIGNBIT(t) << 3);
    t = bs1-pts1; sidebit3 |= (FLOATSIGNBIT(t) << 4);
    t = bs1-pts2; sidebit3 |= (FLOATSIGNBIT(t) << 5);
    t = bs2-pts1; sidebit3 |= (FLOATSIGNBIT(t) << 6);
    t = bs2-pts2; sidebit3 |= (FLOATSIGNBIT(t) << 7);
    
    t = pbs1-bs1; sidebit4 |= (FLOATSIGNBIT(t) << 0);
    t = pbs2-bs1; sidebit4 |= (FLOATSIGNBIT(t) << 1);
    t = pbs1-bs2; sidebit4 |= (FLOATSIGNBIT(t) << 2);
    t = pbs2-bs2; sidebit4 |= (FLOATSIGNBIT(t) << 3);
    t = pbs1-ts1; sidebit4 |= (FLOATSIGNBIT(t) << 4);
    t = pbs1-ts2; sidebit4 |= (FLOATSIGNBIT(t) << 5);
    t = pbs2-ts1; sidebit4 |= (FLOATSIGNBIT(t) << 6);
    t = pbs2-ts2; sidebit4 |= (FLOATSIGNBIT(t) << 7);
    
    if(!(sidebit3 == 0xff || sidebit4 == 0xff))
    {
        return !(sidebit1 == 0xff || sidebit2 == 0xff);
    }
    
    return false;
}

//
// kexWorld::RecursiveSectorPortals
//

void kexWorld::RecursiveSectorPortals(kexRenderView &view, portal_t *portal)
{
    mapFace_t *face;
    int start, end;
    mapSector_t *sector;
    float pleft[2];
    float pright[2];
    float ptop[2];
    float pbottom[2];
    
    sector = &sectors[portal->face->sector];

    if(sector->floodCount > 1)
    {
        return;
    }

    if(!view.Frustum().TestBoundingBox(sector->bounds))
    {
        return;
    }
    
    sector->floodCount++;
    
    if(!SectorInPVS(portal->face->sector))
    {
        MarkSectorInPVS(portal->face->sector);
        visibleSectors.Set(portal->face->sector);
    }

    start = sector->faceStart;
    end = sector->faceEnd;
    
    for(int i = start; i < end+1; ++i)
    {
        face = &faces[i];

        if(face->validcount == 1)
        {
            //continue;
        }
        
        if(!face->InFront(view.Origin()))
        {
            continue;
        }

        SetFaceSpans(view, face);
        
        if(!view.Frustum().TestBoundingBox(face->bounds))
        {
            continue;
        }

        if(!FaceInPortalView(view, sector, face))
        {
            continue;
        }

        face->validcount = 1;

        if(!(face->flags & FF_PORTAL) || face->sector <= -1)
        {
            continue;
        }

        if(face->portal == NULL)
        {
            continue;
        }
        
        pleft[0] = left[0];
        pleft[1] = left[1];
        pright[0] = right[0];
        pright[1] = right[1];
        ptop[0] = top[0];
        ptop[1] = top[1];
        pbottom[0] = bottom[0];
        pbottom[1] = bottom[1];
        
        if(face->leftSpan[0] > left[0]) left[0] = face->leftSpan[0];
        if(face->leftSpan[1] > left[1]) left[1] = face->leftSpan[1];
        if(face->rightSpan[0] < right[0]) right[0] = face->rightSpan[0];
        if(face->rightSpan[1] < right[1])right[1] = face->rightSpan[1];
        if(face->topSpan[0] > top[0]) top[0] = face->topSpan[0];
        if(face->topSpan[1] > top[1]) top[1] = face->topSpan[1];
        if(face->bottomSpan[0] < bottom[0]) bottom[0] = face->bottomSpan[0];
        if(face->bottomSpan[1] < bottom[1]) bottom[1] = face->bottomSpan[1];
        
        RecursiveSectorPortals(view, face->portal);
        
        left[0] = pleft[0];
        left[1] = pleft[1];
        right[0] = pright[0];
        right[1] = pright[1];
        top[0] = ptop[0];
        top[1] = ptop[1];
        bottom[0] = pbottom[0];
        bottom[1] = pbottom[1];
    }
    
    if(sector->floodCount > 0)
    {
        sector->floodCount--;
    }
}

//
// kexWorld::FindVisibleSectors
//

void kexWorld::FindVisibleSectors(kexRenderView &view, mapSector_t *sector)
{
    int secnum = sector - sectors;
    int start, end;
    kexVec3 origin;

    memset(pvsMask, 0, pvsSize);
    
    left[0] = left[1] = -kexMath::infinity;
    top[0] = top[1] = -kexMath::infinity;
    right[0] = right[1] = kexMath::infinity;
    bottom[0] = bottom[1] = kexMath::infinity;

    for(unsigned int i = 0; i < numSectors; ++i)
    {
        sectors[i].leftSpan[0] = sectors[i].leftSpan[1] = 0;
        sectors[i].rightSpan[0] = sectors[i].rightSpan[1] = 320;
        sectors[i].topSpan[0] = sectors[i].topSpan[1] = 0;
        sectors[i].bottomSpan[0] = sectors[i].bottomSpan[1] = 240;

        sectors[i].floodCount = 0;
    }

    visibleSectors.Reset();
    visibleSectors.Set(secnum);

    MarkSectorInPVS(secnum);

    start = sector->faceStart;
    end = sector->faceEnd;

    origin = view.Origin();

    for(int i = start; i < end+1; ++i)
    {
        mapFace_t *face = &faces[i];

        face->validcount = 1;

        if(!(face->flags & FF_PORTAL) || face->sector <= -1)
        {
            continue;
        }

        if(face->portal == NULL)
        {
            continue;
        }

        SetFaceSpans(view, face);
        
        left[0] = face->leftSpan[0];
        left[1] = face->leftSpan[1];
        right[0] = face->rightSpan[0];
        right[1] = face->rightSpan[1];
        top[0] = face->topSpan[0];
        top[1] = face->topSpan[1];
        bottom[0] = face->bottomSpan[0];
        bottom[1] = face->bottomSpan[1];

        RecursiveSectorPortals(view, face->portal);
    }
}
