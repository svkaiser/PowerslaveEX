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
//      Conversion utilities
//

#include "kexlib.h"

typedef struct
{
    int16_t     u1;
    int16_t     u2;
    int16_t     ceilSlope;
    int16_t     floorSlope;
    int16_t     ceilingHeight;
    int16_t     floorHeight;
    int16_t     faceStart;
    int16_t     faceEnd;
    int16_t     lightLevel;
    int16_t     flags;
    int16_t     u3;
    int16_t     u4;
} saturnSector_t;

typedef struct
{
    int32_t     normal[3];
    int32_t     angle;
    int16_t     u1;
    int16_t     u2;
    uint16_t    flags;
    uint16_t    genTextureID;
    int16_t     polyStart;
    int16_t     polyEnd;
    uint16_t    vertexStart;
    uint16_t    vertexEnd;
    uint16_t    polyVert[4];
    int16_t     sector;
    uint16_t    lookup1;
    int16_t     u3;
    int16_t     u4;
} saturnFace_t;

typedef struct
{
    int16_t x;
    int16_t y;
    int16_t z;
    int16_t light;
} saturnVertex_t;

typedef struct
{
    int16_t indices[4];
    int8_t  texture;
    int8_t  u1;
} saturnPoly_t;

#ifdef _MSC_VER
#pragma pack(push, 1)
#endif

typedef struct
{
    int16_t wallptr;
    int16_t wallnum;
    int16_t ceilingpicnum;
    int16_t floorpicnum;
    int16_t ceilingheinum;
    int16_t floorheinum;
    int32_t ceilingz;
    int32_t floorz;
    int8_t ceilingshade;
    int8_t floorshade;
    int8_t ceilingxpanning;
    int8_t floorxpanning;
    int8_t ceilingypanning;
    int8_t floorypanning;
    int16_t ceilingstat;
    int16_t floorstat;
    int8_t ceilingpal;
    int8_t floorpal;
    int8_t visibility;
    int16_t lotag;
    int16_t hitag;
    int16_t extra;
} PACKED buildSector_t;

typedef struct
{
    int32_t x;
    int32_t y;
    int16_t point2;
    int16_t nextsector;
    int16_t nextwall;
    int16_t picnum;
    int16_t overpicnum;
    int8_t shade;
    int8_t pal;
    int16_t cstat;
    int8_t xrepeat;
    int8_t yrepeat;
    int8_t xpanning;
    int8_t ypanning;
    int16_t lotag;
    int16_t hitag;
    int16_t extra;
} PACKED buildWall_t;

#ifdef _MSC_VER
#pragma pack(pop)
#endif

class kexDataConvert
{
public:
    kexDataConvert(void);
    ~kexDataConvert(void);

    void                DumpSaturnLevelData(const char *file);
    void                DumpBuildLevelData(const char *file);
};

static kexDataConvert dataConvertLocal;

//
// dumpsaturnmap
//

COMMAND(dumpsaturnmap)
{
    int argc = kex::cCommands->GetArgc();

    if(argc != 2)
    {
        kex::cSystem->Printf("dumpsaturnmap <filename>\n");
        return;
    }

    dataConvertLocal.DumpSaturnLevelData(kex::cCommands->GetArgv(1));
}

//
// dumpbuildmap
//

COMMAND(dumpbuildmap)
{
    int argc = kex::cCommands->GetArgc();

    if(argc != 2)
    {
        kex::cSystem->Printf("dumpbuildmap <filename>\n");
        return;
    }

    dataConvertLocal.DumpBuildLevelData(kex::cCommands->GetArgv(1));
}

//
// kexDataConvert::kexDataConvert
//

kexDataConvert::kexDataConvert(void)
{
}

//
// kexDataConvert::~kexDataConvert
//

kexDataConvert::~kexDataConvert(void)
{
}

//
// kexDataConvert::DumpBuildLevelData
//

