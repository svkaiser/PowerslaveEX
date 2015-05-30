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
#include "mover.h"
#include "renderScene.h"

static kexWorld worldLocal;
kexWorld *kexGame::cWorld = &worldLocal;

kexHeapBlock kexWorld::hb_world("world", false, NULL, NULL);

//
// kexWorld::kexWorld
//

kexWorld::kexWorld(void)
{
    this->skyTexture    = NULL;
    this->textures      = NULL;
    this->vertices      = NULL;
    this->sectors       = NULL;
    this->faces         = NULL;
    this->polys         = NULL;
    this->texCoords     = NULL;
    this->events        = NULL;
    this->actors        = NULL;
    this->animPics      = NULL;
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

    animPics = (animPic_t*)Mem_Calloc(sizeof(animPic_t) * count, hb_world);

    len = mapfile.Read16();

    if(len > 0)
    {
        kexStr str;

        for(int j = 0; j < len; ++j)
        {
            char c = (char)mapfile.Read8();
            str += c;
        }

        skyTexture = kexRender::cTextures->Cache(str.c_str(), TC_REPEAT, TF_NEAREST);
    }
    else
    {
        mapfile.Read16();
        skyTexture = NULL;
    }

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
            kexDict *dict;

            for(int j = 0; j < len; ++j)
            {
                char c = (char)mapfile.Read8();
                str += c;
            }

            textures[i] = kexRender::cTextures->Cache(str.c_str(), TC_REPEAT, TF_NEAREST);

            if((dict = kexGame::cLocal->AnimPicDefs().GetEntry(str.c_str())))
            {
                kexStrList pics;
                int idx = 1;

                dict->GetFloat("speed", animPics[i].speed);

                while(1)
                {
                    kexStr picStr;

                    if(!dict->GetString(kexStr::Format("texture_%i", idx), picStr))
                    {
                        break;
                    }

                    pics.Push(picStr);
                    idx++;
                }

                if(pics.Length() > 0)
                {
                    animPics[i].textures =
                        (kexTexture**)Mem_Calloc(sizeof(kexTexture*) * (pics.Length()+1), hb_world);

                    animPics[i].numFrames = (uint16_t)pics.Length() + 1;
                    animPics[i].textures[0] = textures[i];

                    for(uint j = 0; j < pics.Length(); ++j)
                    {
                        animPics[i].textures[j+1] =
                            kexRender::cTextures->Cache(pics[j].c_str(), TC_REPEAT, TF_NEAREST);
                    }
                }
            }
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

    for(unsigned int i = 0; i < numFaces; ++i)
    {
        mapFace_t *f = &faces[i];

        f->sectorOwner = -1;
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
        sectors[i].linkedSector     = -1;
        sectors[i].floodCount       = 0;
        sectors[i].objectThinker    = NULL;
        sectors[i].ceilingFace      = &faces[sectors[i].faceEnd+1];
        sectors[i].floorFace        = &faces[sectors[i].faceEnd+2];

        sectors[i].actorList.Reset();
        sectors[i].bufferIndex.Init();
        
        memset(&sectors[i].portalBuffer, 0, sizeof(bufferIndex_t));
        sectors[i].portalBuffer.sector = -1;
        
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

        if(f->flags & (FF_WATER|FF_TOGGLE|FF_FORCEFIELD) || EventIsASwitch(f->tag))
        {
            f->flags |= FF_DYNAMIC;
        }
        
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

    for(unsigned int i = 0; i < numFaces; ++i)
    {
        if(faces[i].polyStart <= -1 || faces[i].polyEnd <= -1)
        {
            continue;
        }

        if(faces[i].sectorOwner <= -1)
        {
            continue;
        }

        for(int j = faces[i].polyStart; j <= faces[i].polyEnd; ++j)
        {
            polys[j].faceRef = i;
        }
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
        events[i].params    = mapfile.Read16();
        
        if(events[i].sector >= 0)
        {
            float height;
            mapSector_t *s = &sectors[events[i].sector];

            s->event = i;

            switch(events[i].type)
            {
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
                MakeSectorDynamic(s, true);
                break;

            case 8:
            case 9:
                MakeSectorDynamic(s, true);
                MoveSector(s, true, (float)(s->ceilingHeight - s->floorHeight));
                break;

            case 21:
            case 22:
                MakeSectorDynamic(s, false);
                height = GetHighestSurroundingFloor(s);
                MoveSector(s, false, -(s->floorFace->plane.d - height));
                break;

            case 23:
                height = (float)s->floorHeight + 16;
                MakeSectorDynamic(s, false);
                MoveSector(s, false, -(s->floorFace->plane.d - height));
                break;

            case 24:
                height = GetLowestSurroundingFloor(s);
                MakeSectorDynamic(s, false);
                MoveSector(s, false, -(s->floorFace->plane.d - height));
                break;

            case 25:
                MakeSectorDynamic(s, false);
                kexGame::cActorFactory->SpawnMover("kexFloor", events[i].type, events[i].sector);
                break;

            case 41:
            case 44:
            case 45:
            case 60:
            case 65:
            case 68:
                SetupFloatingPlatforms(&events[i], s, "kexFloatingPlatform");
                break;

            case 48:
                SetupFloatingPlatforms(&events[i], s, "kexDropPad");
                break;

            default:
                break;
            }
        }
    }

    for(unsigned int i = 0; i < numFaces; ++i)
    {
        if(EventIsASwitch(faces[i].tag))
        {
            faces[i].flags |= FF_DYNAMIC;
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
        actors[i].tag       = mapfile.Read16();
        actors[i].params1   = mapfile.Read16();
        actors[i].params2   = mapfile.Read16();
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

            if(face->flags & FF_SOLID)
            {
                continue;
            }
            
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
    
    if(mapActor->type <= -1)
    {
        return;
    }
    
    x   = (float)mapActor->x;
    y   = (float)mapActor->y;
    z   = (float)mapActor->z;
    an  = kexMath::Deg2Rad((360 - (float)mapActor->angle) + 90);
    
    actor = kexGame::cActorFactory->Spawn(mapActor->type, x, y, z, an, mapActor->sector);

    if(!actor)
    {
        return;
    }

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
// kexWorld::UpdateAnimPics
//

void kexWorld::UpdateAnimPics(void)
{
    for(uint i = 0; i < numTextures; ++i)
    {
        if(animPics[i].textures == NULL)
        {
            continue;
        }

        animPics[i].time += 0.5f;

        if(animPics[i].time >= animPics[i].speed)
        {
            animPics[i].time = 0;

            if(++animPics[i].frame >= animPics[i].numFrames)
            {
                animPics[i].frame = 0;
            }

            textures[i] = animPics[i].textures[animPics[i].frame];
        }
    }
}

//
// kexWorld::SetupFloatingPlatforms
//

void kexWorld::SetupFloatingPlatforms(mapEvent_t *ev, mapSector_t *sector, const char *className)
{
    mapSector_t *s;

    sector->event = -1;
    sectors[ev->params].event = ev - events;
    ev->sector = ev->params;
    ev->params = sector - sectors;

    s = &sectors[ev->sector];
    s->linkedSector = ev->params;
    s->objectThinker = kexGame::cActorFactory->SpawnMover(className, ev->type, ev->sector);
}

//
// kexWorld::EventIsASwitch
//

bool kexWorld::EventIsASwitch(const int eventID)
{
    if(eventID < 0 || eventID > (int)numEvents)
    {
        return false;
    }

    return (events[eventID].type == 200 ||
            events[eventID].type == 201 ||
            events[eventID].type == 202 ||
            events[eventID].type == 203);
}

//
// kexWorld::MakeSectorDynamic
//
// This should be called on the sector that needs to be a movable
//

void kexWorld::MakeSectorDynamic(mapSector_t *sector, const bool bCeiling)
{
    for(int i = sector->faceStart; i < sector->faceEnd+3; ++i)
    {
        mapFace_t *face = &faces[i];
        
        if(face->flags & FF_PORTAL && face->sector >= 0)
        {
            mapSector_t *s = &sectors[face->sector];
            
            for(int j = s->faceStart; j < s->faceEnd+3; ++j)
            {
                mapFace_t *f = &faces[j];

                if(f->tag == -1 || f->tag != sector->event || f->flags & FF_PORTAL)
                {
                    continue;
                }

                if(!(f->flags & FF_PORTAL))
                {
                    f->flags |= FF_DYNAMIC;
                }
            }
            
            if(i <= sector->faceEnd && !(face->flags & FF_PORTAL))
            {
                face->flags |= FF_DYNAMIC;
            }

            continue;
        }

        if(i != sector->faceEnd + (bCeiling ? 1 : 2))
        {
            continue;
        }

        face->flags |= FF_DYNAMIC;
    }
}

//
// kexWorld::MoveSector
//

void kexWorld::MoveSector(mapSector_t *sector, bool bCeiling, const float moveAmount)
{
    static kexStack<int> updatedFaces;
    updatedFaces.Reset();

    for(int i = sector->faceStart; i < sector->faceEnd+3; ++i)
    {
        mapFace_t *face = &faces[i];

        if(face->flags & FF_UPDATED)
        {
            continue;
        }
        
        if(face->flags & FF_PORTAL && face->sector >= 0)
        {
            mapSector_t *s = &sectors[face->sector];
            mapVertex_t *fv[4];
            
            fv[0] = &vertices[face->vertexStart+0];
            fv[1] = &vertices[face->vertexStart+1];
            fv[2] = &vertices[face->vertexStart+2];
            fv[3] = &vertices[face->vertexStart+3];
            
            for(int j = s->faceStart; j < s->faceEnd+3; ++j)
            {
                mapFace_t *f = &faces[j];

                if(f->flags & FF_PORTAL && f->sector == face->sectorOwner && j <= s->faceEnd)
                {
                    if(bCeiling)
                    {
                        vertices[f->vertexStart+0].origin.z += moveAmount;
                        vertices[f->vertexStart+1].origin.z += moveAmount;
                    }
                    else
                    {
                        vertices[f->vertexStart+2].origin.z += moveAmount;
                        vertices[f->vertexStart+3].origin.z += moveAmount;
                    }
                    
                    UpdateFacePlaneAndBounds(f);
                    continue;
                }
                
                if(f->tag == -1 || f->tag != sector->event || f->flags & FF_PORTAL)
                {
                    continue;
                }
                
                if(f->flags & FF_UPDATED)
                {
                    continue;
                }

                for(int k = f->vertStart; k <= f->vertEnd; ++k)
                {
                    mapVertex_t *v = &vertices[k];
                    v->origin.z += moveAmount;

                    if(f->flags & FF_DYNAMIC)
                    {
                        continue;
                    }

                    bufferUpdate_t *bufUpdate = kexRenderScene::bufferUpdateList.Get();

                    bufUpdate->index = k;
                    bufUpdate->newVec = v->origin;
                    bufUpdate->newColor[0] = v->rgba[0];
                    bufUpdate->newColor[1] = v->rgba[1];
                    bufUpdate->newColor[2] = v->rgba[2];
                    bufUpdate->newColor[3] = v->rgba[3];
                }
                
                vertices[f->vertexStart+0].origin.z += moveAmount;
                vertices[f->vertexStart+1].origin.z += moveAmount;
                vertices[f->vertexStart+2].origin.z += moveAmount;
                vertices[f->vertexStart+3].origin.z += moveAmount;
                
                UpdateFacePlaneAndBounds(f);
                f->flags |= FF_UPDATED;
                updatedFaces.Set(j);
            }
            
            if(i <= sector->faceEnd)
            {
                if(bCeiling)
                {
                    fv[0]->origin.z += moveAmount;
                    fv[1]->origin.z += moveAmount;
                }
                else
                {
                    fv[2]->origin.z += moveAmount;
                    fv[3]->origin.z += moveAmount;
                }
                
                UpdateFacePlaneAndBounds(face);
            }
            
            UpdateSectorBounds(s);
        }
        
        if(i != sector->faceEnd + (bCeiling ? 1 : 2))
        {
            continue;
        }

        if(face->flags & FF_UPDATED)
        {
            continue;
        }
        
        for(int j = face->vertStart; j <= face->vertEnd; ++j)
        {
            mapVertex_t *v = &vertices[j];
            v->origin.z += moveAmount;

            if(face->flags & FF_DYNAMIC)
            {
                continue;
            }

            bufferUpdate_t *bufUpdate = kexRenderScene::bufferUpdateList.Get();

            bufUpdate->index = j;
            bufUpdate->newVec = v->origin;
            bufUpdate->newColor[0] = v->rgba[0];
            bufUpdate->newColor[1] = v->rgba[1];
            bufUpdate->newColor[2] = v->rgba[2];
            bufUpdate->newColor[3] = v->rgba[3];
        }
        
        vertices[face->vertexStart+0].origin.z += moveAmount;
        vertices[face->vertexStart+1].origin.z += moveAmount;
        vertices[face->vertexStart+2].origin.z += moveAmount;
        vertices[face->vertexStart+3].origin.z += moveAmount;
        
        UpdateFacePlaneAndBounds(face);
        face->flags |= FF_UPDATED;
        updatedFaces.Set(i);
    }
    
    UpdateSectorBounds(sector);
    
    for(kexActor *actor = sector->actorList.Next();
        actor != NULL;
        actor = actor->SectorLink().Next())
    {
        if(actor->Origin().z <= actor->FloorHeight() || kexMath::Fabs(actor->Velocity().z) <= 0.001f)
        {
            float d = kexGame::cLocal->CModel()->GetFloorHeight(actor->Origin(), sector);
            float floorDist = actor->Origin().z - d;
            
            if(floorDist <= -(moveAmount*2))
            {
                actor->FloorHeight() = d;
                actor->Origin().z = actor->FloorHeight();
            }

            // dumb hack
            if(!actor->InstanceOf(&kexPuppet::info) && moveAmount > 0 && floorDist <= 0.1f)
            {
                actor->Velocity().z += (moveAmount * 0.5f);
            }
        }
    }

    for(uint i = 0; i < updatedFaces.CurrentLength(); ++i)
    {
        mapFace_t *face = &Faces()[updatedFaces[i]];
        face->flags &= ~FF_UPDATED;
    }
}

//
// kexWorld::GetHighestSurroundingFloor
//

float kexWorld::GetHighestSurroundingFloor(mapSector_t *sector)
{
    float height = (float)sector->floorHeight;

    for(int i = sector->faceStart; i < sector->faceEnd+3; ++i)
    {
        mapFace_t *face = &faces[i];
        
        if(face->flags & FF_PORTAL && face->sector >= 0)
        {
            mapSector_t *s = &sectors[face->sector];

            if((float)s->floorHeight > height)
            {
                height = (float)s->floorHeight;
            }
        }
    }

    if((int)height == sector->floorHeight)
    {
        height = faces[sector->faceEnd+2].plane.d;
    }

    return height;
}

//
// kexWorld::GetLowestSurroundingFloor
//

float kexWorld::GetLowestSurroundingFloor(mapSector_t *sector)
{
    float height = faces[sector->faceEnd+2].plane.d;

    for(int i = sector->faceStart; i < sector->faceEnd+3; ++i)
    {
        mapFace_t *face = &faces[i];
        
        if(face->flags & FF_PORTAL && face->sector >= 0)
        {
            mapSector_t *s = &sectors[face->sector];

            if((float)s->floorHeight < height)
            {
                height = (float)s->floorHeight;
            }
        }
    }

    if(kexMath::FCmp(height, faces[sector->faceEnd+2].plane.d))
    {
        height = (float)sector->floorHeight;
    }

    return height;
}

//
// kexWorld::UseSectorSpecial
//

void kexWorld::UseWallSpecial(kexPlayer *player, mapFace_t *face)
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
            if(ev->type != 7)
            {
                return;
            }
        }
    }

    switch(ev->type)
    {
    case 1:
        kexGame::cActorFactory->SpawnMover("kexDoor", ev->type, ev->sector);
        break;
    case 3:
    case 4:
    case 5:
    case 6:
        UseLockedDoor(player, ev);
        break;
    case 7:
        kexGame::cLocal->PlayLoop()->Print("$str_92");
        break;
    case 8:
    case 9:
        kexGame::cActorFactory->SpawnMover("kexDoor", ev->type, ev->sector);
        break;
    case 21:
        kexGame::cActorFactory->SpawnMover("kexLiftImmediate", ev->type, ev->sector);
        break;
    case 200:
    case 201:
    case 202:
    case 203:
        UseWallSwitch(player, face, ev);
        break;

    default:
        break;
    }
}

//
// kexWorld::UseLockedDoor
//

void kexWorld::UseLockedDoor(kexPlayer *player, mapEvent_t *ev)
{
    if(!player->CheckKey(ev->type - 3))
    {
        kexGame::cLocal->PlayLoop()->Print(kexStr::Format("$str_%03d", 116 + (ev->type - 3)));
        return;
    }

    kexGame::cActorFactory->SpawnMover("kexDoor", ev->type, ev->sector);
}

//
// kexWorld::UseWallSwitch
//

void kexWorld::UseWallSwitch(kexPlayer *player, mapFace_t *face, mapEvent_t *ev)
{
    int switchTex = (ev->params >> 8) & 0xff;
    bool bFound = false;

    for(int i = face->polyStart; i <= face->polyEnd; ++i)
    {
        mapPoly_t *poly = &polys[i];

        if(poly->texture == switchTex)
        {
            poly->texture = (ev->params & 0xff);
            bFound = true;
        }
    }

    if(!bFound)
    {
        return;
    }

    switch(ev->type)
    {
    case 202:
    case 203:
        player->Actor()->PlaySound("sounds/switch2.wav");
        break;

    default:
        player->Actor()->PlaySound("sounds/switch.wav");
        break;
    }
    
    SendRemoteTrigger(&sectors[ev->sector], ev);
}

//
// kexWorld::ResetWallSwitchFromTag
//

void kexWorld::ResetWallSwitchFromTag(const int tag)
{
    for(unsigned int i = 0; i < numEvents; ++i)
    {
        mapSector_t *sector;
        int switchTex;

        if(events[i].tag != tag)
        {
            continue;
        }

        if(!EventIsASwitch(i))
        {
            if(events[i].type == 23)
            {
                sector = &sectors[events[i].sector];

                if(sector->objectThinker != NULL)
                {
                    static_cast<kexFloor*>(sector->objectThinker)->Trigger();
                }
            }
            continue;
        }

        sector = &sectors[events[i].sector];
        switchTex = (events[i].params & 0xff);

        for(int j = sector->faceStart; j < sector->faceEnd+3; ++j)
        {
            mapFace_t *face = &faces[j];

            if(face->tag != i)
            {
                continue;
            }

            for(int k = face->polyStart; k <= face->polyEnd; ++k)
            {
                mapPoly_t *poly = &polys[k];

                if(poly->texture == switchTex)
                {
                    poly->texture = (events[i].params >> 8) & 0xff;
                }
            }
        }
    }
}

//
// kexWorld::FireRemoteEventFromTag
//

void kexWorld::FireRemoteEventFromTag(const int tag)
{
    for(unsigned int i = 0; i < numEvents; ++i)
    {
        mapEvent_t *ev = &events[i];

        if(ev->tag != tag)
        {
            continue;
        }

        if(ev->type == 70)
        {
            continue;
        }

        TriggerEvent(ev);
    }
}

//
// kexWorld::MoveScriptedSector
//

void kexWorld::MoveScriptedSector(const int tag, const float height,
                                  const float speed, const bool bCeiling)
{
    for(unsigned int i = 0; i < numEvents; ++i)
    {
        mapEvent_t *ev = &events[i];
        kexScriptedMover *mover;

        if(ev->tag != tag)
        {
            continue;
        }

        if(ev->type != 49)
        {
            continue;
        }

        // negative speeds indicate 'instant'
        if(speed <= -1)
        {
            MoveSector(&sectors[ev->sector], false^bCeiling, height);

            if(ev->params >= 0)
            {
                MoveSector(&sectors[ev->params], true^bCeiling, height);
            }

            // no moving thinker is needed
            continue;
        }

        mover = static_cast<kexScriptedMover*>(
            kexGame::cActorFactory->SpawnMover("kexScriptedMover", ev->type, ev->sector));

        mover->Start(height, speed, ev, bCeiling);
    }
}

//
// kexWorld::TriggerEvent
//

void kexWorld::TriggerEvent(mapEvent_t *ev)
{
    switch(ev->type)
    {
    case 2:
    case 7:
    case 8:
    case 9:
        kexGame::cActorFactory->SpawnMover("kexDoor", ev->type, ev->sector);
        break;
    case 21:
        kexGame::cActorFactory->SpawnMover("kexLiftImmediate", ev->type, ev->sector);
        break;
    case 22:
        kexGame::cActorFactory->SpawnMover("kexFloor", ev->type, ev->sector);
        break;
    case 24:
        kexGame::cActorFactory->SpawnMover("kexLiftImmediate", ev->type, ev->sector);
        break;
    case 25:
        static_cast<kexFloor*>(sectors[ev->sector].objectThinker)->Trigger();
        break;
    case 48:
        static_cast<kexDropPad*>(sectors[ev->sector].objectThinker)->Reset();
        break;

    default:
        break;
    }
}

//
// kexWorld::EnterSectorSpecial
//

void kexWorld::EnterSectorSpecial(kexActor *actor, mapSector_t *sector)
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

    if(ev->type != 50)
    {
        if(sector->floorFace->plane.Distance(actor->Origin()) > actor->StepHeight())
        {
            // must be on the ground
            return;
        }
    }

    switch(ev->type)
    {
    case 21:
        kexGame::cActorFactory->SpawnMover("kexLift", ev->type, ev->sector);
        break;
    case 23:
        actor->PlaySound("sounds/fplate.wav");
        ResetWallSwitchFromTag(ev->tag+1);
        kexGame::cActorFactory->SpawnMover("kexFloor", ev->type, ev->sector);
        break;
    case 24:
        kexGame::cActorFactory->SpawnMover("kexLift", ev->type, ev->sector);
        break;
    case 48:
        static_cast<kexDropPad*>(sector->objectThinker)->Start();
        break;
    case 50:
        SendRemoteTrigger(sector, ev);
        break;
    case 52:
        kexGame::cScriptManager->CallDelayedMapScript(ev->tag+1, actor, 0);
        sector->event = -1;
        break;
    case 53:
        ArtifactEvent(actor, ev);
        break;
    case 63:
        kexGame::cScriptManager->CallDelayedMapScript(ev->tag+1, actor, 0);
        break;
    case 71:
        TeleportEvent(actor, ev);
        break;

    default:
        break;
    }
}

