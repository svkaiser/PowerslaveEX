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

//-----------------------------------------------------------------------------
//
// kexTitleMenuItem
//
//-----------------------------------------------------------------------------

DECLARE_KEX_CLASS(kexTitleMenuItem, kexObject)

//
// kexTitleMenuItem::kexTitleMenuItem
//

kexTitleMenuItem::kexTitleMenuItem(void)
{
    this->x             = 0;
    this->y             = 0;
    this->scale         = 1;
    this->bLerping      = false;
    this->bHighLighted  = false;
    this->bInteract     = true;
    this->bDisabled     = false;
    this->destX         = 0;
    this->destY         = 0;
    this->time          = 0;
    this->highlightTime = 0;
    this->lerpCallback  = NULL;
}

//
// kexTitleMenuItem::kexTitleMenuItem
//

kexTitleMenuItem::kexTitleMenuItem(const char *label, const float x, const float y,
                                   const float scale, menuItemLerpDone_t callback)
{
    this->x             = x;
    this->y             = y;
    this->scale         = scale;
    this->bLerping      = false;
    this->bInteract     = true;
    this->destX         = 0;
    this->destY         = 0;
    this->time          = 0;
    this->label         = label;
    this->highlightTime = 0;
    this->lerpCallback  = callback;
}

//
// kexTitleMenuItem::~kexTitleMenuItem
//

kexTitleMenuItem::~kexTitleMenuItem(void)
{
}

//
// kexTitleMenuItem::LerpTo
//

void kexTitleMenuItem::LerpTo(const float destx, const float desty)
{
    this->bLerping = true;
    this->startX = x;
    this->startY = y;
    this->destX = destx;
    this->destY = desty;
    this->time = 1;
    this->bInteract = false;
    this->bHighLighted = false;
    this->highlightTime = 0;
}

//
// kexTitleMenuItem::LerpTo
//

void kexTitleMenuItem::LerpTo(const float destx)
{
    LerpTo(destx, y);
}

//
// kexTitleMenuItem::Move
//

void kexTitleMenuItem::Move(void)
{
    float c = 1.0f - time;

    x = c * (destX - startX) + startX;
    y = c * (destY - startY) + startY;

    time -= 0.03f;

    if(time <= 0)
    {
        bLerping = false;
        bInteract = true;
        x = destX;
        y = destY;

        if(lerpCallback)
        {
            lerpCallback(this);
        }
    }
}

//
// kexTitleMenuItem::OnCursor
//

bool kexTitleMenuItem::OnCursor(void)
{
    float width = kexGame::cLocal->BigFont()->StringWidth(label.c_str(), scale, 0) * 0.5f;
    float height = kexGame::cLocal->BigFont()->StringHeight(label.c_str(), scale, 0) * 0.5f;
    float sy = y + height;
    float mx = (float)kex::cInput->MouseX();
    float my = (float)kex::cInput->MouseY();

    kexRender::cScreen->CoordsToRenderScreenCoords(mx, my);

    return !(mx < (x - width) || my < (sy - height) ||
             mx > (x + width) || my > (sy + height));
}

//
// kexTitleMenuItem::Tick
//

void kexTitleMenuItem::Tick(void)
{
    static kexTitleMenuItem *lastItem;

    if(bLerping)
    {
        Move();
    }

    if(bInteract && !bSelected && !bDisabled)
    {
        if(!(bHighLighted = OnCursor()))
        {
            highlightTime = 0;

            if(lastItem == this)
            {
                lastItem = NULL;
            }
        }
        else
        {
            highlightTime += 1.0f;
        }
    }

    if(bHighLighted && lastItem != this)
    {
        lastItem = this;
        kexGame::cLocal->PlaySound("sounds/menu_highlight.wav");
    }
}

//
// kexTitleMenuItem::DrawSmallString
//

