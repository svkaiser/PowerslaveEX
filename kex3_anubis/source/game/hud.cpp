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
    this->currentHealth = 0;
    this->currentMessage = 0;
    this->airSupplyGfxOffset = -300;
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
    backImage       = kexRender::cTextures->Cache("gfx/hud.png", TC_CLAMP, TF_NEAREST);
    airSupplyFront  = kexRender::cTextures->Cache("gfx/airsupply_front.png", TC_CLAMP, TF_NEAREST);
    airSupplyBack   = kexRender::cTextures->Cache("gfx/airsupply_back.png", TC_CLAMP, TF_NEAREST);
    fillPic         = kexRender::cTextures->Cache("gfx/menu/menu_bg.png", TC_REPEAT, TF_NEAREST);
    dolphinPic      = kexRender::cTextures->Cache("gfx/dolphin.png", TC_CLAMP, TF_NEAREST);
    vulturePic      = kexRender::cTextures->Cache("gfx/vulture.png", TC_CLAMP, TF_NEAREST);
    crossHairPic    = kexRender::cTextures->Cache("gfx/crosshair.png", TC_CLAMP, TF_NEAREST);
}

//
// kexHud::Reset
//

void kexHud::Reset(void)
{
    currentHealth = 0;
    currentMessage = 0;

    damageFlashTicks = 0;
    pickupFlashTicks = 0;
    electrocuteFlashTicks = 0;
    teleportFlashTicks = 0;

    for(int i = 0; i < MAXMESSAGES; ++i)
    {
        messages[i].msg.Clear();
        messages[i].ticks = 0;
    }
}

//
// kexHud::DrawAmmoBar
//

