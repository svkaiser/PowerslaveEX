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
//      Sprite Animation Manager
//

#include "kexlib.h"
#include "game.h"
#include "renderMain.h"

//
// kexSpriteAnimManager::kexSpriteAnimManager
//

kexSpriteAnimManager::kexSpriteAnimManager(void)
{
}

//
// kexSpriteAnimManager::~kexSpriteAnimManager
//

kexSpriteAnimManager::~kexSpriteAnimManager(void)
{
}

//
// kexSpriteAnimManager::Init
//

void kexSpriteAnimManager::Init(void)
{
    kexStrList list;
    spriteFrame_t *frame;
    spriteSet_t *spriteSet;

    kex::cPakFiles->GetMatchingFiles(list, "sprites/");

    for(unsigned int i = 0; i < list.Length(); ++i)
    {
        if(list[i].IndexOf("_anim.\0") == -1)
        {
            continue;
        }
        
        Load(list[i].c_str());
    }
    
    defaultAnim.name = "_default";
    
    frame = defaultAnim.frames.Grow();
    frame->delay = 0;
    frame->flags = 0;
    frame->nextFrame = "-";
    frame->refireFrame = "-";
    
    spriteSet = frame->spriteSet.Grow();
    spriteSet->x = -32;
    spriteSet->y = -64;
    spriteSet->bFlipped = false;
    spriteSet->index = 0;
    spriteSet->sprite = &kexGame::cLocal->SpriteManager()->defaultSprite;
}

//
// kexSpriteAnimManager::Shutdown
//

void kexSpriteAnimManager::Shutdown(void)
{
    for(int i = 0; i < MAX_HASH; i++)
    {
        for(spriteAnim_t *anim = spriteAnimList.GetData(i); anim; anim = spriteAnimList.Next())
        {
            anim->frames.Empty();
        }
    }
}

//
// kexSpriteAnimManager::ParseSpriteSet
//

void kexSpriteAnimManager::ParseSpriteSet(kexLexer *lexer, spriteFrame_t *frame)
{
    lexer->ExpectNextToken(TK_LBRACK);
    lexer->Find();

    while(lexer->TokenType() != TK_RBRACK)
    {
        if(lexer->TokenType() == TK_LBRACK)
        {
            spriteSet_t sprSet;

            sprSet.sprite = NULL;
            sprSet.index = 0;
            sprSet.x = 0;
            sprSet.y = 0;
            sprSet.bFlipped = false;

            lexer->GetString();
            sprSet.sprite = kexGame::cLocal->SpriteManager()->Get(lexer->StringToken());
            lexer->ExpectNextToken(TK_COMMA);

            sprSet.index = lexer->GetNumber();
            lexer->ExpectNextToken(TK_COMMA);

            sprSet.x = lexer->GetNumber();
            lexer->ExpectNextToken(TK_COMMA);

            sprSet.y = lexer->GetNumber();
            lexer->ExpectNextToken(TK_COMMA);

            sprSet.bFlipped = (lexer->GetNumber() != 0);
            lexer->ExpectNextToken(TK_RBRACK);

            if(sprSet.sprite == NULL)
            {
                sprSet.sprite = &kexGame::cLocal->SpriteManager()->defaultSprite;
            }

            frame->spriteSet.Push(sprSet);
        }

        lexer->Find();
    }
}

//
// kexSpriteAnimManager::ParseFrame
//

void kexSpriteAnimManager::ParseFrame(kexLexer *lexer, spriteFrame_t *frame)
{
    lexer->ExpectNextToken(TK_LBRACK);
    lexer->Find();

    while(lexer->TokenType() != TK_RBRACK)
    {
        if(lexer->Matches("delay"))
        {
            frame->delay = lexer->GetNumber();
        }
        else if(lexer->Matches("fullbright"))
        {
            frame->flags |= SFF_FULLBRIGHT;
        }
        else if(lexer->Matches("removeSelf"))
        {
            frame->flags |= SFF_REMOVESELF;
        }
        else if(lexer->Matches("goto"))
        {
            lexer->GetString();
            frame->nextFrame = lexer->StringToken();
        }
        else if(lexer->Matches("refire"))
        {
            lexer->GetString();
            frame->refireFrame = lexer->StringToken();
        }
        else if(lexer->Matches("sprites"))
        {
            // enter sprites block
            ParseSpriteSet(lexer, frame);
        }
        else if(lexer->Matches("action"))
        {
            lexer->ExpectNextToken(TK_IDENIFIER);
            if((frame->action = kexGame::cActionDefManager->CreateInstance(lexer->Token())))
            {
                frame->action->Parse(lexer);
            }
        }

        lexer->Find();
    }
}

//
// kexSpriteAnimManager::Load
//

void kexSpriteAnimManager::Load(const char *name)
{
    spriteAnim_t *anim;
    kexLexer *lexer;
    kexStr str1, str2, sprName;
    const char *sprDirectory = "sprites/";
    int len;
    int sep;

    if(!(lexer = kex::cParser->Open(name)))
    {
        return;
    }

    len = strlen(sprDirectory);

    str1 = name;
    sep = str1.IndexOf(sprDirectory);

    sprName = str1.Substr(sep + len, str1.Length() - (sep + len));
    sprName.StripFile();

    while(lexer->CheckState())
    {
        lexer->Find();

        if(lexer->TokenType() == TK_IDENIFIER)
        {
            anim = spriteAnimList.Add(sprName + kexStr(lexer->Token()));

            // enter block
            lexer->ExpectNextToken(TK_LBRACK);
            lexer->Find();

            while(lexer->TokenType() != TK_RBRACK)
            {
                if(lexer->Matches("frame"))
                {
                    spriteFrame_t *frame = anim->frames.Grow();

                    frame->delay = 1;
                    frame->flags = 0;
                    frame->nextFrame = "-";
                    frame->refireFrame = "-";
                    frame->action = NULL;

                    // enter frame block
                    ParseFrame(lexer, frame);
                }

                lexer->Find();
            }
        }
    }

   // we're done with the file
    kex::cParser->Close();
}
