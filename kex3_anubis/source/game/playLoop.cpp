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
//      In-Game logic. Handles level transitions (fading) automap,
//      environement effects, player hud, world updates, etc
//

#include "kexlib.h"
#include "renderMain.h"
#include "renderView.h"
#include "game.h"
#include "localization.h"

kexCvar kexPlayLoop::cvarCrosshair("g_showcrosshair", CVF_BOOL|CVF_CONFIG, "0", "Displays crosshair");

//
// gprint
//

COMMAND(gprint)
{
    if(kexGame::cLocal->GameState() != GS_LEVEL)
    {
        return;
    }

    int argc = kex::cCommands->GetArgc();

    if(argc != 2)
    {
        kex::cSystem->Printf("gprint <message>\n");
        return;
    }

    kexGame::cLocal->PlayLoop()->Print(kex::cCommands->GetArgv(1));
}

//
// automap
//

COMMAND(automap)
{
    if(kexGame::cLocal->GameState() != GS_LEVEL)
    {
        return;
    }

    kexPlayLoop *pl = kexGame::cLocal->PlayLoop();
    pl->ToggleAutomap(pl->AutomapEnabled() ^ 1);
}

//
// mapall
//

COMMAND(mapall)
{
    if(kexGame::cLocal->GameState() != GS_LEVEL)
    {
        return;
    }

    kexGame::cLocal->PlayLoop()->ToggleMapAll(true);
}

//
// pausegame
//

COMMAND(pausegame)
{
    if(kexGame::cLocal->GameState() != GS_LEVEL)
    {
        return;
    }

    if(kex::cCommands->GetArgc() < 1)
    {
        return;
    }

    kexGame::cLocal->PlayLoop()->TogglePause(kexGame::cLocal->PlayLoop()->IsPaused() ^ 1);
}

//
// inventorymenu
//

COMMAND(inventorymenu)
{
    if(kexGame::cLocal->GameState() != GS_LEVEL)
    {
        return;
    }

    if(kexGame::cLocal->ActiveMenu() != NULL)
    {
        return;
    }

    if(kex::cCommands->GetArgc() < 1)
    {
        return;
    }

    kexGame::cLocal->PlayLoop()->InventoryMenu().Toggle();
}

//
// puke
//

COMMAND(puke)
{
    if(kexGame::cLocal->GameState() != GS_LEVEL)
    {
        return;
    }

    if(kexGame::cLocal->ActiveMenu() != NULL)
    {
        return;
    }

    if(kex::cCommands->GetArgc() != 2)
    {
        kex::cSystem->Printf("puke <script number>\n");
        return;
    }

    kexGame::cScriptManager->CallDelayedMapScript(kexStr::Format("mapscript_%i_root",
        atoi(kex::cCommands->GetArgv(1))), NULL, 0);
}

//
// statplayloop
//

COMMAND(statplayloop)
{
    kexPlayLoop::bPrintStats ^= 1;
}

//
// kexPlayLoop::kexPlayLoop
//

kexPlayLoop::kexPlayLoop(void)
{
}

bool kexPlayLoop::bPrintStats = false;

//
// kexPlayLoop::~kexPlayLoop
//

kexPlayLoop::~kexPlayLoop(void)
{
}

//
// kexPlayLoop::Init
//

void kexPlayLoop::Init(void)
{
    hud.Init();
    inventoryMenu.Init();
    bShowAutomap = false;
    bPaused = false;
    bFadeOut = false;
    bNoFadeOutPause = false;
    bRestartLevel = false;
    mapChange = NULL;
    restartDelayTicks = 0;
}

//
// kexPlayLoop::Start
//

void kexPlayLoop::Start(void)
{
    kexGameLocal *game = kexGame::cLocal;

    ticks = 0;
    bRestartLevel = false;
    
    if(game->Player()->Actor() == NULL)
    {
        kex::cSystem->Warning("No player starts present\n");
        game->SetGameState(GS_TITLE);
        return;
    }
    
    hud.SetPlayer(game->Player());

    renderScene.SetWorld(kexGame::cWorld);
    renderScene.InitVertexBuffer();
    renderScene.DLights().Init();

    kexGame::cScriptManager->LoadLevelScript(game->ActiveMap()->script.c_str());
    kexGame::cScriptManager->CallDelayedMapScript(0, game->Player()->Actor(), 0);

    game->Player()->Ready();
    hud.Reset();
    inventoryMenu.Reset();
    InitWater();

    kex::cSound->PlayMusic(game->ActiveMap()->musicTrack.c_str());

    automapZoom = 2048;
    fadeInTicks = 0xff;
    restartDelayTicks = 60;
    requestedGameState = GS_NONE;
    
    bShowAutomap = false;
    bMapAll = false;
    bPaused = false;
    bFadeOut = false;
    mapChange = NULL;

    kex::cSession->ForceSingleFrame();
}

