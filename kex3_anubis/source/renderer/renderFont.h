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

#ifndef __RENDERFONT_H__
#define __RENDERFONT_H__

typedef struct
{
    int x;
    int y;
    int w;
    int h;
    int o;
} atlas_t;

class kexTexture;

class kexFont
{
public:
    kexFont(void);
    ~kexFont(void);

    void                        LoadKFont(const char *file);
    void                        Delete(void);
    void                        DrawString(const char *string, float x, float y, float scale,
                                           bool center, byte *rgba1, byte *rgba2);
    void                        DrawString(const char *string, float x, float y, float scale,
                                           bool center, rcolor color);
    void                        DrawString(const char *string, float x, float y, float scale,
                                           bool center);
    float                       StringWidth(const char* string, float scale, int fixedLen);
    float                       StringHeight(const char* string, float scale, int fixedLen);
    
    static kexFont              *Alloc(const char *name);
    static kexFont              *Get(const char *name);

    const bool                  IsLoaded(void) const { return bLoaded; }
    const kexTexture            *Texture(void) { return texture; }

private:
    static kexHashList<kexFont> fontList;
    
    atlas_t                     atlas[256];
    bool                        bLoaded;
    float                       padWidth;
    kexTexture                  *texture;
};

#endif
