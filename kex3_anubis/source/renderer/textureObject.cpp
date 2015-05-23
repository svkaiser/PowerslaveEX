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
//      Texture object class. Handles all opengl
//      texture binding and uploading as well as
//      all RGB(A) manipulations such as scaling,
//      image flipping, etc
//

#include "kexlib.h"
#include "renderMain.h"

kexCvar cvarGLFilter("gl_filter", CVF_INT|CVF_CONFIG, "0", "Texture filter mode");
kexCvar cvarGLAnisotropic("gl_anisotropic", CVF_INT|CVF_CONFIG, "0", "TODO");

kexHeapBlock kexTexture::hb_texture("texture", false, NULL, NULL);

//
// kexTexture::kexTexture
//

kexTexture::kexTexture(void)
{
    this->width = 0;
    this->height = 0;
    this->origwidth = 0;
    this->origheight = 0;
    this->clampMode = TC_CLAMP;
    this->filterMode = TF_LINEAR;
    this->colorMode = TCR_RGBA;
    this->texid = 0;
    this->bLoaded = false;
    this->next = NULL;
}

//
// kexTexture::~kexTexture
//

kexTexture::~kexTexture(void)
{
    Delete();
}

//
// kexTexture::SetParameters
//

void kexTexture::SetParameters(void)
{
    unsigned int clamp;
    unsigned int filter;

    switch(clampMode)
    {
    case TC_CLAMP:
        clamp = GL_CLAMP_TO_EDGE;
        break;

    case TC_REPEAT:
        clamp = GL_REPEAT;
        break;

    case TC_MIRRORED:
        clamp = GL_MIRRORED_REPEAT;
        break;

    default:
        return;
    }

    switch(filterMode)
    {
    case TF_LINEAR:
        filter = GL_LINEAR;
        break;

    case TF_NEAREST:
        filter = GL_NEAREST;
        break;

    default:
        return;
    }

    dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, clamp);
    dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, clamp);
    dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
    dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);

    if(has_GL_EXT_texture_filter_anisotropic)
    {
        if(cvarGLAnisotropic.GetInt())
        {
            dglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, kexRender::cBackend->MaxAnisotropic());
        }
    }
}

//
// kexTexture::ChangeParameters
//

void kexTexture::ChangeParameters(const texClampMode_t clamp, const texFilterMode_t filter)
{
    if(clampMode == clamp && filterMode == filter)
    {
        return;
    }

    clampMode = clamp;
    filterMode = filter;

    SetParameters();
}

//
// kexTexture::LoadFromFile
//

void kexTexture::LoadFromFile(const char *file, const texClampMode_t clamp, const texFilterMode_t filter)
{
    kexImage image;

    filePath = file;
    image.LoadFromFile(file);

    origwidth = image.OriginalWidth();
    origheight = image.OriginalHeight();
    width = image.Width();
    height = image.Height();
    colorMode = image.ColorMode();

    Upload(image, clamp, filter);
}

//
// kexTexture::Upload
//

void kexTexture::Upload(const byte *data, texClampMode_t clamp, texFilterMode_t filter)
{
    if(data == NULL)
    {
        return;
    }

    clampMode = clamp;
    filterMode = filter;

    if(!kexRender::cBackend->IsInitialized())
    {
        return;
    }

    dglGenTextures(1, &texid);

    if(texid == 0)
    {
        // renderer is not initialized yet
        return;
    }

    dglBindTexture(GL_TEXTURE_2D, texid);

    dglTexImage2D(
        GL_TEXTURE_2D,
        0,
        (colorMode == TCR_RGBA) ? GL_RGBA8 : GL_RGB8,
        width,
        height,
        0,
        (colorMode == TCR_RGBA) ? GL_RGBA : GL_RGB,
        GL_UNSIGNED_BYTE,
        data);

    SetParameters();

    bLoaded = true;
    dglBindTexture(GL_TEXTURE_2D, 0);
}

//
// kexTexture::Upload
//

void kexTexture::Upload(kexImage &image, texClampMode_t clamp, texFilterMode_t filter)
{
    colorMode = image.ColorMode();
    Upload(image.Data(), clamp, filter);
}

//
// kexTexture::Bind
//