//
// kexWorld::FireActorEventFromTag
//

void kexWorld::FireActorEventFromTag(const int tag)
{
    kexFireballFactory *fbFactory;
    float extraDelay = 0;

    for(unsigned int j = 0; j < numActors; ++j)
    {
        if(actors[j].sector < 0)
        {
            continue;
        }

        if(actors[j].tag == tag)
        {
            mapSector_t *sec = &sectors[actors[j].sector];

            if(sec->flags & SF_SPECIAL)
            {
                continue;
            }

            switch(actors[j].type)
            {
            case AT_FIREBALLSPAWNER:
            case AT_LASERSPAWNER:
                fbFactory = kexGame::cActorFactory->SpawnFireballFactory(&actors[j]);
                fbFactory->ExtraDelay() = extraDelay;
                extraDelay += 8;
                break;

            default:
                break;
            }
        }

        if(actors[j].tag+1 == tag)
        {
            mapSector_t *sec = &sectors[actors[j].sector];

            if(!(sec->flags & SF_SPECIAL) || sec->objectThinker == NULL)
            {
                continue;
            }

            sec->objectThinker->Remove();
        }
    }
}

//
// kexWorld::SendMapActorEvent
//

void kexWorld::SendMapActorEvent(mapSector_t *sector, mapEvent_t *ev)
{
    if(sector != NULL && (ev->sector < 0 || &sectors[ev->sector] != sector))
    {
        return;
    }

    FireActorEventFromTag(ev->tag);
}

