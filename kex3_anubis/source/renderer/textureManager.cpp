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
// kexTextureManager::Init
//

void kexTextureManager::Init(void)
{
    defaultTexture = Cache("textures/default.tga", TC_CLAMP, TF_LINEAR);
    whiteTexture = Cache("textures/white.tga", TC_CLAMP, TF_LINEAR);
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
