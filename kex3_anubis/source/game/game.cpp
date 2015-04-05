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
//      High level game logic class. This class runs above all
//      other sub-game loop classes and processes all high-level
//      input events (player commands, mouse response, etc), updating
//      active game objects, sounds, and scheduling map changes.
//
//      This class is also responsible for initializing all global
//      game data and scripts on startup.
//

#include "kexlib.h"
#include "renderMain.h"
#include "renderView.h"
#include "game.h"
#include "mover.h"
#include "titlescreen.h"
#include "playLoop.h"
#include "overWorld.h"
#include "menuPanel.h"
#include "localization.h"
#include "mapEditor.h"

static kexGameLocal gameLocal;
kexGameLoop *kex::cGame = &gameLocal;
kexGameLocal *kexGame::cLocal = &gameLocal;

kexMenu *kexGameLocal::menus[NUMMENUS];
bool kexGameLocal::bShowSoundStats = false;

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

    kexGame::cActorFactory->Spawn(kex::cCommands->GetArgv(1), x, y, z, a->Yaw());
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
        kex::cSystem->Printf("give <name>\n");
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
    else if(!kexStr::Compare(kex::cCommands->GetArgv(1), "keys"))
    {
        gameLocal.Player()->GiveKey(0);
        gameLocal.Player()->GiveKey(1);
        gameLocal.Player()->GiveKey(2);
        gameLocal.Player()->GiveKey(3);

        kex::cSystem->Printf("Got all keys!\n");
    }
    else if(!kexStr::Compare(kex::cCommands->GetArgv(1), "artifact"))
    {
        if(argc != 3)
        {
            kex::cSystem->Printf("give artifact <0 - 5>\n");
            return;
        }
        else
        {
            int arti = atoi(kex::cCommands->GetArgv(2));
            
            if(arti <= -1 || arti >= 5)
            {
                kex::cSystem->Printf("give artifact <0 - 5>\n");
                return;
            }
            
            gameLocal.Player()->Artifacts() |= BIT(arti);
            kex::cSystem->Printf("Got %s!\n", kexGame::cLocal->Translation()->GetString(100+arti));
        }
    }
    else
    {
        kex::cSystem->Printf("give <weapon#, weapons, keys, artifact#>\n");
    }
}

//
// weapon
//

COMMAND(weapon)
{
    int argc = kex::cCommands->GetArgc();
    playerWeapons_t wpn;
    
    if(gameLocal.GameState() != GS_LEVEL || gameLocal.Player()->Actor() == NULL)
    {
        return;
    }
    
    if(argc != 2)
    {
        kex::cSystem->Printf("weapon <weapon num>\n");
        return;
    }
    
    wpn = static_cast<playerWeapons_t>(atoi(kex::cCommands->GetArgv(1)));
    if(wpn < 0 || wpn >= NUMPLAYERWEAPONS)
    {
        return;
    }

    if(!kexGame::cLocal->Player()->WeaponOwned(wpn))
    {
        return;
    }
    
    kexGame::cLocal->Player()->PendingWeapon() = wpn;
}

//
// showmenu
//

COMMAND(showmenu)
{
    int argc = kex::cCommands->GetArgc();
    menus_t menuType;
    
    if(argc != 2)
    {
        kex::cSystem->Printf("showmenu <type>\n");
        return;
    }
    
    menuType = static_cast<menus_t>(atoi(kex::cCommands->GetArgv(1)));
    gameLocal.SetMenu(menuType);
}

//
// statsound
//

COMMAND(statsound)
{
    kexGameLocal::bShowSoundStats ^= 1;
}

//
// kexGameLocal::kexGameLocal
//

