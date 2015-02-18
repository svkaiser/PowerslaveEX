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

#ifndef __SPRITEANIM_H__
#define __SPRITEANIM_H__

class kexSprite;
class kexActionDef;

typedef struct
{
    kexSprite   *sprite;
    uint16_t    index;
    int16_t     x;
    int16_t     y;
    bool        bFlipped;
} spriteSet_t;

typedef enum
{
    SFF_FULLBRIGHT      = BIT(1),
    SFF_HASROTATIONS    = BIT(2)
} frameFlags_t;

typedef struct
{
    uint16_t                    delay;
    uint16_t                    flags;
    kexArray<kexActionDef*>     actions;
    kexStr                      nextFrame;
    kexStr                      refireFrame;
    kexArray<spriteSet_t>       spriteSet[8];

    bool                        HasNextFrame(void) { return nextFrame[0] != '-'; }
    bool                        HasRefireFrame(void) { return refireFrame[0] != '-'; }
} spriteFrame_t;

typedef struct
{
    kexStr                      name;
    kexArray<spriteFrame_t>     frames;

    const unsigned int          NumFrames(void) const { return frames.Length(); }
} spriteAnim_t;

class kexSpriteAnimManager
{
public:
    kexSpriteAnimManager(void);
    ~kexSpriteAnimManager(void);

    void                        Init(void);
    void                        Shutdown(void);

    spriteAnim_t                *Get(const char *name) { return spriteAnimList.Find(name); }
    
    spriteAnim_t                defaultAnim;

private:
    kexHashList<spriteAnim_t>   spriteAnimList;

    void                        Load(const char *name);
    void                        ParseFrame(kexLexer *lexer, spriteFrame_t *frame);
    void                        ParseRotation(kexLexer *lexer, spriteFrame_t *frame);
    void                        ParseSpriteSet(kexLexer *lexer, spriteFrame_t *frame, const int rotation);
};

#endif
