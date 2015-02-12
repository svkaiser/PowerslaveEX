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

#ifndef __PICKUP_H__
#define __PICKUP_H__

#include "actor.h"

//-----------------------------------------------------------------------------
//
// kexPickup
//
//-----------------------------------------------------------------------------

BEGIN_EXTENDED_KEX_CLASS(kexPickup, kexActor);
public:
    kexPickup(void);
    ~kexPickup(void);

    virtual void                    Tick(void);
    virtual void                    OnTouch(kexActor *instigator);
END_KEX_CLASS();

//-----------------------------------------------------------------------------
//
// kexWeaponPickup
//
//-----------------------------------------------------------------------------

BEGIN_EXTENDED_KEX_CLASS(kexWeaponPickup, kexPickup);
public:
    kexWeaponPickup(void);
    ~kexWeaponPickup(void);

    virtual void                    Tick(void);
    virtual void                    OnTouch(kexActor *instigator);

    void                            Spawn(void);

private:
    int                             weaponSlotToGive;
    bool                            bRemoveIfOwned;
END_KEX_CLASS();

#endif
