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
//      Sprite utilities
//

#include "kexlib.h"
#include "renderMain.h"
#include "spriteMake.h"

static kexSpriteMake spriteMakeLocal;

//
// makesprite
//

COMMAND(makesprite)
{
    int tw = 0, th = 0;

    if(kex::cCommands->GetArgc() < 3)
    {
        return;
    }

    if(kex::cCommands->GetArgc() >= 4)
    {
        tw = kexMath::RoundPowerOfTwo(atoi(kex::cCommands->GetArgv(3)));
    }
    if(kex::cCommands->GetArgc() >= 5)
    {
        th = kexMath::RoundPowerOfTwo(atoi(kex::cCommands->GetArgv(4)));
    }

    spriteMakeLocal.GenerateSprite(kex::cCommands->GetArgv(1),
                                   kex::cCommands->GetArgv(2),
                                   tw, th);
}

//
// kexSpriteMake::kexSpriteMake
//

kexSpriteMake::kexSpriteMake(void)
{
    this->textureWidth = 0;
    this->textureHeight = 0;
    this->allocBlocks = NULL;
}

//
// kexSpriteMake::~kexSpriteMake
//

kexSpriteMake::~kexSpriteMake(void)
{
}

//
// kexSpriteMake::GenerateSprite
//

void kexSpriteMake::GenerateSprite(const char *path, const char *outname,
                                   const int texWidth, const int texHeight)
{
    kexArray<kexImage*> images;
    kexBinFile tgaFile;
    kexStr outPath;
    int outWidth, outHeight;
    char endChr;

    outPath = path;
    outPath.NormalizeSlashes();

    endChr = outPath.c_str()[outPath.Length() - 1];

    if(endChr != DIR_SEPARATOR)
    {
        outPath += DIR_SEPARATOR;
    }

    // search for sprite images
    FindImages(outPath.c_str(), images);

    if(images.Length() <= 0)
    {
        kex::cSystem->Warning("kexSpriteMake::GenerateSprite: no sprites found\n");
        return;
    }

    // determine the size of the sprite atlas texture
    SumTextureSize(images);

    if(texWidth > 0)
    {
        textureWidth = texWidth;
    }
    if(texHeight > 0)
    {
        textureHeight = texHeight;
    }

    // determine where each sprite will be placed in the atlas
    SetBlocks(images, &outWidth, &outHeight);

    // create atlas texture
    kexImage output(outWidth, outHeight, TCR_RGBA);
    MakeTexture(output, images);

    kexStr outFile = kexStr::Format("%s/%s.png", kex::cvarBasePath.GetValue(), outname);
    outFile.NormalizeSlashes();

    // save atlas texture
    tgaFile.Create(outFile.c_str());
    output.FlipVertical();
    output.WritePNG(tgaFile);
    tgaFile.Close();

    kex::cSystem->Printf("wrote %s\n", outFile.c_str());

    // generate sprite information
    GenerateSpriteInfo(outname);

    spriteFiles.Empty();
    spriteInfos.Empty();

    for(unsigned int i = 0; i < images.Length(); i++)
    {
        delete images[i];
    }
}

//
// kexSpriteMake::GenerateSpriteInfo
//

void kexSpriteMake::GenerateSpriteInfo(const char *outname)
{
    kexStr buffer;

    assert(spriteInfos.Length() == spriteFiles.Length());
    buffer += kexStr::Format("texture \"%s.png\"\n\n", outname);

    for(unsigned int i = 0; i < spriteInfos.Length(); i++)
    {
        buffer += kexStr::Format("// %s\n", spriteFiles[i].c_str());
        buffer += kexStr::Format("sprite %i %i %i %i %i\n\n", i,
                                 spriteInfos[i].x, spriteInfos[i].y,
                                 spriteInfos[i].w, spriteInfos[i].h);
    }

    kexStr outFile = kexStr::Format("%s/%s_sprite.txt", kex::cvarBasePath.GetValue(), outname);
    outFile.NormalizeSlashes();

    buffer.WriteToFile(outFile.c_str());
    kex::cSystem->Printf("wrote %s\n", outFile.c_str());
}

//
// kexSpriteMake::FindImages
//

