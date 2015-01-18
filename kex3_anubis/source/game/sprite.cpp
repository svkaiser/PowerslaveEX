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
//      Sprite Object and Manager
//

#include "kexlib.h"
#include "game.h"
#include "renderMain.h"
#include "sprite.h"

//-----------------------------------------------------------------------------
//
// kexSprite
//
//-----------------------------------------------------------------------------

//
// kexSprite::kexSprite
//

kexSprite::kexSprite(void)
{
    this->texture = NULL;
}

//
// kexSprite::Delete
//

void kexSprite::Delete(void)
{
    infoList.Empty();
}

//-----------------------------------------------------------------------------
//
// kexSpriteManager
//
//-----------------------------------------------------------------------------

//
// kexSpriteManager::kexSpriteManager
//

kexSpriteManager::kexSpriteManager(void)
{
}

//
// kexSpriteManager::~kexSpriteManager
//

kexSpriteManager::~kexSpriteManager(void)
{
}

//
// kexSpriteManager::Init
//

void kexSpriteManager::Init(void)
{
    kexStrList list;

    kex::cPakFiles->GetMatchingFiles(list, "sprites/");

    for(unsigned int i = 0; i < list.Length(); ++i)
    {
        if(list[i].IndexOf("_sprite.\0") == -1)
        {
            continue;
        }
        
        Load(list[i].c_str());
    }
}

//
// kexSpriteManager::Shutdown
//

void kexSpriteManager::Shutdown(void)
{
    for(int i = 0; i < MAX_HASH; i++)
    {
        for(kexSprite *spr = spriteList.GetData(i); spr; spr = spriteList.Next())
        {
            spr->Delete();
        }
    }
}

//
// kexSpriteManager::Load
//

kexSprite *kexSpriteManager::Load(const char *name)
{
    kexSprite *sprite;

    if(!(sprite = spriteList.Find(name)))
    {
        kexLexer *lexer;

        if(!(lexer = kex::cParser->Open(name)))
        {
            kex::cSystem->Warning("kexSpriteManager::Load: %s not found\n", name);
            return NULL;
        }

        sprite = spriteList.Add(name);

        while(lexer->CheckState())
        {
            lexer->Find();

            if(lexer->TokenType() != TK_IDENIFIER)
            {
                continue;
            }

            if(lexer->Matches("texture"))
            {
                lexer->GetString();
                sprite->texture = kexRender::cTextures->defaultTexture;
                sprite->textureFile = lexer->StringToken();
            }
            else if(lexer->Matches("sprite"))
            {
                kexSprite::spriteInfo_t info;

                info.spriteID = lexer->GetNumber();
                info.atlas.x = lexer->GetNumber();
                info.atlas.y = lexer->GetNumber();
                info.atlas.w = lexer->GetNumber();
                info.atlas.h = lexer->GetNumber();

                sprite->infoList.Push(info);
            }
        }

        // we're done with the file
        kex::cParser->Close();
    }

    return sprite;
}
