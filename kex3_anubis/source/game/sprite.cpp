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
#include "renderMain.h"
#include "game.h"

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

//
// kexSprite::LoadTexture
//

void kexSprite::LoadTexture(void)
{
    if(texture == NULL)
    {
        if(!(texture = kexRender::cTextures->Cache(textureFile.c_str(), TC_CLAMP, TF_NEAREST)))
        {
            texture = kexRender::cTextures->defaultTexture;
        }
    }
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
    spriteInfo_t *info;

    kex::cPakFiles->GetMatchingFiles(list, "sprites/");

    for(unsigned int i = 0; i < list.Length(); ++i)
    {
        if(list[i].IndexOf("_sprite.\0") == -1)
        {
            continue;
        }
        
        Load(list[i].c_str());
    }
    
    defaultSprite.texture = kexRender::cTextures->defaultTexture;
    
    info = defaultSprite.InfoList().Grow();
    info->u[0] = 0; info->u[1] = 1;
    info->v[0] = 0; info->v[1] = 1;
    info->atlas.x = 0;
    info->atlas.y = 0;
    info->atlas.w = defaultSprite.texture->OriginalWidth();
    info->atlas.h = defaultSprite.texture->OriginalHeight();
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
    kexStr str1, str2, sprName;
    const char *sprDirectory = "sprites/";
    int len;
    int sep;

    len = strlen(sprDirectory);

    str1 = name;
    sep = str1.IndexOf(sprDirectory);
    str2 = str1.Substr(sep + len, str1.Length() - (sep + len));
    sep = str2.IndexOf("_sprite.\0");

    sprName = str2.Substr(0, sep);

    if(!(sprite = spriteList.Find(sprName.c_str())))
    {
        kexLexer *lexer;

        if(!(lexer = kex::cParser->Open(name)))
        {
            return NULL;
        }

        sprite = spriteList.Add(sprName.c_str());

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
                sprite->texture = NULL;
                sprite->textureFile = lexer->StringToken();

                sprite->LoadTexture();
            }
            else if(lexer->Matches("sprite"))
            {
                spriteInfo_t info;

                info.spriteID = lexer->GetNumber();
                info.atlas.x = lexer->GetNumber();
                info.atlas.y = lexer->GetNumber();
                info.atlas.w = lexer->GetNumber();
                info.atlas.h = lexer->GetNumber();

                if(sprite->texture)
                {
                    float w = (float)sprite->texture->Width();
                    float h = (float)sprite->texture->Height();

                    assert(w != 0);
                    assert(h != 0);
                    
                    if(w == 0 || h == 0)
                    {
                        sprite->texture = kexRender::cTextures->defaultTexture;
                        w = (float)sprite->texture->Width();
                        h = (float)sprite->texture->Height();
                        memcpy(&info, &defaultSprite, sizeof(spriteInfo_t));
                    }
                    else
                    {
                        info.u[0] = (float)info.atlas.x / w;
                        info.v[0] = (float)info.atlas.y / h;
                        info.u[1] = (float)(info.atlas.x + info.atlas.w) / w;
                        info.v[1] = (float)(info.atlas.y + info.atlas.h) / h;
                    }
                }

                sprite->infoList.Push(info);
            }
        }

        // we're done with the file
        kex::cParser->Close();
    }

    return sprite;
}