void kexTitleMenuItem::DrawSmallString(const char *string, float x, float y,
                                       float scale, bool center, bool flash)
{
    byte c = (flash || bSelected || bDisabled) ? 255 : 224;
    kexFont *font = kexGame::cLocal->SmallFont();

    kexGame::cLocal->DrawSmallString(string, x, y, scale, center, c, c, c);

    if(flash)
    {
        byte pulse = (byte)(kexMath::Sin(highlightTime * 0.1f) * 64.0f) + 64;

        kexRender::cBackend->SetBlend(GLSRC_ONE, GLDST_ONE);
        font->DrawString(string, x, y, scale, center, RGBA(pulse, pulse, pulse, 0xff));
    }
}

//
// kexTitleMenuItem::DrawBigString
//

void kexTitleMenuItem::DrawBigString(const char *string, float x, float y,
                                     float scale, bool center, bool flash)
{
    byte c = (flash || bSelected || bDisabled) ? 255 : 224;
    kexFont *font = kexGame::cLocal->BigFont();

    kexGame::cLocal->DrawBigString(string, x, y, scale, center, c, c, c);

    if(flash)
    {
        byte pulse = (byte)(kexMath::Sin(highlightTime * 0.1f) * 64.0f) + 64;

        kexRender::cBackend->SetBlend(GLSRC_ONE, GLDST_ONE);
        font->DrawString(string, x, y, scale, center, RGBA(pulse, pulse, pulse, 0xff));
    }
}

//
// kexTitleMenuItem::Draw
//

void kexTitleMenuItem::Draw(void)
{
    DrawBigString(label.c_str(), x, y, scale, true, bHighLighted);
}

//
// kexTitleMenuItem::operator=
//

kexTitleMenuItem &kexTitleMenuItem::operator=(const kexTitleMenuItem &item)
{
    this->x             = item.x;
    this->y             = item.y;
    this->scale         = item.scale;
    this->label         = item.label;
    this->bInteract     = item.bInteract;
    this->bLerping      = item.bLerping;

    return *this;
}

//-----------------------------------------------------------------------------
//
// Titlescreen menu structure
//
//-----------------------------------------------------------------------------

typedef void(*menuItemSelect_t)(kexTitleMenuItem*);

typedef enum
{
    TSI_NEWGAME     = 0,
    TSI_LOADGAME,
    TSI_OPTIONS,
    TSI_ABOUT,
    TSI_QUIT,
    TSI_GAMEPLAY,
    TSI_INPUT,
    TSI_BINDINGS,
    TSI_MOUSE,
    TSI_GAMEPAD,
    TSI_GRAPHICS,
    TSI_AUDIO,
    TSI_EXIT_OPTIONS,
    TSI_EXIT_INPUT,

    NUMTITLESCREENITEMS
} titleScreenItems_t;

typedef enum
{
    TSS_IDLE    = 0,
    TSS_NEWGAME,
    TSS_LOADGAME,

    NUMTITLESCREENSTATES
} titleScreenState_t;

typedef struct
{
    kexTitleMenuItem    *item;
    menuItemSelect_t    callback;
} menuGroup_t;

static menuGroup_t *titleMenu[NUMTITLESCREENITEMS];

#define MENUITEM(name, label, x, y, center, lerpblock, block)   \
    static void LerpCallback_ ## name(kexTitleMenuItem *item);   \
    static void Callback_ ## name(kexTitleMenuItem *item);   \
    static menuGroup_t menuGroup_ ## name = { new kexTitleMenuItem(label, x, y, center, LerpCallback_ ## name), Callback_ ## name };   \
    static void LerpCallback_ ## name(kexTitleMenuItem *item)    \
    lerpblock   \
    static void Callback_ ## name(kexTitleMenuItem *item)    \
    block

//-----------------------------------------------------------------------------
//
// New Game
//
//-----------------------------------------------------------------------------

MENUITEM(NewGame, "New Game", -100, 110, 1,
{
    if(kexGame::cLocal->TitleScreen()->SelectedItem() != TSI_NEWGAME)
    {
        return;
    }
    
    kexGame::cLocal->TitleScreen()->FadeOut(TSS_NEWGAME);
},
{
    kexGame::cLocal->SetMenu(MENU_NEWGAME);
    kexGame::cLocal->TitleScreen()->DeselectAllItems();
});

