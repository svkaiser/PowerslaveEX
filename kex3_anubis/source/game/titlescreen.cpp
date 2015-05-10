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

typedef void(*menuItemSelect_t)(kexMenuItem*);

typedef enum
{
    TSI_NEWGAME     = 0,
    TSI_LOADGAME,
    TSI_OPTIONS,
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

    NUMTITLESCREENSTATES
} titleScreenState_t;

typedef struct
{
    kexMenuItem         *item;
    menuItemSelect_t    callback;
} menuGroup_t;

static menuGroup_t *titleMenu[NUMTITLESCREENITEMS];

#define MENUITEM(name, label, x, y, center, lerpblock, block)   \
    static void LerpCallback_ ## name(kexMenuItem *item);   \
    static void Callback_ ## name(kexMenuItem *item);   \
    static menuGroup_t menuGroup_ ## name = { new kexMenuItem(label, x, y, center, LerpCallback_ ## name), Callback_ ## name };   \
    static void LerpCallback_ ## name(kexMenuItem *item)    \
    lerpblock   \
    static void Callback_ ## name(kexMenuItem *item)    \
    block

//-----------------------------------------------------------------------------
//
// New Game
//
//-----------------------------------------------------------------------------

MENUITEM(NewGame, "New Game", -100, 128, 1,
{
    if(kexGame::cLocal->TitleScreen()->SelectedItem() != TSI_NEWGAME)
    {
        return;
    }
    
    kexGame::cLocal->TitleScreen()->FadeOut(TSS_NEWGAME);
},
{
    titleMenu[TSI_NEWGAME]->item->LerpTo(-100);
    titleMenu[TSI_LOADGAME]->item->LerpTo(420);
    titleMenu[TSI_OPTIONS]->item->LerpTo(-100);
    titleMenu[TSI_QUIT]->item->LerpTo(420);
});

//-----------------------------------------------------------------------------
//
// Load Game
//
//-----------------------------------------------------------------------------

MENUITEM(LoadGame, "Load Game", 420, 146, 1,
{
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

MENUITEM(Options, "Options", 420, 164, 1,
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
    titleMenu[TSI_QUIT]->item->LerpTo(-100);
});

//-----------------------------------------------------------------------------
//
// Quit
//
//-----------------------------------------------------------------------------

MENUITEM(Quit, "Quit", 420, 182, 1,
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

MENUITEM(OptionExit, "Exit", -100, 186, 1,
{
    if(kexGame::cLocal->TitleScreen()->SelectedItem() != TSI_EXIT_OPTIONS)
    {
        return;
    }
    
    titleMenu[TSI_NEWGAME]->item->LerpTo(160);
    titleMenu[TSI_LOADGAME]->item->LerpTo(160);
    titleMenu[TSI_OPTIONS]->item->LerpTo(160, 164);
    titleMenu[TSI_OPTIONS]->item->Toggle(true);
    titleMenu[TSI_QUIT]->item->LerpTo(160);
    kexGame::cLocal->TitleScreen()->DeselectAllItems();
},
{
    titleMenu[TSI_GAMEPLAY]->item->LerpTo(-100);
    titleMenu[TSI_INPUT]->item->LerpTo(420);
    titleMenu[TSI_GRAPHICS]->item->LerpTo(-100);
    titleMenu[TSI_AUDIO]->item->LerpTo(420);
    titleMenu[TSI_EXIT_OPTIONS]->item->LerpTo(-100);
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

MENUITEM(InputExit, "Exit", 420, 168, 1,
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
        kexMenuItem *item = titleMenu[i]->item;
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
        titleMenu[TSI_QUIT]->item->LerpTo(160);
        break;
    case TSS_NEWGAME:
        kexGame::cLocal->StartNewGame();
        kexGame::cLocal->SetGameState(GS_OVERWORLD);
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
        kexMenuItem *item = titleMenu[i]->item;
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
        kexMenuItem *item = titleMenu[i]->item;
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

    return false;
}