//
// kexPlayLoop::Stop
//

void kexPlayLoop::Stop(void)
{
    kexPlayer *player = kexGame::cLocal->Player();
    
    fadeInTicks = 0;
    player->Health() = player->Actor()->Health();

    if(!bFadeOut)
    {
        FadeToBlack();
    }
    
    kex::cSound->StopMusic();

    renderScene.DestroyVertexBuffer();

    kexGame::cScriptManager->DestroyLevelScripts();
    kexGame::cWorld->UnloadMap();

    kex::cSession->ForceSingleFrame();
}

//
// kexPlayLoop::Draw
//

void kexPlayLoop::Draw(void)
{
    kexPlayer *p = kexGame::cLocal->Player();

    if(inventoryMenu.IsActive())
    {
        inventoryMenu.Display();
        return;
    }
    
    renderView.SetupFromPlayer(p);
    
    renderScene.DrawView(renderView, p->Actor()->Sector());

    p->Weapon().Draw();
    
    DrawAutomap();
    
    hud.Display();

    DrawFadeIn();

    PrintStats();
}

//
// kexPlayLoop::Tick
//

void kexPlayLoop::Tick(void)
{
    if(kexGame::cLocal->ButtonEvent() & GBE_MENU_ACTIVATE)
    {
        kexGame::cLocal->SetMenu(MENU_PAUSE);
        return;
    }

    if(bPrintStats)
    {
        debugTickTime = kex::cTimer->GetPerformanceCounter();
    }

    if(!bFadeOut && kexGame::cLocal->Player()->Actor()->PlayerFlags() & PF_DEAD)
    {
        if(--restartDelayTicks <= 0)
        {
            RestartLevel();
        }
    }

    if(fadeInTicks > 0)
    {
        if(bFadeOut)
        {
            if(requestedGameState != GS_NONE)
            {
                fadeInTicks += 2;
            }
            else
            {
                fadeInTicks += 4;
            }

            if(fadeInTicks > 255)
            {
                fadeInTicks = 255;

                if(mapChange)
                {
                    kexGame::cLocal->ChangeMap(mapChange);
                }
                else
                {
                    switch(requestedGameState)
                    {
                    case GS_ENDING_GOOD:
                    case GS_ENDING_BAD:
                        kexGame::cLocal->SetGameState(requestedGameState);
                        break;

                    default:
                        kexGame::cLocal->SetGameState(GS_OVERWORLD);
                        break;
                    }
                }
            }
        }

        if(!bNoFadeOutPause)
        {
            ticks++;
            return;
        }
    }

    if(ticks > 4 && !bPaused && !inventoryMenu.IsActive())
    {
        renderScene.DLights().Clear();
        kexRenderScene::bufferUpdateList.Reset();

        kexGame::cWorld->UpdateAnimPics();
        kexGame::cLocal->UpdateGameObjects();
        kexGame::cLocal->Player()->Tick();

        hud.Update();

        UpdateWater();
        WaterBubbles();

        kexGame::cScriptManager->UpdateLevelScripts();
    }
    else if(inventoryMenu.IsActive())
    {
        inventoryMenu.Update();
    }
    
    ticks++;

    if(bPrintStats)
    {
        debugTickTime = kex::cTimer->GetPerformanceCounter() - debugTickTime;
    }
}

//
// kexPlayLoop::ProcessInput
//

bool kexPlayLoop::ProcessInput(inputEvent_t *ev)
{
    if(kexGame::cLocal->Player()->Actor()->PlayerFlags() & PF_DEAD)
    {
        return false;
    }

    if(inventoryMenu.IsActive())
    {
        return inventoryMenu.ProcessInput(ev);
    }

    return false;
}

//
// kexPlayLoop::FadeToBlack
//