//
// kexWorld::TeleportEvent
//

void kexWorld::TeleportEvent(kexActor *actor, mapEvent_t *event)
{
    kexVec3 org;
    mapFace_t *face;

    for(unsigned int i = 0; i < numEvents; ++i)
    {
        mapEvent_t *ev = &events[i];

        if(ev == event || ev->type != 72 || ev->tag != event->tag || ev->sector <= -1)
        {
            continue;
        }

        face = sectors[ev->sector].floorFace;
        org = (vertices[face->vertexStart+0].origin +
               vertices[face->vertexStart+1].origin +
               vertices[face->vertexStart+2].origin) / 3;

        actor->Origin() = org;
        actor->SetSector(&sectors[ev->sector]);

        // player hack
        if(actor->InstanceOf(&kexPuppet::info))
        {
            actor->PlaySound("sounds/teleport.wav");
            kexGame::cLocal->PlayLoop()->TeleportFlash();
        }

        break;
    }
}

//
// kexWorld::ArtifactEvent
//

void kexWorld::ArtifactEvent(kexActor *actor, mapEvent_t *event)
{
    mapActor_t *actorTemplate = NULL;
    kexVec3 org;
    kexPlayer *player;
    mapFace_t *face;
    int artiBit;

    if(!actor->InstanceOf(&kexPuppet::info))
    {
        return;
    }

    player = static_cast<kexPuppet*>(actor)->Owner();
    artiBit = event->tag-1;

    if(!(player->Artifacts() & BIT(artiBit)))
    {
        return;
    }

    for(unsigned int j = 0; j < numActors; ++j)
    {
        kexDict *def;

        if((def = kexGame::cLocal->ActorDefs().GetEntry(actors[j].type)))
        {
            int artifactBit;

            if(def->GetInt("type", artifactBit))
            {
                if(artifactBit == (artiBit))
                {
                    actorTemplate = &actors[j];
                    break;
                }
            }
        }
    }

    if(!actorTemplate)
    {
        return;
    }

    for(unsigned int i = 0; i < numEvents; ++i)
    {
        kexActor *obj;

        mapEvent_t *ev = &events[i];

        if(ev == event || ev->type != 54 || ev->tag != event->tag || ev->sector <= -1)
        {
            continue;
        }

        face = sectors[ev->sector].floorFace;
        org = (vertices[face->vertexStart+0].origin +
               vertices[face->vertexStart+1].origin +
               vertices[face->vertexStart+2].origin) / 3;

        player->Artifacts() &= ~BIT(artiBit);
        FireRemoteEventFromTag(1000 + artiBit);

        obj = kexGame::cLocal->SpawnActor(actorTemplate->type, org.x, org.y, org.z + 16, 0, ev->sector);

        obj->PlaySound("sounds/ding02.wav");
        kexGame::cLocal->SpawnDynamicLight(obj, 512, kexVec3(1, 1, 1), 32.0f);
    }
}

