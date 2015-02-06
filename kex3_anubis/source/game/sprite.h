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

#ifndef __SPRITE_H__
#define __SPRITE_H__

#include "renderFont.h"

typedef struct
{
    int spriteID;
    atlas_t atlas;
    float u[2];
    float v[2];
} spriteInfo_t;

class kexSprite
{
    friend class kexSpriteManager;
public:
    kexSprite(void);

    void                            Delete(void);
    void                            LoadTexture(void);

    kexTexture                      *Texture(void) { return texture; }
    kexStr                          &TextureFile(void) { return textureFile; }
    kexArray<spriteInfo_t>          &InfoList(void) { return infoList; }

private:
    kexTexture                      *texture;
    kexStr                          textureFile;

    kexArray<spriteInfo_t>          infoList;
};

class kexSpriteManager
{
public:
    kexSpriteManager(void);
    ~kexSpriteManager(void);

    void                            Init(void);
    void                            Shutdown(void);

    kexSprite                       *Get(const char *name) { return spriteList.Find(name); }
    
    kexSprite                       defaultSprite;

private:
    kexHashList<kexSprite>          spriteList;

    kexSprite                       *Load(const char *name);
};

#endif