void kexPlayLoop::FadeToBlack(void)
{
    int fade = 0xff;
    float w, h;
    float y;
    int time;
    kexFBO fadeScreen;

    fadeScreen.InitColorAttachment(0);
    fadeScreen.CopyBackBuffer();
    fadeScreen.BindImage();

    kexRender::cBackend->SetBlend(GLSRC_SRC_ALPHA, GLDST_ONE_MINUS_SRC_ALPHA);
    kexRender::cBackend->SetState(GLSTATE_CULL, true);
    kexRender::cBackend->SetState(GLSTATE_BLEND, false);
    kexRender::cBackend->SetState(GLSTATE_ALPHATEST, false);
    kexRender::cBackend->SetState(GLSTATE_DEPTHTEST, false);
    kexRender::cBackend->SetCull(GLCULL_BACK);

    w = (float)kexMath::RoundPowerOfTwo(kex::cSystem->VideoWidth());
    h = (float)kexMath::RoundPowerOfTwo(kex::cSystem->VideoHeight());
    y = (float)kex::cSystem->VideoHeight() - h;

    kexRender::cBackend->SetOrtho();
    kexRender::cVertList->BindDrawPointers();

    time = kex::cTimer->GetMS();

    while(fade > 0)
    {
        int elapsed = kex::cTimer->GetMS() - time;

        if(elapsed < 16)
        {
            kex::cTimer->Sleep(1);
            continue;
        }

        kexRender::cBackend->ClearBuffer();

        kexRender::cVertList->AddQuad(0, y, 0, w, h, 0, 1, 1, 0, fade, fade, fade, 255);

        kexRender::cVertList->DrawElements();
        kexRender::cBackend->SwapBuffers();

        fade -= 8;
        time = kex::cTimer->GetMS();
    }

    fadeScreen.UnBindImage();
}

//
// kexPlayLoop::RequestExit
//

void kexPlayLoop::RequestExit(const char *map)
{
    mapChange = map;
    fadeInTicks = 1;
    bFadeOut = true;
    bNoFadeOutPause = false;
    bRestartLevel = false;

    kexGame::cLocal->SavePersistentData();
    kexGame::cLocal->SaveGame();
}

//
// kexPlayLoop::RequestExit
//

void kexPlayLoop::RequestExit(const gameState_t gameState)
{
    mapChange = NULL;
    fadeInTicks = 1;
    bFadeOut = true;
    bNoFadeOutPause = false;
    bRestartLevel = false;
    requestedGameState = gameState;

    kexGame::cLocal->SaveGame();
}

//
// kexPlayLoop::RestartLevel
//

void kexPlayLoop::RestartLevel(void)
{
    mapChange = kexGame::cLocal->ActiveMap()->map;
    fadeInTicks = 1;
    bFadeOut = true;
    bNoFadeOutPause = true;
    bRestartLevel = true;
}

//
// kexPlayLoop::DrawFadeIn
//

void kexPlayLoop::DrawFadeIn(void)
{
    float w, h;
    byte fade;

    if(!bFadeOut && fadeInTicks <= 0)
    {
        return;
    }

    if(!bFadeOut)
    {
        fadeInTicks -= 8;

        if(fadeInTicks < 0)
        {
            fadeInTicks = 0;
        }
    }

    kexRender::cBackend->SetOrtho();
    kexRender::cVertList->BindDrawPointers();

    kexRender::cBackend->SetBlend(GLSRC_SRC_ALPHA, GLDST_ONE_MINUS_SRC_ALPHA);
    kexRender::cBackend->SetState(GLSTATE_CULL, true);
    kexRender::cBackend->SetState(GLSTATE_BLEND, true);
    kexRender::cBackend->SetState(GLSTATE_ALPHATEST, true);
    kexRender::cBackend->SetState(GLSTATE_DEPTHTEST, false);
    kexRender::cBackend->SetCull(GLCULL_BACK);

    w = (float)kexMath::RoundPowerOfTwo(kex::cSystem->VideoWidth());
    h = (float)kexMath::RoundPowerOfTwo(kex::cSystem->VideoHeight());

    fade = (byte)fadeInTicks;
    kexMath::Clamp(fade, 0, 0xff);

    kexRender::cTextures->whiteTexture->Bind();

    kexRender::cVertList->AddQuad(0, 0, w, h, 0, 0, 0, fade);
    kexRender::cVertList->DrawElements();
}