//-----------------------------------------------------------------------------
//
// Load Game
//
//-----------------------------------------------------------------------------

MENUITEM(LoadGame, "Load Game", 420, 128, 1,
{
    if(kexGame::cLocal->TitleScreen()->SelectedItem() != TSI_LOADGAME)
    {
        return;
    }

    kexGame::cLocal->TitleScreen()->FadeOut(TSS_LOADGAME);
},
{
    kexGame::cLocal->SetMenu(MENU_LOADGAME);
    kexGame::cLocal->TitleScreen()->DeselectAllItems();
});

//-----------------------------------------------------------------------------
//
// Options
//
//-----------------------------------------------------------------------------

MENUITEM(Options, "Options", -100, 146, 1,
{
    if(kexGame::cLocal->TitleScreen()->SelectedItem() != TSI_OPTIONS)
    {
        return;
    }
    
    titleMenu[TSI_GAMEPLAY]->item->LerpTo(160);
    titleMenu[TSI_INPUT]->item->LerpTo(160);
    titleMenu[TSI_GRAPHICS]->item->LerpTo(160);
    titleMenu[TSI_AUDIO]->item->LerpTo(160);
    titleMenu[TSI_EXIT_OPTIONS]->item->LerpTo(160);
},
{
    titleMenu[TSI_NEWGAME]->item->LerpTo(-100);
    titleMenu[TSI_LOADGAME]->item->LerpTo(420);
    titleMenu[TSI_OPTIONS]->item->LerpTo(160, 64);
    titleMenu[TSI_OPTIONS]->item->Toggle(false);
    titleMenu[TSI_ABOUT]->item->LerpTo(-100);
    titleMenu[TSI_QUIT]->item->LerpTo(420);

    kexGame::cLocal->TitleScreen()->CurrentMenu() = TSI_OPTIONS;
});

//-----------------------------------------------------------------------------
//
// About
//
//-----------------------------------------------------------------------------

MENUITEM(About, "About", 420, 164, 1,
{
},
{
    kexGame::cLocal->SetMenu(MENU_ABOUT);
    kexGame::cLocal->TitleScreen()->DeselectAllItems();
});

//-----------------------------------------------------------------------------
//
// Quit
//
//-----------------------------------------------------------------------------

MENUITEM(Quit, "Quit", -100, 182, 1,
{
},
{
    kexGame::cLocal->SetMenu(MENU_QUITCONFIRM);
    kexGame::cLocal->TitleScreen()->DeselectAllItems();
});

//-----------------------------------------------------------------------------
//
// Options: Gameplay
//
//-----------------------------------------------------------------------------

MENUITEM(Gameplay, "Gameplay", -100, 114, 1,
{
},
{
    kexGame::cLocal->SetMenu(MENU_GAMEPLAY);
    kexGame::cLocal->TitleScreen()->DeselectAllItems();
});

//-----------------------------------------------------------------------------
//
// Options: Input
//
//-----------------------------------------------------------------------------

MENUITEM(Input, "Input", 420, 132, 1,
{
    if(kexGame::cLocal->TitleScreen()->SelectedItem() != TSI_INPUT)
    {
        return;
    }
    
    titleMenu[TSI_BINDINGS]->item->LerpTo(160);
    titleMenu[TSI_MOUSE]->item->LerpTo(160);
    titleMenu[TSI_GAMEPAD]->item->LerpTo(160);
    titleMenu[TSI_EXIT_INPUT]->item->LerpTo(160);
},
{
    titleMenu[TSI_OPTIONS]->item->LerpTo(-100, 64);
    titleMenu[TSI_GAMEPLAY]->item->LerpTo(-100);
    titleMenu[TSI_INPUT]->item->LerpTo(160, 64);
    titleMenu[TSI_INPUT]->item->Toggle(false);
    titleMenu[TSI_GRAPHICS]->item->LerpTo(-100);
    titleMenu[TSI_AUDIO]->item->LerpTo(420);
    titleMenu[TSI_EXIT_OPTIONS]->item->LerpTo(-100);

    kexGame::cLocal->TitleScreen()->CurrentMenu() = TSI_INPUT;
});

