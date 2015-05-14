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

    void                            Spawn(void);

protected:
    kexStr                          pickupSound;
    kexStr                          pickupMessage;
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

//-----------------------------------------------------------------------------
//
// kexAmmoPickup
//
//-----------------------------------------------------------------------------

BEGIN_EXTENDED_KEX_CLASS(kexAmmoPickup, kexPickup);
public:
    kexAmmoPickup(void);
    ~kexAmmoPickup(void);

    virtual void                    Tick(void);
    virtual void                    OnTouch(kexActor *instigator);

    void                            Spawn(void);

private:
    int                             weaponSlotToGive;
    int                             divisor;
    float                           multiplier;
    bool                            bFullAmmo;
END_KEX_CLASS();

//-----------------------------------------------------------------------------
//
// kexHealthPickup
//
//-----------------------------------------------------------------------------

BEGIN_EXTENDED_KEX_CLASS(kexHealthPickup, kexPickup);
public:
    kexHealthPickup(void);
    ~kexHealthPickup(void);

    virtual void                    Tick(void);
    virtual void                    OnTouch(kexActor *instigator);

    void                            Spawn(void);

private:
    int                             giveAmount;
    float                           multiplier;

END_KEX_CLASS();

//-----------------------------------------------------------------------------
//
// kexMaxHealthPickup
//
//-----------------------------------------------------------------------------

BEGIN_EXTENDED_KEX_CLASS(kexMaxHealthPickup, kexPickup);
public:
    kexMaxHealthPickup(void);
    ~kexMaxHealthPickup(void);

    virtual void                    Tick(void);
    virtual void                    OnTouch(kexActor *instigator);

    void                            Spawn(void);

private:
    int                             bits;
END_KEX_CLASS();

//-----------------------------------------------------------------------------
//
// kexKeyPickup
//
//-----------------------------------------------------------------------------

BEGIN_EXTENDED_KEX_CLASS(kexKeyPickup, kexPickup);
public:
    kexKeyPickup(void);
    ~kexKeyPickup(void);

    virtual void                    Tick(void);
    virtual void                    OnTouch(kexActor *instigator);

    void                            Spawn(void);

private:
    int                             bits;
END_KEX_CLASS();

//-----------------------------------------------------------------------------
//
// kexArtifactPickup
//
//-----------------------------------------------------------------------------

BEGIN_EXTENDED_KEX_CLASS(kexArtifactPickup, kexPickup);
public:
    kexArtifactPickup(void);
    ~kexArtifactPickup(void);

    virtual void                    Tick(void);
    virtual void                    OnTouch(kexActor *instigator);

    void                            Spawn(void);

private:
    int                             bits;
END_KEX_CLASS();

//-----------------------------------------------------------------------------
//
// kexQuestPickup
//
//-----------------------------------------------------------------------------

BEGIN_EXTENDED_KEX_CLASS(kexQuestPickup, kexPickup);
public:
    kexQuestPickup(void);
    ~kexQuestPickup(void);

    virtual void                    Tick(void);
    virtual void                    OnTouch(kexActor *instigator);

    void                            Spawn(void);

private:
    int                             bits;
END_KEX_CLASS();

//-----------------------------------------------------------------------------
//
// kexTeamDollPickup
//
//-----------------------------------------------------------------------------

BEGIN_EXTENDED_KEX_CLASS(kexTeamDollPickup, kexPickup);
public:
    kexTeamDollPickup(void);
    ~kexTeamDollPickup(void);

    virtual void                    Tick(void);
    virtual void                    OnTouch(kexActor *instigator);

    void                            Spawn(void);

private:
    int                             bits;
END_KEX_CLASS();

//-----------------------------------------------------------------------------
//
// kexMapPickup
//
//-----------------------------------------------------------------------------

BEGIN_EXTENDED_KEX_CLASS(kexMapPickup, kexPickup);
public:
    kexMapPickup(void);
    ~kexMapPickup(void);

    virtual void                    Tick(void);
    virtual void                    OnTouch(kexActor *instigator);

    void                            Spawn(void);
END_KEX_CLASS();

#endif