void kexHud::DrawAmmoBar(void)
{
    kexCpuVertList *vl = kexRender::cVertList;
    playerWeapons_t weapon = player->PendingWeapon();
    
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
// kexHud::DrawHealthBar
//

void kexHud::DrawHealthBar(void)
{
    kexCpuVertList *vl = kexRender::cVertList;
    float maxHealth = (float)kexPlayer::maxHealth;

    currentHealth = ((float)player->Actor()->Health() - currentHealth) * 0.125f + currentHealth;
    float width = kexMath::FMod(currentHealth, maxHealth) / maxHealth;

    kexRender::cTextures->whiteTexture->Bind();
    vl->AddQuad(193, 222, 0, 88 * width, 8, 255, 32, 32, 255);
    vl->AddQuad(193, 222, 0, 88 * width, 1, 64, 8, 8, 255);
    vl->DrawElements();
}

//
// kexHud::DrawFlash
//

void kexHud::DrawFlash(void)
{
    kexCpuVertList *vl = kexRender::cVertList;

    if(damageFlashTicks <= 0 &&
       pickupFlashTicks <= 0 &&
       electrocuteFlashTicks <= 0 &&
       teleportFlashTicks <= 0)
    {
        return;
    }

    kexRender::cScreen->SetOrtho(true);

    kexRender::cTextures->whiteTexture->Bind();

    if(damageFlashTicks > 0)
    {
        vl->AddQuad(0, 0, 320, 240, 255, 32, 32, damageFlashTicks);
    }

    if(pickupFlashTicks > 0)
    {
        vl->AddQuad(0, 0, 320, 240, 32, 32, 255, pickupFlashTicks);
    }

    if(electrocuteFlashTicks > 0)
    {
        vl->AddQuad(0, 0, 320, 240, 32, 255, 32, electrocuteFlashTicks);
    }

    if(teleportFlashTicks > 0)
    {
        vl->AddQuad(0, 0, 320, 240, 0, 255, 8, teleportFlashTicks);
    }

    vl->DrawElements();

    // change back
    kexRender::cScreen->SetOrtho();
}

//
// kexHud::DrawMessage
//

void kexHud::DrawMessage(const char *msg, const float x, const float y)
{
    float flash = (float)kexGame::cLocal->PlayLoop()->Ticks();
    byte pulse = (byte)(kexMath::Sin(flash * 0.1f) * 64.0f) + 191;

    kexGame::cLocal->DrawSmallString(msg, x, y, 1, true, pulse, pulse, pulse);
}

//
// kexHud::DrawMessages
//

void kexHud::DrawMessages(void)
{
    float y = 24;

    for(int i = 0; i < MAXMESSAGES; ++i)
    {
        int msg = ((currentMessage-1)-i)&3;

        if(messages[msg].ticks <= 0)
        {
            break;
        }

        DrawMessage(messages[msg].msg, 160, y);
        y += 10;
    }
}

//
// kexHud::AddMessage
//

void kexHud::AddMessage(const char *msg)
{
    messages[currentMessage].msg = msg;
    messages[currentMessage].ticks = 300;

    currentMessage = (currentMessage + 1) % MAXMESSAGES;
}

//
// kexHud::DrawCompass
//

void kexHud::DrawCompass(void)
{
    kexCpuVertList *vl = kexRender::cVertList;
    float tw = (float)backImage->Width();
    float th = (float)backImage->Height();
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
// kexHud::DrawDot
//

void kexHud::DrawDot(const float x, const float y, bool bRedDot, bool bOn)
{
    kexCpuVertList *vl = kexRender::cVertList;
    float u1, u2;
    float v1, v2;

    backImage->Bind();

    v1 = 48.0f / (float)backImage->Height();
    v2 = 54.0f / (float)backImage->Height();

    if(!bOn)
    {
        u1 = 136.0f / (float)backImage->Width();
        u2 = 146.0f / (float)backImage->Width();
    }
    else
    {
        if(!bRedDot)
        {
            u1 = 146.0f / (float)backImage->Width();
            u2 = 156.0f / (float)backImage->Width();
        }
        else
        {
            u1 = 156.0f / (float)backImage->Width();
            u2 = 166.0f / (float)backImage->Width();
        }
    }

    vl->AddQuad(x, y, 0, 9, 6, u1, v1, u2, v2, 255, 255, 255, 255);
    vl->DrawElements();
}

//
// kexHud::DrawDots
//

void kexHud::DrawDots(void)
{
    kexPlayer *player = kexGame::cLocal->Player();
    int health = player->Actor()->Health();
    int maxHealth;

    for(int i = 0; i < NUMPLAYERWEAPONS; ++i)
    {
        if(!player->WeaponOwned(i))
        {
            continue;
        }

        DrawDot(55 + (10.0f * (float)i), 228, false, player->PendingWeapon() == i);
    }

    for(int i = 0; i < player->Ankahs(); ++i)
    {
        maxHealth = kexPlayer::maxHealth * (i+1);
        DrawDot(196 + (10.0f * (float)i), 228, true, health >= maxHealth);
    }
}

//
// kexHud::DrawFillPic
//

void kexHud::DrawFillPic(void)
{
    kexCpuVertList *vl = kexRender::cVertList;
    float width = (float)kexRender::cScreen->SCREEN_WIDTH;
    float x = 0;
    float y = 0;
    float w = width;
    float h = (float)kexRender::cScreen->SCREEN_HEIGHT;

    kexRender::cScreen->SetAspectDimentions(x, y, w, h);
    w = ((float)kexRender::cScreen->SCREEN_WIDTH - w);
    kexRender::cScreen->DrawFillPic(fillPic, -x, 216, width - (width - w), 32);
    kexRender::cScreen->DrawFillPic(fillPic, width, 216, width - w, 32);

    kexRender::cTextures->whiteTexture->Bind();

    vl->AddQuad(-x, 216, width + w, 1, 156, 100, 70, 255);
    vl->AddQuad(-x, 217, width + w, 1, 80, 40, 10, 255);
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
// kexHud::DrawAbilityIcons
//

void kexHud::DrawAbilityIcons(void)
{
    kexPlayer *player = kexGame::cLocal->Player();
    byte pulse;

    if(player->Abilities() == 0)
    {
        return;
    }

    if(player->Abilities() & PAB_DOLPHIN)
    {
        kexRender::cScreen->DrawTexture(dolphinPic, 25, 12);
    }

    if(player->Abilities() & PAB_VULTURE)
    {
        kexRender::cScreen->DrawTexture(vulturePic, 52, 12);
    }

    pulse = (byte)(kexMath::Sin((float)kex::cSession->GetTime() * 0.0035f) * 64.0f) + 64;
    kexRender::cBackend->SetBlend(GLSRC_ONE, GLDST_ONE);

    if(player->Abilities() & PAB_DOLPHIN)
    {
        kexRender::cScreen->DrawTexture(dolphinPic, 25, 12, pulse, pulse, pulse, 255);
    }

    if(player->Abilities() & PAB_VULTURE)
    {
        kexRender::cScreen->DrawTexture(vulturePic, 52, 12, pulse, pulse, pulse, 255);
    }

    kexRender::cBackend->SetBlend(GLSRC_SRC_ALPHA, GLDST_ONE_MINUS_SRC_ALPHA);
}

//
// kexHud::DrawAirSupply
//

void kexHud::DrawAirSupply(void)
{
    kexPlayer *player = kexGame::cLocal->Player();
    kexCpuVertList *vl = kexRender::cVertList;
    float x, y, w, h;
    float scale;

    if(!(player->Actor()->Flags() & AF_INWATER))
    {
        airSupplyGfxOffset = (-300 - airSupplyGfxOffset) * 0.08f + airSupplyGfxOffset;
    }

    airSupplyGfxOffset = (116 - airSupplyGfxOffset) * 0.25f + airSupplyGfxOffset;

    w = (float)airSupplyBack->Width();
    h = (float)airSupplyBack->Height();
    x = 272 - (w * 0.5f);
    y = (airSupplyGfxOffset - 64) - (h * 0.5f);

    airSupplyBack->Bind();
    vl->AddQuad(x, y, 0, w, h, 0, 0, 1, 1, 255, 255, 255, 224);
    vl->DrawElements();

    scale = (float)player->AirSupply() / 64.0f;

    w = (float)airSupplyFront->Width() * scale;
    h = (float)airSupplyFront->Height() * scale;
    x = 272 - (w * 0.5f);
    y = (airSupplyGfxOffset - 64) - (h * 0.5f);

    airSupplyFront->Bind();
    vl->AddQuad(x, y, 0, w, h, 0, 0, 1, 1, 255, 255, 255, 224);
    vl->DrawElements();
}

//
// kexHud::DrawCrosshair
//

void kexHud::DrawCrosshair(void)
{
    float w, h;
    float vw, vh;
    float x, y;
    int weaponState;

    if(!kexPlayLoop::cvarCrosshair.GetBool())
    {
        return;
    }

    weaponState = kexGame::cLocal->Player()->Weapon().State();

    if( weaponState == WS_HOLDSTER ||
        weaponState == WS_RAISE ||
        weaponState == WS_LOWER)
    {
        return;
    }

    w = (float)crossHairPic->OriginalWidth();
    h = (float)crossHairPic->OriginalHeight();
    vw = (float)kex::cSystem->VideoWidth();
    vh = (float)kex::cSystem->VideoHeight();
    x = vw * 0.5f;
    y = vh * 0.5f;

    kexRender::cBackend->SetOrtho();
    crossHairPic->Bind();

    kexRender::cVertList->BindDrawPointers();
    kexRender::cVertList->AddQuad(x-(w*0.5f), y-(h*0.5f), w, h, 255, 255, 255, 128);
    kexRender::cVertList->DrawElements();
}

//
// kexHud::Update
//

void kexHud::Update(void)
{
    for(int i = 0; i < MAXMESSAGES; ++i)
    {
        if(messages[i].ticks > 0)
        {
            messages[i].ticks--;
        }
    }

    if(damageFlashTicks > 0) damageFlashTicks -= 8;
    if(pickupFlashTicks > 0) pickupFlashTicks -= 8;
    if(electrocuteFlashTicks > 0) electrocuteFlashTicks -= 4;
    if(teleportFlashTicks > 0) teleportFlashTicks -= 4;
}

//
// kexHud::Display
//

void kexHud::Display(void)
{
    kexRender::cScreen->SetOrtho();
    
    kexRender::cBackend->SetState(GLSTATE_DEPTHTEST, false);
    kexRender::cBackend->SetState(GLSTATE_SCISSOR, false);
    kexRender::cBackend->SetState(GLSTATE_ALPHATEST, true);
    kexRender::cBackend->SetState(GLSTATE_BLEND, true);
    kexRender::cBackend->SetBlend(GLSRC_SRC_ALPHA, GLDST_ONE_MINUS_SRC_ALPHA);
    
    kexRender::cVertList->BindDrawPointers();
    
    DrawFillPic();

    DrawAmmoBar();

    DrawHealthBar();

    DrawBackPic();

    DrawCompass();

    DrawDots();

    DrawAirSupply();

    DrawAbilityIcons();

    DrawFlash();

    DrawMessages();

    DrawCrosshair();
}
