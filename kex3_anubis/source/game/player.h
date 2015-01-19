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

BEGIN_EXTENDED_CLASS(kexPuppet, kexActor);
public:
    kexPuppet(void);
    ~kexPuppet(void);

    virtual void                    Tick(void);
    void                            Spawn(void);

    kexPlayer                       *Owner(void) { return owner; }
    unsigned int                    &PlayerFlags(void) { return playerFlags; }

private:
    void                            GroundMove(kexPlayerCmd *cmd);
    void                            FlyMove(kexPlayerCmd *cmd);

    kexPlayer                       *owner;
    unsigned int                    playerFlags;
    byte                            jumpTicks;
END_CLASS();

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

    kexPlayerCmd                &Cmd(void) { return cmd; }
    kexActor                    *Actor(void) { return actor; }
    void                        SetActor(kexActor *_actor) { actor = _actor; }
    void                        ClearActor(void) { actor = NULL; }

    const float                 Bob(void) const { return bob; }
    float                       &LandTime(void) { return landTime; }
    float                       &StepViewZ(void) { return stepViewZ; }

    spriteAnim_t                *WeaponAnim(void) { return weaponAnim; }
    spriteFrame_t               *WeaponFrame(void) { return &weaponAnim->frames[weaponFrame]; }
    const int                   WeaponFrameID(void) const { return weaponFrame; }
    weaponState_t               &WeaponState(void) { return weaponState; }
    float                       &WeaponBobX(void) { return weaponBob_x; }
    float                       &WeaponBobY(void) { return weaponBob_y; }
    float                       &WeaponTicks(void) { return weaponTicks; }
    const playerWeapons_t       CurrentWeapon(void) const { return currentWeapon; }
    playerWeapons_t             &PendingWeapon(void) { return pendingWeapon; }

private:
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

    float                       weaponBob_x;
    float                       weaponBob_y;
    int                         weaponBobTime;
    spriteAnim_t                *weaponAnim;
    playerWeapons_t             currentWeapon;
    playerWeapons_t             pendingWeapon;
    weaponState_t               weaponState;
    int16_t                     weaponFrame;
    float                       weaponTicks;

    bool                        weapons[NUMPLAYERWEAPONS];
    int16_t                     ammo[NUMPLAYERWEAPONS];

    static const int            maxAmmo[NUMPLAYERWEAPONS];
    static const int16_t        maxHealth;

    int16_t                     artifacts;
    int16_t                     keys;
    int16_t                     transmitter;
    int                         teamDolls;
    kexActor                    *actor;
};

#endif
