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

#include "menuPanel.h"

#ifndef __INVENTORY_MENU_H__
#define __INVENTORY_MENU_H__

class kexInventoryMenu
{
public:
    kexInventoryMenu(void);
    ~kexInventoryMenu(void);
    
    void                        Init(void);
    void                        Display(void);
    void                        Reset(void);
    void                        Update(void);
    bool                        ProcessInput(inputEvent_t *ev);
    void                        Toggle(void);
    void                        ShowArtifact(const int artifact);

    const bool                  IsActive(void) { return bActive; }
    
private:
    void                        DrawLeftArrow(void);
    void                        DrawRightArrow(void);
    bool                        CursorOnLeftArrow(float &mx, float &my);
    bool                        CursorOnRightArrow(float &mx, float &my);
    void                        DrawBackground(void);
    void                        DrawButtons(void);
    void                        DrawKeys(void);
    void                        DrawAutomap(void);
    void                        DrawArtifacts(void);
    void                        DrawCenteredImage(kexTexture *texture, const float x, const float y);

    kexTexture                  *keyTextures[2][4];
    kexTexture                  *artifactTextures[6];
    kexTexture                  *weaponTextures[NUMPLAYERWEAPONS];
    kexTexture                  *arrows[2];
    kexTexture                  *mapClosedTexture;
    kexTexture                  *mapOpenTexture;
    kexFont                     *font;
    kexMenuPanel::buttonSet_t   buttonSet;
    bool                        bActive;
    bool                        bFlashArtifact;
    int                         flashBits;
    short                       flashCount;
    short                       categorySelected;
    short                       artifactSelected;
};

#endif
