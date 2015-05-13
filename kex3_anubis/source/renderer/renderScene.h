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

#ifndef __RENDERSCENE_H__
#define __RENDERSCENE_H__

#include "world.h"
#include "vertexBuffer.h"

class kexRenderView;
class kexRenderScene;
class kexWorld;
class kexDLight;

#define MAX_DLIGHTS     32

typedef struct
{
    int index;
    kexVec3 newVec;
    byte newColor[4];
} bufferUpdate_t;

typedef kexStack<bufferUpdate_t> bufferUpdateList_t;

class kexRenderDLight
{
public:
    kexRenderDLight(void);

    void                            Init(void);
    void                            Clear(void);
    void                            AddLight(kexDLight *light);
    void                            Draw(kexRenderScene *rScene);

    bool                            MaxedOutLights(void) { return numDLights >= MAX_DLIGHTS; }

private:
    void                            RenderLitPolygon(mapPoly_t *poly, int &tris);

    uint                            *lightMarks;
    uint                            numDLights;
    kexDLight                       *dLightList[MAX_DLIGHTS];
};

class kexRenderScene
{
public:
    friend class kexRenderDLight;
    friend class kexRenderPortal;

    kexRenderScene(void);
    ~kexRenderScene(void);
    
    void                            DrawView(kexRenderView &view, mapSector_t *sector);

    void                            SetWorld(kexWorld *wld) { world = wld; }
    void                            InitVertexBuffer(void);
    void                            DestroyVertexBuffer(void);

    static bufferUpdateList_t       bufferUpdateList;

    kexStack<int>                   &VisibleSectors(void) { return visibleSectors; }
    kexStack<int>                   &VisibleSkyFaces(void) { return visibleSkyFaces; }
    kexRenderDLight                 &DLights(void) { return dLights; }
    
    static bool                     bPrintStats;
    static bool                     bShowPortals;
    static bool                     bShowWaterPortals;
    static bool                     bShowCollision;
    static bool                     bShowBounds;
    static bool                     bShowDynamic;
    static bool                     bDrawDynamicOnly;
    static bool                     bDrawStaticOnly;

    static kexCvar                  cvarRenderWireframe;
    static kexCvar                  cvarRenderFixSpriteClipping;

private:
    typedef struct
    {
        kexActor *actor;
        float dist;
    } visSprite_t;

    static int                      SortPolys(const int *p1, const int *p2);
    static int                      SortBufferLists(const bufferIndex_t *p1, const bufferIndex_t *p2);
    static int                      SortSprites(const visSprite_t *vis1, const visSprite_t *vis2);

    void                            BuildSky(void);
    void                            DrawIndividualPolygons(kexStack<int> &polys);
    void                            DrawGroupedPolygons(void);
    void                            SetSectorScissor(mapSector_t *sector);
    void                            Prepare(kexRenderView &view);
    void                            DrawSky(kexRenderView &view);
    void                            DrawSectors(kexRenderView &view);
    void                            FixSpriteClipping(kexRenderView &view, const bool bInWaterOnly);
    void                            PrepareSprites(kexRenderView &view);
    void                            DrawActors(kexRenderView &view, const bool bInWaterOnly);
    void                            DrawSector(kexRenderView &view, mapSector_t *sector);
    void                            DrawFace(kexRenderView &view, mapSector_t *sector, int faceID);
    void                            DrawPortal(kexRenderView &view, mapFace_t *face, byte r, byte g, byte b);
    void                            DrawDebug(kexRenderView &view);
    void                            DrawPolygon(mapFace_t *face, mapPoly_t *poly);
    void                            DrawSprite(kexRenderView &view, mapSector_t *sector, kexActor *actor);
    void                            DrawStretchSprite(kexRenderView &view, mapSector_t *sector, kexActor *actor);
    void                            DrawWater(kexRenderView &view);
    void                            ShadeWaterColor(kexVec3 &origin, int &r, int &g, int &b);
    void                            PrintStats(void);
    void                            BuildSectorBuffer(mapSector_t *sector);
    void                            UpdateBuffer(void);
    void                            FindVisibleSectors(kexRenderView &view, mapSector_t *sector);
    bool                            SetScissorRect(kexRenderView &view, mapFace_t *face);
    void                            SetFaceDistance(kexRenderView &view, mapFace_t *face);
    bool                            ClipFaceToPlane(kexRenderView &view, kexPlane &plane, mapFace_t *face,
                                                    float &bx1, float &bx2, float &by1, float &by2);
    
    kexMatrix                       spriteMatrix;
    kexWorld                        *world;
    int                             clipY;
    int                             vertCount;
    int                             triCount;
    kexStack<int>                   visibleSectors;
    kexStack<int>                   visibleSkyFaces;
    kexStack<int>                   polyList;
    kexStack<int>                   dynamicPolyList;
    kexStack<int>                   waterFaces;
    kexStack<visSprite_t>           visSprites;
    kexStack<bufferIndex_t>         bufferList;

    // used to keep track of shared vertices for moving sectors.
    kexArray<int>                   *vertexBufferLookup;

    kexRenderDLight                 dLights;
    uint64_t                        floodFillTime;
    uint64_t                        drawSectorTime;
    uint64_t                        drawActorTime;
    uint64_t                        polySortTime;
    kexVertBuffer                   worldVertexBuffer;
    kexVertBuffer                   skyBuffer;
    uint                            vertexCount;
    uint                            indiceCount;
    int                             drawTris;
    kexVertBuffer::drawVert_t       *drawVerts;
    uint                            *drawIndices;
    kexVertBuffer::drawVertList_t   skyVerts;
    kexVertBuffer::drawIndiceList_t skyIndices;
};

#endif
