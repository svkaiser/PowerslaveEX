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
        vertices[i].x       = mapfile.Read16();
        vertices[i].y       = mapfile.Read16();
        vertices[i].z       = mapfile.Read16();
        vertices[i].rgba[0] = mapfile.Read8();
        vertices[i].rgba[1] = mapfile.Read8();
        vertices[i].rgba[2] = mapfile.Read8();
        vertices[i].rgba[3] = mapfile.Read8();
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
    }
}

//
// kexWorld::ReadFaces
//

void kexWorld::ReadFaces(kexBinFile &mapfile, const unsigned int count)
{
    kexVec3 point;
    float x, y, z;
    mapVertex_t *v;
    
    if(count == 0)
    {
        kex::cSystem->Error("kexWorld::ReadFaces - No faces present\n");
        return;
    }

    for(unsigned int i = 0; i < count; ++i)
    {
        faces[i].polyStart  = mapfile.Read16();
        faces[i].polyEnd    = mapfile.Read16();
        faces[i].sector     = mapfile.Read16();
        faces[i].angle      = mapfile.ReadFloat();
        faces[i].plane.a    = mapfile.ReadFloat();
        faces[i].plane.b    = mapfile.ReadFloat();
        faces[i].plane.c    = mapfile.ReadFloat();
        faces[i].flags      = mapfile.Read16();
        faces[i].tag        = mapfile.Read16();
        faces[i].vertStart  = mapfile.Read16();
        faces[i].vertEnd    = mapfile.Read16();
        
        v = &vertices[faces[i].vertStart];
        x = (float)v->x;
        y = (float)v->y;
        z = (float)v->z;
        
        point.Set(x, y, z);
        
        faces[i].plane.SetDistance(point);
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
        point.Set((float)vertices[i].x, (float)vertices[i].y, 0);
        rootBounds.AddPoint(point);
    }
    
    areaNodes.AddBoxToRoot(rootBounds);
    areaNodes.BuildNodes();
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
        kex::cGame->RemoveAllActors();
        kex::cGame->Player()->ClearActor();
        areaNodes.Destroy();
    }
    
    Mem_Purge(hb_world);
}
