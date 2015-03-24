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
}

//
// kexMenuPanel::DrawPanel
//

void kexMenuPanel::DrawPanel(const float x, const float y, const float w, const float h,
                             const float borderSize)
{
    kexCpuVertList *vl = kexRender::cVertList;

    byte w_rgba[4] = { 255, 255, 255, 160 };
    byte b_rgba[4] = { 80, 40, 10, 144 };

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
