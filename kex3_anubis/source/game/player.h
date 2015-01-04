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
    PW_MACHETE     =  0,
    PW_PISTOL,
    PW_M60,
    PW_BOMBS,
    PW_FLAMETHROWER,
    PW_COBRASTAFF,
    PW_RINGOFRA,
    PW_BRACELET,

    NUMPLAYERWEAPONS
} playerWeapons_t;

typedef enum
{
    PA_SANDALS  = BIT(0),
    PA_MASK     = BIT(1),
    PA_SHAWL    = BIT(2),
    PA_ANKLETS  = BIT(3),
    PA_SCEPTER  = BIT(4),
    PA_FEATHER  = BIT(5),
    PA_DOLPHIN  = BIT(6),
    PA_EAGLE    = BIT(7)
} playerArtifacts_t;

typedef enum
{
    PT_TRANSMITTER1 = BIT(0),
    PT_TRANSMITTER2 = BIT(1),
    PT_TRANSMITTER3 = BIT(2),
    PT_TRANSMITTER4 = BIT(3),
    PT_TRANSMITTER5 = BIT(4),
    PT_TRANSMITTER6 = BIT(5),
    PT_TRANSMITTER7 = BIT(6),
    PT_TRANSMITTER8 = BIT(7)
} playerTransmitter_t;

typedef enum
{
    PK_TIME     = BIT(0),
    PK_WAR      = BIT(1),
    PK_POWER    = BIT(2),
    PK_EARTH    = BIT(3)
} playerKeys_t;

typedef enum
{
    TD_DOLL01   = BIT(0),
    TD_DOLL02   = BIT(1),
    TD_DOLL03   = BIT(2),
    TD_DOLL04   = BIT(3),
    TD_DOLL05   = BIT(4),
    TD_DOLL06   = BIT(5),
    TD_DOLL07   = BIT(6),
    TD_DOLL08   = BIT(7),
    TD_DOLL09   = BIT(8),
    TD_DOLL10   = BIT(9),
    TD_DOLL11   = BIT(10),
    TD_DOLL12   = BIT(11),
    TD_DOLL13   = BIT(12),
    TD_DOLL14   = BIT(13),
    TD_DOLL15   = BIT(14),
    TD_DOLL16   = BIT(15),
    TD_DOLL17   = BIT(16),
    TD_DOLL18   = BIT(17),
    TD_DOLL19   = BIT(18),
    TD_DOLL20   = BIT(19),
    TD_DOLL21   = BIT(20),
    TD_DOLL22   = BIT(21),
    TD_DOLL23   = BIT(22)
} teamDolls_t;

class kexPlayer
{
public:
    kexPlayer(void);
    ~kexPlayer(void);

    void                    Reset(void);

    kexPlayerCmd            &Cmd(void) { return cmd; }

private:
    kexPlayerCmd            cmd;

    int16_t                 health;
    int16_t                 ankahs;
    kexAngle                angle;
    float                   bob;
    kexVec2                 weaponBob;

    byte                    currentWeapon;
    byte                    weaponState;
    int16_t                 weaponFrame;
    int16_t                 weaponTicks;
    bool                    weapons[NUMPLAYERWEAPONS];
    int16_t                 ammo[NUMPLAYERWEAPONS];

    static const int        maxAmmo[NUMPLAYERWEAPONS];
    static const int16_t    maxHealth;

    int16_t                 artifacts;
    int16_t                 keys;
    int16_t                 transmitter;
    int                     teamDolls;
};

#endif
