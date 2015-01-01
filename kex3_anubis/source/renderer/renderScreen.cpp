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

void kexRenderScreen::SetOrtho(void)
{
    kexRender::cBackend->SetOrtho((float)SCREEN_WIDTH, (float)SCREEN_HEIGHT);
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
        width = (width * ratio) - aspectwidth;
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
                                  const int fixedWidth, const int fixedHeight)
{
    float texwidth;
    float texheight;
    float tex_x;
    float tex_y;
    
    texwidth = fixedWidth > 0 ? (float)fixedWidth : (float)texture->Width();
    texheight = fixedHeight > 0 ? (float)fixedHeight : (float)texture->Height();
    tex_x = x;
    tex_y = y;
    
    SetAspectDimentions(tex_x, tex_y, texwidth, texheight);
    
    kexRender::cBackend->SetBlend(GLSRC_SRC_ALPHA, GLDST_ONE_MINUS_SRC_ALPHA);
    kexRender::cBackend->SetState(GLSTATE_CULL, true);
    kexRender::cBackend->SetState(GLSTATE_BLEND, true);
    kexRender::cBackend->SetState(GLSTATE_ALPHATEST, false);
    kexRender::cBackend->SetState(GLSTATE_DEPTHTEST, false);
    kexRender::cBackend->SetCull(GLCULL_BACK);
    
    texture->Bind();
    DrawQuad(tex_x, texwidth, tex_y, texheight);
}

//
// kexRenderScreen::DrawTexture
//
// Draws a textured quad that's confined to the aspect ratio
//

void kexRenderScreen::DrawTexture(const char *name, const float x, const float y,
                                  const int fixedWidth, const int fixedHeight)
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
    kexRender::cBackend->SetBlend(GLSRC_SRC_ALPHA, GLDST_ONE_MINUS_SRC_ALPHA);
    kexRender::cBackend->SetState(GLSTATE_CULL, true);
    kexRender::cBackend->SetState(GLSTATE_BLEND, true);
    kexRender::cBackend->SetState(GLSTATE_ALPHATEST, false);
    kexRender::cBackend->SetState(GLSTATE_DEPTHTEST, false);
    kexRender::cBackend->SetCull(GLCULL_BACK);
    
    texture->Bind();
    DrawQuad(x, width, y, height);
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
// kexRenderScreen::DrawQuad
//

void kexRenderScreen::DrawQuad(float x, float w, float y, float h,
                               float tu1, float tu2, float tv1, float tv2,
                               byte r, byte g, byte b, byte a)
{
    dglBegin(GL_QUADS);
    dglColor4ub(r, g, b, a);
    dglTexCoord2f(tu1, tv1); dglVertex2f(x,   y);
    dglTexCoord2f(tu1, tv2); dglVertex2f(x,   y+h);
    dglTexCoord2f(tu2, tv2); dglVertex2f(x+w, y+h);
    dglTexCoord2f(tu2, tv1); dglVertex2f(x+w, y);
    dglEnd();
}

//
// kexRenderScreen::DrawQuad
//

void kexRenderScreen::DrawQuad(float x, float w, float y, float h)
{
    DrawQuad(x, w, y, h, 0, 1, 0, 1, 255, 255, 255, 255);
}
