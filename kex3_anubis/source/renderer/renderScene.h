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
class kexWorld;

class kexRenderScene
{
public:
    kexRenderScene(void);
    ~kexRenderScene(void);
    
    void                    Draw(void);

    void                    SetWorld(kexWorld *wld) { world = wld; }
    void                    SetView(kexRenderView *v) { view = v; }
    
    static bool             bPrintStats;
    static bool             bShowPortals;
    static bool             bShowWaterPortals;
    static bool             bShowCollision;

private:
    void                    Prepare(void);
    void                    DrawSky(void);
    void                    DrawSectors(void);
    void                    DrawActors(void);
    void                    DrawSector(mapSector_t *sector);
    void                    DrawFace(mapSector_t *sector, int faceID);
    void                    DrawPortal(mapFace_t *face, byte r, byte g, byte b);
    void                    DrawPolygon(mapFace_t *face, mapPoly_t *poly);
    void                    DrawActorList(mapSector_t *sector);
    void                    DrawWater(void);
    void                    PrintStats(void);
    
    kexStack<int>           waterFaces;
    kexMatrix               spriteMatrix;
    kexWorld                *world;
    kexRenderView           *view;
    int                     clipY;
    int                     vertCount;
    int                     triCount;
};

#endif