void kexDataConvert::DumpBuildLevelData(const char *file)
{
    kexBinFile mapFile;
    kexStr fPath;
    buildSector_t *sectors;
    buildWall_t *walls;
    int numSectors;
    int numWalls;
    int numVerts;
    FILE *f;

    fPath = kexStr::Format("%s\\%s", kex::cvarBasePath.GetValue(), file);
    fPath.NormalizeSlashes();

    kex::cSystem->Printf("Reading %s\n", file);

    if(!mapFile.Open(file))
    {
        kex::cSystem->Warning("File not found\n");
        return;
    }

    mapFile.SetPosition(20);

    numSectors = mapFile.Read16();
    sectors = new buildSector_t[numSectors];

    for(int i = 0; i < numSectors; ++i)
    {
        sectors[i].wallptr = mapFile.Read16();
        sectors[i].wallnum = mapFile.Read16();
        sectors[i].ceilingpicnum = mapFile.Read16();
        sectors[i].floorpicnum = mapFile.Read16();
        sectors[i].ceilingheinum = mapFile.Read16();
        sectors[i].floorheinum = mapFile.Read16();
        sectors[i].ceilingz = mapFile.Read32();
        sectors[i].floorz = mapFile.Read32();
        sectors[i].ceilingshade = mapFile.Read8();
        sectors[i].floorshade = mapFile.Read8();
        sectors[i].ceilingxpanning = mapFile.Read8();
        sectors[i].floorxpanning = mapFile.Read8();
        sectors[i].ceilingypanning = mapFile.Read8();
        sectors[i].floorypanning = mapFile.Read8();
        sectors[i].ceilingstat = mapFile.Read8();
        sectors[i].floorstat = mapFile.Read8();
        sectors[i].ceilingpal = mapFile.Read8();
        sectors[i].floorpal = mapFile.Read8();
        sectors[i].visibility = mapFile.Read8();
        sectors[i].lotag = mapFile.Read16();
        sectors[i].hitag = mapFile.Read16();
        sectors[i].extra = mapFile.Read16();
    }

    numWalls = mapFile.Read16();
    walls = new buildWall_t[numWalls];

    for(int i = 0; i < numWalls; ++i)
    {
        walls[i].x = mapFile.Read32();
        walls[i].y = mapFile.Read32();
        walls[i].point2 = mapFile.Read16();
        walls[i].nextsector = mapFile.Read16();
        walls[i].nextwall = mapFile.Read16();
        walls[i].picnum = mapFile.Read16();
        walls[i].overpicnum = mapFile.Read16();
        walls[i].shade = mapFile.Read8();
        walls[i].pal = mapFile.Read8();
        walls[i].cstat = mapFile.Read16();
        walls[i].xrepeat = mapFile.Read8();
        walls[i].yrepeat = mapFile.Read8();
        walls[i].xpanning = mapFile.Read8();
        walls[i].ypanning = mapFile.Read8();
        walls[i].lotag = mapFile.Read16();
        walls[i].hitag = mapFile.Read16();
        walls[i].extra = mapFile.Read16();
    }

    f = fopen(kexStr(fPath + "_BUILD.obj").c_str(), "w");
    numVerts = 0;

    for(int i = 0; i < numSectors; ++i)
    {
        fprintf(f, "o sector_%03d\n", i);

        for(int j = sectors[i].wallptr; j < sectors[i].wallptr + sectors[i].wallnum; ++j)
        {
            buildWall_t *wall = &walls[j];

            fprintf(f, "v %f %f %f\n", (float)wall->x / 1024.0f,
                                       -(float)sectors[i].ceilingz / 16384.0f,
                                       (float)wall->y / 1024.0f);
            fprintf(f, "v %f %f %f\n", (float)walls[wall->point2].x / 1024.0f,
                                       -(float)sectors[i].ceilingz / 16384.0f,
                                       (float)walls[wall->point2].y / 1024.0f);
            fprintf(f, "v %f %f %f\n", (float)walls[wall->point2].x / 1024.0f,
                                       -(float)sectors[i].floorz / 16384.0f,
                                       (float)walls[wall->point2].y / 1024.0f);
            fprintf(f, "v %f %f %f\n", (float)wall->x / 1024.0f,
                                       -(float)sectors[i].floorz / 16384.0f,
                                       (float)wall->y / 1024.0f);

            fprintf(f, "f %i %i %i %i\n", numVerts+0+1, numVerts+1+1, numVerts+2+1, numVerts+3+1);
            numVerts += 4;
        }
    }

    fclose(f);

    mapFile.Close();

    delete[] sectors;
    delete[] walls;

    kex::cSystem->Printf("Done\n");
}

