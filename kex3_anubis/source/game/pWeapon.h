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

#ifndef __PWEAPON_H__
#define __PWEAPON_H__

class kexPlayer;

class kexPlayerWeapon
{
    friend class kexPlayer;
public:
    kexPlayerWeapon(void);
    ~kexPlayerWeapon(void);

    void                        ChangeAnim(spriteAnim_t *changeAnim);
    void                        ChangeAnim(const weaponState_t changeState);
    void                        ChangeAnim(const char *animName);
    void                        Update(void);
    void                        Draw(void);

    spriteAnim_t                *Anim(void) { return anim; }
    spriteFrame_t               *Frame(void) { return &anim->frames[frameID]; }
    const int                   FrameID(void) const { return frameID; }
    weaponState_t               &State(void) { return state; }
    float                       &BobX(void) { return bob_x; }
    float                       &BobY(void) { return bob_y; }
    float                       &Ticks(void) { return ticks; }
    kexPlayer                   *Owner(void) { return owner; }

private:
    void                        DrawAnimFrame(spriteAnim_t *sprAnim);
    void                        DrawFlame(void);
    void                        UpdateBob(void);
    void                        UpdateSprite(void);

    kexPlayer                   *owner;
    float                       bob_x;
    float                       bob_y;
    int                         bobTime;
    spriteAnim_t                *anim;
    weaponState_t               state;
    int16_t                     frameID;
    float                       ticks;
};

#endif