//
// kexWorld::SendRemoteTrigger
//

void kexWorld::SendRemoteTrigger(mapSector_t *sector, mapEvent_t *event)
{
    bool bClearEventRef = false;

    for(unsigned int i = 0; i < numEvents; ++i)
    {
        mapEvent_t *ev = &events[i];

        if(!EventIsASwitch(event - events) && event->type != 23 && ev->tag == (event->tag+1))
        {
            ResetWallSwitchFromTag(ev->tag);
        }

        SendMapActorEvent(sector, ev);

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
                if(event->type == 66)
                {
                    ev->tag = -1;
                    sector->event = -1;
                    ExplodeWallEvent(&sectors[ev->sector]);
                }

                continue;
            }

            TriggerEvent(ev);

            switch(ev->type)
            {
            case 7:
            case 8:
            case 9:
            case 21:
            case 22:
            case 24:
                bClearEventRef = true;
                break;
            default:
                break;
            }
        }
        else if(ev->tag+1 == event->tag)
        {
            mapSector_t *sec = &sectors[ev->sector];

            if(!(sec->flags & SF_SPECIAL) || sec->objectThinker == NULL)
            {
                continue;
            }

            switch(ev->type)
            {
            case 23:
                static_cast<kexFloor*>(sec->objectThinker)->Trigger();
                break;
            default:
                break;
            }
        }
    }

    if(bClearEventRef && sector && event->type != 23)
    {
        sector->event = -1;
    }
}

