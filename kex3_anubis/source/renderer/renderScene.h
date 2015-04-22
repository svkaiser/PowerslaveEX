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

class kexRenderView;
class kexRenderScene;
class kexWorld;
class kexDLight;

#define MAX_DLIGHTS     32

class kexRenderDLight
{
public:
    kexRenderDLight(void);

    void                    Init(void);
    void                    Clear(void);
    void                    AddLight(kexDLight *light);
    void                    Draw(kexRenderScene *rScene, kexStack<int> &polygons);

    bool                    MaxedOutLights(void) { return numDLights >= MAX_DLIGHTS; }

private:
    uint                    *lightMarks;
    uint                    numDLights;
    kexDLight               *dLightList[MAX_DLIGHTS];
};

class kexRenderScene
{
public:
    friend class kexRenderDLight;

    kexRenderScene(void);
    ~kexRenderScene(void);
    
    void                    DrawView(kexRenderView &view, mapSector_t *sector);

    void                    SetWorld(kexWorld *wld) { world = wld; }

    kexStack<int>           &VisibleSectors(void) { return visibleSectors; }
    kexStack<int>           &VisibleSkyFaces(void) { return visibleSkyFaces; }
    kexRenderDLight         &DLights(void) { return dLights; }
    
    static bool             bPrintStats;
    static bool             bShowPortals;
    static bool             bShowWaterPortals;
    static bool             bShowCollision;

private:
    static int              SortPolys(const int *p1, const int *p2);

    void                    Prepare(kexRenderView &view);
    void                    DrawSky(kexRenderView &view);
    void                    DrawSectors(kexRenderView &view);
    void                    DrawActors(kexRenderView &view);
    void                    DrawSector(kexRenderView &view, mapSector_t *sector);
    void                    DrawFace(kexRenderView &view, mapSector_t *sector, int faceID);
    void                    DrawPortal(kexRenderView &view, mapFace_t *face, byte r, byte g, byte b);
    void                    DrawPolygon(mapFace_t *face, mapPoly_t *poly);
    void                    DrawActorList(kexRenderView &view, mapSector_t *sector);
    void                    DrawSprite(kexRenderView &view, mapSector_t *sector, kexActor *actor);
    void                    DrawStretchSprite(kexRenderView &view, mapSector_t *sector, kexActor *actor);
    void                    DrawWater(kexRenderView &view);
    void                    PrintStats(void);
    void                    FindVisibleSectors(kexRenderView &view, mapSector_t *sector);
    bool                    SetScissorRect(kexRenderView &view, mapFace_t *face);
    void                    SetFaceDistance(kexRenderView &view, mapFace_t *face);
    bool                    ClipFaceToPlane(kexRenderView &view, kexPlane &plane, mapFace_t *face,
                                            float &bx1, float &bx2, float &by1, float &by2);
    
    kexStack<int>           waterFaces;
    kexMatrix               spriteMatrix;
    kexWorld                *world;
    int                     clipY;
    int                     vertCount;
    int                     triCount;
    kexStack<int>           visibleSectors;
    kexStack<int>           visibleSkyFaces;
    kexStack<int>           polyList;
    kexRenderDLight         dLights;
};

#endif
