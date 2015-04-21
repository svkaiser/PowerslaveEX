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

#ifndef __DLIGHTOBJ_H__
#define __DLIGHTOBJ_H__

#include "gameObject.h"
#include "world.h"

BEGIN_EXTENDED_KEX_CLASS(kexDLight, kexGameObject);
public:
    kexDLight(void);
    ~kexDLight(void);

    virtual void                    Tick(void);
    void                            Spawn(void);

    d_inline mapSector_t            *&Sector(void) { return sector; }
    d_inline float                  &Radius(void) { return radius; }
    d_inline kexBBox                &Bounds(void) { return bounds; }
    d_inline byte                   *Color(void) { return rgb; }
    d_inline float                  &FadeTime(void) { return fadeTime; }
    d_inline int                    &Passes(void) { return passes; }
    d_inline const float            FadeFrac(void) { return fadeTime / initialTime; }

private:
    byte                            rgb[3];
    float                           radius;
    kexBBox                         bounds;
    mapSector_t                     *sector;
    float                           fadeTime;
    float                           initialTime;
    int                             passes;
END_KEX_CLASS();

#endif
