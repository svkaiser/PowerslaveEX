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
    
private:
    kexTexture              *bgTexture;
    kexTexture              *buttonTexture[2];
    kexFont                 *font;
};

#endif
