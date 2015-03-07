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

#ifndef __MENU_H__
#define __MENU_H__

class kexMenuItem;
typedef void(*menuItemLerpDone_t)(kexMenuItem*);

//-----------------------------------------------------------------------------
//
// kexMenuItem
//
//-----------------------------------------------------------------------------

BEGIN_EXTENDED_KEX_CLASS(kexMenuItem, kexObject);
public:
    kexMenuItem(void);
    kexMenuItem(const char *label, const float x, const float y, const float scale,
                menuItemLerpDone_t callback = NULL);
    ~kexMenuItem(void);

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
    void                            Toggle(const bool b) { bDisabled = !b; }

    virtual kexMenuItem             &operator=(const kexMenuItem &item);

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

//-----------------------------------------------------------------------------
//
// kexMenuItemLabel
//
//-----------------------------------------------------------------------------

BEGIN_EXTENDED_KEX_CLASS(kexMenuItemLabel, kexMenuItem);
    public:
    kexMenuItemLabel(void);
    kexMenuItemLabel(const char *label, const float x, const float y, const float scale);
    ~kexMenuItemLabel(void);

    virtual void                    Draw(void);
    virtual void                    Tick(void);
END_KEX_CLASS();

//-----------------------------------------------------------------------------
//
// kexMenuItemSlider
//
//-----------------------------------------------------------------------------

BEGIN_EXTENDED_KEX_CLASS(kexMenuItemSlider, kexMenuItem);
public:
    kexMenuItemSlider(void);
    kexMenuItemSlider(const float x, const float y, const float scale, kexCvar &cvar);
    ~kexMenuItemSlider(void);

    virtual void                    Draw(void);
    virtual void                    Tick(void);
    virtual void                    Select(const bool b);

private:
    virtual bool                    OnCursor(void);

    kexCvar                         *cvar;
    int                             numBars;
END_KEX_CLASS();

#endif
