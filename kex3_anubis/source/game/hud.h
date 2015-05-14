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

#ifndef __HUD_H__
#define __HUD_H__

class kexHud
{
public:
    kexHud(void);
    ~kexHud(void);
    
    void                Init(void);
    void                Display(void);
    void                Reset(void);
    void                Update(void);
    void                AddMessage(const char *msg);
    void                SetDamageFlash(void) { damageFlashTicks = 192; }
    void                SetPickupFlash(void) { pickupFlashTicks = 192; }
    void                SetElectrocuteFlash(void) { electrocuteFlashTicks = 192; }
    void                SetTeleportFlash(void) { teleportFlashTicks = 255; }
    
    void                SetPlayer(kexPlayer *p) { player = p; }
    
private:

    typedef struct
    {
        kexStr msg;
        int ticks;
    } hudMessage_t;

#define MAXMESSAGES     4

    void                DrawHealthBar(void);
    void                DrawAmmoBar(void);
    void                DrawBackPic(void);
    void                DrawFillPic(void);
    void                DrawCompass(void);
    void                DrawAbilityIcons(void);
    void                DrawFlash(void);
    void                DrawCrosshair(void);
    void                DrawMessages(void);
    void                DrawMessage(const char *msg, const float x, const float y);
    void                DrawDot(const float x, const float y, bool bRedDot, bool bOn);
    void                DrawDots(void);
    void                DrawAirSupply(void);
    
    kexTexture          *backImage;
    kexTexture          *airSupplyFront;
    kexTexture          *airSupplyBack;
    kexTexture          *fillPic;
    kexTexture          *dolphinPic;
    kexTexture          *vulturePic;
    kexTexture          *crossHairPic;
    kexPlayer           *player;
    float               currentHealth;
    hudMessage_t        messages[MAXMESSAGES];
    int                 currentMessage;
    int                 damageFlashTicks;
    int                 pickupFlashTicks;
    int                 electrocuteFlashTicks;
    int                 teleportFlashTicks;
    float               airSupplyGfxOffset;
};

#endif
