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
//      Timer Class
//

#include "SDL.h"
#include "kexlib.h"

static kexTimer timer;
kexTimer *kex::cTimer = &timer;

//
// kexTimer::Init
//

void kexTimer::Init(void)
{
    SDL_Init(SDL_INIT_TIMER);
}

//
// kexTimer::Sleep
//

void kexTimer::Sleep(unsigned long usecs)
{
    SDL_Delay(usecs);
}

//
// kexTimer::GetTicks
//

int kexTimer::GetTicks(void)
{
    return SDL_GetTicks();
}

//
// kexTimer::GetMS
//

int kexTimer::GetMS(void)
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
// kexTimer::GetPerformanceCounter
//

uint64_t kexTimer::GetPerformanceCounter(void)
{
    return SDL_GetPerformanceCounter();
}
