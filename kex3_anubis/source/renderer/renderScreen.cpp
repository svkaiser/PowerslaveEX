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
// DESCRIPTION:
//      Simple 2D drawing utilties
//

#include "kexlib.h"
#include "renderMain.h"

static kexRenderScreen renderScreen;
kexRenderScreen *kexRender::cScreen = &renderScreen;

const int kexRenderScreen::SCREEN_WIDTH     = 320;
const int kexRenderScreen::SCREEN_HEIGHT    = 240;

//
// kexRenderScreen::SetOrtho
//

void kexRenderScreen::SetOrtho(const bool bNoAspectCorrection)
{
    float x = 0;
    float y = 0;
    float w = (float)SCREEN_WIDTH;
    float h = (float)SCREEN_HEIGHT;
    
    if(bNoAspectCorrection == false)
    {
        SetAspectDimentions(x, y, w, h);
    }

    kexRender::cBackend->SetOrtho(-x, -y, (float)SCREEN_WIDTH+x, (float)SCREEN_HEIGHT+y);
}

//
// kexRenderScreen::CoordsToRenderScreenCoords
//

void kexRenderScreen::CoordsToRenderScreenCoords(float &x, float &y)
{
    float sx = 0;
    float sy = 0;
    float w = (float)SCREEN_WIDTH;
    float h = (float)SCREEN_HEIGHT;
    float sw = w;
    float sh = h;

    SetAspectDimentions(sx, sy, sw, sh);

    x = ((x / (float)kex::cSystem->VideoWidth()) * (w + (w - sw))) - sx;
    y = ((y / (float)kex::cSystem->VideoHeight()) * (h + (h - sh))) - sy;
}

//
// kexRenderScreen::PointOnPic
//

bool kexRenderScreen::PointOnPic(kexTexture *texture, const float x, const float y,
                                 const float mx, const float my)
{
    return (mx >= x && mx <= x + texture->OriginalWidth() &&
            my >= y && my <= y + texture->OriginalHeight());
}

//
// kexRenderScreen::GetRatio
//

const kexRenderScreen::screenRatio_t kexRenderScreen::GetRatio(void) const
{
    int aspect = (kex::cSystem->VideoWidth() << 16) / kex::cSystem->VideoHeight();
    const int dims = (4 << 16) / 3;
    
    if(aspect == dims)
    {
        return SR_NORMAL;
    }
    else if(aspect > dims)
    {
        return SR_WIDESCREEN;
    }
    
    return SR_NARROWSCREEN;
}

//
// kexRenderScreen::SetAspectDimentions
//
// Sets up the dimentions that confines to the aspect ratio
//

void kexRenderScreen::SetAspectDimentions(float &x, float &y, float &width, float &height)
{
    int aspect;
    float ratio, aspectwidth;
    const int dims = (4 << 16) / 3;
    
    aspect = (kex::cSystem->VideoWidth() << 16) / kex::cSystem->VideoHeight();
    
    if(aspect == dims) // nominal
    {
        return;
    }
    else if(aspect > dims) // widescreen (pillarboxed)
    {
        ratio = (4.0f / (3.0f * (aspect / 65536.0f)));
        aspectwidth = (SCREEN_WIDTH - (SCREEN_WIDTH * ratio)) / 2;
        
        x = (x * ratio) + aspectwidth;
        width = (width * ratio);
    }
    else // narrow (letterboxed)
    {
        float fratio;
        
        ratio = (3.0f / (4.0f * (aspect / 65536.0f)));
        aspectwidth = SCREEN_WIDTH * ratio;
        fratio = aspectwidth / (float)SCREEN_HEIGHT;
        
        y = y * fratio + ((SCREEN_HEIGHT - aspectwidth) / 2);
        height = height * fratio;
    }
}

//
// kexRenderScreen::DrawTexture
//
// Draws a textured quad that's confined to the aspect ratio
//

void kexRenderScreen::DrawTexture(kexTexture *texture, const float x, const float y,
                                  byte r, byte g, byte b, byte a,
                                  const float fixedWidth, const float fixedHeight)
{
    float texwidth;
    float texheight;
    float tex_x;
    float tex_y;
    
    texwidth = fixedWidth > 0 ? (float)fixedWidth : (float)texture->Width();
    texheight = fixedHeight > 0 ? (float)fixedHeight : (float)texture->Height();
    tex_x = x;
    tex_y = y;
    
    texture->Bind();
    DrawQuad(tex_x, texwidth, tex_y, texheight, r, g, b, a);
}

