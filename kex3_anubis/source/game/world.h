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

typedef struct
{
    short       x;
    short       y;
    short       z;
    byte        rgba[4];
} mapVertex_t;

typedef struct
{
    word        faceStart;
    word        faceEnd;
    word        lightLevel;
    short       ceilingHeight;
    short       floorHeight;
    float       ceilingSlope;
    float       floorSlope;
    word        flags;
} mapSector_t;

typedef struct
{
    short       polyStart;
    short       polyEnd;
    short       sector;
    float       angle;
    kexPlane    plane;
    word        flags;
    short       tag;
    word        vertStart;
    word        vertEnd;
} mapFace_t;

typedef struct
{
    byte        indices[4];
    short       texture;
    short       flipped;
    word        tcoord;
} mapPoly_t;

typedef struct
{
    float s;
    float t;
} mapUV_t;

typedef struct
{
    mapUV_t     uv[4];
} mapTexCoords_t;

typedef struct
{
    short       type;
    short       sector;
    short       tag;
    short       unknown;
} mapEvent_t;

typedef struct
{
    short       type;
    short       sector;
    short       x;
    short       y;
    short       z;
    float       angle;
} mapActor_t;

class kexWorld
{
public:
    kexWorld(void);
    ~kexWorld(void);

    bool                    LoadMap(const char *mapname);

    const bool              MapLoaded(void) const { return bMapLoaded; }

    const unsigned int      NumVertices(void) const { return numVertices; }
    const unsigned int      NumSectors(void) const { return numSectors; }
    const unsigned int      NumFaces(void) const { return numFaces; }
    const unsigned int      NumPolys(void) const { return numPolys; }
    const unsigned int      NumTexCoords(void) const { return numTCoords; }
    const unsigned int      NumEvents(void) const { return numEvents; }
    const unsigned int      NumActors(void) const { return numActors; }

    mapVertex_t             *Vertices(void) { return vertices; }
    mapSector_t             *Sectors(void) { return sectors; }
    mapFace_t               *Faces(void) { return faces; }
    mapPoly_t               *Polys(void) { return polys; }
    mapTexCoords_t          *TexCoords(void) { return texCoords; }
    mapEvent_t              *Events(void) { return events; }
    mapActor_t              *Actors(void) { return actors; }

    static kexHeapBlock     hb_world;

private:
    void                    ReadVertices(kexBinFile &mapfile, const unsigned int count);
    void                    ReadSectors(kexBinFile &mapfile, const unsigned int count);
    void                    ReadFaces(kexBinFile &mapfile, const unsigned int count);
    void                    ReadPolys(kexBinFile &mapfile, const unsigned int count);
    void                    ReadTexCoords(kexBinFile &mapfile, const unsigned int count);
    void                    ReadEvents(kexBinFile &mapfile, const unsigned int count);
    void                    ReadActors(kexBinFile &mapfile, const unsigned int count);

    bool                    bMapLoaded;

    unsigned int            numVertices;
    unsigned int            numSectors;
    unsigned int            numFaces;
    unsigned int            numPolys;
    unsigned int            numTCoords;
    unsigned int            numEvents;
    unsigned int            numActors;

    mapVertex_t             *vertices;
    mapSector_t             *sectors;
    mapFace_t               *faces;
    mapPoly_t               *polys;
    mapTexCoords_t          *texCoords;
    mapEvent_t              *events;
    mapActor_t              *actors;
};

#endif
