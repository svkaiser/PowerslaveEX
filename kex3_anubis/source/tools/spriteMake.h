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

#ifndef __SPRITEMAKE_H__
#define __SPRITEMAKE_H__

class kexImage;

class kexSpriteMake
{
public:
    kexSpriteMake(void);
    ~kexSpriteMake(void);

    void                    GenerateSprite(const char *path, const char *outname,
                                           const int texWidth, const int texHeight);

private:
    void                    GenerateSpriteInfo(const char *outname);
    void                    FindImages(const char *path, kexArray<kexImage*> &images);
    void                    SumTextureSize(kexArray<kexImage*> &images);
    void                    SetBlocks(kexArray<kexImage*> &images, int *outWidth, int *outHeight);
    void                    MakeTexture(kexImage &texture, kexArray<kexImage*> &images);
    bool                    MakeRoomForBlock(const int width, const int height, int *x, int *y);

    typedef struct
    {
        int x;
        int y;
        int w;
        int h;
    } spriteInfo_t;

    kexArray<spriteInfo_t>  spriteInfos;

    int                     *allocBlocks;
    int                     textureWidth;
    int                     textureHeight;
    kexStrList              spriteFiles;
};

#endif
