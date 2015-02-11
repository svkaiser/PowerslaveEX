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

#ifndef __INVENTORY_H__
#define __INVENTORY_H__

#include "actor.h"

//-----------------------------------------------------------------------------
//
// kexInventory
//
//-----------------------------------------------------------------------------

BEGIN_EXTENDED_KEX_CLASS(kexInventory, kexActor);
public:
    kexInventory(void);
    ~kexInventory(void);

    virtual void                    Tick(void);
    virtual void                    OnTouch(kexActor *instigator);
    void                            Spawn(void);

protected:
    int                             amount;
    int                             maxAmount;
END_KEX_CLASS();

#endif
