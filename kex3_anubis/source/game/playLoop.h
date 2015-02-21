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

#ifndef __PLAYLOOP_H__
#define __PLAYLOOP_H__

#include "renderView.h"
#include "renderScene.h"
#include "hud.h"

class kexPlayLoop : public kexGameLoop
{
public:
    kexPlayLoop(void);
    ~kexPlayLoop(void);

    void                        Init(void);
    void                        Start(void);
    void                        Stop(void);
    void                        Draw(void);
    void                        Tick(void);
    bool                        ProcessInput(inputEvent_t *ev);
    const int                   GetWaterVelocityPoint(const int index);
    
    const int                   Ticks(void) const { return ticks; }
    const int                   MaxWaterMagnitude(void) { return waterMaxMagnitude; }
    
private:
    void                        InitWater(void);
    void                        UpdateWater(void);

    int                         ticks;
    kexHud                      hud;
    kexRenderView               renderView;
    kexRenderScene              renderScene;
    int                         waterAccelPoints[16][16];
    int                         waterVelocityPoints[16][16];
    int                         waterMaxMagnitude;
};

#endif