//
// kexRenderScreen::DrawTexture
//
// Draws a textured quad that's confined to the aspect ratio
//

void kexRenderScreen::DrawTexture(kexTexture *texture, const float x, const float y,
                                  const float fixedWidth, const float fixedHeight)
{
    DrawTexture(texture, x, y, 255, 255, 255, 255, fixedWidth, fixedHeight);
}

//
// kexRenderScreen::DrawTexture
//
// Draws a textured quad that's confined to the aspect ratio
//

void kexRenderScreen::DrawTexture(const char *name, const float x, const float y,
                                  const float fixedWidth, const float fixedHeight)
{
    kexTexture *texture = kexRender::cTextures->Cache(name, TC_CLAMP, TF_NEAREST);
    DrawTexture(texture, x, y, fixedWidth, fixedHeight);
}

//
// kexRenderScreen::DrawStretchPic
//
// Simply draws a textured quad without being
// resized to match the aspect ratio
//

void kexRenderScreen::DrawStretchPic(kexTexture *texture, const float x, const float y,
                                     const float width, const float height)
{
    float sx = 0;
    float sy = 0;
    float sw = (float)SCREEN_WIDTH;
    float sh = (float)SCREEN_HEIGHT;

    SetAspectDimentions(sx, sy, sw, sh);

    sw = (float)SCREEN_WIDTH - sw;
    sh = (float)SCREEN_HEIGHT - sh;
    
    texture->Bind();
    DrawQuad(x-sx, width+sw, y-sy, height+sh);
}

//
// kexRenderScreen::DrawStretchPic
//
// Simply draws a textured quad without being
// resized to match the aspect ratio
//

void kexRenderScreen::DrawStretchPic(const char *name, const float x, const float y,
                                     const float width, const float height)
{
    kexTexture *texture = kexRender::cTextures->Cache(name, TC_CLAMP, TF_NEAREST);
    DrawStretchPic(texture, x, y, width, height);
}

//
// kexRenderScreen::DrawStretchPic
//
// Simply draws a textured quad without being
// resized to match the aspect ratio
//

void kexRenderScreen::DrawStretchPic(kexTexture *texture, const float x, const float y,
                                     const float width, const float height,
                                     byte r, byte g, byte b, byte a)
{
    float sx = 0;
    float sy = 0;
    float sw = (float)SCREEN_WIDTH;
    float sh = (float)SCREEN_HEIGHT;

    SetAspectDimentions(sx, sy, sw, sh);

    sw = (float)SCREEN_WIDTH - sw;
    sh = (float)SCREEN_HEIGHT - sh;
    
    texture->Bind();
    DrawQuad(x-sx, width+sw, y-sy, height+sh, 0, 1, 0, 1, r, g, b, a);
}

//
// kexRenderScreen::DrawFillPic
//

void kexRenderScreen::DrawFillPic(kexTexture *texture, const float x, const float y,
                                  const float w, const float h)
{
    float texwidth;
    float texheight;
    float tex_x;
    float tex_y;
    
    texwidth = w;
    texheight = h;
    tex_x = x;
    tex_y = y;
    
    texture->Bind();
    DrawQuad(tex_x, texwidth, tex_y, texheight, 0,
             texwidth / (float)texture->Width(), 0,
             texheight / (float)texture->Height(),
             255, 255, 255, 255);
}

//
// kexRenderScreen::DrawQuad
//

void kexRenderScreen::DrawQuad(float x, float w, float y, float h,
                               float tu1, float tu2, float tv1, float tv2,
                               byte r, byte g, byte b, byte a)
{
    kexRender::cVertList->BindDrawPointers();
    kexRender::cVertList->AddQuad(x, y, 0, w, h, tu1, tv1, tu2, tv2, r, g, b, a);
    kexRender::cVertList->DrawElements();
}

//
// kexRenderScreen::DrawQuad
//

void kexRenderScreen::DrawQuad(float x, float w, float y, float h)
{
    DrawQuad(x, w, y, h, 0, 1, 0, 1, 255, 255, 255, 255);
}

//
// kexRenderScreen::DrawQuad
//

void kexRenderScreen::DrawQuad(float x, float w, float y, float h, byte r, byte g, byte b, byte a)
{
    DrawQuad(x, w, y, h, 0, 1, 0, 1, r, g, b, a);
}
