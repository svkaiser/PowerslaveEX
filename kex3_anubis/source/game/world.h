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

#ifndef __WORLD_H__
#define __WORLD_H__

#include "sdNodes.h"
#include "clipper.h"

class kexActor;
class kexTexture;
class kexCModel;

typedef struct
{
    kexVec3             origin;
    byte                rgba[4];
} mapVertex_t;

typedef enum
{
    SF_DEBUG            = BIT(0),
    SF_WATER            = BIT(8)
} sectorFlags_t;

typedef struct
{
    word                faceStart;
    word                faceEnd;
    word                lightLevel;
    short               ceilingHeight;
    short               floorHeight;
    float               ceilingSlope;
    float               floorSlope;
    word                flags;
    kexBBox             bounds;
    int                 validcount;
    int                 floodCount;
    struct mapFace_s    *floorFace;
    struct mapFace_s    *ceilingFace;
} mapSector_t;

typedef enum
{
    FF_SOLID            = BIT(0),
    FF_UNKNOWN1         = BIT(1),
    FF_SECRET           = BIT(2),
    FF_UNKNOWN2         = BIT(3),
    FF_INVISIBLE        = BIT(4),
    FF_LAVA             = BIT(5),
    FF_WATER            = BIT(6),
    FF_SLIME            = BIT(7),
    FF_TOGGLE           = BIT(8),
    FF_FORCEFIELD       = BIT(9),
    FF_FULLBRIGHT       = BIT(10),
    FF_HIDDEN           = BIT(12),
    FF_PORTAL           = BIT(13),
    FF_UNDERWATER       = BIT(14),
} faceFlags_t;

typedef enum
{
    EGF_TOPSTEP         = BIT(0),
    EGF_BOTTOMSTEP      = BIT(1)
} edgeFlags_t;

typedef struct
{
    kexVec3             *v1;
    kexVec3             *v2;
    unsigned int        flags;
} mapEdge_t;

typedef struct mapFace_s
{
    short               polyStart;
    short               polyEnd;
    word                vertexStart;
    short               sector;
    float               angle;
    kexPlane            plane;
    word                flags;
    short               tag;
    word                vertStart;
    word                vertEnd;
    kexBBox             bounds;
    int                 validcount;
    int                 sectorOwner;
    float               leftSpan;
    float               rightSpan;
    float               topSpan;
    float               bottomSpan;
    mapEdge_t           edges[4];
    float               h[4];
    float               v[4];
    struct portal_s     *portal;
    
    mapEdge_t           *BottomEdge(void) { return &edges[2]; }
    mapEdge_t           *TopEdge(void) { return &edges[0]; }
    mapEdge_t           *RightEdge(void) { return &edges[1]; }
    mapEdge_t           *LeftEdge(void) { return &edges[3]; }
    
    bool                InFront(const kexVec3 &origin) { return (plane.Distance(origin) - plane.d) >= 0; };
} mapFace_t;

typedef struct portal_s
{
    mapFace_t           *face;
    mapSector_t         *sector;

    kexClipper          hClipSpan;
    kexClipper          vClipSpan[2];
} portal_t;

typedef struct
{
    byte                indices[4];
    short               texture;
    short               flipped;
    word                tcoord;
} mapPoly_t;

typedef struct
{
    float               s;
    float               t;
} mapUV_t;

typedef struct
{
    mapUV_t             uv[4];
} mapTexCoords_t;

typedef struct
{
    short               type;
    short               sector;
    short               tag;
    short               unknown;
} mapEvent_t;

typedef struct
{
    short               type;
    short               sector;
    short               x;
    short               y;
    short               z;
    float               angle;
} mapActor_t;

class kexWorld
{
public:
    kexWorld(void);
    ~kexWorld(void);

    bool                    LoadMap(const char *mapname);
    void                    UnloadMap(void);

    const bool              MapLoaded(void) const { return bMapLoaded; }

    const unsigned int      NumVertices(void) const { return numVertices; }
    const unsigned int      NumSectors(void) const { return numSectors; }
    const unsigned int      NumFaces(void) const { return numFaces; }
    const unsigned int      NumPolys(void) const { return numPolys; }
    const unsigned int      NumTexCoords(void) const { return numTCoords; }
    const unsigned int      NumEvents(void) const { return numEvents; }
    const unsigned int      NumActors(void) const { return numActors; }
    const unsigned int      NumPortals(void) const { return numPortals; }

    kexTexture              **Textures(void) { return textures; }
    mapVertex_t             *Vertices(void) { return vertices; }
    mapSector_t             *Sectors(void) { return sectors; }
    mapFace_t               *Faces(void) { return faces; }
    mapPoly_t               *Polys(void) { return polys; }
    mapTexCoords_t          *TexCoords(void) { return texCoords; }
    mapEvent_t              *Events(void) { return events; }
    mapActor_t              *Actors(void) { return actors; }
    portal_t                *Portals(void) { return portals; }

    kexSDNode<kexActor>     &AreaNodes(void) { return areaNodes; }

    static kexHeapBlock     hb_world;

private:
    void                    BuildAreaNodes(void);
    void                    BuildSectorBounds(void);
    void                    SetupEdges(void);
    void                    SpawnMapActor(mapActor_t *mapActor);
    void                    BuildPortals(unsigned int count);
    
    void                    ReadTextures(kexBinFile &mapfile, const unsigned int count);
    void                    ReadVertices(kexBinFile &mapfile, const unsigned int count);
    void                    ReadSectors(kexBinFile &mapfile, const unsigned int count);
    void                    ReadFaces(kexBinFile &mapfile, const unsigned int count);
    void                    ReadPolys(kexBinFile &mapfile, const unsigned int count);
    void                    ReadTexCoords(kexBinFile &mapfile, const unsigned int count);
    void                    ReadEvents(kexBinFile &mapfile, const unsigned int count);
    void                    ReadActors(kexBinFile &mapfile, const unsigned int count);

    bool                    bMapLoaded;

    unsigned int            numTextures;
    unsigned int            numVertices;
    unsigned int            numSectors;
    unsigned int            numFaces;
    unsigned int            numPolys;
    unsigned int            numTCoords;
    unsigned int            numEvents;
    unsigned int            numActors;
    unsigned int            numPortals;

    kexTexture              **textures;
    mapVertex_t             *vertices;
    mapSector_t             *sectors;
    mapFace_t               *faces;
    mapPoly_t               *polys;
    mapTexCoords_t          *texCoords;
    mapEvent_t              *events;
    mapActor_t              *actors;
    portal_t                *portals;

    kexSDNode<kexActor>     areaNodes;
};

#endif
