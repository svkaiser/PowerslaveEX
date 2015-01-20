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
//      Game logic
//

#include "kexlib.h"
#include "renderMain.h"
#include "renderView.h"
#include "game.h"
#include "titlescreen.h"
#include "playLoop.h"
#include "localization.h"

static kexGameLocal gameLocal;
kexGameLoop *kex::cGame = &gameLocal;
kexGameLocal *kexGame::cLocal = &gameLocal;

//
// map
//

COMMAND(map)
{
    int argc = kex::cCommands->GetArgc();

    if(argc != 2)
    {
        kex::cSystem->Printf("map <filename>\n");
        return;
    }

    gameLocal.ChangeMap(kex::cCommands->GetArgv(1));
}

//
// noclip
//

COMMAND(noclip)
{
    kexGameLocal *game = kexGame::cLocal;
    kexPuppet *puppet;

    if(kex::cCommands->GetArgc() < 1)
    {
        return;
    }

    if(game->GameState() != GS_LEVEL || game->Player()->Actor() == NULL)
    {
        return;
    }

    puppet = static_cast<kexPuppet*>(game->Player()->Actor());

    if(puppet->PlayerFlags() & PF_NOCLIP)
    {
        kex::cSystem->Printf("no clipping off\n");
        puppet->PlayerFlags() &= ~PF_NOCLIP;
        puppet->FindSector(puppet->Origin());
    }
    else
    {
        kex::cSystem->Printf("no clipping on\n");
        puppet->PlayerFlags() |= PF_NOCLIP;
        puppet->Velocity().Clear();
    }
}

//
// fly
//

COMMAND(fly)
{
    kexGameLocal *game = kexGame::cLocal;
    kexPuppet *puppet;

    if(kex::cCommands->GetArgc() < 1)
    {
        return;
    }

    if(game->GameState() != GS_LEVEL || game->Player()->Actor() == NULL)
    {
        return;
    }

    puppet = static_cast<kexPuppet*>(game->Player()->Actor());

    if(puppet->PlayerFlags() & PF_FLY)
    {
        kex::cSystem->Printf("fly mode off\n");
        puppet->PlayerFlags() &= ~PF_FLY;
    }
    else
    {
        kex::cSystem->Printf("fly mode on\n");
        puppet->PlayerFlags() |= PF_FLY;
        puppet->Velocity().Clear();
    }
}

//
// kexGameLocal::kexGameLocal
//

kexGameLocal::kexGameLocal(void)
{
    this->smallFont         = NULL;
    this->bigFont           = NULL;
    this->ticks             = 0;
    this->gameState         = GS_NONE;
    this->pendingGameState  = GS_NONE;
    this->gameLoop          = &this->gameLoopStub;

    this->titleScreen       = new kexTitleScreen;
    this->playLoop          = new kexPlayLoop;
    this->translation       = new kexTranslation;
    this->world             = new kexWorld;
    this->player            = new kexPlayer;
    this->renderView        = new kexRenderView;
    this->cmodel            = new kexCModel;
    this->spriteManager     = new kexSpriteManager;
    this->spriteAnimManager = new kexSpriteAnimManager;

    memset(weaponInfo, 0, sizeof(weaponInfo_t) * NUMPLAYERWEAPONS);
}

//
// kexGameLocal::~kexGameLocal
//

kexGameLocal::~kexGameLocal(void)
{
    delete titleScreen;
    delete playLoop;
    delete translation;
    delete world;
    delete player;
    delete renderView;
    delete cmodel;
    delete spriteManager;
    delete spriteAnimManager;
}

//
// kexGameLocal::Init
//

void kexGameLocal::Init(void)
{
    kex::cActions->AddAction(IA_ATTACK, "attack");
    kex::cActions->AddAction(IA_JUMP, "jump");
    kex::cActions->AddAction(IA_FORWARD, "forward");
    kex::cActions->AddAction(IA_BACKWARD, "backward");
    kex::cActions->AddAction(IA_LEFT, "left");
    kex::cActions->AddAction(IA_RIGHT, "right");
    kex::cActions->AddAction(IA_STRAFELEFT, "strafeleft");
    kex::cActions->AddAction(IA_STRAFERIGHT, "straferight");
    kex::cActions->AddAction(IA_WEAPNEXT, "+weapnext");
    kex::cActions->AddAction(IA_WEAPPREV, "+weapprev");
    
    kex::cSystem->ReadConfigFile("config.cfg");
    kex::cPakFiles->LoadZipFile("game.kpf");
}

//
// kexGameLocal::Start
//

