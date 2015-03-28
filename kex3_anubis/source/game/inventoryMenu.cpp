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
#include "menuPanel.h"
#include "inventoryMenu.h"

#define NUM_ARTIFACTS   6
#define BUTTON_X        52
#define BUTTON_OFFSET   39
#define LEFT_ARROW_X    156
#define RIGHT_ARROW_Y   256
#define ARROW_OFFSET    78
#define PIC_X           210
#define PIC_Y           83

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

    font = kexFont::Alloc("smallfont");

    for(int i = 0; i < 4; ++i)
    {
        for(int j = 0; j < 2; ++j)
        {
            str = kexStr("gfx/menu/menukey_") + i + kexStr("_") + j + kexStr(".png");
            keyTextures[j][i] = kexRender::cTextures->Cache(str, TC_CLAMP, TF_NEAREST);
        }
    }

    for(int i = 0; i < NUM_ARTIFACTS; ++i)
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
    artifactSelected = 0;
    flashBits = 0;
    flashCount = 0;
    bFlashArtifact = false;
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
    kexPlayer *p = kexGame::cLocal->Player();

    switch(categorySelected)
    {
    case 2:
        kexMath::Clamp(artifactSelected, 0, NUM_ARTIFACTS-1);

        if(p->Artifacts() != 0 && !(p->Artifacts() & BIT(artifactSelected)))
        {
            for(int i = 0; i < NUM_ARTIFACTS; ++i)
            {
                int arti = (artifactSelected + i) % NUM_ARTIFACTS;

                if(p->Artifacts() & BIT(arti))
                {
                    artifactSelected = arti;
                    break;
                }
            }
        }

        if(bFlashArtifact)
        {
            int bits;

            bits = (kexGame::cLocal->PlayLoop()->Ticks() & 0x10);

            if(bits != 0)
            {
                flashBits = bits;
                return;
            }

            if((bits ^ flashBits) != 0)
            {
                if(++flashCount >= 4)
                {
                    kexGame::cLocal->PlaySound("sounds/ding02.wav");
                    flashBits = 0;
                    flashCount = 0;
                    bFlashArtifact = false;
                }
                else
                {
                    kexGame::cLocal->PlaySound("sounds/ding01.wav");
                }
            }

            flashBits = bits;
        }
        break;

    default:
        break;
    }
}

//
// kexInventoryMenu::ProcessInput
//

