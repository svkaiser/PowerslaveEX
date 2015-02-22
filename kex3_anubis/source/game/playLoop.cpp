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
    InitWater();
}

//
// kexPlayLoop::Stop
//

void kexPlayLoop::Stop(void)
{
    kexGame::cLocal->World()->UnloadMap();
}

//
// kexPlayLoop::Draw
//

void kexPlayLoop::Draw(void)
{
    kexPlayer *p = kexGame::cLocal->Player();
    kexWorld *world = kexGame::cLocal->World();
    
    renderView.SetupFromPlayer(p);
    
    world->FindVisibleSectors(renderView, p->Actor()->Sector());
    
    renderScene.Draw();

    p->Weapon().Draw();
    
    kexRender::cBackend->SetState(GLSTATE_SCISSOR, false);
    hud.Display();
}

//
// kexPlayLoop::Tick
//

void kexPlayLoop::Tick(void)
{
    if(ticks > 4)
    {
        kexGame::cLocal->UpdateGameObjects();
        kexGame::cLocal->Player()->Tick();
        hud.Update();
        UpdateWater();
    }
    
    ticks++;
}

//
// kexPlayLoop::ProcessInput
//

bool kexPlayLoop::ProcessInput(inputEvent_t *ev)
{
    return false;
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
