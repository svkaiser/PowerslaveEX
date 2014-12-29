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
//      Timer Class (SDL)
//

#include "SDL.h"
#include "kexlib.h"

class kexTimerSDL : public kexTimer
{
public:
    virtual void            Init(void);
    virtual void            Sleep(unsigned long usecs);
    virtual int             GetMS(void);
    virtual uint64_t        GetPerformanceCounter(void);
    virtual int             GetTicks(void);
};

static kexTimerSDL timer;
kexTimer *kex::cTimer = &timer;

//
// kexTimerSDL::Init
//

void kexTimerSDL::Init(void)
{
    SDL_Init(SDL_INIT_TIMER);
}

//
// kexTimerSDL::Sleep
//

void kexTimerSDL::Sleep(unsigned long usecs)
{
    SDL_Delay(usecs);
}

//
// kexTimerSDL::GetTicks
//

int kexTimerSDL::GetTicks(void)
{
    return SDL_GetTicks();
}

//
// kexTimerSDL::GetMS
//

int kexTimerSDL::GetMS(void)
{
    uint32_t ticks;
    static int basetime = 0;
    
    ticks = GetTicks();
    
    if(basetime == 0)
    {
        basetime = ticks;
    }
    
    return ticks - basetime;
}

//
// kexTimerSDL::GetPerformanceCounter
//

uint64_t kexTimerSDL::GetPerformanceCounter(void)
{
    return SDL_GetPerformanceCounter();
}
