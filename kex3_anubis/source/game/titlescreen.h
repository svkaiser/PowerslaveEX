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

class kexTitleMenuItem;
typedef void(*menuItemLerpDone_t)(kexTitleMenuItem*);

//-----------------------------------------------------------------------------
//
// kexTitleMenuItem
//
//-----------------------------------------------------------------------------

BEGIN_EXTENDED_KEX_CLASS(kexTitleMenuItem, kexObject);
public:
    kexTitleMenuItem(void);
    kexTitleMenuItem(const char *label, const float x, const float y, const float scale,
                     menuItemLerpDone_t callback = NULL);
    ~kexTitleMenuItem(void);

    virtual void                    Draw(void);
    virtual void                    Tick(void);
    virtual void                    Select(const bool b) { bSelected = b; }

    void                            DrawSmallString(const char *string, float x, float y, float scale,
                                                    bool center, bool flash);
    void                            DrawBigString(const char *string, float x, float y, float scale,
                                                  bool center, bool flash);
    void                            LerpTo(const float destx, const float desty);
    void                            LerpTo(const float destx);

    const bool                      IsHighlighted(void) const { return bHighLighted; }
    const bool                      Lerping(void) const { return bLerping; }
    const bool                      IsDisabled(void) const { return bDisabled; }
    void                            Toggle(const bool b) { bDisabled = !b; }

    virtual kexTitleMenuItem        &operator=(const kexTitleMenuItem &item);

    float                           x;
    float                           y;
    float                           scale;

    menuItemLerpDone_t              lerpCallback;

protected:
    void                            Move(void);
    virtual bool                    OnCursor(void);

    float                           highlightTime;
    kexStr                          label;
    bool                            bDisabled;
    bool                            bSelected;
    bool                            bInteract;
    bool                            bLerping;
    bool                            bHighLighted;
    float                           time;
    float                           startX;
    float                           startY;
    float                           destX;
    float                           destY;
END_KEX_CLASS();

class kexTitleScreen : public kexGameLoop
{
public:
    kexTitleScreen(void);
    ~kexTitleScreen(void);

    void                        Init(void);
    void                        Start(void);
    void                        Stop(void);
    void                        Draw(void);
    void                        Tick(void);
    bool                        ProcessInput(inputEvent_t *ev);
    void                        FadeDone(void);
    void                        DeselectAllItems(void);
    void                        FadeOut(int state);
    void                        StartLoadGame(const int slot);
    void                        StartNewGame(const int slot);

    const int                   SelectedItem(void) const { return selectedItem; }
    void                        SetState(const int s) { state = s; }
    int                         &CurrentMenu(void) { return currentMenu; }

private:
    int                         state;
    int                         fadeTime;
    int                         curFadeTime;
    bool                        bFadeIn;
    bool                        bFading;
    kexTexture                  *titlePic;
    int                         selectedItem;
    int                         loadGameSlot;
    int                         currentMenu;
};

#endif
