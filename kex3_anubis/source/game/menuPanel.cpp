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
//      Menu Panel Drawing Utilities
//

#include "kexlib.h"
#include "game.h"
#include "renderMain.h"
#include "menuPanel.h"

static kexMenuPanel menuPanelLocal;
kexMenuPanel *kexGame::cMenuPanel = &menuPanelLocal;

//
// kexMenuPanel::kexMenuPanel
//

kexMenuPanel::kexMenuPanel(void)
{
}

//
// kexMenuPanel::~kexMenuPanel
//

kexMenuPanel::~kexMenuPanel(void)
{
}

//
// kexMenuPanel::Init
//

void kexMenuPanel::Init(void)
{
    bgTexture = kexRender::cTextures->Cache("gfx/menu/menu_bg.png", TC_REPEAT, TF_NEAREST);
    buttonTexture[0] = kexRender::cTextures->Cache("gfx/menu/menubutton_up.png", TC_CLAMP, TF_NEAREST);
    buttonTexture[1] = kexRender::cTextures->Cache("gfx/menu/menubutton_down.png", TC_CLAMP, TF_NEAREST);
    arrows[0] = kexRender::cTextures->Cache("gfx/menu/menuarrow_left.png", TC_CLAMP, TF_NEAREST);
    arrows[1] = kexRender::cTextures->Cache("gfx/menu/menuarrow_right.png", TC_CLAMP, TF_NEAREST);

    font = kexFont::Alloc("smallfont");
}

//
// kexMenuPanel::DrawPanel
//

void kexMenuPanel::DrawPanel(const float x, const float y, const float w, const float h,
                             const float borderSize)
{
    kexCpuVertList *vl = kexRender::cVertList;

    static const byte w_rgba[4] = { 255, 255, 255, 160 };
    static const byte b_rgba[4] = { 80, 40, 10, 144 };

    kexRender::cScreen->DrawFillPic(bgTexture, x, y, w, h);

    kexRender::cBackend->SetBlend(GLSRC_DST_COLOR, GLDST_ONE_MINUS_SRC_ALPHA);

    vl->AddVertex(x, y, 0, 0, 0, w_rgba);
    vl->AddVertex(x+w, y, 0, 0, 0, w_rgba);
    vl->AddVertex(x+borderSize, y+borderSize, 0, 0, 0, w_rgba);
    vl->AddVertex(x+w-borderSize, y+borderSize, 0, 0, 0, w_rgba);
    vl->AddVertex(x, y, 0, 0, 0, w_rgba);
    vl->AddVertex(x+borderSize, y+borderSize, 0, 0, 0, w_rgba);
    vl->AddVertex(x, y+h, 0, 0, 0, w_rgba);
    vl->AddVertex(x+borderSize, y+h-borderSize, 0, 0, 0, w_rgba);
    vl->AddVertex(x+borderSize, y+h-borderSize, 0, 0, 0, b_rgba);
    vl->AddVertex(x+w-borderSize, y+h-borderSize, 0, 0, 0, b_rgba);
    vl->AddVertex(x, y+h, 0, 0, 0, b_rgba);
    vl->AddVertex(x+w, y+h, 0, 0, 0, b_rgba);
    vl->AddVertex(x+w-borderSize, y+borderSize, 0, 0, 0, b_rgba);
    vl->AddVertex(x+w, y, 0, 0, 0, b_rgba);
    vl->AddVertex(x+w-borderSize, y+h-borderSize, 0, 0, 0, b_rgba);
    vl->AddVertex(x+w, y+h, 0, 0, 0, b_rgba);
    
    vl->AddTriangle(0,  2,  1);
    vl->AddTriangle(1,  2,  3);
    vl->AddTriangle(4,  6,  5);
    vl->AddTriangle(5,  6,  7);
    vl->AddTriangle(8,  10, 9);
    vl->AddTriangle(9,  10, 11);
    vl->AddTriangle(12, 14, 13);
    vl->AddTriangle(13, 14, 15);

    kexRender::cTextures->whiteTexture->Bind();
    vl->DrawElements();
}

