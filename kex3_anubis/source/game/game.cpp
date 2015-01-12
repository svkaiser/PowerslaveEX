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
#include "actor.h"
#include "localization.h"
#include "world.h"
#include "cmodel.h"
#include "player.h"

static kexGame gameLocal;
kexGame *kex::cGame = &gameLocal;

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
// kexGame::kexGame
//

kexGame::kexGame(void)
{
    this->smallFont         = NULL;
    this->bigFont           = NULL;
    this->ticks             = 0;
    this->gameState         = GS_NONE;
    this->pendingGameState  = GS_NONE;
    this->gameLoop          = &this->gameLoopStub;

    this->titleScreen   = new kexTitleScreen;
    this->playLoop      = new kexPlayLoop;
    this->translation   = new kexTranslation;
    this->world         = new kexWorld;
    this->player        = new kexPlayer;
    this->renderView    = new kexRenderView;
    this->cmodel        = new kexCModel;
}

//
// kexGame::~kexGame
//

kexGame::~kexGame(void)
{
    delete titleScreen;
    delete playLoop;
    delete translation;
    delete world;
    delete player;
    delete renderView;
    delete cmodel;
}

//
// kexGame::Init
//

void kexGame::Init(void)
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
}

//
// kexGame::Start
//

void kexGame::Start(void)
{
    smallFont   = kexFont::Alloc("smallfont");
    bigFont     = kexFont::Alloc("bigfont");

    titleScreen->Init();
    translation->Init();

    player->Reset();
    pendingGameState = GS_TITLE;
}

//
// kexGame::Shutdown
//

void kexGame::Shutdown(void)
{
    world->UnloadMap();
}

//
// kexGame::Tick
//

void kexGame::Tick(void)
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
// kexGame::Draw
//

void kexGame::Draw(void)
{
    if(pendingGameState != GS_NONE)
    {
        return;
    }
    
    gameLoop->Draw();
}

//
// kexGame::ProcessInput
//

bool kexGame::ProcessInput(inputEvent_t *ev)
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
// kexGame::ConstructObject
//

kexObject *kexGame::ConstructObject(const char *className)
{
    kexObject *obj;
    kexRTTI *objType;
    
    if(!(objType = kexObject::Get(className)))
    {
        kex::cSystem->Error("kexGame::ConstructObject: unknown class (\"%s\")\n", className);
        return NULL;
    }
        
    if(!(obj = objType->Create()))
    {
        kex::cSystem->Error("kexGame::ConstructObject: could not spawn (\"%s\")\n", className);
        return NULL;
    }
    
    return obj;
}

//
// kexGame::ChangeMap
//

void kexGame::ChangeMap(const char *name)
{
    SetGameState(GS_CHANGELEVEL);
    pendingMap = name;
}

//
// kexGame::LoadMap
//

void kexGame::LoadNewMap(void)
{
    if(!world->LoadMap(pendingMap.c_str()))
    {
        kex::cGame->SetGameState(GS_TITLE);
        return;
    }
    
    kex::cGame->SetGameState(GS_LEVEL);
}

//
// kexGame::DrawSmallString
//

void kexGame::DrawSmallString(const char *string, float x, float y, float scale, bool center,
                              byte r, byte g, byte b)
{
    kexRender::cBackend->SetBlend(GLSRC_SRC_ALPHA, GLDST_ONE_MINUS_SRC_ALPHA);
    smallFont->DrawString(string, x+1, y+1, scale, center, RGBA(0, 0, 0, 0xff));
    smallFont->DrawString(string, x, y, scale, center, RGBA(r, g, b, 0xff));
}

//
// kexGame::DrawBigString
//

void kexGame::DrawBigString(const char *string, float x, float y, float scale, bool center,
                            byte r, byte g, byte b)
{
    kexRender::cBackend->SetBlend(GLSRC_SRC_ALPHA, GLDST_ONE_MINUS_SRC_ALPHA);
    bigFont->DrawString(string, x+1, y+1, scale, center, RGBA(0, 0, 0, 0xff));
    bigFont->DrawString(string, x, y, scale, center, RGBA(r, g, b, 0xff));
}

//
// kexGame::UpdateActors
//

void kexGame::UpdateActors(void)
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
// kexGame::RemoveActor
//

void kexGame::RemoveActor(kexActor *actor)
{
    actor->SetTarget(NULL);
    actor->Link().Remove();
    actor->UnlinkArea();
    delete actor;
}

//
// kexGame::RemoveAllActors
//

void kexGame::RemoveAllActors(void)
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
// kexGame::SpawnActor
//

kexActor *kexGame::SpawnActor(const int type, const float x, const float y, const float z, const float yaw)
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
