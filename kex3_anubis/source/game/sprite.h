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

class kexSprite
{
    friend class kexSpriteManager;
public:
    kexSprite(void);

    void                            Delete(void);

    atlas_t                         *Atlas(const int idx) { return &infoList[idx].atlas; }
    kexTexture                      *Texture(void) { return texture; }
    kexStr                          &TextureFile(void) { return textureFile; }

private:
    kexTexture                      *texture;
    kexStr                          textureFile;

    typedef struct
    {
        int spriteID;
        atlas_t atlas;
    } spriteInfo_t;

    kexArray<spriteInfo_t>          infoList;
};

class kexSpriteManager
{
public:
    kexSpriteManager(void);
    ~kexSpriteManager(void);

    void                            Init(void);
    void                            Shutdown(void);
    kexSprite                       *Load(const char *name);

private:
    kexHashList<kexSprite>          spriteList;
};

#endif
