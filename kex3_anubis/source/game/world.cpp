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
#include "world.h"
#include "cmodel.h"
#include "actor.h"
#include "player.h"
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
        
        f->bounds.Clear();
        kexAngle::Clamp(f->angle);
        
        for(int j = 0; j < 4; ++j)
        {
            f->bounds.AddPoint(vertices[f->vertexStart+j].origin);
            f->edges[j].v1 = &vertices[f->vertexStart+j].origin;
            f->edges[j].v2 = &vertices[f->vertexStart+((j+1)&3)].origin;
            f->edges[j].p.SetLine(*f->edges[j].v1, *f->edges[j].v2);
            f->edges[j].flags = 0;
        }
        
        f->plane.SetDistance(vertices[f->vertexStart].origin);
    }
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
    
    actor = kex::cGame->SpawnActor(mapActor->type, x, y, z, an);
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
    kex::cGame->CModel()->Setup(this);

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
        kex::cGame->RemoveAllActors();
        kex::cGame->Player()->ClearActor();
        areaNodes.Destroy();
    }
    
    kex::cGame->CModel()->Reset();
    Mem_Purge(hb_world);
}
