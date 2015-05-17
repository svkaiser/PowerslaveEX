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

#ifndef __MOUSE_H__
#define __MOUSE_H__

typedef enum
{
    KMSB_UNDEFINED  = 0,
    KMSB_LEFT,
    KMSB_MIDDLE,
    KMSB_RIGHT,
    KMSB_WHEEL_UP,
    KMSB_WHEEL_DOWN,
    KMSB_MISC1,
    KMSB_MISC2,
    KMSB_MISC3,
    
    NUMMOUSEBUTTONS
} mouseButtons_t;

#endif
