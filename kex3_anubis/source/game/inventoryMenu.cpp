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
//      Inventory Menu Interface
//

#include "kexlib.h"
#include "game.h"
#include "localization.h"
#include "renderMain.h"
#include "inventoryMenu.h"

//
// kexInventoryMenu::kexInventoryMenu
//

kexInventoryMenu::kexInventoryMenu(void)
{
}

//
// kexInventoryMenu::~kexInventoryMenu
//

kexInventoryMenu::~kexInventoryMenu(void)
{
}

//
// kexInventoryMenu::Init
//

void kexInventoryMenu::Init(void)
{
    kexStr str;
    backTexture = kexRender::cTextures->Cache("gfx/menu/menuback.png", TC_CLAMP, TF_NEAREST);

    font = kexFont::Alloc("smallfont");

    for(int i = 0; i < 4; ++i)
    {
        for(int j = 0; j < 2; ++j)
        {
            str = kexStr("gfx/menu/menukey_") + i + kexStr("_") + j + kexStr(".png");
            keyTextures[j][i] = kexRender::cTextures->Cache(str, TC_CLAMP, TF_NEAREST);
        }
    }

    for(int i = 0; i < 6; ++i)
    {
        str = kexStr("gfx/menu/menuartifact_") + i + kexStr(".png");
        artifactTextures[i] = kexRender::cTextures->Cache(str, TC_CLAMP, TF_NEAREST);
    }

    for(int i = 0; i < NUMPLAYERWEAPONS; ++i)
    {
        str = kexStr("gfx/menu/menuweapon_") + i + kexStr(".png");
        weaponTextures[i] = kexRender::cTextures->Cache(str, TC_CLAMP, TF_NEAREST);
    }

    buttonTexture[0] = kexRender::cTextures->Cache("gfx/menu/menubutton_up.png", TC_CLAMP, TF_NEAREST);
    buttonTexture[1] = kexRender::cTextures->Cache("gfx/menu/menubutton_down.png", TC_CLAMP, TF_NEAREST);

    arrows[0] = kexRender::cTextures->Cache("gfx/menu/menuarrow_left.png", TC_CLAMP, TF_NEAREST);
    arrows[1] = kexRender::cTextures->Cache("gfx/menu/menuarrow_right.png", TC_CLAMP, TF_NEAREST);
    mapClosedTexture = kexRender::cTextures->Cache("gfx/menu/menumap_closed.png", TC_CLAMP, TF_NEAREST);
    mapOpenTexture = kexRender::cTextures->Cache("gfx/menu/menumap_open.png", TC_CLAMP, TF_NEAREST);

    Reset();
}

//
// kexInventoryMenu::Reset
//

void kexInventoryMenu::Reset(void)
{
    bActive = false;
    categorySelected = 0;
    bButtonPressed[0] = true;
    bButtonPressed[1] = false;
    bButtonPressed[2] = false;
    bButtonPressed[3] = false;
}

//
// kexInventoryMenu::Update
//

void kexInventoryMenu::Update(void)
{
}

//
// kexInventoryMenu::ProcessInput
//

bool kexInventoryMenu::ProcessInput(inputEvent_t *ev)
{
    if(ev->type == ev_mousedown)
    {
        if(ev->data1 == KMSB_LEFT)
        {
            float mx = (float)kex::cInput->MouseX();
            float my = (float)kex::cInput->MouseY();
            float y = 39;
        
            kexRender::cScreen->CoordsToRenderScreenCoords(mx, my);

            switch(categorySelected)
            {
            case 0:
                if(kexGame::cLocal->PlayLoop()->AutomapEnabled())
                {
                    if(CursorOnLeftArrow(mx, my))
                    {
                        kexGame::cLocal->PlaySound("sounds/select.wav");
                        kexGame::cLocal->PlayLoop()->ToggleAutomap(false);
                        return true;
                    }
                }
                else
                {
                    if(CursorOnRightArrow(mx, my))
                    {
                        kexGame::cLocal->PlaySound("sounds/select.wav");
                        kexGame::cLocal->PlayLoop()->ToggleAutomap(true);
                        return true;
                    }
                }
                break;

            default:
                break;
            }

            for(int i = 0; i < 4; ++i)
            {
                if(bButtonPressed[i])
                {
                    y += 24;
                    continue;
                }

                if(mx >= 52 && mx <= 52 + buttonTexture[0]->OriginalWidth() &&
                   my >= y  && my <= y  + buttonTexture[0]->OriginalHeight())
                {
                    bButtonPressed[0] = bButtonPressed[1] = bButtonPressed[2] = bButtonPressed[3] = false;
                    bButtonPressed[i] = true;
                    categorySelected = i;
                    kexGame::cLocal->PlaySound("sounds/click.wav");
                    return true;
                }

                y += 24;
            }
        }
    }

    return false;
}