//
// kexMenuPanel::DrawInset
//

void kexMenuPanel::DrawInset(const float x, const float y, const float w, const float h)
{
    kexCpuVertList *vl = kexRender::cVertList;

    kexRender::cTextures->whiteTexture->Bind();
    kexRender::cBackend->SetBlend(GLSRC_ZERO, GLDST_SRC_COLOR);

    vl->AddQuad(x, y, w, h, 192, 160, 128, 255);
    vl->DrawElements();

    kexRender::cBackend->SetBlend(GLSRC_SRC_ALPHA, GLDST_ONE_MINUS_SRC_ALPHA);

    vl->AddQuad(x, y, w, 1, 70, 36, 18, 255);
    vl->AddQuad(x, y, 1, h, 70, 36, 18, 255);
    vl->AddQuad(x+w, y, 1, h, 156, 100, 70, 255);
    vl->AddQuad(x, y+h-1, w, 1, 156, 100, 70, 255);
    vl->AddQuad(x+1, y+1, w-1, 1, 0, 0, 0, 255);
    vl->AddQuad(x+w-1, y+1, 1, h-2, 0, 0, 0, 255);
    vl->AddQuad(x+1, y+h-2, w-1, 1, 0, 0, 0, 255);
    vl->AddQuad(x+1, y+1, 1, h-2, 0, 0, 0, 255);

    vl->DrawElements();
}

//
// kexMenuPanel::DrawButton
//

void kexMenuPanel::DrawButton(const float x, const float y, bool bPressed, const char *text)
{
    kexRender::cScreen->DrawTexture(buttonTexture[bPressed], x, y, 255, 255, 255, 255);
    font->DrawString(text, x+4, y+4, 1, false);
}

//
// kexMenuPanel::PointOnButton
//

bool kexMenuPanel::PointOnButton(const float x, const float y, const float mx, const float my)
{
    return kexRender::cScreen->PointOnPic(buttonTexture[0], x, y, mx, my);
}

//
// kexMenuPanel::TestPointInButtonSet
//

bool kexMenuPanel::TestPointInButtonSet(buttonSet_t *buttonSet)
{
    float y = buttonSet->y;
    float mx = (float)kex::cInput->MouseX();
    float my = (float)kex::cInput->MouseY();
        
    kexRender::cScreen->CoordsToRenderScreenCoords(mx, my);
    
    for(unsigned int i = 0; i < buttonSet->labels.Length(); ++i)
    {
        if(i == buttonSet->pressedIndex)
        {
            y += 24;
            continue;
        }
        
        if(PointOnButton(buttonSet->x, y, mx, my))
        {
            buttonSet->pressedIndex = i;
            return true;
        }
        
        y += 24;
    }
    
    return false;
}

//
// kexMenuPanel::DrawButtonSet
//

void kexMenuPanel::DrawButtonSet(buttonSet_t *buttonSet)
{
    for(unsigned int i = 0; i < buttonSet->labels.Length(); ++i)
    {
        float offs = (24 * (float)i);
        bool bPressed = (buttonSet->pressedIndex == i);
        
        DrawButton(buttonSet->x, buttonSet->y + offs, bPressed, buttonSet->labels[i].c_str());
    }
}

//
// kexMenuPanel::UpdateSelectButton
//

void kexMenuPanel::UpdateSelectButton(selectButton_t *button)
{
    float mx = (float)kex::cInput->MouseX();
    float my = (float)kex::cInput->MouseY();
        
    kexRender::cScreen->CoordsToRenderScreenCoords(mx, my);

    button->bHover = (mx >= button->x && mx <= button->x + button->w &&
                      my >= button->y && my <= button->y + button->h);
}

//
// kexMenuPanel::TestSelectButtonInput
//