bool kexInventoryMenu::ProcessInput(inputEvent_t *ev)
{
    kexPlayer *p = kexGame::cLocal->Player();

    if(ev->type == ev_mousedown)
    {
        if(ev->data1 == KMSB_LEFT)
        {
            float mx = (float)kex::cInput->MouseX();
            float my = (float)kex::cInput->MouseY();
            float y = BUTTON_OFFSET;
        
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

            case 2:
                if(bFlashArtifact)
                {
                    return false;
                }

                if(artifactSelected < NUM_ARTIFACTS-1)
                {
                    for(int i = artifactSelected+1; i < NUM_ARTIFACTS; ++i)
                    {
                        if(p->Artifacts() & BIT(i))
                        {
                           if(CursorOnRightArrow(mx, my))
                            {
                                artifactSelected = i;
                                kexGame::cLocal->PlaySound("sounds/select.wav");
                            }
                            break;
                        }
                    }
                }

                if(artifactSelected > 0)
                {
                    for(int i = artifactSelected-1; i >= 0; --i)
                    {
                        if(p->Artifacts() & BIT(i))
                        {
                            if(CursorOnLeftArrow(mx, my))
                            {
                                artifactSelected = i;
                                kexGame::cLocal->PlaySound("sounds/select.wav");
                            }
                            break;
                        }
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

                if(mx >= BUTTON_X && mx <= BUTTON_X + buttonTexture[0]->OriginalWidth() &&
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
// kexInventoryMenu::ShowArtifact
//

void kexInventoryMenu::ShowArtifact(const int artifact)
{
    categorySelected = 2;
    artifactSelected = artifact;
    flashBits = 0;
    flashCount = 0;
    bFlashArtifact = true;

    bButtonPressed[0] = false;
    bButtonPressed[1] = false;
    bButtonPressed[2] = true;
    bButtonPressed[3] = false;

    Toggle();
}

//
// kexInventoryMenu::DrawLeftArrow
//

void kexInventoryMenu::DrawLeftArrow(void)
{
    kexRender::cScreen->DrawTexture(arrows[0], LEFT_ARROW_X, ARROW_OFFSET, 255, 255, 255, 255);
}

//
// kexInventoryMenu::DrawRightArrow
//

void kexInventoryMenu::DrawRightArrow(void)
{
    kexRender::cScreen->DrawTexture(arrows[1], RIGHT_ARROW_Y, ARROW_OFFSET, 255, 255, 255, 255);
}

//
// kexInventoryMenu::CursorOnLeftArrow
//

bool kexInventoryMenu::CursorOnLeftArrow(float &mx, float &my)
{
    return(mx >= LEFT_ARROW_X && mx <= LEFT_ARROW_X + arrows[0]->OriginalWidth() &&
           my >= ARROW_OFFSET  && my <= ARROW_OFFSET  + arrows[0]->OriginalHeight());
}

//
// kexInventoryMenu::CursorOnRightArrow
//

bool kexInventoryMenu::CursorOnRightArrow(float &mx, float &my)
{
    return(mx >= RIGHT_ARROW_Y && mx <= RIGHT_ARROW_Y + arrows[1]->OriginalWidth() &&
           my >= ARROW_OFFSET  && my <= ARROW_OFFSET  + arrows[1]->OriginalHeight());
}

//
// kexInventoryMenu::DrawBackground
//

void kexInventoryMenu::DrawBackground(void)
{
    kexGame::cMenuPanel->DrawPanel(32, 24, 256, 192, 4);
    kexGame::cMenuPanel->DrawInset(152, BUTTON_OFFSET, 117, 87);
    kexGame::cMenuPanel->DrawInset(BUTTON_X, 160, 217, 41);
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

        kexRender::cScreen->DrawTexture(texture, BUTTON_X, BUTTON_OFFSET + offs, 255, 255, 255, 255);
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
// kexInventoryMenu::DrawCenteredImage
//

void kexInventoryMenu::DrawCenteredImage(kexTexture *texture, const float x, const float y)
{
    float w, h;

    w = (float)texture->OriginalWidth();
    h = (float)texture->OriginalHeight();

    kexRender::cScreen->DrawTexture(texture, x-(w*0.5f), y-(h*0.5f), 255, 255, 255, 255);
}

//
// kexInventoryMenu::DrawAutomap
//

void kexInventoryMenu::DrawAutomap(void)
{
    if(kexGame::cLocal->PlayLoop()->AutomapEnabled())
    {
        DrawLeftArrow();
        DrawCenteredImage(mapOpenTexture, PIC_X, PIC_Y);
        font->DrawString(kexGame::cLocal->Translation()->GetString(55), 160, 176, 1, true);
    }
    else
    {
        DrawRightArrow();
        DrawCenteredImage(mapClosedTexture, PIC_X, PIC_Y);
        font->DrawString(kexGame::cLocal->Translation()->GetString(54), 160, 176, 1, true);
    }
}

//
// kexInventoryMenu::DrawArtifacts
//

void kexInventoryMenu::DrawArtifacts(void)
{
    kexPlayer *p = kexGame::cLocal->Player();
    kexStrList labels;
    kexStr label;

    if(p->Artifacts() == 0)
    {
        font->DrawString(kexGame::cLocal->Translation()->GetString(64), 152, 164, 1, true);
        return;
    }

    if(!(p->Artifacts() & BIT(artifactSelected)))
    {
        return;
    }

    if(!bFlashArtifact)
    {
        if(artifactSelected < NUM_ARTIFACTS-1)
        {
            for(int i = artifactSelected+1; i < NUM_ARTIFACTS; ++i)
            {
                if(p->Artifacts() & BIT(i))
                {
                    DrawRightArrow();
                    break;
                }
            }
        }
        if(artifactSelected > 0)
        {
            for(int i = artifactSelected-1; i >= 0; --i)
            {
                if(p->Artifacts() & BIT(i))
                {
                    DrawLeftArrow();
                    break;
                }
            }
        }
    }

    if(!bFlashArtifact || (bFlashArtifact && (flashBits & 0x10) == 0))
    {
        DrawCenteredImage(artifactTextures[artifactSelected], PIC_X, PIC_Y);
    }

    label = kexGame::cLocal->Translation()->GetString(65 + artifactSelected);
    label.Split(labels, '\n');

    for(unsigned int i = 0; i < labels.Length(); ++i)
    {
        float height = font->StringHeight(labels[i], 1, 0);
        font->DrawString(labels[i], 152, 164 + (height * (float)i), 1, true);
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

    case 2:
        DrawArtifacts();
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
