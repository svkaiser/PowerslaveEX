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

#ifndef __HUD_H__
#define __HUD_H__

class kexHud
{
public:
    kexHud(void);
    ~kexHud(void);
    
    void                Init(void);
    void                Display(void);
    
    void                SetPlayer(kexPlayer *p) { player = p; }
    
private:
    void                DrawAmmoBar(void);
    void                DrawBackPic(void);
    void                DrawCompass(void);
    
    kexTexture          *backImage;
    kexPlayer           *player;
};

#endif