//
// kexWorld::ExplodeWall
//

void kexWorld::ExplodeWall(mapFace_t *face)
{
    kexActorFactory *af = kexGame::cActorFactory;
    kexVec3 org;

    face->flags &= ~(FF_SOLID|FF_TOGGLE);
    face->flags |= FF_PORTAL;

    for(int j = face->polyStart; j <= face->polyEnd; ++j)
    {
        mapPoly_t *poly = &polys[j];

        org = (vertices[face->vertStart + poly->indices[0]].origin +
               vertices[face->vertStart + poly->indices[1]].origin +
               vertices[face->vertStart + poly->indices[2]].origin) / 3;

        if(!(kexRand::Max(0xff) & 0x1))
        {
            org.x += 128.0f * kexRand::CFloat();
            org.y += 128.0f * kexRand::CFloat();
            org.z += 128.0f * kexRand::CFloat();

            af->Spawn(AT_EXPLODEPUFF, org.x, org.y, org.z, 0, face->sectorOwner);
        }
        else
        {
            kexActor *debris = af->Spawn(AT_DEBRIS, org.x, org.y, org.z, 0, face->sectorOwner);

            if(debris == NULL)
            {
                continue;
            }

            debris->Velocity() = (face->plane.Normal() * (16 * kexRand::Float()));
            debris->Velocity().z += (16 * kexRand::Float());
        }
    }

    face->polyStart = face->polyEnd = -1;
}

