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
#include "mover.h"
#include "titlescreen.h"
#include "playLoop.h"
#include "localization.h"
#include "mapEditor.h"

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

    puppet = game->Player()->Actor();

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

    puppet = game->Player()->Actor();

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
// summon
//

COMMAND(summon)
{
    int argc = kex::cCommands->GetArgc();
    kexVec3 forward;
    kexActor *a;
    float x, y, z;
    
    if(gameLocal.GameState() != GS_LEVEL || gameLocal.Player()->Actor() == NULL)
    {
        return;
    }

    if(argc != 2)
    {
        kex::cSystem->Printf("summon <name>\n");
        return;
    }

    a = gameLocal.Player()->Actor();
    kexVec3::ToAxis(&forward, 0, 0, a->Yaw(), 0, 0);
    x = a->Origin().x + (forward.x * a->Radius());
    y = a->Origin().y + (forward.y * a->Radius());
    z = a->Origin().z + (forward.z * a->Radius());

    gameLocal.SpawnActor(kex::cCommands->GetArgv(1), x, y, z, a->Yaw());
}

//
// give
//

COMMAND(give)
{
    int argc = kex::cCommands->GetArgc();
    
    if(gameLocal.GameState() != GS_LEVEL || gameLocal.Player()->Actor() == NULL)
    {
        return;
    }
    
    if(argc < 2)
    {
        kex::cSystem->Printf("give <weapon#, weapons>\n");
        return;
    }
    
    if(!kexStr::Compare(kex::cCommands->GetArgv(1), "weapons"))
    {
        for(int i = 0; i < NUMPLAYERWEAPONS; ++i)
        {
            gameLocal.Player()->GiveWeapon(i, false);
        }
        
        kex::cSystem->Printf("Got all weapons!\n");
    }
    else if(!kexStr::Compare(kex::cCommands->GetArgv(1), "weapon"))
    {
        if(argc != 3)
        {
            kex::cSystem->Printf("give weapon <0 - 8>\n");
            return;
        }
        else
        {
            int weap = atoi(kex::cCommands->GetArgv(2));
            
            if(weap <= -1 || weap >= NUMPLAYERWEAPONS)
            {
                kex::cSystem->Printf("give weapon <0 - 8>\n");
                return;
            }
            
            gameLocal.Player()->GiveWeapon(weap);
            kex::cSystem->Printf("Got weapon ## %i!\n", weap);
        }
    }
    else
    {
        kex::cSystem->Printf("give <weapon#, weapons>\n");
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
    kex::cActions->AddAction(IA_USE, "+use");
    
    kex::cSystem->ReadConfigFile("config.cfg");
    kex::cPakFiles->LoadZipFile("game.kpf");

    actorDefs.LoadFilesInDirectory("defs/actors/");
    weaponDef.LoadFile("defs/weaponInfo.txt");
    
    kexGame::cScriptManager->Init();
    kexGame::cActionDefManager->RegisterActions();
}

//
// kexGameLocal::Start
//

void kexGameLocal::Start(void)
{
    smallFont   = kexFont::Alloc("smallfont");
    bigFont     = kexFont::Alloc("bigfont");

    titleScreen->Init();
    playLoop->Init();
    translation->Init();
    spriteManager->Init();
    spriteAnimManager->Init();

    pendingGameState = GS_TITLE;
    InitWeaponDefs();

    player->Reset();
}

//
// kexGameLocal::InitWeaponDefs
//

void kexGameLocal::InitWeaponDefs(void)
{
    weaponInfo_t *weapon;
    kexStr animName;
    
    for(int i = 0; i < NUMPLAYERWEAPONS; ++i)
    {
        kexDict *dict = weaponDef.GetEntry(i);
        weapon = &weaponInfo[i];
        
        memset(weapon, 0, sizeof(weaponInfo_t));
        
        if(!dict)
        {
            continue;
        }
        
        dict->GetBool("persistant", weapon->bPersistent, false);
        dict->GetInt("maxAmmo", weapon->maxAmmo, 1);
        dict->GetFloat("offsetX", weapon->offsetX, 0);
        dict->GetFloat("offsetY", weapon->offsetY, 0);
        
        if(dict->GetString("idleAnim", animName))       weapon->idle         = spriteAnimManager->Get(animName);
        if(dict->GetString("raiseAnim", animName))      weapon->raise        = spriteAnimManager->Get(animName);
        if(dict->GetString("lowerAnim", animName))      weapon->lower        = spriteAnimManager->Get(animName);
        if(dict->GetString("fireAnim", animName))       weapon->fire         = spriteAnimManager->Get(animName);
        if(dict->GetString("flameAnim", animName))      weapon->flame        = spriteAnimManager->Get(animName);
        if(dict->GetString("ammoIdle_Full", animName))  weapon->ammoIdle[0]  = spriteAnimManager->Get(animName);
        if(dict->GetString("ammoLower_Full", animName)) weapon->ammoLower[0] = spriteAnimManager->Get(animName);
        if(dict->GetString("ammoRaise_Full", animName)) weapon->ammoRaise[0] = spriteAnimManager->Get(animName);
        if(dict->GetString("ammoFire_Full", animName))  weapon->ammoFire[0]  = spriteAnimManager->Get(animName);
        if(dict->GetString("ammoIdle_Half", animName))  weapon->ammoIdle[1]  = spriteAnimManager->Get(animName);
        if(dict->GetString("ammoLower_Half", animName)) weapon->ammoLower[1] = spriteAnimManager->Get(animName);
        if(dict->GetString("ammoRaise_Half", animName)) weapon->ammoRaise[1] = spriteAnimManager->Get(animName);
        if(dict->GetString("ammoFire_Half", animName))  weapon->ammoFire[1]  = spriteAnimManager->Get(animName);
        if(dict->GetString("ammoIdle_Zero", animName))  weapon->ammoIdle[2]  = spriteAnimManager->Get(animName);
        if(dict->GetString("ammoLower_Zero", animName)) weapon->ammoLower[2] = spriteAnimManager->Get(animName);
        if(dict->GetString("ammoRaise_Zero", animName)) weapon->ammoRaise[2] = spriteAnimManager->Get(animName);
        if(dict->GetString("ammoFire_Zero", animName))  weapon->ammoFire[2]  = spriteAnimManager->Get(animName);
    }
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
    kexRand::SetSeed(kex::cTimer->GetTicks());

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
                
            case GS_MAPEDITOR:
                extern kexMapEditor mapEditorLocal;
                gameLoop = &mapEditorLocal;
                break;
                
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
    kexGame::cScriptManager->DrawGCStats();
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
// kexGameLocal::UpdateGameObjects
//

void kexGameLocal::UpdateGameObjects(void)
{
    kexGameObject *next = NULL;
    
    for(goRover = gameObjects.Next(); goRover != NULL; goRover = next)
    {
        next = goRover->Link().Next();
        goRover->Tick();
        
        if(goRover->Removing())
        {
            RemoveGameObject(goRover);
        }
    }
}

//
// kexGameLocal::RemoveActor
//

void kexGameLocal::RemoveGameObject(kexGameObject *go)
{
    go->OnRemove();
    delete go;
}

//
// kexGameLocal::RemoveAllGameObjects
//

void kexGameLocal::RemoveAllGameObjects(void)
{
    kexGameObject *go;
    kexGameObject *next;
    
    // remove all game objects
    for(go = gameObjects.Next(); go != NULL; go = next)
    {
        next = go->Link().Next();
        RemoveGameObject(go);
    }
    
    gameObjects.Clear();
}

//
// kexGameLocal::ConstructActor
//

kexActor *kexGameLocal::ConstructActor(const char *className, kexDict *def, const int type,
                                       const float x, const float y, const float z,
                                       const float yaw, const int sector)
{
    kexActor *actor;

    if(!(actor = static_cast<kexActor*>(ConstructObject(className))))
    {
        return NULL;
    }

    actor->SetDefinition(def);
    
    actor->Origin().Set(x, y, z);
    actor->Yaw() = yaw;
    actor->Type() = type;
    
    if(sector <= -1)
    {
        actor->FindSector(actor->Origin());
    }
    else
    {
        actor->SetSector(&world->Sectors()[sector]);
    }
    
    actor->CallSpawn();
    return actor;
}

//
// kexGameLocal::SpawnActor
//

kexActor *kexGameLocal::SpawnActor(const int type, const float x, const float y, const float z,
                                   const float yaw, const int sector)
{
    kexStr className;
    kexDict *def;
    kexActor *actor;
    
    if((def = actorDefs.GetEntry(type)))
    {
        if(!def->GetString("classname", className))
        {
            className = "kexActor";
        }
    }
    else
    {
        switch(type)
        {
        case AT_PLAYER:
            className = "kexPuppet";
            break;
        
        default:
            className = "kexActor";
            break;
        }
    }
    
    actor = ConstructActor(className, def, type, x, y, z, yaw, sector);
    return actor;
}

//
// kexGameLocal::SpawnActor
//

kexActor *kexGameLocal::SpawnActor(const char *name, const float x, const float y, const float z,
                                   const float yaw, const int sector)
{
    kexStr className = "kexActor";
    kexDict *def = NULL;
    kexActor *actor;
    int type = -1;
    kexHashList<kexDict>::hashKey_t *hashKey = actorDefs.defs.GetHashKey(name);
    
    // when looking up the name from the hash, we have no way of knowing what
    // the type index is, so we need to pull the actual hash key and look
    // up the reference index
    if(hashKey)
    {
        if((def = &hashKey->data))
        {
            def->GetString("classname", className);
        }
    }

    if(hashKey)
    {
        type = hashKey->refIndex;
    }
    
    actor = ConstructActor(className, def, type, x, y, z, yaw, sector);
    return actor;
}

//
// kexGameLocal::SpawnActor
//

kexActor *kexGameLocal::SpawnActor(const kexStr &name, const float x, const float y, const float z,
                                   const float yaw, const int sector)
{
    return SpawnActor(name.c_str(), x, y, z, yaw, sector);
}

//
// kexGameLocal::SpawnMover
//

kexMover *kexGameLocal::SpawnMover(const char *className, const int type, const int sector)
{
    kexMover *mover;
    
    if(sector <= -1 || sector >= (int)world->NumSectors())
    {
        return NULL;
    }
    
    if(!(mover = static_cast<kexMover*>(ConstructObject(className))))
    {
        return NULL;
    }
    
    mover->Type() = type;
    mover->SetSector(&world->Sectors()[sector]);
    mover->CallSpawn();

    return mover;
}
