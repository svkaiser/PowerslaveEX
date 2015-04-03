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

#ifndef __MENU_PANEL_H__
#define __MENU_PANEL_H__

class kexMenuPanel
{
public:
    kexMenuPanel(void);
    ~kexMenuPanel(void);
    
    void                    Init(void);
    void                    DrawPanel(const float x, const float y, const float w, const float h,
                                      const float borderSize);
    void                    DrawInset(const float x, const float y, const float w, const float h);
    void                    DrawButton(const float x, const float y, bool bPressed, const char *text);
    bool                    PointOnButton(const float x, const float y, const float mx, const float my);
    void                    DrawLeftArrow(const float x, const float y);
    void                    DrawRightArrow(const float x, const float y);
    bool                    CursorOnLeftArrow(const float x, const float y);
    bool                    CursorOnRightArrow(const float x, const float y);
    
    typedef struct
    {
        float x;
        float y;
        kexStrList labels;
        int pressedIndex;
    } buttonSet_t;

    typedef struct
    {
        float x;
        float y;
        float w;
        float h;
        kexStr label;
        bool bPressed;
        bool bSelected;
        bool bHover;
    } selectButton_t;
    
    bool                    TestPointInButtonSet(buttonSet_t *buttonSet);
    void                    DrawButtonSet(buttonSet_t *buttonSet);

    void                    UpdateSelectButton(selectButton_t *button);
    bool                    TestSelectButtonInput(selectButton_t *button, inputEvent_t *ev);
    void                    DrawSelectButton(selectButton_t *button);
    
private:
    kexTexture              *bgTexture;
    kexTexture              *buttonTexture[2];
    kexTexture              *arrows[2];
    kexFont                 *font;
};

#endif