bool kexMenuPanel::TestSelectButtonInput(selectButton_t *button, inputEvent_t *ev)
{
    switch(ev->type)
    {
    case ev_mousedown:
        if(ev->data1 == KMSB_LEFT)
        {
            if(button->bHover)
            {
                button->bPressed = true;
            }
        }
        break;

    case ev_mouseup:
        if(ev->data1 == KMSB_LEFT)
        {
            button->bPressed = false;

            if(button->bHover)
            {
                return true;
            }
        }
        break;
    }

    return false;
}

//
// kexMenuPanel::DrawSelectButton
//

void kexMenuPanel::DrawSelectButton(selectButton_t *button)
{
    kexCpuVertList *vl = kexRender::cVertList;
    byte r, g, b;

    kexRender::cTextures->whiteTexture->Bind();
    kexRender::cBackend->SetBlend(GLSRC_SRC_ALPHA, GLDST_ONE_MINUS_SRC_ALPHA);

    vl->AddQuad(button->x, button->y, button->w, 1, 0, 0, 0, 255);
    vl->AddQuad(button->x, button->y, 1, button->h, 0, 0, 0, 255);
    vl->AddQuad(button->x+button->w, button->y, 1, button->h+1, 0, 0, 0, 255);
    vl->AddQuad(button->x, button->y+button->h, button->w, 1, 0, 0, 0, 255);

    r = 96;
    g = 96;
    b = 120;

    if(button->bPressed)
    {
        r /= 2;
        g /= 2;
        b /= 2;
    }
    else if(button->bHover)
    {
        r *= 2;
        g *= 2;
        b *= 2;
    }

    vl->AddQuad(button->x+1, button->y+1, button->w-1, 1, r, g, b, 255);
    vl->AddQuad(button->x+1, button->y+1, 1, button->h-1, r, g, b, 255);

    r = 16;
    g = 16;
    b = 80;

    if(button->bPressed)
    {
        r /= 2;
        g /= 2;
        b /= 2;
    }
    else if(button->bHover)
    {
        r *= 2;
        g *= 2;
        b *= 2;
    }

    vl->AddQuad(button->x+button->w-1, button->y+1, 1, button->h-1, r, g, b, 255);
    vl->AddQuad(button->x+1, button->y+button->h-1, button->w-1, 1, r, g, b, 255);

    r = 48;
    g = 48;
    b = 120;

    if(button->bPressed)
    {
        r /= 2;
        g /= 2;
        b /= 2;
    }
    else if(button->bHover)
    {
        r *= 2;
        g *= 2;
        b *= 2;
    }

    vl->AddQuad(button->x+2, button->y+2, button->w-3, button->h-3, r, g, b, 255);
    vl->DrawElements();

    font->DrawString(button->label, button->x+(button->w*0.5f), button->y+(button->h*0.5f)-2, 1, true);
}

//
// kexMenuPanel::DrawLeftArrow
//

void kexMenuPanel::DrawLeftArrow(const float x, const float y)
{
    kexRender::cScreen->DrawTexture(arrows[0], x, y, 255, 255, 255, 255);
}

//
// kexMenuPanel::DrawRightArrow
//

void kexMenuPanel::DrawRightArrow(const float x, const float y)
{
    kexRender::cScreen->DrawTexture(arrows[1], x, y, 255, 255, 255, 255);
}

//
// kexMenuPanel::CursorOnLeftArrow
//

bool kexMenuPanel::CursorOnLeftArrow(const float x, const float y)
{
    float mx = (float)kex::cInput->MouseX();
    float my = (float)kex::cInput->MouseY();
        
    kexRender::cScreen->CoordsToRenderScreenCoords(mx, my);

    return kexRender::cScreen->PointOnPic(arrows[0], x, y, mx, my);
}

//
// kexMenuPanel::CursorOnRightArrow
//

bool kexMenuPanel::CursorOnRightArrow(const float x, const float y)
{
    float mx = (float)kex::cInput->MouseX();
    float my = (float)kex::cInput->MouseY();
        
    kexRender::cScreen->CoordsToRenderScreenCoords(mx, my);

    return kexRender::cScreen->PointOnPic(arrows[1], x, y, mx, my);
}