//-----------------------------------------------------------------------------
//
// Options: Graphics
//
//-----------------------------------------------------------------------------

MENUITEM(Graphics, "Graphics", -100, 150, 1,
{
},
{
    kexGame::cLocal->SetMenu(MENU_GRAPHICS);
    kexGame::cLocal->TitleScreen()->DeselectAllItems();
});

//-----------------------------------------------------------------------------
//
// Options: Audio
//
//-----------------------------------------------------------------------------

MENUITEM(Audio, "Audio", 420, 168, 1,
{
},
{
    kexGame::cLocal->SetMenu(MENU_AUDIO);
    kexGame::cLocal->TitleScreen()->DeselectAllItems();
});

//-----------------------------------------------------------------------------
//
// Options: Exit
//
//-----------------------------------------------------------------------------

MENUITEM(OptionExit, "Back", -100, 186, 1,
{
    if(kexGame::cLocal->TitleScreen()->SelectedItem() != TSI_EXIT_OPTIONS)
    {
        return;
    }
    
    titleMenu[TSI_NEWGAME]->item->LerpTo(160);
    titleMenu[TSI_LOADGAME]->item->LerpTo(160);
    titleMenu[TSI_OPTIONS]->item->LerpTo(160, 146);
    titleMenu[TSI_OPTIONS]->item->Toggle(true);
    titleMenu[TSI_ABOUT]->item->LerpTo(160);
    titleMenu[TSI_QUIT]->item->LerpTo(160);
    kexGame::cLocal->TitleScreen()->DeselectAllItems();
},
{
    titleMenu[TSI_GAMEPLAY]->item->LerpTo(-100);
    titleMenu[TSI_INPUT]->item->LerpTo(420);
    titleMenu[TSI_GRAPHICS]->item->LerpTo(-100);
    titleMenu[TSI_AUDIO]->item->LerpTo(420);
    titleMenu[TSI_EXIT_OPTIONS]->item->LerpTo(-100);

    kexGame::cLocal->TitleScreen()->CurrentMenu() = -1;
});

//-----------------------------------------------------------------------------
//
// Input: Bindings
//
//-----------------------------------------------------------------------------

MENUITEM(Bindings, "Bindings", -100, 114, 1,
{
},
{
    kexGame::cLocal->SetMenu(MENU_BINDINGS);
    kexGame::cLocal->TitleScreen()->DeselectAllItems();
});

//-----------------------------------------------------------------------------
//
// Input: Mouse
//
//-----------------------------------------------------------------------------

MENUITEM(Mouse, "Mouse", 420, 132, 1,
{
},
{
    kexGame::cLocal->SetMenu(MENU_MOUSE);
    kexGame::cLocal->TitleScreen()->DeselectAllItems();
});

//-----------------------------------------------------------------------------
//
// Input: Gamepad
//
//-----------------------------------------------------------------------------

MENUITEM(Gamepad, "Gamepad", -100, 150, 1,
{
},
{
    kexGame::cLocal->SetMenu(MENU_JOYSTICK);
    kexGame::cLocal->TitleScreen()->DeselectAllItems();
});

//-----------------------------------------------------------------------------
//
// Input: Exit
//
//-----------------------------------------------------------------------------

