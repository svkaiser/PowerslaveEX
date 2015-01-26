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
    void                    FindVisibleSectors(mapSector_t *startSector);
    
    void                    SetWorld(kexWorld *wld) { world = wld; }
    void                    SetView(kexRenderView *v) { view = v; }

    kexStack<mapSector_t*>  &VisibleSectors(void) { return visibleSectors; }

private:
    void                    RecursiveSectorPortals(portal_t *portal);
    void                    FloodPortalView(portal_t *portal, portal_t *prevPortal);
    bool                    FaceInPortalView(portal_t *portal, mapFace_t *face);
    
    kexWorld                *world;
    kexRenderView           *view;
    kexStack<mapSector_t*>  visibleSectors;
    
    int                     validcount;
};

#endif
