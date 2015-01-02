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
//      Title screen logic
//

#include "kexlib.h"
#include "renderMain.h"
#include "game.h"
#include "titlescreen.h"

static void TitleMenuCallback_Options(kexMenuItem *item);
static void TitleMenuCallback_Quit(kexMenuItem *item);
static void TitleMenuCallback_ExitOptions(kexMenuItem *item);

static void MenuItemLerpCallback_Options(kexMenuItem *item);

kexTitleScreen::menuGroup_t kexTitleScreen::titleMenu[11] =
{
    { kexMenuItem("New Game",  -100, 128, 1) },
    { kexMenuItem("Load Game",  420, 146, 1) },
    { kexMenuItem("Options",    420, 164, 1, MenuItemLerpCallback_Options), TitleMenuCallback_Options },
    { kexMenuItem("Quit",       420, 182, 1), TitleMenuCallback_Quit },
    { kexMenuItem("Gameplay",  -100, 96,  1) },
    { kexMenuItem("Keyboard",   420, 114, 1) },
    { kexMenuItem("Mouse",     -100, 132, 1) },
    { kexMenuItem("Gamepad",    420, 150, 1) },
    { kexMenuItem("Graphics",  -100, 168, 1) },
    { kexMenuItem("Audio",      420, 186, 1) },
    { kexMenuItem("Exit",      -100, 204, 1), TitleMenuCallback_ExitOptions }
};

//
// TitleMenuCallback_Options
//

static void TitleMenuCallback_Options(kexMenuItem *item)
{
    kex::cGame->TitleScreen()->OnSelectOptions(item);
}

//
// TitleMenuCallback_Quit
//

static void TitleMenuCallback_Quit(kexMenuItem *item)
{
    kex::cCommands->Execute("quit");
}

//
// TitleMenuCallback_ExitOptions
//

static void TitleMenuCallback_ExitOptions(kexMenuItem *item)
{
    kex::cGame->TitleScreen()->OnSelectExitOptions(item);
}

//
// MenuItemLerpCallback_Options
//

static void MenuItemLerpCallback_Options(kexMenuItem *item)
{
    switch(kex::cGame->TitleScreen()->SelectedItem())
    {
    case 2:
        kexTitleScreen::titleMenu[ 4].item.LerpTo(160, 96);
        kexTitleScreen::titleMenu[ 5].item.LerpTo(160, 114);
        kexTitleScreen::titleMenu[ 6].item.LerpTo(160, 132);
        kexTitleScreen::titleMenu[ 7].item.LerpTo(160, 150);
        kexTitleScreen::titleMenu[ 8].item.LerpTo(160, 168);
        kexTitleScreen::titleMenu[ 9].item.LerpTo(160, 186);
        kexTitleScreen::titleMenu[10].item.LerpTo(160, 204);
        break;

    case 10:
        kexTitleScreen::titleMenu[0].item.LerpTo(160, 128);
        kexTitleScreen::titleMenu[1].item.LerpTo(160, 146);
        kexTitleScreen::titleMenu[3].item.LerpTo(160, 182);
        kex::cGame->TitleScreen()->DeselectAllItems();
        break;

    default:
        break;
    }
}

//
// kexTitleScreen::kexTitleScreen
//

kexTitleScreen::kexTitleScreen(void)
{
    this->fadeTime = 0;
    this->curFadeTime = 0;
    this->bFadeIn = true;
    this->bFading = false;
    this->selectedItem = -1;
}

//
// kexTitleScreen::~kexTitleScreen
//

kexTitleScreen::~kexTitleScreen(void)
{
}

//
// kexTitleScreen::Init
//

void kexTitleScreen::Init(void)
{
    titlePic = kexRender::cTextures->Cache("gfx/title.png", TC_CLAMP, TF_NEAREST);
    Start();
}

//
// kexTitleScreen::Start
//

void kexTitleScreen::Start(void)
{
    fadeTime = kex::cSession->GetTicks();
    curFadeTime = 0;
    bFading = true;
}

//
// kexTitleScreen::DeselectAllItems
//

void kexTitleScreen::DeselectAllItems(void)
{
    for(unsigned int i = 0; i < ARRLEN(titleMenu); ++i)
    {
        kexMenuItem *item = &titleMenu[i].item;
        item->Select(false);
    }
}

//
// kexTitleScreen::FadeDone
//

void kexTitleScreen::FadeDone(void)
{
    titleMenu[0].item.LerpTo(160, 128);
    titleMenu[1].item.LerpTo(160, 146);
    titleMenu[2].item.LerpTo(160, 164);
    titleMenu[3].item.LerpTo(160, 182);
}

//
// kexTitleScreen::OnSelectOptions
//

void kexTitleScreen::OnSelectOptions(kexMenuItem *item)
{
    titleMenu[0].item.LerpTo(-100, 128);
    titleMenu[1].item.LerpTo(420, 146);
    titleMenu[2].item.LerpTo(160, 64);
    titleMenu[3].item.LerpTo(-100, 182);
}

//
// kexTitleScreen::OnSelectExitOptions
//

void kexTitleScreen::OnSelectExitOptions(kexMenuItem *item)
{
    titleMenu[ 2].item.LerpTo(160, 164);
    titleMenu[ 4].item.LerpTo(-100, 96);
    titleMenu[ 5].item.LerpTo(420, 114);
    titleMenu[ 6].item.LerpTo(-100, 132);
    titleMenu[ 7].item.LerpTo(420, 150);
    titleMenu[ 8].item.LerpTo(-100, 168);
    titleMenu[ 9].item.LerpTo(420, 186);
    titleMenu[10].item.LerpTo(-100, 204);
}

//
// kexTitleScreen::Draw
//

void kexTitleScreen::Draw(void)
{
    int fade;

    if(!bFading)
    {
        fade = 0xff;
    }
    else if(bFadeIn)
    {
        fade = curFadeTime;
        kexMath::Clamp(fade, 0, 255);

        if(fade >= 255)
        {
            bFading = false;
            FadeDone();
        }
    }
    else
    {
        fade = 255 - curFadeTime;
        kexMath::Clamp(fade, 0, 255);

        if(fade <= 0)
        {
            bFading = false;
            FadeDone();
        }
    }

    kexRender::cScreen->SetOrtho();
    kexRender::cScreen->DrawTexture(titlePic, 0, 0, fade, fade, fade, 0xff);

    for(unsigned int i = 0; i < ARRLEN(titleMenu); ++i)
    {
        kexMenuItem *item = &titleMenu[i].item;
        item->Draw();
    }
}

//
// kexTitleScreen::Tick
//

void kexTitleScreen::Tick(void)
{
    if(bFading)
    {
        curFadeTime = ((kex::cSession->GetTicks() - fadeTime)) << 3;
    }

    for(unsigned int i = 0; i < ARRLEN(titleMenu); ++i)
    {
        kexMenuItem *item = &titleMenu[i].item;
        item->Tick();
    }
}

//
// kexTitleScreen::ProcessInput
//

bool kexTitleScreen::ProcessInput(inputEvent_t *ev)
{
    if(ev->type == ev_mousedown)
    {
        if(ev->data1 == KMSB_LEFT)
        {
            for(unsigned int i = 0; i < ARRLEN(titleMenu); ++i)
            {
                if(titleMenu[i].item.IsHighlighted())
                {
                    if(titleMenu[i].callback)
                    {
                        selectedItem = i;
                        titleMenu[i].item.Select(true);
                        titleMenu[i].callback(&titleMenu[i].item);
                        return true;
                    }
                }
            }
        }
    }

    return false;
}
