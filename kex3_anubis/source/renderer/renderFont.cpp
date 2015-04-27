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
//      Font mapping
//

#include "renderMain.h"
#include "renderFont.h"

kexHashList<kexFont> kexFont::fontList;

//
// kexFont::kexFont
//

kexFont::kexFont(void)
{
    this->bLoaded = false;
    this->texture = NULL;
    this->padWidth = 0;
}

//
// kexFont::~kexFont
//

kexFont::~kexFont(void)
{
}

//
// kexFont::Delete
//

void kexFont::Delete(void)
{
}

//
// kexFont::Alloc
//

kexFont *kexFont::Alloc(const char *name)
{
    kexFont *font;
    
    if(!(font = fontList.Find(name)))
    {
        kexStr fontfile(kexStr::Format("fonts/%s.kfont", name));
        
        font = fontList.Add(name, hb_static);
        font->LoadKFont(fontfile.c_str());
    }
    
    return font;
}

//
// kexFont::Get
//

kexFont *kexFont::Get(const char *name)
{
    return fontList.Find(name);
}

//
// kexFont::LoadKFont
//

void kexFont::LoadKFont(const char *file)
{
    kexLexer *lexer;
    filepath_t fileName;

    if(!(lexer = kex::cParser->Open(file)))
    {
        kex::cSystem->Warning("kexFont::LoadKFont: %s not found\n", file);
        return;
    }

    memset(fileName, 0, sizeof(filepath_t));

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
            texture = kexRender::cTextures->Cache(lexer->StringToken(), TC_CLAMP, TF_NEAREST);
            
            if(!texture)
            {
                texture = kexRender::cTextures->defaultTexture;
            }
        }

        if(lexer->Matches("padwidth"))
        {
            padWidth = (float)lexer->GetFloat();
        }

        if(lexer->Matches("mapchar"))
        {
            lexer->ExpectNextToken(TK_LBRACK);
            lexer->Find();

            while(lexer->TokenType() != TK_RBRACK)
            {
                int ch;

                if(lexer->TokenType() == TK_NUMBER)
                {
                    ch = atoi(lexer->Token());
                }
                else
                {
                    kex::cParser->Error("%s is not a number", lexer->Token());
                    kex::cParser->Close();
                    return;
                }

                atlas[ch].x = lexer->GetNumber();
                atlas[ch].y = lexer->GetNumber();
                atlas[ch].w = lexer->GetNumber();
                atlas[ch].h = lexer->GetNumber();
                atlas[ch].o = lexer->GetNumber();

                lexer->Find();
            }
        }
    }

    // we're done with the file
    kex::cParser->Close();
    bLoaded = true;
}

//
// kexFont::DrawString
//

void kexFont::DrawString(const char *string, float x, float y, float scale,
                         bool center, byte *rgba1, byte *rgba2)
{
    float w;
    float h;
    int tri;
    unsigned int i;
    unsigned int len;
    char ch;
    atlas_t *at;
    float vx1;
    float vy1;
    float vx2;
    float vy2;
    float tx1;
    float tx2;
    float ty1;
    float ty2;
    char *check;

    if(scale <= 0.01f)
    {
        scale = 1;
    }

    if(center)
    {
        x -= StringWidth(string, scale, 0) * 0.5f;
    }

    w = (float)texture->OriginalWidth();
    h = (float)texture->OriginalHeight();

    tri = 0;
    len = strlen(string);

    kexRender::cVertList->BindDrawPointers();

    for(i = 0; i < len; i++)
    {
        ch      = string[i];
        at      = &atlas[ch];
        vx1     = x;
        vy1     = y + (at->o * scale);
        vx2     = vx1 + at->w * scale;
        vy2     = vy1 + at->h * scale;
        tx1     = (at->x / w) + 0.001f;
        tx2     = (tx1 + at->w / w) - 0.002f;
        ty1     = (at->y / h);
        ty2     = (ty1 + at->h / h) - 0.001f;
        check   = (char*)string+i;

        kexRender::cVertList->AddVertex(vx1, vy1, 0, tx1, ty1, rgba1);
        kexRender::cVertList->AddVertex(vx2, vy1, 0, tx2, ty1, rgba1);
        kexRender::cVertList->AddVertex(vx1, vy2, 0, tx1, ty2, rgba2);
        kexRender::cVertList->AddVertex(vx2, vy2, 0, tx2, ty2, rgba2);

        kexRender::cVertList->AddTriangle(0+tri, 2+tri, 1+tri);
        kexRender::cVertList->AddTriangle(1+tri, 2+tri, 3+tri);

        x += (at->w + padWidth) * scale;
        tri += 4;
    }

    texture->Bind();
    
    kexRender::cBackend->SetState(GLSTATE_ALPHATEST, true);
    kexRender::cBackend->SetState(GLSTATE_BLEND, true);
    kexRender::cVertList->DrawElements();
}

//
// kexFont::DrawString
//

void kexFont::DrawString(const char *string, float x, float y, float scale,
                         bool center, rcolor color)
{
    byte c[4];

    c[0] =  color        & 0xff;
    c[1] = (color >> 8)  & 0xff;
    c[2] = (color >> 16) & 0xff;
    c[3] = (color >> 24) & 0xff;

    DrawString(string, x, y, scale, center, c, c);
}

//
// kexFont::DrawString
//

void kexFont::DrawString(const char *string, float x, float y, float scale, bool center)
{
    DrawString(string, x, y, scale, center, RGBA(255, 255, 255, 255));
}

//
// kexFont::StringWidth
//

float kexFont::StringWidth(const char* string, float scale, int fixedLen)
{
    float width = 0;
    int len = strlen(string);
    int i;

    if(fixedLen > 0)
    {
        len = fixedLen;
    }

    for(i = 0; i < len; i++)
    {
        width += ((atlas[string[i]].w + padWidth) * scale);
    }

    return width;
}

//
// kexFont::StringHeight
//

float kexFont::StringHeight(const char* string, float scale, int fixedLen)
{
    float height = 0;
    float th;
    int len = strlen(string);
    int i;

    if(fixedLen > 0)
    {
        len = fixedLen;
    }

    for(i = 0; i < len; i++)
    {
        th = (atlas[string[i]].h * scale);
        if(th > height)
        {
            height = th;
        }
    }

    return height;
}