void kexGameLocal::Start(void)
{
    weaponInfo_t *weapon;

    smallFont   = kexFont::Alloc("smallfont");
    bigFont     = kexFont::Alloc("bigfont");

    titleScreen->Init();
    translation->Init();
    spriteManager->Init();
    spriteAnimManager->Init();

    pendingGameState = GS_TITLE;

    //
    // setup weapon info
    //

    // machete
    weapon = &weaponInfo[PW_MACHETE];
    weapon->type = PW_MACHETE;
    weapon->offsetX = 160;
    weapon->offsetY = 230;
    weapon->idle = spriteAnimManager->Get("weapons/machete_idle");
    weapon->raise = spriteAnimManager->Get("weapons/machete_raise");
    weapon->lower = spriteAnimManager->Get("weapons/machete_lower");
    weapon->fire = spriteAnimManager->Get("weapons/machete_fire");

    // pistol
    weapon = &weaponInfo[PW_PISTOL];
    weapon->type = PW_PISTOL;
    weapon->offsetX = 160;
    weapon->offsetY = 232;
    weapon->idle = spriteAnimManager->Get("weapons/pistol_idle");
    weapon->raise = spriteAnimManager->Get("weapons/pistol_raise");
    weapon->lower = spriteAnimManager->Get("weapons/pistol_lower");
    weapon->fire = spriteAnimManager->Get("weapons/pistol_fire");

    // M60
    weapon = &weaponInfo[PW_M60];
    weapon->type = PW_M60;
    weapon->offsetX = 160;
    weapon->offsetY = 132;
    weapon->idle = spriteAnimManager->Get("weapons/m60_idle");
    weapon->raise = spriteAnimManager->Get("weapons/m60_raise_100");
    weapon->lower = spriteAnimManager->Get("weapons/m60_lower");
    weapon->fire = spriteAnimManager->Get("weapons/m60_fire");

    // bomb
    weapon = &weaponInfo[PW_BOMBS];
    weapon->type = PW_BOMBS;
    weapon->offsetX = 160;
    weapon->offsetY = 196;
    weapon->idle = spriteAnimManager->Get("weapons/bomb_idle");
    weapon->raise = spriteAnimManager->Get("weapons/bomb_raise");
    weapon->lower = spriteAnimManager->Get("weapons/bomb_lower");
    weapon->fire = spriteAnimManager->Get("weapons/bomb_fire");

    // flamethrower
    weapon = &weaponInfo[PW_FLAMETHROWER];
    weapon->type = PW_FLAMETHROWER;
    weapon->offsetX = 160;
    weapon->offsetY = 132;
    weapon->idle = spriteAnimManager->Get("weapons/flamethrower_idle");
    weapon->raise = spriteAnimManager->Get("weapons/flamethrower_raise");
    weapon->lower = spriteAnimManager->Get("weapons/flamethrower_lower");
    weapon->fire = spriteAnimManager->Get("weapons/flamethrower_fire");

    // cobra staff
    weapon = &weaponInfo[PW_COBRASTAFF];
    weapon->type = PW_COBRASTAFF;
    weapon->offsetX = 160;
    weapon->offsetY = 210;
    weapon->idle = spriteAnimManager->Get("weapons/cstaff_idle");
    weapon->raise = spriteAnimManager->Get("weapons/cstaff_raise");
    weapon->lower = spriteAnimManager->Get("weapons/cstaff_lower");
    weapon->fire = spriteAnimManager->Get("weapons/cstaff_fire");

    // ring of ra
    weapon = &weaponInfo[PW_RINGOFRA];
    weapon->type = PW_RINGOFRA;
    weapon->offsetX = 160;
    weapon->offsetY = 214;
    weapon->idle = spriteAnimManager->Get("weapons/ring_ra_idle");
    weapon->raise = spriteAnimManager->Get("weapons/ring_ra_raise");
    weapon->lower = spriteAnimManager->Get("weapons/ring_ra_lower");
    weapon->fire = spriteAnimManager->Get("weapons/ring_ra_fire");

    // bracelet
    weapon = &weaponInfo[PW_BRACELET];
    weapon->type = PW_BRACELET;
    weapon->offsetX = 160;
    weapon->offsetY = 208;
    weapon->idle = spriteAnimManager->Get("weapons/manacle_idle");
    weapon->raise = spriteAnimManager->Get("weapons/manacle_raise");
    weapon->lower = spriteAnimManager->Get("weapons/manacle_lower");
    weapon->fire = spriteAnimManager->Get("weapons/manacle_fire");

    player->Reset();
}

//
// kexGameLocal::Stop
//

void kexGameLocal::Stop(void)
{
    kex::cSystem->WriteConfigFile();
    
    world->UnloadMap();
    spriteAnimManager->Shutdown();
    spriteManager->Shutdown();
}

//
// kexGameLocal::Tick
//

void kexGameLocal::Tick(void)
{
    if(pendingGameState != GS_NONE)
    {
        gameLoop->Stop();
        
        switch(pendingGameState)
        {
            case GS_TITLE:
                gameLoop = titleScreen;
                break;

            case GS_LEVEL:
                gameLoop = playLoop;
                break;
                
            case GS_CHANGELEVEL:
                gameLoop = &gameLoopStub;
                pendingGameState = GS_NONE;
                LoadNewMap();
                return;
                
            default:
                gameLoop = &gameLoopStub;
                break;
        }
        
        gameState = pendingGameState;
        pendingGameState = GS_NONE;
        
        gameLoop->Start();
        
        if(pendingGameState != GS_NONE)
        {
            return;
        }
    }
    
    player->Cmd().BuildCommands();
    
    gameLoop->Tick();
    
    player->Cmd().Reset();
    
    ticks++;
}