//
// kexWorld::ExplodeWallEvent
//

void kexWorld::ExplodeWallEvent(mapSector_t *sector)
{
    for(int i = sector->faceStart; i < sector->faceEnd+3; ++i)
    {
        mapFace_t *face = &faces[i];

        if(!(face->flags & FF_TOGGLE))
        {
            continue;
        }

        ExplodeWall(face);

        if(face->sector <= -1)
        {
            continue;
        }

        ExplodeWallEvent(&sectors[face->sector]);
    }

    if(sector->event >= 0 && events[sector->event].type == 66)
    {
        SendRemoteTrigger(sector, &events[sector->event]);
        sector->event = -1;
    }
}

//
// kexWorld::CheckActorsForRadialBlast
//

void kexWorld::CheckActorsForRadialBlast(mapSector_t *sector, kexActor *source, const kexVec3 &origin,
                                         const float radius, const int damage, bool bCanIgnite)
{
    kexVec3 end;
    float dist;
    float dmgAmount;
    
    for(kexActor *actor = sector->actorList.Next();
        actor != NULL;
        actor = actor->SectorLink().Next())
    {
        if(actor == source)
        {
            // don't check self
            continue;
        }
        
        if(!(actor->Flags() & AF_SHOOTABLE))
        {
            continue;
        }
        
        end = actor->Origin() + kexVec3(0, 0, actor->Height() * 0.5f);
        dist = origin.DistanceSq(end);
        
        if(dist > (radius * radius))
        {
            continue;
        }
        
        if(source->Sector() != sector)
        {
            if(!kexGame::cLocal->CModel()->Trace(source, source->Sector(), origin, end))
            {
                continue;
            }
            
            if(kexGame::cLocal->CModel()->ContactActor() != actor)
            {
                continue;
            }
        }
        
        dmgAmount = ((radius - kexMath::Sqrt(dist)) * (float)damage) / radius;

        if(dmgAmount > 0)
        {
            if(bCanIgnite && actor->InstanceOf(&kexAI::info))
            {
                static_cast<kexAI*>(actor)->Ignite(source);
            }
            
            actor->InflictDamage(source, (int)dmgAmount);
        }
    }
}

