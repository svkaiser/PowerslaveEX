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

#include "kexlib.h"

//
// kexTimer::Init
//

void kexTimer::Init(void)
{
}

//
// kexTimer::Sleep
//

void kexTimer::Sleep(unsigned long usecs)
{
}

//
// kexTimer::GetTicks
//

int kexTimer::GetTicks(void)
{
    return 0;
}

//
// kexTimer::GetMS
//

int kexTimer::GetMS(void)
{
    return 0;
}

//
// kexTimer::GetPerformanceCounter
//

uint64_t kexTimer::GetPerformanceCounter(void)
{
    return 0;
}

//
// kexTimer::MeasurePerformance
//

double kexTimer::MeasurePerformance(const uint64_t value)
{
    return 0;
}

//
// kexTimer::AddTimer
//

int kexTimer::AddTimer(const int delay, timerFunction_t function, void *data)
{
    return -1;
}

//
// kexTimer::RemoveTimer
//

void kexTimer::RemoveTimer(const int id)
{
}
