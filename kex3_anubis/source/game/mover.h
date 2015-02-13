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

#ifndef __MOVER_H__
#define __MOVER_H__

#include "gameObject.h"

//-----------------------------------------------------------------------------
//
// kexMover
//
//-----------------------------------------------------------------------------

BEGIN_EXTENDED_KEX_CLASS(kexMover, kexGameObject);
public:
    kexMover(void);
    ~kexMover(void);

    virtual void            Tick(void);

    void                    SetDefinition(kexDict *dict) { definition = dict; }
    int                     &Type(void) { return type; }
    mapSector_t             *Sector(void) { return sector; }
    void                    SetSector(mapSector_t *s);

protected:
    kexDict                 *definition;
    int                     type;
    mapSector_t             *sector;
END_KEX_CLASS();

//-----------------------------------------------------------------------------
//
// kexDoor
//
//-----------------------------------------------------------------------------

BEGIN_EXTENDED_KEX_CLASS(kexDoor, kexMover);
public:
    kexDoor(void);
    ~kexDoor(void);

    virtual void            Tick(void);
    void                    Spawn(void);

private:
    float                   waitDelay;
    float                   moveSpeed;
    float                   lip;
    bool                    bDirection;
    float                   destHeight;
    float                   moveAmount;
    float                   moveTime;
END_KEX_CLASS();

#endif
