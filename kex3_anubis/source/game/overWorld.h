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

#ifndef __OVERWORLD_H__
#define __OVERWORLD_H__

class kexOverWorld : public kexGameLoop
{
public:
    kexOverWorld(void);
    ~kexOverWorld(void);

    void                        Init(void);
    void                        Start(void);
    void                        Stop(void);
    void                        Draw(void);
    void                        Tick(void);
    bool                        ProcessInput(inputEvent_t *ev);

    int16_t                     &SelectedMap(void) { return selectedMap; }

private:
    void                        GetScreenOffsets(float &sx, float &sy);
    void                        DrawSprite(const float sx, const float sy, const byte color,
                                           spriteAnim_t *anim);
    void                        DrawCursor(const int fade);
    void                        DrawArrows(void);
    void                        DrawArrow(const float ax, const float ay, const int pic, const byte color);
    void                        DrawBackground(const int fade);
    void                        DrawTitle(void);
    void                        SetupMatrix(const int zoom);
    const int                   GetFade(void);

    kexTexture                  pic;
    float                       camera_x;
    float                       camera_y;
    int16_t                     selectedMap;
    int                         fadeTime;
    int                         curFadeTime;
    bool                        bFadeIn;
    bool                        bFading;
    spriteAnim_t                *mapCursor;
    spriteAnim_t                *arrows[4];
};

#endif
