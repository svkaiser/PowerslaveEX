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

#include "renderMain.h"

static kexTextureManager textureManagerLocal;
kexTextureManager *kexRender::cTextures = &textureManagerLocal;

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
// kexTextureManager::CreateLightTexture
//

void kexTextureManager::CreateLightTexture(void)
{
    kexBinFile lightFile;
    byte rgb[64*64*4];
    memset(rgb, 0, 64*64*4);

    lightFile.Open("gfx/light.bin");
    lightTexture = textureList.Add("_light", kexTexture::hb_texture);

    lightTexture->colorMode = TCR_RGBA;
    lightTexture->origwidth = 64;
    lightTexture->origheight = 64;
    lightTexture->width = 64;
    lightTexture->height = 64;

    for(int h = 0; h < 64; ++h)
    {
        for(int w = 0; w < 64; ++w)
        {
            rgb[(64 * h + w) * 4 + 0] = lightFile.Read8();
            rgb[(64 * h + w) * 4 + 1] = lightFile.Read8();
            rgb[(64 * h + w) * 4 + 2] = lightFile.Read8();
            rgb[(64 * h + w) * 4 + 3] = rgb[(64 * h + w) * 4 + 2];
        }
    }

    lightTexture->Upload(rgb, TC_CLAMP, TF_LINEAR);
}

//
// kexTextureManager::Init
//

void kexTextureManager::Init(void)
{
    CreateWhiteTexture();
    CreateDefaultTexture();
    CreateLightTexture();
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