kexGameLocal::kexGameLocal(void)
{
    this->smallFont         = NULL;
    this->bigFont           = NULL;
    this->activeMap         = NULL;
    this->ticks             = 0;
    this->gameState         = GS_NONE;
    this->pendingGameState  = GS_NONE;
    this->gameLoop          = &this->gameLoopStub;

    this->titleScreen       = new kexTitleScreen;
    this->playLoop          = new kexPlayLoop;
    this->overWorld         = new kexOverWorld;
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
    delete overWorld;
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
    kex::cActions->AddAction(IA_MAPZOOMIN, "mapzoomin");
    kex::cActions->AddAction(IA_MAPZOOMOUT, "mapzoomout");
    
    kex::cSystem->ReadConfigFile("config.cfg");

    kex::cPakFiles->LoadUserFiles();
    kex::cPakFiles->LoadZipFile("game.kpf");

    actorDefs.LoadFilesInDirectory("defs/actors/");
    weaponDefs.LoadFile("defs/weaponInfo.txt");
    mapDefs.LoadFile("defs/mapInfo.txt");
    
    kexGame::cMenuPanel->Init();
    kexGame::cScriptManager->Init();
    kexGame::cActionDefManager->RegisterActions();

    for(int i = 0; i < NUMMENUS; ++i)
    {
        menus[i]->Init();
    }
}

//
// kexGameLocal::Start
//