//
// kexPlayLoop::Print
//

void kexPlayLoop::Print(const char *string)
{
    hud.AddMessage(kexGame::cLocal->Translation()->TranslateString(string));
}

//
// kexPlayLoop::InitWater
//

void kexPlayLoop::InitWater(void)
{
    waterMaxMagnitude = 0;
    
    for(int i = 0; i < 16; i++)
    {
        for(int j = 0; j < 16; j++)
        {
            int r = (kexRand::Max(255) - 128) << 12;

            waterAccelPoints[i][j] = 0;
            waterVelocityPoints[i][j] = r;
            
            if(waterMaxMagnitude < kexMath::Abs(r))
            {
                waterMaxMagnitude = kexMath::Abs(r);
            }
        }
    }
}

//
// kexPlayLoop::UpdateWater
//

void kexPlayLoop::UpdateWater(void)
{
    for(int i = 0; i < 16; i++)
    {
        for(int j = 0; j < 16; j++)
        {
            int v1 = waterVelocityPoints[(i + 1) & 15][j];
            
            v1 += waterVelocityPoints[(i - 1) & 15][j];
            v1 += waterVelocityPoints[i][(j + 1) & 15];
            v1 += waterVelocityPoints[i][(j - 1) & 15];
            v1 -= (waterVelocityPoints[i][j] << 2);
            v1 +=  waterAccelPoints[i][j];
            
            waterAccelPoints[i][j] = v1;
        }
    }
    
    for(int i = 0; i < 16; i++)
    {
        for(int j = 0; j < 16; j++)
        {
            waterVelocityPoints[i][j] += (waterAccelPoints[i][j] >> 9);
        }
    }
}

//
// kexPlayLoop::WaterBubbles
//

void kexPlayLoop::WaterBubbles(void)
{
    mapSector_t *sector;
    mapFace_t *face;
    mapPoly_t *poly;
    mapVertex_t *verts;
    kexWorld *world;
    kexVec3 org;
    uint numVisSectors;
    int secnum;

    if((ticks & 63) != 0)
    {
        return;
    }

    world = kexGame::cWorld;
    numVisSectors = renderScene.VisibleSectors().CurrentLength();

    if(numVisSectors == 0)
    {
        return;
    }
    
    secnum = renderScene.VisibleSectors()[kexRand::Max(numVisSectors)];
    sector = &world->Sectors()[secnum];

    if(!(sector->flags & SF_WATER))
    {
        return;
    }

    face = sector->floorFace;
    poly = &world->Polys()[face->polyStart + (kexRand::Max(face->polyEnd - face->polyStart))];
    verts = world->Vertices();

    org = (verts[face->vertStart + poly->indices[0]].origin +
           verts[face->vertStart + poly->indices[1]].origin +
           verts[face->vertStart + poly->indices[2]].origin) / 3;

    kexGame::cActorFactory->Spawn(AT_WATERBUBBLE, org.x, org.y, org.z+16, 0, sector - world->Sectors());
}

//
// kexPlayLoop::GetWaterVelocityPoint
//

const int kexPlayLoop::GetWaterVelocityPoint(const float x, const float y)
{
    int *vel = (int*)waterVelocityPoints;
    int ix = (int)x;
    int iy = (int)y;
    int index = ((((ix + iy) >> 4) & 60) + (ix & 960)) >> 2;

    return vel[index & 0xff];
}

//
// kexPlayLoop::DrawAutomapWalls
//