void kexSpriteMake::FindImages(const char *path, kexArray<kexImage*> &images)
{
    kex::cPakFiles->GetMatchingFiles(spriteFiles, path);

    for(unsigned int i = 0; i < spriteFiles.Length(); i++)
    {
        if(spriteFiles[i].IndexOf(".tga\0") == -1 &&
            spriteFiles[i].IndexOf(".png\0") == -1)
        {
            continue;
        }

        kexImage *sprite = new kexImage;
        sprite->LoadFromFile(spriteFiles[i].c_str());

        images.Push(sprite);
    }
}

//
// kexSpriteMake::SumTextureSize
//

void kexSpriteMake::SumTextureSize(kexArray<kexImage*> &images)
{
    int sum = 0;
    int bit, mod;

    for(unsigned int i = 0; i < images.Length(); i++)
    {
        sum += (images[i]->Width() * images[i]->Height());
    }

    assert(sum != 0);

    // hopefully, the texture shouldn't need to be larger than 2048x2048...
    for(int i = 1; i <= 12; i++)
    {
        bit = BIT(i);
        mod = (bit * bit);

        if(mod % sum != mod)
        {
            textureWidth = bit << 1;
            textureHeight = bit << 1;
            break;
        }
    }
}

//
// kexSpriteMake::MakeRoomForBlock
//

bool kexSpriteMake::MakeRoomForBlock(const int width, const int height, int *x, int *y)
{
    int i;
    int j;
    int bestRow1;
    int bestRow2;

    if(allocBlocks == NULL)
    {
        return false;
    }

    bestRow1 = textureHeight;

    for(i = 0; i <= textureWidth - width; i++)
    {
        bestRow2 = 0;

        for(j = 0; j < width; j++)
        {
            if(allocBlocks[i + j] >= bestRow1)
            {
                break;
            }

            if(allocBlocks[i + j] > bestRow2)
            {
                bestRow2 = allocBlocks[i + j];
            }
        }

        // found a free block
        if(j == width)
        {
            *x = i;
            *y = bestRow1 = bestRow2;
        }
    }

    if(bestRow1 + height > textureHeight)
    {
        // no room
        return false;
    }

    for(i = 0; i < width; i++)
    {
        // store row offset
        allocBlocks[*x + i] = bestRow1 + height;
    }

    return true;
}

//
// kexSpriteMake::SetBlocks
//

void kexSpriteMake::SetBlocks(kexArray<kexImage*> &images, int *outWidth, int *outHeight)
{
    int x, y;
    int hiW = -1;
    int hiH = -1;

    *outWidth = textureWidth;
    *outHeight = textureHeight;

    allocBlocks = (int*)Mem_Calloc(sizeof(int) * textureWidth, hb_static);

    for(unsigned int i = 0; i < images.Length(); i++)
    {
        kexImage *image = images[i];
        spriteInfo_t info;
        x = y = 0;

        MakeRoomForBlock(image->OriginalWidth(), image->OriginalHeight(), &x, &y);

        info.x = x;
        info.y = y;
        info.w = image->OriginalWidth();
        info.h = image->OriginalHeight();

        if(x + info.w > hiW) { hiW = x + info.w; }
        if(y + info.h > hiH) { hiH = y + info.h; }

        spriteInfos.Push(info);
    }

    // crop atlas texture if there's a lot of unused space left
    if(hiW > 0 && hiH > 0)
    {
        hiW = kexMath::RoundPowerOfTwo(hiW);
        hiH = kexMath::RoundPowerOfTwo(hiH);

        if(hiW < textureWidth) { *outWidth = hiW; }
        if(hiH < textureHeight) { *outHeight = hiH; }
    }

    textureWidth = *outWidth;
    textureHeight = *outHeight;

    Mem_Free(allocBlocks);
    allocBlocks = NULL;
}

//
// kexSpriteMake::MakeTexture
//

void kexSpriteMake::MakeTexture(kexImage &texture, kexArray<kexImage*> &images)
{
    assert(spriteInfos.Length() == images.Length());

    for(unsigned int i = 0; i < images.Length(); i++)
    {
        kexImage *image = images[i];
        spriteInfo_t *info = &spriteInfos[i];

        texture.Blit(*image, info->x, info->y);
    }
}
