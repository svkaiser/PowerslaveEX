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

#ifndef __PLAYERCMD_H__
#define __PLAYERCMD_H__

typedef enum
{
    BC_ATTACK       = BIT(0),
    BC_JUMP         = BIT(1),
    BC_FORWARD      = BIT(2),
    BC_BACKWARD     = BIT(3),
    BC_LEFT         = BIT(4),
    BC_RIGHT        = BIT(5),
    BC_STRAFELEFT   = BIT(6),
    BC_STRAFERIGHT  = BIT(7),
    BC_WEAPONRIGHT  = BIT(8),
    BC_WEAPONLEFT   = BIT(9)
} buttonCommand_t;

class kexPlayerCmd
{
public:
    kexPlayerCmd(void);
    ~kexPlayerCmd(void);

    void                BuildCommands(void);
    void                Reset(void);

    const word          Buttons(void) const { return buttons; }
    void                SetTurnXY(const int x, const int y) { turnx = x; turny = y; }
    const int           TurnX(void) const { return turnx; }
    const int           TurnY(void) const { return turny; }
    float               *Angles(void) { return angles; }

private:
    void                BuildButtons(void);
    void                BuildTurning(void);

    word                buttons;
    float               angles[2];
    int                 frame;
    int                 time;
    int                 turnx;
    int                 turny;
};

#endif
