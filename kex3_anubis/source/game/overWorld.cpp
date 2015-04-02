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
//      Map Overworld Screen
//

#include "kexlib.h"
#include "renderMain.h"
#include "game.h"
#include "overworld.h"

//
// kexOverWorld::kexOverWorld
//

kexOverWorld::kexOverWorld(void)
{
}

//
// kexOverWorld::~kexOverWorld
//

kexOverWorld::~kexOverWorld(void)
{
}

//
// kexOverWorld::Init
//

void kexOverWorld::Init(void)
{
    mapCursor = kexGame::cLocal->SpriteAnimManager()->Get("misc/mapcursor");
    selectedMap = 0;
}

//
// kexOverWorld::Start
//

void kexOverWorld::Start(void)
{
    if(kexGame::cLocal->MapInfoList().Length() == 0)
    {
        kex::cSystem->Warning("No map definitions present\n");
        kexGame::cLocal->SetGameState(GS_TITLE);
        return;
    }

    pic.LoadFromFile("gfx/overworld.png", TC_CLAMP, TF_NEAREST);

    kex::cInput->ToggleMouseGrab(false);
    kex::cSession->ToggleCursor(true);

    camera_x = kexGame::cLocal->MapInfoList()[selectedMap].overworldX;
    camera_y = kexGame::cLocal->MapInfoList()[selectedMap].overworldY;

    fadeTime = kex::cSession->GetTicks();
    curFadeTime = 0;
    bFading = true;
    bFadeIn = true;
}

//
// kexOverWorld::Stop
//

void kexOverWorld::Stop(void)
{
    pic.Delete();

    kex::cInput->ToggleMouseGrab(true);
    kex::cSession->ToggleCursor(false);
}

//
// kexOverWorld::SetupMatrix
//

void kexOverWorld::SetupMatrix(const int zoom)
{
    kexMatrix mtx;
    float x, y;

    kexRender::cScreen->SetOrtho();

    x = -camera_x*0.5f;
    y = -camera_y*0.5f;

    mtx.AddTranslation(x, y, 0);

    if(bFading)
    {
        float scale = 2.0f - ((float)zoom / 255.0f);
        int sx = (kexRenderScreen::SCREEN_WIDTH >> 1);
        int sy = (kexRenderScreen::SCREEN_HEIGHT >> 1);

        mtx.AddTranslation((x - sx) * (scale-1), (y - sy) * (scale-1), 0);
        mtx.Scale(scale, scale, 1);
    }

    dglMultMatrixf(mtx.ToFloatPtr());
}

//
// kexOverWorld::GetFade
//

const int kexOverWorld::GetFade(void)
{
    int c = 0;
    float scale = 2;

    if(!bFading)
    {
        c = bFadeIn ? 0xff : 0;
    }
    else if(bFadeIn)
    {
        c = curFadeTime;
        kexMath::Clamp(c, 0, 255);

        if(c >= 255)
        {
            bFading = false;
        }
    }
    else
    {
        c = 255 - curFadeTime;
        kexMath::Clamp(c, 0, 255);

        if(c <= 0)
        {
            bFading = false;
        }
    }

    return c;
}

//
// kexOverWorld::DrawBackground
//

void kexOverWorld::DrawBackground(const int fade)
{
    kexRender::cScreen->DrawTexture(&pic, 0, 0, fade, fade, fade, 255);
}

//
// kexOverWorld::DrawCursor
//