//
// kexGameLocal::Draw
//

void kexGameLocal::Draw(void)
{
    if(pendingGameState != GS_NONE)
    {
        return;
    }
    
    gameLoop->Draw();
}

//
// kexGameLocal::ProcessInput
//

bool kexGameLocal::ProcessInput(inputEvent_t *ev)
{
    switch(ev->type)
    {
    case ev_mouse:
        player->Cmd().SetTurnXY(ev->data1, ev->data2);
        break;

    case ev_keydown:
    case ev_mousedown:
        kex::cActions->ExecuteCommand(ev->data1, false, ev->type);
        break;

    case ev_keyup:
    case ev_mouseup:
        kex::cActions->ExecuteCommand(ev->data1, true, ev->type);
        break;
    }

    return gameLoop->ProcessInput(ev);
}

//
// kexGameLocal::ConstructObject
//

kexObject *kexGameLocal::ConstructObject(const char *className)
{
    kexObject *obj;
    kexRTTI *objType;
    
    if(!(objType = kexObject::Get(className)))
    {
        kex::cSystem->Error("kexGameLocal::ConstructObject: unknown class (\"%s\")\n", className);
        return NULL;
    }
        
    if(!(obj = objType->Create()))
    {
        kex::cSystem->Error("kexGameLocal::ConstructObject: could not spawn (\"%s\")\n", className);
        return NULL;
    }
    
    return obj;
}

//
// kexGameLocal::ChangeMap
//

void kexGameLocal::ChangeMap(const char *name)
{
    SetGameState(GS_CHANGELEVEL);
    pendingMap = name;
}

//
// kexGameLocal::LoadMap
//

void kexGameLocal::LoadNewMap(void)
{
    if(!world->LoadMap(pendingMap.c_str()))
    {
        SetGameState(GS_TITLE);
        return;
    }
    
    SetGameState(GS_LEVEL);
}

//
// kexGameLocal::DrawSmallString
//

void kexGameLocal::DrawSmallString(const char *string, float x, float y, float scale, bool center,
                                   byte r, byte g, byte b)
{
    kexRender::cBackend->SetBlend(GLSRC_SRC_ALPHA, GLDST_ONE_MINUS_SRC_ALPHA);
    smallFont->DrawString(string, x+1, y+1, scale, center, RGBA(0, 0, 0, 0xff));
    smallFont->DrawString(string, x, y, scale, center, RGBA(r, g, b, 0xff));
}

//
// kexGameLocal::DrawBigString
//

void kexGameLocal::DrawBigString(const char *string, float x, float y, float scale, bool center,
                                 byte r, byte g, byte b)
{
    kexRender::cBackend->SetBlend(GLSRC_SRC_ALPHA, GLDST_ONE_MINUS_SRC_ALPHA);
    bigFont->DrawString(string, x+1, y+1, scale, center, RGBA(0, 0, 0, 0xff));
    bigFont->DrawString(string, x, y, scale, center, RGBA(r, g, b, 0xff));
}

//
// kexGameLocal::UpdateActors
//

void kexGameLocal::UpdateActors(void)
{
    kexActor *next = NULL;
    
    for(actorRover = actors.Next(); actorRover != NULL; actorRover = next)
    {
        next = actorRover->Link().Next();
        actorRover->Tick();
        
        if(actorRover->Removing())
        {
            RemoveActor(actorRover);
        }
    }
}

//
// kexGameLocal::RemoveActor
//

void kexGameLocal::RemoveActor(kexActor *actor)
{
    actor->SetTarget(NULL);
    actor->Link().Remove();
    actor->UnlinkArea();
    delete actor;
}

//
// kexGameLocal::RemoveAllActors
//

void kexGameLocal::RemoveAllActors(void)
{
    kexActor *actor;
    kexActor *next;
    
    // remove all actors
    for(actor = actors.Next(); actor != NULL; actor = next)
    {
        next = actor->Link().Next();
        RemoveActor(actor);
    }
    
    actors.Clear();
}

//
// kexGameLocal::SpawnActor
//

kexActor *kexGameLocal::SpawnActor(const int type, const float x, const float y, const float z, const float yaw)
{
    kexActor *actor;
    
    switch(type)
    {
    case AT_PLAYER:
        actor = static_cast<kexActor*>(ConstructObject("kexPuppet"));
        break;
            
    default:
        actor = static_cast<kexActor*>(ConstructObject("kexActor"));
        break;
    }
    
    actor->Origin().Set(x, y, z);
    actor->Yaw() = yaw;
    
    actor->Link().Add(actors);
    actor->LinkArea();
    actor->CallSpawn();
    
    return actor;
}
