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

#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "playerCmd.h"

typedef enum
{
    PF_JUMPING      = BIT(0),
    PF_USERJUMPED   = BIT(1),
    PF_NOCLIP       = BIT(2),
    PF_FLY          = BIT(3)
} playerFlags_t;

class kexActor;
class kexPlayer;

//-----------------------------------------------------------------------------
//
// kexPuppet
//
//-----------------------------------------------------------------------------

BEGIN_EXTENDED_KEX_CLASS(kexPuppet, kexActor);
public:
    kexPuppet(void);
    ~kexPuppet(void);

    virtual void                    Tick(void);
    void                            Spawn(void);

    kexPlayer                       *Owner(void) { return owner; }
    unsigned int                    &PlayerFlags(void) { return playerFlags; }

private:
    void                            Jump(kexPlayerCmd *cmd);
    void                            GroundMove(kexPlayerCmd *cmd);
    void                            FlyMove(kexPlayerCmd *cmd);
    void                            WaterMove(kexPlayerCmd *cmd);

    kexPlayer                       *owner;
    unsigned int                    playerFlags;
    byte                            jumpTicks;
END_KEX_CLASS();

//-----------------------------------------------------------------------------
//
// kexPlayer
//
//-----------------------------------------------------------------------------

class kexPlayer
{
public:
    kexPlayer(void);
    ~kexPlayer(void);

    void                        Reset(void);
    void                        Tick(void);
    void                        Ready(void);
    bool                        GiveWeapon(const int weaponID, const bool bAutoSwitch = true);
    bool                        WeaponOwned(const int weaponID);
    void                        ConsumeAmmo(const int16_t amount);
    const int16_t               GetAmmo(const int weaponID);
    void                        GiveAmmo(const int weaponID, int16_t amount);

    kexPlayerCmd                &Cmd(void) { return cmd; }
    kexPuppet                   *Actor(void) { return actor; }
    void                        SetActor(kexPuppet *_actor) { actor = _actor; }
    void                        ClearActor(void) { actor = NULL; }

    const float                 Bob(void) const { return bob; }
    float                       &LandTime(void) { return landTime; }
    float                       &StepViewZ(void) { return stepViewZ; }
    float                       &ViewZ(void) { return viewZ; }

    kexPlayerWeapon             &Weapon(void) { return weapon; }
    const playerWeapons_t       CurrentWeapon(void) const { return currentWeapon; }
    void                        ChangeWeapon(void) { currentWeapon = pendingWeapon; }
    playerWeapons_t             &PendingWeapon(void) { return pendingWeapon; }

private:
    void                        TryUse(void);
    void                        UpdateWeaponBob(void);
    void                        UpdateViewBob(void);
    void                        UpdateWeaponSprite(void);
    void                        CycleNextWeapon(void);
    void                        CyclePrevWeapon(void);

    kexPlayerCmd                cmd;

    int16_t                     health;
    int16_t                     ankahs;
    float                       bob;
    float                       bobTime;
    float                       bobSpeed;
    float                       landTime;
    float                       stepViewZ;
    float                       viewZ;

    kexPlayerWeapon             weapon;
    playerWeapons_t             currentWeapon;
    playerWeapons_t             pendingWeapon;

    bool                        weapons[NUMPLAYERWEAPONS];
    int16_t                     ammo[NUMPLAYERWEAPONS];

    static const int16_t        maxHealth;

    int16_t                     artifacts;
    int16_t                     keys;
    int16_t                     transmitter;
    int                         teamDolls;
    kexPuppet                   *actor;
};

#endif