void kexOverWorld::DrawCursor(const int fade)
{
    int frm = 0;
    float nx = kexGame::cLocal->MapInfoList()[selectedMap].overworldX;
    float ny = kexGame::cLocal->MapInfoList()[selectedMap].overworldY;

    if(mapCursor->NumFrames() > 0)
    {
        frm = (kex::cSession->GetTicks() >> 1) % mapCursor->NumFrames();
    }

    kexCpuVertList  *vl = kexRender::cVertList;
    spriteFrame_t   *frame = &mapCursor->frames[frm];
    spriteSet_t     *spriteSet;
    kexSprite       *sprite;
    spriteInfo_t    *info;

    for(unsigned int i = 0; i < frame->spriteSet[0].Length(); ++i)
    {
        spriteSet = &frame->spriteSet[0][i];
        sprite = spriteSet->sprite;
        info = &sprite->InfoList()[spriteSet->index];

        float x = (float)spriteSet->x + nx;
        float y = (float)spriteSet->y + ny;
        float w = (float)info->atlas.w;
        float h = (float)info->atlas.h;

        float u1, u2, v1, v2;
        
        u1 = info->u[0 ^ spriteSet->bFlipped];
        u2 = info->u[1 ^ spriteSet->bFlipped];
        v1 = info->v[0];
        v2 = info->v[1];

        kexRender::cScreen->SetAspectDimentions(x, y, w, h);
        sprite->Texture()->Bind();

        vl->AddQuad(x, y, 0, w, h, u1, v1, u2, v2, fade, fade, fade, 255);
        vl->DrawElements();
    }
}

//
// kexOverWorld::DrawDots
//

void kexOverWorld::DrawDots(const int fade)
{
    kexCpuVertList *vl = kexRender::cVertList;
    kexRender::cTextures->whiteTexture->Bind();

    for(unsigned int i = 0; i < kexGame::cLocal->MapInfoList().Length(); ++i)
    {
        kexGameLocal::mapInfo_t *minfo = &kexGame::cLocal->MapInfoList()[i];

        vl->AddQuad(minfo->overworldX+16, minfo->overworldY+16, 4, 4, fade, fade, fade, 255);
    }

    vl->DrawElements();
}

//
// kexOverWorld::Draw
//

void kexOverWorld::Draw(void)
{
    kexRender::cBackend->SetState(GLSTATE_DEPTHTEST, false);
    kexRender::cBackend->SetState(GLSTATE_ALPHATEST, false);
    kexRender::cBackend->SetState(GLSTATE_SCISSOR, true);
    kexRender::cBackend->SetState(GLSTATE_BLEND, false);

    int c = GetFade();

    SetupMatrix(c);

    DrawBackground(c);

    DrawDots(c);

    DrawCursor(c);
}

//
// kexOverWorld::Tick
//

void kexOverWorld::Tick(void)
{
    float mx = (float)kex::cInput->MouseX();
    float my = (float)kex::cInput->MouseY();
    float nx;
    float ny;
        
    kexRender::cScreen->CoordsToRenderScreenCoords(mx, my);

    if(bFading)
    {
        curFadeTime = ((kex::cSession->GetTicks() - fadeTime)) << 3;
    }
    // TEMP
    else if(!bFadeIn)
    {
        kexGame::cLocal->ChangeMap("maps/TOMB.MAP");
        return;
    }

    nx = kexGame::cLocal->MapInfoList()[selectedMap].overworldX;
    ny = kexGame::cLocal->MapInfoList()[selectedMap].overworldY;

    camera_x = (nx - camera_x) * 0.05f + camera_x;
    camera_y = (ny - camera_y) * 0.05f + camera_y;

    for(unsigned int i = 0; i < kexGame::cLocal->MapInfoList().Length(); ++i)
    {
        kexGameLocal::mapInfo_t *minfo = &kexGame::cLocal->MapInfoList()[i];
        float sx = ((minfo->overworldX+18) - (camera_x*0.5f)) - mx;
        float sy = ((minfo->overworldY+18) - (camera_y*0.5f)) - my;

        if(sx * sx + sy * sy > 4096)
        {
            continue;
        }

        selectedMap = (int16_t)i;
    }
}

//
// kexOverWorld::ProcessInput
//

bool kexOverWorld::ProcessInput(inputEvent_t *ev)
{
    // TEMP
    if(ev->type == ev_mousedown && ev->data1 == KMSB_LEFT)
    {
        bFading = true;
        bFadeIn = false;
        fadeTime = kex::cSession->GetTicks();
        curFadeTime = 0;
        return true;
    }

    return false;
}
