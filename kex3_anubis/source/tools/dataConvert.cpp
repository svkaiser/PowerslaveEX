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

extern kexCvar cvarBasePath;

typedef struct
{
    int16_t u1;
    int16_t u2;
    int16_t ceilSlope;
    int16_t floorSlope;
    int16_t ceilingHeight;
    int16_t floorHeight;
    int16_t faceStart;
    int16_t faceEnd;
    int16_t lightLevel;
    int16_t flags;
    int16_t u3;
    int16_t u4;
} saturnSector_t;

typedef struct
{
    int32_t normal[3];
    int32_t angle;
    int16_t u1;
    int16_t u2;
    uint16_t flags;
    int16_t texCoord;
    int16_t polyStart;
    int16_t polyEnd;
    int16_t vertexStart;
    int16_t vertexEnd;
    int16_t polyVert[4];
    int16_t sector;
    int16_t lookup1;
    int16_t u3;
    int16_t u4;
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

typedef struct
{
    int8_t  u[4];
    int8_t  v[4];
} saturnTexCoord_t;

class kexDataConvert
{
public:
    kexDataConvert(void);
    ~kexDataConvert(void);

    void                DumpSaturnLevelData(const char *file);
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
// kexDataConvert::DumpSaturnLevelData
//

void kexDataConvert::DumpSaturnLevelData(const char *file)
{
    kexBinFile mapFile;
    int numSectors, numFaces, numVertices, numPolys, numTexCoords;
    int numUnknown[9];
    saturnSector_t *sectors;
    saturnFace_t *faces;
    saturnVertex_t *vertices;
    saturnPoly_t *polys;
    kexStr fPath;
    int16_t *lookupTable[2];
    int missingFaces = 0;
    FILE *f;

    fPath = kexStr::Format("%s\\%s", cvarBasePath.GetValue(), file);
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
    numTexCoords    = kex::cEndian->SwapBE32(mapFile.Read32());
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
        faces[i].texCoord           = kex::cEndian->SwapBE16(mapFile.Read16());
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
        fprintf(f, "%8i ", faces[i].texCoord);
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

            if(face->sector != -1)
            {
                continue;
            }

            if(face->polyStart <= -1 || face->polyEnd <= -1)
            {
                missingFaces++;
                continue;
            }

            fprintf(f, "o face_%03d_%03d\n", i, j);

            for(int k = face->polyStart; k <= face->polyEnd; ++k)
            {
                saturnPoly_t *poly = &polys[k];
                int indices[4];

                indices[0] = face->vertexStart + poly->indices[0];
                indices[1] = face->vertexStart + poly->indices[1];
                indices[2] = face->vertexStart + poly->indices[2];
                indices[3] = face->vertexStart + poly->indices[3];

                fprintf(f, "f %i %i %i %i\n", indices[0]+1, indices[1]+1, indices[2]+1, indices[3]+1);
            }
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
