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
//      Heads up display
//

#include "kexlib.h"
#include "game.h"
#include "renderMain.h"
#include "hud.h"

//
// kexHud::kexHud
//

kexHud::kexHud(void)
{
    this->backImage = NULL;
    this->player = NULL;
}

//
// kexHud::~kexHud
//

kexHud::~kexHud(void)
{
}

//
// kexHud::Init
//

void kexHud::Init(void)
{
    backImage = kexRender::cTextures->Cache("gfx/hud.png", TC_CLAMP, TF_NEAREST);
}

//
// kexHud::DrawAmmoBar
//

void kexHud::DrawAmmoBar(void)
{
    kexCpuVertList *vl = kexRender::cVertList;
    playerWeapons_t weapon = player->CurrentWeapon();
    
    float max = (float)kexGame::cLocal->WeaponInfo(weapon)->maxAmmo;
    float width = 1;
    
    if(max > 0)
    {
        width = (float)player->GetAmmo(weapon) / max;
    }
    
    kexRender::cTextures->whiteTexture->Bind();
    vl->AddQuad(52, 222, 0, 88 * width, 8, 32, 32, 255, 255);
    vl->AddQuad(52, 222, 0, 88 * width, 1, 8, 8, 64, 255);
    vl->DrawElements();
}

//
// kexHud::DrawCompass
//

void kexHud::DrawCompass(void)
{
    kexCpuVertList *vl = kexRender::cVertList;
    float tw = (float)backImage->OriginalWidth();
    float th = (float)backImage->OriginalHeight();
    float u1 = 225.0f / tw;
    float u2 = 1;
    float v1 = 0;
    float v2 = 22.0f / th;
    float angle = kexMath::Rad2Deg(player->Actor()->Yaw());
    float x = 146;
    float y = 217;
    
    backImage->Bind();
    
    if(angle >= 22.5f && angle < 67.5f)
    {
        v1 = 21.0f / th;
        v2 = 43.0f / th;
    }
    else if(angle >= 67.5f && angle < 112.5f)
    {
        v1 = 42.0f / th;
        v2 = 64.0f / th;
    }
    else if(angle >= 112.5f && angle < 157.5f)
    {
        v2 = 21.0f / th;
        v1 = 43.0f / th;
    }
    else if((angle >= 157.5f && angle <= 180) || (angle >= -180 && angle <= -157.5f))
    {
        v2 = 0;
        v1 = 22.0f / th;
    }
    else if(angle >= -157.5f && angle < -112.5f)
    {
        float tmp = u1;
        
        v2 = 21.0f / th;
        v1 = 43.0f / th;
        
        x -= 4;
        
        u1 = u2;
        u2 = tmp;
    }
    else if(angle >= -112.5f && angle < -67.5f)
    {
        float tmp = u1;
        
        v2 = 42.0f / th;
        v1 = 64.0f / th;
        
        x -= 4;
        
        u1 = u2;
        u2 = tmp;
    }
    else if(angle >= -67.5f && angle < -22.5f)
    {
        float tmp = u1;
        
        v1 = 21.0f / th;
        v2 = 43.0f / th;
        
        x -= 4;
        
        u1 = u2;
        u2 = tmp;
    }
    
    vl->AddQuad(x, y, 0, 31, 22, u1, v1, u2, v2, 255, 255, 255, 255);
    vl->DrawElements();
}

//
// kexHud::DrawBackPic
//

void kexHud::DrawBackPic(void)
{
    kexCpuVertList *vl = kexRender::cVertList;
    
    backImage->Bind();
    
    vl->AddQuad(0, 192, 0, 64, 64, 0, 0, 0.25f, 1, 255, 255, 255, 255);
    vl->AddQuad(64, 216, 0, 96, 24, 0.25f, 0, 0.625f, 0.375f, 255, 255, 255, 255);
    vl->AddQuad(160, 216, 0, 96, 24, 0.25f, 0.375f, 0.625f, 0.75f, 255, 255, 255, 255);
    vl->AddQuad(256, 192, 0, 64, 64, 0.625f, 0, 0.875f, 1, 255, 255, 255, 255);
    vl->DrawElements();
}

//
// kexHud::Display
//

void kexHud::Display(void)
{
    kexRender::cVertList->BindDrawPointers();
    
    DrawAmmoBar();
    DrawBackPic();
    DrawCompass();
}