void kexGameLocal::Start(void)
{
    smallFont   = kexFont::Alloc("smallfont");
    bigFont     = kexFont::Alloc("bigfont");

    loadingPic.LoadFromFile("gfx/loadback.png", TC_CLAMP, TF_NEAREST);

    kexRender::cScreen->SetOrtho();
    kexRender::cScreen->DrawTexture(&loadingPic, 0, 0, 255, 255, 255, 255);
    DrawSmallString("Loading", 160, 120, 1, true);
    kexRender::cBackend->SwapBuffers();

    titleScreen->Init();
    translation->Init();
    playLoop->Init();
    spriteManager->Init();
    spriteAnimManager->Init();
    overWorld->Init();

    pendingGameState = GS_TITLE;
    InitWeaponDefs();
    InitMapDefs();

    loadingPic.Delete();
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
        kexDict *dict = weaponDefs.GetEntry(i);
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
// kexGameLocal::InitMapDefs
//

void kexGameLocal::InitMapDefs(void)
{
    int totalMaps = 0;
    kexDict *dict;
    mapInfo_t *mapInfo;
    
    for(int i = 0; i < MAX_HASH; i++)
    {
        kexHashList<kexDict>::hashKey_t *hashKey = mapDefs.defs.GetHashKey(i);
        
        if(hashKey == NULL)
        {
            continue;
        }
        
        if(hashKey->refIndex > totalMaps)
        {
            totalMaps = hashKey->refIndex;
        }
    }
    
    if(totalMaps <= 0)
    {
        return;
    }

    totalMaps++;
    
    mapInfoList.Resize(totalMaps);
    bMapUnlockList.Resize(totalMaps);
    
    for(int i = 0; i < totalMaps; ++i)
    {
        int nextMap[4];

        bMapUnlockList[i] = false;

        dict = mapDefs.GetEntry(i);
        
        if(!dict)
        {
            continue;
        }
        
        mapInfo = &mapInfoList[i];
        
        dict->GetString("map", mapInfo->map);
        dict->GetString("title", mapInfo->title);
        dict->GetString("musicTrack", mapInfo->musicTrack);
        dict->GetString("script", mapInfo->script);
        dict->GetFloat("overworld_x", mapInfo->overworldX);
        dict->GetFloat("overworld_y", mapInfo->overworldY);
        dict->GetInt("transmitter", mapInfo->transmitterBit);
        dict->GetInt("nextmap_north", nextMap[0], -1);
        dict->GetInt("nextmap_east", nextMap[1], -1);
        dict->GetInt("nextmap_south", nextMap[2], -1);
        dict->GetInt("nextmap_west", nextMap[3], -1);

        for(int j = 0; j < 4; ++j)
        {
            if(nextMap[j] <= -1 || nextMap[j] >= (int)mapInfoList.Length())
            {
                mapInfo->nextMap[j] = -1;
                continue;
            }

            mapInfo->nextMap[j] = nextMap[j];
        }
        
        mapInfo->refID = i;
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
        StopSounds();
        gameLoop->Stop();
        
        if(activeMenu != NULL)
        {
            activeMenu->Reset();
            ClearMenu();
        }

        kex::cSession->ForceSingleFrame();
        
        switch(pendingGameState)
        {
            case GS_TITLE:
                gameLoop = titleScreen;
                break;

            case GS_LEVEL:
                gameLoop = playLoop;
                break;

            case GS_OVERWORLD:
                gameLoop = overWorld;
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
    
    if(activeMenu == NULL)
    {
        gameLoop->Tick();
    }
    else
    {
        activeMenu->Update();
    }
    
    player->Cmd().Reset();

    UpdateSounds();
    
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

    if(activeMenu != NULL)
    {
        activeMenu->Display();
    }

    kexGame::cScriptManager->DrawGCStats();
    PrintSoundStats();
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

    case ev_joystick:
        player->Cmd().SetJoy(ev);
        break;

    case ev_keydown:
    case ev_mousedown:
    case ev_joybtndown:
        kex::cActions->ExecuteCommand(ev->data1, false, ev->type);
        break;

    case ev_keyup:
    case ev_mouseup:
    case ev_joybtnup:
        kex::cActions->ExecuteCommand(ev->data1, true, ev->type);
        break;
    }

    if(activeMenu != NULL)
    {
        return activeMenu->ProcessInput(ev);
    }

    return gameLoop->ProcessInput(ev);
}

//
// kexGameLocal::SetMenu
//

void kexGameLocal::SetMenu(const menus_t menu)
{
    activeMenu = menus[menu];
    bCursorEnabled.Push(!kex::cInput->MouseGrabbed() && kex::cSession->CursorVisible());

    kex::cInput->ToggleMouseGrab(false);
    kex::cSession->ToggleCursor(true);
}

//
// kexGameLocal::ClearMenu
//

void kexGameLocal::ClearMenu(void)
{
    unsigned int len = bCursorEnabled.Length();

    activeMenu = NULL;

    if(len != 0 && bCursorEnabled[len-1] == false)
    {
        kex::cInput->ToggleMouseGrab(true);
        kex::cInput->CenterMouse();
        kex::cSession->ToggleCursor(false);
    }

    bCursorEnabled.Pop();
}

//
// kexGameLocal::PlaySound
//

void kexGameLocal::PlaySound(const char *name)
{
    kex::cSound->Play((void*)name, 128, 0);
}

//
// kexGameLocal::StopSounds
//

void kexGameLocal::StopSounds(void)
{
    for(int i = 0; i < kex::cSound->NumSources(); ++i)
    {
        kex::cSound->Stop(i);
    }
}

//
// kexGameLocal::UpdateSounds
//

void kexGameLocal::UpdateSounds(void)
{
    for(int i = 0; i < kex::cSound->NumSources(); ++i)
    {
        kexObject *obj = kex::cSound->GetRefObject(i);
        float volume, pan;

        if(!obj || !obj->InstanceOf(&kexGameObject::info))
        {
            continue;
        }

        if(obj == player->Actor())
        {
            continue;
        }

        static_cast<kexGameObject*>(obj)->GetSoundParameters(volume, pan);
        kex::cSound->UpdateSource(i, (int)volume, (int)pan);
    }
}

//
// kexGameLocal::PrintSoundStats
//

void kexGameLocal::PrintSoundStats(void)
{
    int active = 0;

    if(!bShowSoundStats)
    {
        return;
    }

    for(int i = 0; i < kex::cSound->NumSources(); ++i)
    {
        if(kex::cSound->Playing(i))
        {
            active++;
        }
    }

    kexRender::cUtils->PrintStatsText("Active Sounds:", ": %i", active);
    kexRender::cUtils->AddDebugLineSpacing();
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
    for(unsigned int i = 0; i < mapInfoList.Length(); ++i)
    {
        if(!kexStr::Compare(pendingMap, mapInfoList[i].map))
        {
            activeMap = &mapInfoList[i];
            break;
        }
    }

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
// kexGameLocal::SpawnActor
//

kexActor *kexGameLocal::SpawnActor(const int type, const float x, const float y, const float z,
                                   const float yaw, const int sector)
{
    return kexGame::cActorFactory->Spawn(type, x, y, z, yaw, sector);
}

//
// kexGameLocal::SpawnActor
//

kexActor *kexGameLocal::SpawnActor(const kexStr &name, const float x, const float y, const float z,
                                   const float yaw, const int sector)
{
    return kexGame::cActorFactory->Spawn(name, x, y, z, yaw, sector);
}
