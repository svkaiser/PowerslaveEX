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
class kexViewBounds;
class kexWorld;

class kexRenderScene
{
public:
    kexRenderScene(void);
    ~kexRenderScene(void);
    
    void                    Draw(void);
    void                    FindVisibleSectors(void);
    
    void                    SetWorld(kexWorld *wld) { world = wld; }
    void                    SetView(kexRenderView *v) { view = v; }

    kexStack<mapSector_t*>  &VisibleSectors(void) { return visibleSectors; }
    kexStack<kexViewBounds> &VisiblePortals(void) { return visiblePortals; }

private:
    void                    SetInitialScissorRect(void);
    void                    RecursiveSectorPortals(mapSector_t *sector, kexViewBounds *vb);
    
    kexWorld                *world;
    kexRenderView           *view;
    kexStack<mapSector_t*>  visibleSectors;
    kexStack<kexViewBounds> visiblePortals;
    
    int                     validcount;
};

#endif
