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
//      Play loop (in-game) logic
//

#include "kexlib.h"
#include "renderMain.h"
#include "renderView.h"
#include "game.h"
#include "localization.h"

//
// gprint
//

COMMAND(gprint)
{
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
    kexPlayLoop *pl = kexGame::cLocal->PlayLoop();
    pl->ToggleAutomap(pl->AutomapEnabled() ^ 1);
}

//
// mapall
//

COMMAND(mapall)
{
    kexGame::cLocal->PlayLoop()->ToggleMapAll(true);
}

//
// pausegame
//

COMMAND(pausegame)
{
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
    if(kex::cCommands->GetArgc() < 1)
    {
        return;
    }

    kexGame::cLocal->PlayLoop()->InventoryMenu().Toggle();
}

//
// kexPlayLoop::kexPlayLoop
//

kexPlayLoop::kexPlayLoop(void)
{
}

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
}

//
// kexPlayLoop::Start
//

void kexPlayLoop::Start(void)
{
    ticks = 0;
    
    if(kexGame::cLocal->Player()->Actor() == NULL)
    {
        kex::cSystem->Warning("No player starts present\n");
        kexGame::cLocal->SetGameState(GS_TITLE);
        return;
    }
    
    hud.SetPlayer(kexGame::cLocal->Player());
    renderScene.SetView(&renderView);
    renderScene.SetWorld(kexGame::cLocal->World());

    kexGame::cLocal->Player()->Ready();
    hud.Reset();
    inventoryMenu.Reset();
    InitWater();

    automapZoom = 2048;
    
    bShowAutomap = false;
    bMapAll = false;
    bPaused = false;
}

//
// kexPlayLoop::Stop
//

void kexPlayLoop::Stop(void)
{
    FadeToBlack();
    kexGame::cLocal->World()->UnloadMap();
}

//
// kexPlayLoop::Draw
//

void kexPlayLoop::Draw(void)
{
    kexPlayer *p = kexGame::cLocal->Player();
    kexWorld *world = kexGame::cLocal->World();

    if(inventoryMenu.IsActive())
    {
        inventoryMenu.Display();
        return;
    }
    
    renderView.SetupFromPlayer(p);
    
    world->FindVisibleSectors(renderView, p->Actor()->Sector());
    
    renderScene.Draw();

    p->Weapon().Draw();
    
    DrawAutomap();
    
    hud.Display();
}

//
// kexPlayLoop::Tick
//

void kexPlayLoop::Tick(void)
{
    if(ticks > 4 && !bPaused && !inventoryMenu.IsActive())
    {
        kexGame::cLocal->UpdateGameObjects();
        kexGame::cLocal->Player()->Tick();
        hud.Update();
        UpdateWater();
    }
    else if(inventoryMenu.IsActive())
    {
        inventoryMenu.Update();
    }
    
    ticks++;
}

//
// kexPlayLoop::ProcessInput
//

bool kexPlayLoop::ProcessInput(inputEvent_t *ev)
{
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

    while(fade > 0)
    {
        kexRender::cBackend->ClearBuffer();

        kexRender::cVertList->AddQuad(0, y, 0, w, h, 0, 1, 1, 0, fade, fade, fade, 255);

        kexRender::cVertList->DrawElements();
        kexRender::cBackend->SwapBuffers();

        fade -= 8;
    }

    fadeScreen.UnBindImage();
}

//
// kexPlayLoop::Print
//

void kexPlayLoop::Print(const char *string)
{
    if(kexStr::IndexOf(string, "$str_") == 0)
    {
        int index = atoi(string + 5);

        hud.AddMessage(kexGame::cLocal->Translation()->GetString(index));
        return;
    }

    hud.AddMessage(string);
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
    kexWorld *w = kexGame::cLocal->World();
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
            
            if(face->flags & FF_HIDDEN || (!(face->flags & FF_MAPPED) && !bMapAll) ||
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
    kexRender::cBackend->SetState(GLSTATE_SCISSOR, false);
    
    kexRender::cTextures->whiteTexture->Bind();
    kexRender::cVertList->BindDrawPointers();

    dglLineWidth(2);
    
    DrawAutomapWalls(automapView);
    DrawAutomapActors(automapView);
    DrawAutomapArrow(automapView, automapView.Yaw(), automapView.Origin(),
                     kexGame::cLocal->Player()->Actor()->Radius(), 255, 255, 255);

    dglLineWidth(1);
}