//
// kexDataConvert::DumpSaturnLevelData
//

void kexDataConvert::DumpSaturnLevelData(const char *file)
{
    kexBinFile mapFile;
    int numSectors, numFaces, numVertices, numPolys, numGenTextures;
    int numUnknown[9];
    saturnSector_t *sectors;
    saturnFace_t *faces;
    saturnVertex_t *vertices;
    saturnPoly_t *polys;
    kexStr fPath;
    int16_t *lookupTable[2];
    int missingFaces = 0;
    FILE *f;

    fPath = kexStr::Format("%s\\%s", kex::cvarBasePath.GetValue(), file);
    fPath.NormalizeSlashes();

    kex::cSystem->Printf("Reading %s\n", file);

    if(!mapFile.Open(file))
    {
        kex::cSystem->Warning("File not found\n");
        return;
    }

    // skip sky data and unknown lookup table
    // sky texture is always 256x320
    // lookup table always contains 321 entries (4 bytes each)
    mapFile.SetPosition(132876);

    numSectors      = kex::cEndian->SwapBE32(mapFile.Read32());
    numFaces        = kex::cEndian->SwapBE32(mapFile.Read32());
    numVertices     = kex::cEndian->SwapBE32(mapFile.Read32());
    numPolys        = kex::cEndian->SwapBE32(mapFile.Read32());
    numGenTextures  = kex::cEndian->SwapBE32(mapFile.Read32());
    numUnknown[0]   = kex::cEndian->SwapBE32(mapFile.Read32());
    numUnknown[1]   = kex::cEndian->SwapBE32(mapFile.Read32());
    numUnknown[2]   = kex::cEndian->SwapBE32(mapFile.Read32());
    numUnknown[3]   = kex::cEndian->SwapBE32(mapFile.Read32());
    numUnknown[4]   = kex::cEndian->SwapBE32(mapFile.Read32());
    numUnknown[5]   = kex::cEndian->SwapBE32(mapFile.Read32());
    numUnknown[6]   = kex::cEndian->SwapBE32(mapFile.Read32());
    numUnknown[7]   = kex::cEndian->SwapBE32(mapFile.Read32());
    numUnknown[8]   = kex::cEndian->SwapBE32(mapFile.Read32());

    sectors         = new saturnSector_t[numSectors];
    faces           = new saturnFace_t[numFaces];
    vertices        = new saturnVertex_t[numVertices];
    polys           = new saturnPoly_t[numPolys];
    lookupTable[0]  = new int16_t[numUnknown[1]];
    lookupTable[1]  = new int16_t[numUnknown[1]];

    for(int i = 0; i < numSectors; ++i)
    {
        sectors[i].u1               = kex::cEndian->SwapBE16(mapFile.Read16());
        sectors[i].u2               = kex::cEndian->SwapBE16(mapFile.Read16());
        sectors[i].ceilSlope        = kex::cEndian->SwapBE16(mapFile.Read16());
        sectors[i].floorSlope       = kex::cEndian->SwapBE16(mapFile.Read16());
        sectors[i].ceilingHeight    = kex::cEndian->SwapBE16(mapFile.Read16());
        sectors[i].floorHeight      = kex::cEndian->SwapBE16(mapFile.Read16());
        sectors[i].faceStart        = kex::cEndian->SwapBE16(mapFile.Read16());
        sectors[i].faceEnd          = kex::cEndian->SwapBE16(mapFile.Read16());
        sectors[i].lightLevel       = kex::cEndian->SwapBE16(mapFile.Read16());
        sectors[i].flags            = kex::cEndian->SwapBE16(mapFile.Read16());
        sectors[i].u3               = kex::cEndian->SwapBE16(mapFile.Read16());
        sectors[i].u4               = kex::cEndian->SwapBE16(mapFile.Read16());
    }

    for(int i = 0; i < numFaces; ++i)
    {
        faces[i].normal[0]          = kex::cEndian->SwapBE32(mapFile.Read32());
        faces[i].normal[1]          = kex::cEndian->SwapBE32(mapFile.Read32());
        faces[i].normal[2]          = kex::cEndian->SwapBE32(mapFile.Read32());
        faces[i].angle              = kex::cEndian->SwapBE32(mapFile.Read32());
        faces[i].u1                 = kex::cEndian->SwapBE16(mapFile.Read16());
        faces[i].u2                 = kex::cEndian->SwapBE16(mapFile.Read16());
        faces[i].flags              = kex::cEndian->SwapBE16(mapFile.Read16());
        faces[i].genTextureID       = kex::cEndian->SwapBE16(mapFile.Read16());
        faces[i].polyStart          = kex::cEndian->SwapBE16(mapFile.Read16());
        faces[i].polyEnd            = kex::cEndian->SwapBE16(mapFile.Read16());
        faces[i].vertexStart        = kex::cEndian->SwapBE16(mapFile.Read16());
        faces[i].vertexEnd          = kex::cEndian->SwapBE16(mapFile.Read16());
        faces[i].polyVert[0]        = kex::cEndian->SwapBE16(mapFile.Read16());
        faces[i].polyVert[1]        = kex::cEndian->SwapBE16(mapFile.Read16());
        faces[i].polyVert[2]        = kex::cEndian->SwapBE16(mapFile.Read16());
        faces[i].polyVert[3]        = kex::cEndian->SwapBE16(mapFile.Read16());
        faces[i].sector             = kex::cEndian->SwapBE16(mapFile.Read16());
        faces[i].lookup1            = kex::cEndian->SwapBE16(mapFile.Read16());
        faces[i].u3                 = kex::cEndian->SwapBE16(mapFile.Read16());
        faces[i].u4                 = kex::cEndian->SwapBE16(mapFile.Read16());
    }

    for(int i = 0; i < numVertices; ++i)
    {
        vertices[i].x               = kex::cEndian->SwapBE16(mapFile.Read16());
        vertices[i].y               = kex::cEndian->SwapBE16(mapFile.Read16());
        vertices[i].z               = kex::cEndian->SwapBE16(mapFile.Read16());
        vertices[i].light           = kex::cEndian->SwapBE16(mapFile.Read16());
    }

    for(int i = 0; i < numPolys; ++i)
    {
        polys[i].indices[0]         = kex::cEndian->SwapBE16(mapFile.Read16());
        polys[i].indices[1]         = kex::cEndian->SwapBE16(mapFile.Read16());
        polys[i].indices[2]         = kex::cEndian->SwapBE16(mapFile.Read16());
        polys[i].indices[3]         = kex::cEndian->SwapBE16(mapFile.Read16());
        polys[i].texture            = mapFile.Read8();
        polys[i].u1                 = mapFile.Read8();
    }

    fPath.StripFile();
    fPath += kexStr(file).StripExtension().StripPath();

    f = fopen(kexStr(fPath + "_sectors.txt").c_str(), "w");

    for(int i = 0; i < numSectors; ++i)
    {
        fprintf(f, "%04d: ", i);
        fprintf(f, "%8i ", sectors[i].u1);
        fprintf(f, "%8i ", sectors[i].u2);
        fprintf(f, "%8i ", sectors[i].ceilSlope);
        fprintf(f, "%8i ", sectors[i].floorSlope);
        fprintf(f, "%8i ", sectors[i].ceilingHeight);
        fprintf(f, "%8i ", sectors[i].floorHeight);
        fprintf(f, "%8i ", sectors[i].faceStart);
        fprintf(f, "%8i ", sectors[i].faceEnd);
        fprintf(f, "%8i ", sectors[i].lightLevel);
        fprintf(f, "%8i ", sectors[i].flags);
        fprintf(f, "%8i ", sectors[i].u3);
        fprintf(f, "%8i ", sectors[i].u4);
        fprintf(f, "\n");
    }

    fclose(f);

    f = fopen(kexStr(fPath + "_faces.txt").c_str(), "w");

    for(int i = 0; i < numFaces; ++i)
    {
        fprintf(f, "%04d: ", i);
        fprintf(f, "%12f ", (float)faces[i].normal[0] / 65536.0f);
        fprintf(f, "%12f ", (float)faces[i].normal[2] / 65536.0f);
        fprintf(f, "%12f ", (float)faces[i].normal[1] / 65536.0f);

        fprintf(f, "%12i ", faces[i].angle >> 16);
        fprintf(f, "%8i ", faces[i].u1);
        fprintf(f, "%8i ", faces[i].u2);
        fprintf(f, "%8i ", faces[i].flags);
        fprintf(f, "%8i ", faces[i].genTextureID);
        fprintf(f, "%8i ", faces[i].polyStart);
        fprintf(f, "%8i ", faces[i].polyEnd);
        fprintf(f, "%8i ", faces[i].vertexStart);
        fprintf(f, "%8i ", faces[i].vertexEnd);
        fprintf(f, "%8i ", faces[i].polyVert[0]);
        fprintf(f, "%8i ", faces[i].polyVert[1]);
        fprintf(f, "%8i ", faces[i].polyVert[2]);
        fprintf(f, "%8i ", faces[i].polyVert[3]);
        fprintf(f, "%8i ", faces[i].sector);
        fprintf(f, "%8i ", faces[i].lookup1);
        fprintf(f, "%8i ", faces[i].u3);
        fprintf(f, "%8i ", faces[i].u4);
        fprintf(f, "\n");
    }

    fclose(f);

    f = fopen(kexStr(fPath + "_vertices.txt").c_str(), "w");

    for(int i = 0; i < numVertices; ++i)
    {
        fprintf(f, "%04d: ", i);
        fprintf(f, "%8i ", vertices[i].x);
        fprintf(f, "%8i ", vertices[i].y);
        fprintf(f, "%8i ", vertices[i].z);
        fprintf(f, "%8i ", vertices[i].light);
        fprintf(f, "\n");
    }

    fclose(f);

    f = fopen(kexStr(fPath + "_polys.txt").c_str(), "w");

    for(int i = 0; i < numPolys; ++i)
    {
        fprintf(f, "%04d: ", i);
        fprintf(f, "%8i ", polys[i].indices[0]);
        fprintf(f, "%8i ", polys[i].indices[1]);
        fprintf(f, "%8i ", polys[i].indices[2]);
        fprintf(f, "%8i ", polys[i].indices[3]);
        fprintf(f, "%8i ", polys[i].texture);
        fprintf(f, "%8i ", polys[i].u1);
        fprintf(f, "\n");
    }

    fclose(f);

    f = fopen(kexStr(fPath + "_unknown1.txt").c_str(), "w");

    for(int i = 0; i < numUnknown[1]; ++i)
    {
        lookupTable[0][i] = kex::cEndian->SwapBE16(mapFile.Read16());
        lookupTable[1][i] = kex::cEndian->SwapBE16(mapFile.Read16());

        fprintf(f, "%04d: ", i);
        fprintf(f, "%8i ", lookupTable[0][i]);
        fprintf(f, "%8i ", lookupTable[1][i]);
        fprintf(f, "\n");
    }

    fclose(f);

    f = fopen(kexStr(fPath + "_unknown3.txt").c_str(), "w");

    for(int i = 0; i < numUnknown[3]; ++i)
    {
        fprintf(f, "%04d: ", i);
        fprintf(f, "%6i ", kex::cEndian->SwapBE16(mapFile.Read16()));
        fprintf(f, "%6i ", kex::cEndian->SwapBE16(mapFile.Read16()));
        fprintf(f, "%6i ", kex::cEndian->SwapBE16(mapFile.Read16()));
        fprintf(f, "%6i ", kex::cEndian->SwapBE16(mapFile.Read16()));
        fprintf(f, "%6i ", kex::cEndian->SwapBE16(mapFile.Read16()));
        fprintf(f, "%6i ", kex::cEndian->SwapBE16(mapFile.Read16()));
        fprintf(f, "%6i ", kex::cEndian->SwapBE16(mapFile.Read16()));
        fprintf(f, "%6i ", kex::cEndian->SwapBE16(mapFile.Read16()));
        fprintf(f, "%6i ", kex::cEndian->SwapBE16(mapFile.Read16()));
        fprintf(f, "\n");
    }

    fclose(f);

    f = fopen(kexStr(fPath + "_unknown5.txt").c_str(), "w");

    for(int i = 0; i < numUnknown[5]; ++i)
    {
        fprintf(f, "%04d: ", i);
        fprintf(f, "%6i ", kex::cEndian->SwapBE16(mapFile.Read16()));
        fprintf(f, "%6i ", kex::cEndian->SwapBE16(mapFile.Read16()));
        fprintf(f, "\n");
    }

    fclose(f);

    f = fopen(kexStr(fPath + "_unknown4.txt").c_str(), "w");

    for(int i = 0; i < numUnknown[4]; ++i)
    {
        fprintf(f, "%04d: ", i);
        fprintf(f, "%6i ", kex::cEndian->SwapBE16(mapFile.Read16()));
        fprintf(f, "\n");
    }

    fclose(f);

    f = fopen(kexStr(fPath + "_unknown1_lookup.txt").c_str(), "w");

    for(int i = 0; i < numUnknown[1]; ++i)
    {
        fprintf(f, "%04d: ", i);
        if(i == numUnknown[1]-1)
        {
            for(int j = 0; j < numUnknown[2] / 2 - (lookupTable[1][i] / 2); ++j)
            {
                fprintf(f, "%6i ", kex::cEndian->SwapBE16(mapFile.Read16()));
            }
        }
        else
        {
            for(int j = 0; j < (lookupTable[1][i+1] / 2) - (lookupTable[1][i] / 2); ++j)
            {
                fprintf(f, "%6i ", kex::cEndian->SwapBE16(mapFile.Read16()));
            }
        }
        fprintf(f, "\n");
    }

    fclose(f);

    f = fopen(kexStr(fPath + "_generatedPolyTextures.txt").c_str(), "w");

    for(int i = 0; i < numFaces; ++i)
    {
        saturnFace_t *face = &faces[i];
        int count = 0;

        if(!(face->flags & 1))
        {
            continue;
        }

        fprintf(f, "%04d: ", i);

        for(int j = i+1; j < numFaces; ++j)
        {
            saturnFace_t *f = &faces[j];

            if(!(f->flags & 1))
            {
                continue;
            }

            count = f->genTextureID - face->genTextureID;
            break;
        }

        for(int j = 0; j < count/2; ++j)
        {
            int16_t val = kex::cEndian->SwapBE16(mapFile.Read16());

            fprintf(f, "%6i/%i ", val & 0xff, (val >> 8) & 0xff);
        }

        fprintf(f, "\n");
    }

    fclose(f);

    f = fopen(kexStr(fPath + "_generatedLightVertex.txt").c_str(), "w");

    for(int i = 0; i < numFaces; ++i)
    {
        saturnFace_t *face = &faces[i];
        int count = 0;

        if(!(face->flags & 1))
        {
            continue;
        }

        fprintf(f, "%04d: ", i);

        for(int j = i+1; j < numFaces; ++j)
        {
            saturnFace_t *f = &faces[j];

            if(!(f->flags & 1))
            {
                continue;
            }

            count = f->lookup1 - face->lookup1;
            break;
        }

        for(int j = 0; j < count; ++j)
        {
            fprintf(f, "%6i ", mapFile.Read8() << 8);
        }

        fprintf(f, "\n");
    }

    fclose(f);

    f = fopen(kexStr(fPath + "_SAT.obj").c_str(), "w");

    for(int i = 0; i < numVertices; ++i)
    {
        float x = (float)vertices[i].x / 256.0f;
        float y = (float)vertices[i].y / 256.0f;
        float z = (float)vertices[i].z / 256.0f;

        fprintf(f, "v %f %f %f\n", x, y, -z);
    }

    for(int i = 0; i < numSectors; ++i)
    {
        //fprintf(f, "o sector_%03d\n", i);
        for(int j = sectors[i].faceStart; j <= sectors[i].faceEnd; ++j)
        {
            saturnFace_t *face = &faces[j];
            int indices[4] = { 0, 0, 0, 0 };
            int idx = 0;

            if(face->sector != -1)
            {
                continue;
            }

            for(int k = 0; k < 4; ++k)
            {
                int x1 = vertices[face->polyVert[k]].x;
                int y1 = vertices[face->polyVert[k]].y;
                int z1 = vertices[face->polyVert[k]].z;
                int x2 = vertices[face->polyVert[(k+1)%4]].x;
                int y2 = vertices[face->polyVert[(k+1)%4]].y;
                int z2 = vertices[face->polyVert[(k+1)%4]].z;

                if(x1 == x2 && y1 == y2 && z1 == z2)
                {
                    continue;
                }

                indices[idx++] = face->polyVert[k];
            }

            if(idx <= 1)
            {
                continue;
            }

            fprintf(f, "o face_%03d_%03d\n", i, j);
            fprintf(f, "f ");

            for(int k = 0; k < idx; ++k)
            {
                fprintf(f, "%i ", indices[k]+1);
            }
            fprintf(f, "\n");

            if(face->polyStart <= -1 || face->polyEnd <= -1)
            {
                missingFaces++;
                /*fprintf(f, "o face_%03d_%03d\n", i, j);
                fprintf(f, "f %i %i %i %i\n", face->polyVert[0]+1,
                                              face->polyVert[1]+1,
                                              face->polyVert[2]+1,
                                              face->polyVert[3]+1);*/
                continue;
            }

            /*fprintf(f, "o face_%03d_%03d\n", i, j);

            for(int k = face->polyStart; k <= face->polyEnd; ++k)
            {
                saturnPoly_t *poly = &polys[k];
                int indices[4];

                indices[0] = face->vertexStart + poly->indices[0];
                indices[1] = face->vertexStart + poly->indices[1];
                indices[2] = face->vertexStart + poly->indices[2];
                indices[3] = face->vertexStart + poly->indices[3];

                fprintf(f, "f %i %i %i %i\n", indices[0]+1, indices[1]+1, indices[2]+1, indices[3]+1);
            }*/
        }
    }

    fclose(f);

    delete[] sectors;
    delete[] faces;
    delete[] vertices;
    delete[] polys;
    delete[] lookupTable[0];
    delete[] lookupTable[1];

    mapFile.Close();

    kex::cSystem->Printf("Done (%i missing faces)\n", missingFaces);
}