//
// kexWorld::RadialDamage
//

void kexWorld::RadialDamage(kexActor *source, const float radius, const int damage,
                            const bool bCanDestroyWalls)
{
    if(source->Sector() == NULL || radius <= 0)
    {
        return;
    }
    
    unsigned int scanCount = 0;
    kexVec3 start = source->Origin() + kexVec3(0, 0, source->Height() * 0.5f);
    kexBBox bounds;
    
    bounds.min.Set(-(radius*0.5f));
    bounds.max.Set( (radius*0.5f));
    bounds.min += start;
    bounds.max += start;
    
    scanSectors.Reset();
    scanSectors.Set(source->Sector());
    source->Sector()->floodCount = 1;
    
    memset(pvsMask, 0, pvsSize);
    MarkSectorInPVS(source->Sector() - sectors);
    
    CheckActorsForRadialBlast(source->Sector(), source, start, radius, damage, bCanDestroyWalls);
    
    do
    {
        mapSector_t *sector = scanSectors[scanCount++];
        bool bWallsDestroyed = false;
        
        if(sector->floodCount == 0)
        {
            continue;
        }
        
        sector->floodCount = 0;

        for(int i = sector->faceStart; i < sector->faceEnd+3; ++i)
        {
            mapFace_t *face = &faces[i];

            if(face->flags & FF_SOLID)
            {
                if(!(face->flags & FF_PORTAL))
                {
                    if( face->sector >= 0 && face->flags & FF_TOGGLE && bCanDestroyWalls &&
                        face->plane.Distance(start) < (radius * 0.5f))
                    {
                        mapSector_t *s = &sectors[face->sector];

                        ExplodeWall(face);

                        for(int j = s->faceStart; j < s->faceEnd+3; ++j)
                        {
                            mapFace_t *f = &faces[j];

                            if(!(f->flags & FF_TOGGLE))
                            {
                                continue;
                            }

                            if(f->sector != face->sectorOwner)
                            {
                                continue;
                            }

                            ExplodeWall(f);
                        }

                        bWallsDestroyed = true;
                    }
                }
                continue;
            }
            
            if(face->sector >= 0)
            {
                mapSector_t *next = &sectors[face->sector];
                
                if(SectorInPVS(face->sector))
                {
                    continue;
                }
                
                CheckActorsForRadialBlast(next, source, start, radius, damage, bCanDestroyWalls);
                MarkSectorInPVS(face->sector);
                
                if(bounds.IntersectingBox(next->bounds))
                {
                    next->floodCount = 1;
                    scanSectors.Set(next);
                }
            }
        }

        if(bCanDestroyWalls && bWallsDestroyed &&
            sector->event >= 0 && events[sector->event].type == 66)
        {
            SendRemoteTrigger(sector, &events[sector->event]);
            sector->event = -1;
        }
        
    } while(scanCount < scanSectors.CurrentLength());
}