MENUITEM(InputExit, "Back", 420, 168, 1,
{
    if(kexGame::cLocal->TitleScreen()->SelectedItem() != TSI_EXIT_INPUT)
    {
        return;
    }
    
    titleMenu[TSI_OPTIONS]->item->LerpTo(160, 64);
    titleMenu[TSI_GAMEPLAY]->item->LerpTo(160);
    titleMenu[TSI_INPUT]->item->LerpTo(160, 132);
    titleMenu[TSI_INPUT]->item->Toggle(true);
    titleMenu[TSI_GRAPHICS]->item->LerpTo(160);
    titleMenu[TSI_AUDIO]->item->LerpTo(160);
    titleMenu[TSI_EXIT_OPTIONS]->item->LerpTo(160);
    kexGame::cLocal->TitleScreen()->DeselectAllItems();
},
{
    titleMenu[TSI_BINDINGS]->item->LerpTo(420);
    titleMenu[TSI_MOUSE]->item->LerpTo(-100);
    titleMenu[TSI_GAMEPAD]->item->LerpTo(420);
    titleMenu[TSI_EXIT_INPUT]->item->LerpTo(-100);

    kexGame::cLocal->TitleScreen()->CurrentMenu() = TSI_OPTIONS;
});

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
    
    titleMenu[TSI_NEWGAME] = &menuGroup_NewGame;
    titleMenu[TSI_LOADGAME] = &menuGroup_LoadGame;
    titleMenu[TSI_OPTIONS] = &menuGroup_Options;
    titleMenu[TSI_ABOUT] = &menuGroup_About;
    titleMenu[TSI_QUIT] = &menuGroup_Quit;
    titleMenu[TSI_GAMEPLAY] = &menuGroup_Gameplay;
    titleMenu[TSI_INPUT] = &menuGroup_Input;
    titleMenu[TSI_BINDINGS] = &menuGroup_Bindings;
    titleMenu[TSI_MOUSE] = &menuGroup_Mouse;
    titleMenu[TSI_GAMEPAD] = &menuGroup_Gamepad;
    titleMenu[TSI_GRAPHICS] = &menuGroup_Graphics;
    titleMenu[TSI_AUDIO] = &menuGroup_Audio;
    titleMenu[TSI_EXIT_OPTIONS] = &menuGroup_OptionExit;
    titleMenu[TSI_EXIT_INPUT] = &menuGroup_InputExit;
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
}

//
// kexTitleScreen::Start
//

void kexTitleScreen::Start(void)
{
    DeselectAllItems();
    fadeTime = kex::cSession->GetTicks();
    state = TSS_IDLE;
    curFadeTime = 0;
    bFading = true;
    bFadeIn = true;
    selectedItem = -1;
    currentMenu = -1;
    kex::cInput->ToggleMouseGrab(false);
    kex::cSession->ToggleCursor(true);

    kex::cSound->PlayMusic("music/title.ogg");

#if 0
    kexGame::cLocal->SetMenu(MENU_STARTUP_NOTICE);
#endif
}

//
// kexTitleScreen::Stop
//

void kexTitleScreen::Stop(void)
{
    DeselectAllItems();
    kex::cInput->ToggleMouseGrab(true);
    kex::cSession->ToggleCursor(false);
    kex::cSound->StopMusic();
}

//
// kexTitleScreen::FadeOut
//

void kexTitleScreen::FadeOut(int state)
{
    fadeTime = kex::cSession->GetTicks();
    this->state = state;
    curFadeTime = 0;
    bFading = true;
    bFadeIn = false;
}

//
// kexTitleScreen::DeselectAllItems
//

void kexTitleScreen::DeselectAllItems(void)
{
    for(unsigned int i = 0; i < ARRLEN(titleMenu); ++i)
    {
        kexTitleMenuItem *item = titleMenu[i]->item;
        item->Select(false);
    }
}

//
// kexTitleScreen::FadeDone
//

void kexTitleScreen::FadeDone(void)
{
    switch(state)
    {
    case TSS_IDLE:
        titleMenu[TSI_NEWGAME]->item->LerpTo(160);
        titleMenu[TSI_LOADGAME]->item->LerpTo(160);
        titleMenu[TSI_OPTIONS]->item->LerpTo(160);
        titleMenu[TSI_ABOUT]->item->LerpTo(160);
        titleMenu[TSI_QUIT]->item->LerpTo(160);
        break;

    case TSS_NEWGAME:
        kexGame::cLocal->StartNewGame(loadGameSlot);
        kexGame::cLocal->SetGameState(GS_OVERWORLD);
        break;
        
    case TSS_LOADGAME:
        kexGame::cLocal->LoadGame(loadGameSlot);
        break;

    default:
        break;
    }
}

