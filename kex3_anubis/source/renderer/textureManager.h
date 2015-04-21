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

#ifndef __TEXTURE_MANAGER_H__
#define __TEXTURE_MANAGER_H__

class kexTextureManager
{
public:
    kexTextureManager(void);
    ~kexTextureManager(void);

    void                    Init(void);
    void                    Shutdown(void);

    kexTexture              *Cache(const char *name, texClampMode_t clampMode,
                                   texFilterMode_t filterMode);

    kexTexture              *defaultTexture;
    kexTexture              *whiteTexture;
    kexTexture              *lightTexture;

private:
    void                    CreateWhiteTexture(void);
    void                    CreateDefaultTexture(void);
    void                    CreateLightTexture(void);

    kexHashList<kexTexture> textureList;
};

#endif