void kexPlayLoop::DrawAutomapWalls(kexRenderView &view)
{
    kexWorld *w = kexGame::cWorld;
    float floorz = kexGame::cLocal->Player()->Actor()->FloorHeight();
    
    for(unsigned int i = 0; i < w->NumSectors(); ++i)
    {
        mapSector_t *sector = &w->Sectors()[i];
        kexBBox bounds = sector->bounds;
        
        bounds.min.z = bounds.max.z = 0;
        
        if(!view.TestBoundingBox(bounds))
        {
            continue;
        }
        
        for(int j = sector->faceStart; j <= sector->faceEnd; ++j)
        {
            mapFace_t *face = &w->Faces()[j];
            float x1, x2, y1, y2;
            float f1, f2;
            float fr, fg, fb;
            byte r1, g1, b1;
            byte r2, g2, b2;
            
            if((face->flags & FF_HIDDEN && !bMapAll) || (!(face->flags & FF_MAPPED) && !bMapAll) ||
               (face->flags & FF_PORTAL && !(face->flags & FF_SOLID)))
            {
                continue;
            }
            
            x1 = w->Vertices()[face->vertexStart+2].origin.x;
            y1 = w->Vertices()[face->vertexStart+2].origin.y;
            x2 = w->Vertices()[face->vertexStart+3].origin.x;
            y2 = w->Vertices()[face->vertexStart+3].origin.y;
            
            if(!(face->flags & FF_MAPPED) && bMapAll)
            {
                kexRender::cVertList->AddLine(x1, y1, 0, x2, y2, 0, 128, 128, 128, 255);
                continue;
            }

            if(face->tag >= 0)
            {
                switch(w->Events()[face->tag].type)
                {
                case 1:
                    kexRender::cVertList->AddLine(x1, y1, 0, x2, y2, 0, 255, 32, 255, 255);
                    continue;

                case 3:
                case 4:
                case 5:
                case 6:
                    kexRender::cVertList->AddLine(x1, y1, 0, x2, y2, 0, 255, 255, 32, 255);
                    continue;

                case 200:
                case 201:
                case 202:
                case 203:
                    kexRender::cVertList->AddLine(x1, y1, 0, x2, y2, 0, 0, 255, 255, 255);
                    continue;
                }
            }

            if(face->flags & FF_FORCEFIELD)
            {
                kexRender::cVertList->AddLine(x1, y1, 0, x2, y2, 0, 0, 255, 0, 255);
                continue;
            }
            
            r1 = 255; g1 = 255; b1 = 255;
            r2 = 255; g2 = 255; b2 = 255;
            
            f1 = (floorz - w->Vertices()[face->vertexStart+2].origin.z) / 2048.0f;
            f2 = (floorz - w->Vertices()[face->vertexStart+3].origin.z) / 2048.0f;
            
            if(f1 >= 0)
            {
                kexMath::Clamp(f1, 0, 1);
                
                fr =   0 - (float)r1 * f1 + (float)r1;
                fg =   0 - (float)g1 * f1 + (float)g1;
                fb = 255 - (float)b1 * f1 + (float)b1;
            }
            else
            {
                f1 = -f1;
                kexMath::Clamp(f1, 0, 1);
                
                fr = 255 - (float)r1 * f1 + (float)r1;
                fg =   0 - (float)g1 * f1 + (float)g1;
                fb =   0 - (float)b1 * f1 + (float)b1;
            }
            
            kexMath::Clamp(fr, 0, 255);
            kexMath::Clamp(fg, 0, 255);
            kexMath::Clamp(fb, 0, 255);
            
            r1 = (byte)fr;
            g1 = (byte)fg;
            b1 = (byte)fb;
            
            if(f2 >= 0)
            {
                kexMath::Clamp(f2, 0, 1);
                
                fr =   0 - (float)r1 * f2 + (float)r1;
                fg =   0 - (float)g1 * f2 + (float)g1;
                fb = 255 - (float)b1 * f2 + (float)b1;
            }
            else
            {
                f2 = -f2;
                kexMath::Clamp(f2, 0, 1);
                
                fr = 255 - (float)r1 * f2 + (float)r1;
                fg =   0 - (float)g1 * f2 + (float)g1;
                fb =   0 - (float)b1 * f2 + (float)b1;
            }
            
            kexMath::Clamp(fr, 0, 255);
            kexMath::Clamp(fg, 0, 255);
            kexMath::Clamp(fb, 0, 255);
            
            r2 = (byte)fr;
            g2 = (byte)fg;
            b2 = (byte)fb;
            
            kexRender::cVertList->AddLine(x1, y1, 0, x2, y2, 0,
                                          r1, g1, b1, 255,
                                          r2, g2, b2, 255);
        }
    }
    
    if(kexRender::cVertList->VertexCount() < 2)
    {
        return;
    }
    
    kexRender::cVertList->DrawLineElements();
}

//
// kexPlayLoop::DrawAutomapArrow
//