//
// kexTitleScreen::Draw
//

void kexTitleScreen::Draw(void)
{
    int fade;

    if(bFading)
    {
        curFadeTime = ((kex::cSession->GetTicks() - fadeTime)) << 3;
    }

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
        kexTitleMenuItem *item = titleMenu[i]->item;
        item->Draw();
    }
}

//
// kexTitleScreen::Tick
//

void kexTitleScreen::Tick(void)
{
    for(unsigned int i = 0; i < ARRLEN(titleMenu); ++i)
    {
        kexTitleMenuItem *item = titleMenu[i]->item;
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
                if(titleMenu[i]->item->IsHighlighted())
                {
                    if(titleMenu[i]->callback)
                    {
                        selectedItem = i;
                        titleMenu[i]->item->Select(true);
                        titleMenu[i]->callback(titleMenu[i]->item);
                        kexGame::cLocal->PlaySound("sounds/menu_select.wav");
                        return true;
                    }
                    else
                    {
                        selectedItem = i;
                        titleMenu[i]->item->Select(true);
                        return true;
                    }
                }
            }
        }
    }
    else if(ev->type == ev_mouseup)
    {
        if(ev->data1 == KMSB_LEFT)
        {
            if(selectedItem >= 0 && !titleMenu[selectedItem]->callback)
            {
                titleMenu[selectedItem]->item->Select(false);
                selectedItem = -1;
            }
        }
    }
    else if(ev->type == ev_keydown)
    {
        if(ev->data1 == KKEY_ESCAPE && currentMenu != -1)
        {
            if( titleMenu[TSI_INPUT]->item->Lerping() ||
                titleMenu[TSI_OPTIONS]->item->Lerping() ||
                titleMenu[TSI_EXIT_OPTIONS]->item->Lerping() ||
                titleMenu[TSI_EXIT_INPUT]->item->Lerping())
            {
                return false;
            }

            if(currentMenu == TSI_OPTIONS)
            {
                selectedItem = TSI_EXIT_OPTIONS;
                titleMenu[TSI_EXIT_OPTIONS]->item->Select(true);
                titleMenu[TSI_EXIT_OPTIONS]->callback(titleMenu[TSI_EXIT_OPTIONS]->item);
                kexGame::cLocal->PlaySound("sounds/menu_select.wav");
            }
            else if(currentMenu == TSI_INPUT)
            {
                selectedItem = TSI_EXIT_INPUT;
                titleMenu[TSI_EXIT_INPUT]->item->Select(true);
                titleMenu[TSI_EXIT_INPUT]->callback(titleMenu[TSI_EXIT_INPUT]->item);
                kexGame::cLocal->PlaySound("sounds/menu_select.wav");
            }
        }
    }

    return false;
}

//
// kexTitleScreen::StartLoadGame
//

void kexTitleScreen::StartLoadGame(const int slot)
{
    titleMenu[TSI_NEWGAME]->item->LerpTo(-100);
    titleMenu[TSI_LOADGAME]->item->LerpTo(420);
    titleMenu[TSI_OPTIONS]->item->LerpTo(-100);
    titleMenu[TSI_ABOUT]->item->LerpTo(420);
    titleMenu[TSI_QUIT]->item->LerpTo(-100);

    selectedItem = TSI_LOADGAME;
    loadGameSlot = slot;
}

//
// kexTitleScreen::StartNewGame
//

void kexTitleScreen::StartNewGame(const int slot)
{
    titleMenu[TSI_NEWGAME]->item->LerpTo(-100);
    titleMenu[TSI_LOADGAME]->item->LerpTo(420);
    titleMenu[TSI_OPTIONS]->item->LerpTo(-100);
    titleMenu[TSI_ABOUT]->item->LerpTo(420);
    titleMenu[TSI_QUIT]->item->LerpTo(-100);

    selectedItem = TSI_NEWGAME;
    loadGameSlot = slot;
}
