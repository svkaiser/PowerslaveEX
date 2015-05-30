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

#ifndef __RENDERSCREEN_H__
#define __RENDERSCREEN_H__

class kexRenderScreen
{
public:
    typedef enum
    {
        SR_NORMAL       = 0,
        SR_WIDESCREEN,
        SR_NARROWSCREEN
    } screenRatio_t;

    void                SetOrtho(const bool bNoAspectCorrection = false);
    const screenRatio_t GetRatio(void) const;
    void                CoordsToRenderScreenCoords(float &x, float &y);
    void                SetAspectDimentions(float &x, float &y, float &width, float &height);
    void                DrawTexture(kexTexture *texture, const float x, const float y,
                                    byte r, byte g, byte b, byte a,
                                    const float fixedWidth = 0, const float fixedHeight = 0);
    void                DrawTexture(kexTexture *texture, const float x, const float y,
                                    const float fixedWidth = 0, const float fixedHeight = 0);
    void                DrawTexture(const char *name, const float x, const float y,
                                    const float fixedWidth = 0, const float fixedHeight = 0);
    void                DrawStretchPic(kexTexture *texture, const float x, const float y,
                                       const float width, const float height);
    void                DrawStretchPic(kexTexture *texture, const float x, const float y,
                                       const float width, const float height,
                                       byte r, byte g, byte b, byte a);
    void                DrawStretchPic(const char *name, const float x, const float y,
                                       const float width, const float height);
    void                DrawFillPic(kexTexture *texture, const float x, const float y,
                                    const float w, const float h);
    void                DrawQuad(float x, float w, float y, float h,
                                 float tu1, float tu2, float tv1, float tv2,
                                 byte r, byte g, byte b, byte a);
    void                DrawQuad(float x, float w, float y, float h);
    void                DrawQuad(float x, float w, float y, float h, byte r, byte g, byte b, byte a);
    bool                PointOnPic(kexTexture *texture, const float x, const float y,
                                   const float mx, const float my);
    
    static const int    SCREEN_WIDTH;
    static const int    SCREEN_HEIGHT;
};

#endif
