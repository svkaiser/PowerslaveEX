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

#ifndef __TEXTURE_OBJ_H__
#define __TEXTURE_OBJ_H__

#include "image.h"

typedef enum
{
    TC_CLAMP    = 0,
    TC_REPEAT,
    TC_MIRRORED
} texClampMode_t;

typedef enum
{
    TF_LINEAR   = 0,
    TF_NEAREST
} texFilterMode_t;

class kexTexture
{
    friend class kexTextureManager;

public:
    kexTexture(void);
    ~kexTexture(void);

    void                    Upload(const byte *data, texClampMode_t clamp, texFilterMode_t filter);
    void                    Upload(kexImage &image, texClampMode_t clamp, texFilterMode_t filter);
    void                    SetParameters(void);
    void                    ChangeParameters(const texClampMode_t clamp, const texFilterMode_t filter);
    void                    LoadFromFile(const char *file, const texClampMode_t clamp, const texFilterMode_t filter);
    void                    Bind(void);
    void                    Unbind(void);
    void                    Update(byte *data);
    void                    BindFrameBuffer(const bool bReadBuffer = true);
    void                    BindDepthBuffer(const bool bReadDepth = true);
    void                    Delete(void);

    int                     &Width(void) { return width; }
    int                     &Height(void) { return height; }
    int                     &OriginalWidth(void) { return origwidth; }
    int                     &OriginalHeight(void) { return origheight; }
    dtexture                *TextureID(void) { return &texid; }
    bool                    IsLoaded(void) const { return bLoaded; }
    texClampMode_t          GetClampMode(void) { return clampMode; }
    void                    SetClampMode(texClampMode_t cm) { clampMode = cm; }
    texFilterMode_t         GetFilterMode(void) { return filterMode; }
    void                    SetFilterMode(texFilterMode_t fm) { filterMode = fm; }
    void                    SetColorMode(texColorMode_t cm) { colorMode = cm; }

    kexStr                  filePath;
    kexTexture              *next;

    static kexHeapBlock     hb_texture;

private:
    int                     width;
    int                     height;
    int                     origwidth;
    int                     origheight;
    texClampMode_t          clampMode;
    texFilterMode_t         filterMode;
    texColorMode_t          colorMode;
    dtexture                texid;
    bool                    bLoaded;
};

#endif
