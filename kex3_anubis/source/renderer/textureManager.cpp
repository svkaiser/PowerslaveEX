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
//      Texture manager
//

#include "kexlib.h"

static kexTextureManager textureManagerLocal;
kexTextureManager *kex::render::cTextures = &textureManagerLocal;

//
// kexTextureManager::kexTextureManager
//

kexTextureManager::kexTextureManager(void)
{
    this->defaultTexture = NULL;
    this->whiteTexture = NULL;
}

//
// kexTextureManager::~kexTextureManager
//

kexTextureManager::~kexTextureManager(void)
{
}

//
// kexTextureManager::CreateWhiteTexture
//

void kexTextureManager::CreateWhiteTexture(void)
{
    byte rgb[48]; // 4x4 RGB texture
    memset(rgb, 0xff, 48);

    whiteTexture = textureList.Add("_white", kexTexture::hb_texture);

    whiteTexture->colorMode = TCR_RGB;
    whiteTexture->origwidth = 4;
    whiteTexture->origheight = 4;
    whiteTexture->width = 4;
    whiteTexture->height = 4;

    whiteTexture->Upload(rgb, TC_CLAMP, TF_NEAREST);
}

//
// kexTextureManager::CreateDefaultTexture
//

void kexTextureManager::CreateDefaultTexture(void)
{
    byte rgb[64*64*3];
    memset(rgb, 0xff, 64*64*3);

    for(int h = 0; h < 32; ++h)
    {
        for(int w = 0; w < 32; ++w)
        {
            rgb[(64 * h + w) * 3 + 0] = 0;
            rgb[(64 * h + w) * 3 + 1] = 0xff;
            rgb[(64 * h + w) * 3 + 2] = 0xff;

            rgb[(64 * (h+32) + (w+32)) * 3 + 0] = 0;
            rgb[(64 * (h+32) + (w+32)) * 3 + 1] = 0xff;
            rgb[(64 * (h+32) + (w+32)) * 3 + 2] = 0xff;
        }
    }

    defaultTexture = textureList.Add("_default", kexTexture::hb_texture);

    defaultTexture->colorMode = TCR_RGB;
    defaultTexture->origwidth = 64;
    defaultTexture->origheight = 64;
    defaultTexture->width = 64;
    defaultTexture->height = 64;

    defaultTexture->Upload(rgb, TC_CLAMP, TF_NEAREST);
}

//
// kexTextureManager::Init
//

void kexTextureManager::Init(void)
{
    CreateWhiteTexture();
    CreateDefaultTexture();
}

//
// kexTextureManager::Shutdown
//

void kexTextureManager::Shutdown(void)
{
    for(int i = 0; i < MAX_HASH; i++)
    {
        for(kexTexture *texture = textureList.GetData(i); texture; texture = textureList.Next())
        {
            texture->Delete();
        }
    }

    // do last round of texture flushing to make sure we freed everything
    Mem_Purge(kexTexture::hb_texture);
}

//
// kexTextureManager::Cache
//

kexTexture *kexTextureManager::Cache(const char *name, texClampMode_t clampMode,
                                     texFilterMode_t filterMode)
{
    kexTexture *texture = NULL;

    if(name == NULL || name[0] == 0)
    {
        return NULL;
    }

    if(!(texture = textureList.Find(name)))
    {
        texture = textureList.Add(name, kexTexture::hb_texture);
        texture->LoadFromFile(name, clampMode, filterMode);
    }

    return texture;
}
