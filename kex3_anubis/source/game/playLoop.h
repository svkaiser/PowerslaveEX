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

#ifndef __PLAYLOOP_H__
#define __PLAYLOOP_H__

#include "renderView.h"
#include "renderScene.h"
#include "hud.h"
#include "inventoryMenu.h"

class kexPlayLoop : public kexGameLoop
{
public:
    kexPlayLoop(void);
    ~kexPlayLoop(void);

    void                        Init(void);
    void                        Start(void);
    void                        Stop(void);
    void                        Draw(void);
    void                        Tick(void);
    void                        Print(const char *string);
    bool                        ProcessInput(inputEvent_t *ev);
    const int                   GetWaterVelocityPoint(const float x, const float y);
    void                        ZoomAutomap(const float amount);
    void                        RequestExit(const char *map);
    void                        RequestExit(const gameState_t gameState);
    void                        RestartLevel(void);
    
    kexRenderView               &View(void) { return renderView; }
    kexRenderScene              &RenderScene(void) { return renderScene; }

    const bool                  RestartRequested(void) const { return bRestartLevel; }

    const int                   Ticks(void) const { return ticks; }
    const int                   MaxWaterMagnitude(void) { return waterMaxMagnitude; }
    void                        PickupFlash(void) { hud.SetPickupFlash(); }
    void                        DamageFlash(void) { hud.SetDamageFlash(); }
    void                        ElectrocuteFlash(void) { hud.SetElectrocuteFlash(); }
    void                        TeleportFlash(void) { hud.SetTeleportFlash(); }
    const bool                  AutomapEnabled(void) { return bShowAutomap; }
    void                        ToggleAutomap(const bool bToggle) { bShowAutomap = bToggle; }
    void                        ToggleMapAll(const bool bToggle) { bMapAll = bToggle; }
    const bool                  LevelIsMapped(void) const { return bMapAll; }
    const bool                  IsPaused(void) const { return bPaused; }
    void                        TogglePause(const bool bToggle) { bPaused = bToggle; }
    kexInventoryMenu            &InventoryMenu(void) { return inventoryMenu; }

    static bool                 bPrintStats;
    static kexCvar              cvarCrosshair;
    
private:
    void                        FadeToBlack(void);
    void                        DrawFadeIn(void);
    void                        InitWater(void);
    void                        UpdateWater(void);
    void                        WaterBubbles(void);
    void                        DrawAutomap(void);
    void                        DrawAutomapArrow(kexRenderView &view, const float angle, const kexVec3 &pos,
                                                 const float size, const byte r, const byte g, const byte b);
    void                        DrawAutomapActors(kexRenderView &view);
    void                        DrawAutomapWalls(kexRenderView &view);
    void                        PrintStats(void);

    gameState_t                 requestedGameState;
    int                         ticks;
    short                       fadeInTicks;
    kexHud                      hud;
    kexInventoryMenu            inventoryMenu;
    kexRenderView               renderView;
    kexRenderScene              renderScene;
    int                         waterAccelPoints[16][16];
    int                         waterVelocityPoints[16][16];
    int                         waterMaxMagnitude;
    int                         restartDelayTicks;
    bool                        bShowAutomap;
    bool                        bMapAll;
    bool                        bPaused;
    bool                        bFadeOut;
    bool                        bNoFadeOutPause;
    bool                        bRestartLevel;
    float                       automapZoom;
    const char                  *mapChange;
    uint64_t                    debugTickTime;
};

#endif
