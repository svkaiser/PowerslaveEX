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

#ifndef __TIMER_H__
#define __TIMER_H__

class kexTimer
{
public:
    virtual void            Init(void);
    virtual void            Sleep(unsigned long usecs);
    virtual int             GetMS(void);
    virtual uint64_t        GetPerformanceCounter(void);
    virtual double        MeasurePerformance(const uint64_t value);
    virtual int             GetTicks(void);
};

#endif
