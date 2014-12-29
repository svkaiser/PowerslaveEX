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
//      Endian Swapping Class
//

#include "SDL_endian.h"
#include "kexlib.h"

// Defines for checking the endianness of the system.

#if SDL_BYTEORDER == SYS_LIL_ENDIAN
#define SYS_LITTLE_ENDIAN
#elif SDL_BYTEORDER == SYS_BIG_ENDIAN
#define SYS_BIG_ENDIAN
#endif

static kexEndian endianLocal;
kexEndian *kex::cEndian = &endianLocal;

//
// kexEndian::SwapLE16
//

short kexEndian::SwapLE16(const short val)
{
    return SDL_SwapLE16(val);
}

//
// kexEndian::SwapBE16
//

short kexEndian::SwapBE16(const short val)
{
    return SDL_SwapBE16(val);
}

//
// kexEndian::SwapLE32
//

int kexEndian::SwapLE32(const int val)
{
    return SDL_SwapLE32(val);
}

//
// kexEndian::SwapBE32
//

int kexEndian::SwapBE32(const int val)
{
    return SDL_SwapBE32(val);
}