//
// kexInventoryMenu::DrawLeftArrow
//

void kexInventoryMenu::DrawLeftArrow(void)
{
    kexRender::cScreen->DrawTexture(arrows[0], 156, 78, 255, 255, 255, 255);
}

//
// kexInventoryMenu::DrawRightArrow
//

void kexInventoryMenu::DrawRightArrow(void)
{
    kexRender::cScreen->DrawTexture(arrows[1], 256, 78, 255, 255, 255, 255);
}

//
// kexInventoryMenu::CursorOnLeftArrow
//

bool kexInventoryMenu::CursorOnLeftArrow(float &mx, float &my)
{
    return(mx >= 156 && mx <= 156 + arrows[0]->OriginalWidth() &&
           my >= 78  && my <= 78  + arrows[0]->OriginalHeight());
}

//
// kexInventoryMenu::CursorOnRightArrow
//

bool kexInventoryMenu::CursorOnRightArrow(float &mx, float &my)
{
    return(mx >= 256 && mx <= 256 + arrows[1]->OriginalWidth() &&
           my >= 78  && my <= 78  + arrows[1]->OriginalHeight());
}

//
// kexInventoryMenu::DrawBackground
//

void kexInventoryMenu::DrawBackground(void)
{
    kexRender::cScreen->DrawTexture(backTexture, 32, -8, 255, 255, 255, 255);
}

//
// kexInventoryMenu::DrawButtons
//

void kexInventoryMenu::DrawButtons(void)
{
    for(int i = 0; i < 4; ++i)
    {
        float offs = (24 * (float)i);
        kexTexture *texture = buttonTexture[bButtonPressed[i]];

        kexRender::cScreen->DrawTexture(texture, 52, 39 + offs, 255, 255, 255, 255);
        font->DrawString(kexGame::cLocal->Translation()->GetString(44+i), 56, 43 + offs, 1, false);
    }
}

//
// kexInventoryMenu::DrawKeys
//

void kexInventoryMenu::DrawKeys(void)
{
    float keyX = 170;

    for(int i = 0; i < 4; ++i)
    {
        int key = kexGame::cLocal->Player()->CheckKey(i);

        kexRender::cScreen->DrawTexture(keyTextures[key][i], keyX, 134, 255, 255, 255, 255);
        keyX += 22;
    }
}

//
// kexInventoryMenu::DrawAutomap
//

void kexInventoryMenu::DrawAutomap(void)
{
    if(kexGame::cLocal->PlayLoop()->AutomapEnabled())
    {
        DrawLeftArrow();
        kexRender::cScreen->DrawTexture(mapOpenTexture, 182, 56, 255, 255, 255, 255);
        font->DrawString(kexGame::cLocal->Translation()->GetString(55), 160, 176, 1, true);
    }
    else
    {
        DrawRightArrow();
        kexRender::cScreen->DrawTexture(mapClosedTexture, 196, 64, 255, 255, 255, 255);
        font->DrawString(kexGame::cLocal->Translation()->GetString(54), 160, 176, 1, true);
    }
}

//
// kexInventoryMenu::Display
//

void kexInventoryMenu::Display(void)
{
    kexRender::cScreen->SetOrtho();

    kexRender::cBackend->SetState(GLSTATE_DEPTHTEST, false);
    kexRender::cBackend->SetState(GLSTATE_SCISSOR, false);
    kexRender::cBackend->SetState(GLSTATE_ALPHATEST, true);
    kexRender::cBackend->SetState(GLSTATE_BLEND, true);
    kexRender::cBackend->SetBlend(GLSRC_SRC_ALPHA, GLDST_ONE_MINUS_SRC_ALPHA);

    DrawBackground();

    DrawButtons();

    DrawKeys();

    switch(categorySelected)
    {
    case 0:
        DrawAutomap();
        break;

    default:
        break;
    }
}

//
// kexInventoryMenu::Toggle
//

void kexInventoryMenu::Toggle(void)
{
    bActive ^= 1;

    if(bActive)
    {
        kex::cInput->ToggleMouseGrab(false);
        kex::cSession->ToggleCursor(true);
    }
    else
    {
        kex::cInput->ToggleMouseGrab(true);
        kex::cSession->ToggleCursor(false);
    }
}
