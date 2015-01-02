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

#ifndef __TITLESCREEN_H__
#define __TITLESCREEN_H__

#include "menu.h"

typedef void(*menuItemSelect_t)(kexMenuItem*);

class kexTitleScreen
{
public:
    kexTitleScreen(void);
    ~kexTitleScreen(void);

    void                        Init(void);
    void                        Start(void);
    void                        Draw(void);
    void                        Tick(void);
    bool                        ProcessInput(inputEvent_t *ev);
    void                        FadeDone(void);
    void                        DeselectAllItems(void);
    void                        OnSelectOptions(kexMenuItem *item);
    void                        OnSelectExitOptions(kexMenuItem *item);

    const int                   SelectedItem(void) const { return selectedItem; }

    typedef struct
    {
        kexMenuItem             item;
        menuItemSelect_t        callback;
    } menuGroup_t;

    static menuGroup_t          titleMenu[11];

private:
    int                         fadeTime;
    int                         curFadeTime;
    bool                        bFadeIn;
    bool                        bFading;
    kexTexture                  *titlePic;
    int                         selectedItem;
};

#endif