void kexPlayLoop::DrawAutomapArrow(kexRenderView &view, const float angle, const kexVec3 &pos,
                                   const float size, const byte r, const byte g, const byte b)
{
    kexMatrix mtx(-angle, 2);
    
    kexRender::cVertList->AddLine(0, 0, 0, 0, 1, 0, r, g, b, 255);
    kexRender::cVertList->AddLine(0, 1, 0, -0.5f, 0.5f, 0, r, g, b, 255);
    kexRender::cVertList->AddLine(0, 1, 0, 0.5f, 0.5f, 0, r, g, b, 255);
    
    mtx.Scale(size, size, size);
    mtx.AddTranslation(pos.x, pos.y, 0);
    mtx = mtx * view.ModelView();
    
    kexRender::cBackend->LoadModelViewMatrix(mtx);
    kexRender::cVertList->DrawLineElements();
}

//
// kexPlayLoop::DrawAutomapActors
//

void kexPlayLoop::DrawAutomapActors(kexRenderView &view)
{
    byte r, g, b;
    
    if(!bMapAll)
    {
        return;
    }
    
    for(kexGameObject *go = kexGame::cLocal->GameObjects().Next(); go != NULL; go = go->Link().Next())
    {
        float radius;
        
        if(!go->InstanceOf(&kexActor::info) || go->InstanceOf(&kexPuppet::info))
        {
            continue;
        }
        
        radius = static_cast<kexActor*>(go)->Radius() * 2;
        
        if(!view.TestSphere(kexVec3(go->Origin().x, go->Origin().y, 0), radius))
        {
            continue;
        }
        
        if(go->InstanceOf(&kexAI::info))
        {
            r = 255; g = 0; b = 0;
        }
        else if(go->InstanceOf(&kexProjectile::info))
        {
            r = 255; g = 255; b = 0;
        }
        else if(go->InstanceOf(&kexPickup::info))
        {
            r = 64; g = 192; b = 255;
        }
        else
        {
            r = 64; g = 255; b = 0;
        }
        
        DrawAutomapArrow(view, go->Yaw(), go->Origin(), radius, r, g, b);
    }
}

//
// kexPlayLoop::ZoomAutomap
//

void kexPlayLoop::ZoomAutomap(const float amount)
{
    if(bShowAutomap == false)
    {
        return;
    }

    automapZoom += amount;
    kexMath::Clamp(automapZoom, 1024, 8192);
}

//
// kexPlayLoop::DrawAutomap
//

void kexPlayLoop::DrawAutomap(void)
{
    int h, clipY;

    if(bShowAutomap == false)
    {
        return;
    }
    
    kexRenderView automapView;
    
    automapView.Fov() = 45;
    
    automapView.Origin() = renderView.Origin();
    automapView.Origin().z = automapZoom;
    
    automapView.Yaw() = renderView.Yaw();
    automapView.Pitch() = kexMath::Deg2Rad(90);
    automapView.Roll() = 0;
    
    automapView.Setup();
    
    kexRender::cBackend->LoadProjectionMatrix(automapView.ProjectionView());
    kexRender::cBackend->LoadModelViewMatrix(automapView.ModelView());
    
    kexRender::cBackend->SetState(GLSTATE_ALPHATEST, false);
    kexRender::cBackend->SetState(GLSTATE_BLEND, false);
    kexRender::cBackend->SetState(GLSTATE_SCISSOR, true);

    h = kex::cSystem->VideoHeight();
    clipY = h - (int)((float)h / (240.0f / 24.0f));

    kexRender::cBackend->SetScissorRect(0, 0, kex::cSystem->VideoWidth(), clipY);
    
    kexRender::cTextures->whiteTexture->Bind();
    kexRender::cVertList->BindDrawPointers();

    dglLineWidth(2);
    
    DrawAutomapWalls(automapView);
    DrawAutomapActors(automapView);
    DrawAutomapArrow(automapView, automapView.Yaw(), automapView.Origin(),
                     kexGame::cLocal->Player()->Actor()->Radius(), 255, 255, 255);

    dglLineWidth(1);

    kexRender::cBackend->SetState(GLSTATE_SCISSOR, false);
}

//
// kexPlayLoop::PrintStats
//

void kexPlayLoop::PrintStats(void)
{
    if(!bPrintStats)
    {
        return;
    }
    
    kexRender::cUtils->PrintStatsText("Playloop Time", "%fms", kex::cTimer->MeasurePerformance(debugTickTime));
    kexRender::cUtils->AddDebugLineSpacing();
}