//
// kexWorld::FloodFill
//

sectorList_t *kexWorld::FloodFill(const kexVec3 &start, mapSector_t *sector, const float maxDistance)
{
    unsigned int scanCount = 0;

    if(sector == NULL)
    {
        return NULL;
    }

    scanSectors.Reset();
    scanSectors.Set(sector);
    sector->floodCount = 1;

    memset(pvsMask, 0, pvsSize);
    MarkSectorInPVS(sector - sectors);

    do
    {
        mapSector_t *sec = scanSectors[scanCount++];
        
        if(sec->floodCount == 0)
        {
            continue;
        }
        
        sec->floodCount = 0;

        for(int i = sec->faceStart; i < sec->faceEnd+3; ++i)
        {
            mapFace_t *face = &faces[i];
            float d;

            if(!(face->flags & FF_PORTAL) || face->sector <= -1)
            {
                continue;
            }

            d = kexGame::cLocal->CModel()->PointOnFaceSide(start, face);

            if(d <= 0)
            {
                continue;
            }

            if(d >= maxDistance)
            {
                break;
            }

            mapSector_t *next = &sectors[face->sector];
                
            if(SectorInPVS(face->sector))
            {
                continue;
            }
            
            MarkSectorInPVS(face->sector);
            
            next->floodCount = 1;
            scanSectors.Set(next);
        }

    } while(scanCount < scanSectors.CurrentLength());

    return &scanSectors;
}

//
// kexWorld::ClearSectorPVS
//

void kexWorld::ClearSectorPVS(void)
{
    memset(pvsMask, 0, pvsSize);
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