void kexTexture::Bind(void)
{
    kexRenderBackend::glState_t *state = &kexRender::cBackend->glState;
    dtexture tid = texid;

    if(bLoaded == false)
    {
        // we may have attempted to cache this texture before the renderer was
        // initialized so try reloading it
        LoadFromFile(filePath, clampMode, filterMode);
        tid = texid;
    }

    int unit = state->currentUnit;
    dtexture currentTexture = state->textureUnits[unit].currentTexture;

    if(tid == currentTexture)
    {
        return;
    }

    dglBindTexture(GL_TEXTURE_2D, tid);

    state->textureUnits[unit].currentTexture = tid;
    state->numTextureBinds++;
}

//
// kexTexture::Unbind
//

void kexTexture::Unbind(void)
{
    int unit = kexRender::cBackend->glState.currentUnit;
    kexRender::cBackend->glState.textureUnits[unit].currentTexture = 0;

    dglBindTexture(GL_TEXTURE_2D, 0);
}

//
// kexTexture::Update
//

void kexTexture::Update(byte *data)
{
    kexRenderBackend::glState_t *state = &kexRender::cBackend->glState;

    if(texid == 0)
    {
        return;
    }

    if(state->textureUnits[state->currentUnit].currentTexture != texid)
    {
        return;
    }

    dglTexSubImage2D(
        GL_TEXTURE_2D,
        0,
        0,
        0,
        width,
        height,
        (colorMode == TCR_RGBA) ? GL_RGBA : GL_RGB,
        GL_UNSIGNED_BYTE,
        data);
}

//
// kexTexture::BindFrameBuffer
//

void kexTexture::BindFrameBuffer(const bool bReadBuffer)
{
    kexRenderBackend::glState_t *state = &kexRender::cBackend->glState;

    if(!kexRender::cBackend->IsInitialized())
    {
        return;
    }

    if(bLoaded == false)
    {
        dglGenTextures(1, &texid);
        bLoaded = true;

        if(texid == 0)
        {
            return;
        }
    }

    int unit = state->currentUnit;
    dtexture currentTexture = state->textureUnits[unit].currentTexture;

    if(texid != currentTexture)
    {
        dglBindTexture(GL_TEXTURE_2D, texid);
        state->textureUnits[unit].currentTexture = texid;
    }

    if(bReadBuffer == false)
    {
        return;
    }

    dglReadBuffer(GL_BACK);

    origwidth   = kex::cSystem->VideoWidth();
    origheight  = kex::cSystem->VideoHeight();
    width       = origwidth;
    height      = origheight;

    dglCopyTexImage2D(GL_TEXTURE_2D,
                      0,
                      GL_RGBA8,
                      0,
                      0,
                      origwidth,
                      origheight,
                      0);
    SetParameters();
}

//
// kexTexture::BindDepthBuffer
//

void kexTexture::BindDepthBuffer(const bool bReadDepth)
{
    kexRenderBackend::glState_t *state = &kexRender::cBackend->glState;

    if(!kexRender::cBackend->IsInitialized())
    {
        return;
    }

    if(bLoaded == false)
    {
        dglGenTextures(1, &texid);
        bLoaded = true;

        if(texid == 0)
        {
            return;
        }
    }

    int unit = state->currentUnit;
    dtexture currentTexture = state->textureUnits[unit].currentTexture;

    if(texid != currentTexture)
    {
        dglBindTexture(GL_TEXTURE_2D, texid);
        state->textureUnits[unit].currentTexture = texid;
    }

    if(bReadDepth == false)
    {
        return;
    }

    origwidth   = kex::cSystem->VideoWidth();
    origheight  = kex::cSystem->VideoHeight();
    width       = origwidth;
    height      = origheight;

    dglCopyTexImage2D(GL_TEXTURE_2D,
                      0,
                      GL_DEPTH_COMPONENT,
                      0,
                      0,
                      origwidth,
                      origheight,
                      0);
    SetParameters();
    dglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
}

//
// kexTexture::Delete
//

void kexTexture::Delete(void)
{
    if(texid == 0 || !bLoaded)
    {
        return;
    }

    dglDeleteTextures(1, &texid);
    texid = 0;
    bLoaded = false;
}
