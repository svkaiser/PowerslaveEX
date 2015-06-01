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
    PF_JUMPING          = BIT(0),
    PF_USERJUMPED       = BIT(1),
    PF_NOCLIP           = BIT(2),
    PF_FLY              = BIT(3),
    PF_ABOVESURFACE     = BIT(4),
    PF_JUMPWASHELD      = BIT(5),
    PF_NEEDTOGASP       = BIT(6),
    PF_FLOATING         = BIT(7),
    PF_ELECTROCUTE      = BIT(8),
    PF_DEAD             = BIT(9),
    PF_GOD              = BIT(10),
    PF_STUNNED          = BIT(11)
} playerFlags_t;

typedef enum
{
    PAB_DOLPHIN         = BIT(0),
    PAB_VULTURE         = BIT(1)
} playerAbilities_t;

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
    virtual void                    OnDamage(kexActor *instigator);
    virtual bool                    OnCollide(kexCModel *cmodel);

    void                            ScheduleWarpForNextMap(const kexVec3 &destination);

    static kexVec3                  mapDestinationPosition;
    static bool                     bScheduleNextMapWarp;

    kexPlayer                       *Owner(void) { return owner; }
    unsigned int                    &PlayerFlags(void) { return playerFlags; }

private:
    void                            Jump(kexPlayerCmd *cmd);
    void                            GroundMove(kexPlayerCmd *cmd);
    void                            FlyMove(kexPlayerCmd *cmd);
    void                            WaterMove(kexPlayerCmd *cmd);
    void                            DeadMove(kexPlayerCmd *cmd);
    void                            TryClimbOutOfWater(void);
    void                            CheckFallDamage(void);
    void                            SlimeDamage(void);
    void                            LavaDamage(mapFace_t *face);

    kexPlayer                       *owner;
    unsigned int                    playerFlags;
    byte                            jumpTicks;
    kexVec3                         oldMovement;
    int                             lavaTicks;
    int                             slimeTicks;
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
    bool                        GiveHealth(const int amount);
    bool                        GiveKey(const int key);
    bool                        IncreaseMaxHealth(const int bits);
    bool                        CheckKey(const int key) { return (keys & BIT(key)) != 0; }
    void                        SetAmmo(const int value, const int weaponID) { ammo[weaponID] = value; }
    void                        SetWeapon(const bool value, const int weaponID) { weapons[weaponID] = value; }

    void                        HoldsterWeapon(void);
    void                        CycleNextWeapon(const bool bCheckAmmo = false);
    void                        CyclePrevWeapon(const bool bCheckAmmo = false);

    bool                        HasAmmo(const int weaponID);

    kexPlayerCmd                &Cmd(void) { return cmd; }
    kexPuppet                   *Actor(void) { return actor; }
    void                        SetActor(kexPuppet *_actor) { actor = _actor; }
    void                        ClearActor(void) { actor = NULL; }

    int16_t                     &Ankahs(void) { return ankahs; }
    int16_t                     &AnkahFlags(void) { return ankahFlags; }
    int16_t                     &Artifacts(void) { return artifacts; }
    int16_t                     &QuestItems(void) { return questItems; }
    int16_t                     &Abilities(void) { return abilities; }
    uint                        &TeamDolls(void) { return teamDolls; }

    const uint16_t              Buttons(void) const { return cmd.Buttons(); }

    kexActor                    *AutoAim(const kexVec3 &start, kexAngle &yaw, kexAngle &pitch,
                                         const float dist, const float aimYaw, const float aimPitch);

    int16_t                     &Health(void) { return health; }
    const float                 Bob(void) const { return bob; }
    float                       &LandTime(void) { return landTime; }
    float                       &StepViewZ(void) { return stepViewZ; }
    float                       &ViewZ(void) { return viewZ; }
    int                         &LockTime(void) { return lockTime; }
    int16_t                     &AirSupply(void) { return airSupply; }
    int                         &ShakeTime(void) { return shakeTime; }
    kexVec2                     &ShakeVector(void) { return shakeVector; }

    kexPlayerWeapon             &Weapon(void) { return weapon; }
    const playerWeapons_t       CurrentWeapon(void) const { return currentWeapon; }
    void                        ChangeWeapon(void) { currentWeapon = pendingWeapon; }
    playerWeapons_t             &PendingWeapon(void) { return pendingWeapon; }

    static const int16_t        maxHealth;
    static kexCvar              cvarAutoAim;

private:
    void                        UpdateAirSupply(void);
    void                        TryUse(void);
    void                        UpdateWeaponBob(void);
    void                        UpdateViewBob(void);
    void                        UpdateWeaponSprite(void);

    kexPlayerCmd                cmd;

    int16_t                     ankahs;
    int16_t                     ankahFlags;
    float                       bob;
    float                       bobTime;
    float                       bobSpeed;
    float                       landTime;
    float                       stepViewZ;
    float                       viewZ;
    int16_t                     health;

    kexPlayerWeapon             weapon;
    playerWeapons_t             currentWeapon;
    playerWeapons_t             pendingWeapon;

    bool                        weapons[NUMPLAYERWEAPONS];
    int16_t                     ammo[NUMPLAYERWEAPONS];

    int16_t                     artifacts;
    int16_t                     keys;
    int16_t                     questItems;
    int16_t                     abilities;
    int16_t                     airSupply;
    int16_t                     airSupplyTime;
    uint                        teamDolls;
    int                         lockTime;
    int                         shakeTime;
    kexVec2                     shakeVector;
    kexPuppet                   *actor;
};

#endif
