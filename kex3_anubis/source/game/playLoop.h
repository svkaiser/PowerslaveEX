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
    
    const int                   Ticks(void) const { return ticks; }
    
private:
    int                         ticks;
    kexRenderView               renderView;
};

#endif
