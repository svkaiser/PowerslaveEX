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
#include "dlightObj.h"

static kexGameLocal gameLocal;
kexGameLoop *kex::cGame = &gameLocal;
kexGameLocal *kexGame::cLocal = &gameLocal;

kexMenu *kexGameLocal::menus[NUMMENUS];
bool kexGameLocal::bShowSoundStats = false;

kexCvar kexGameLocal::cvarShowMovieIntro("g_showintromovie", CVF_BOOL|CVF_CONFIG, "1", "Play intro movies on startup");

//=============================================================================
//
// Commands
//
//=============================================================================

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
        game->PlayLoop()->Print("no clipping off");
        puppet->PlayerFlags() &= ~PF_NOCLIP;
        puppet->FindSector(puppet->Origin());
    }
    else
    {
        game->PlayLoop()->Print("no clipping on");
        puppet->PlayerFlags() |= PF_NOCLIP;
        puppet->Velocity().Clear();
    }
}

//
// moses
//

COMMAND(moses)
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

    if(puppet->PlayerFlags() & PF_GOD)
    {
        game->PlayLoop()->Print("god mode off");
        puppet->PlayerFlags() &= ~PF_GOD;
        puppet->FindSector(puppet->Origin());
    }
    else
    {
        game->PlayLoop()->Print("god mode on");
        puppet->PlayerFlags() |= PF_GOD;
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
        game->PlayLoop()->Print("fly mode off");
        puppet->PlayerFlags() &= ~PF_FLY;
    }
    else
    {
        game->PlayLoop()->Print("fly mode on");
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
    kexGameLocal *game = kexGame::cLocal;
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
        
        game->PlayLoop()->Print("Got all weapons!");
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
            kex::cSystem->Printf("Got weapon %i!", weap);
        }
    }
    else if(!kexStr::Compare(kex::cCommands->GetArgv(1), "keys"))
    {
        gameLocal.Player()->GiveKey(0);
        gameLocal.Player()->GiveKey(1);
        gameLocal.Player()->GiveKey(2);
        gameLocal.Player()->GiveKey(3);

        game->PlayLoop()->Print("Got all keys!");
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
            
            if(arti <= -1 || arti >= 6)
            {
                kex::cSystem->Printf("give artifact <0 - 5>\n");
                return;
            }
            
            gameLocal.Player()->Artifacts() |= BIT(arti);
            game->PlayLoop()->Print(kexStr::Format("Got %s!",
                gameLocal.Translation()->GetString(100+arti)));
        }
    }
    else if(!kexStr::Compare(kex::cCommands->GetArgv(1), "artifacts"))
    {
        gameLocal.Player()->Artifacts() = 0x3F;
        game->PlayLoop()->Print("Got all artifacts!");
    }
    else if(!kexStr::Compare(kex::cCommands->GetArgv(1), "ability"))
    {
        if(argc != 3)
        {
            kex::cSystem->Printf("give ability <0 - 1>\n");
            return;
        }
        else
        {
            int ability = atoi(kex::cCommands->GetArgv(2));
            
            if(ability <= -1 || ability >= 2)
            {
                kex::cSystem->Printf("give ability <0 - 1>\n");
                return;
            }
            
            gameLocal.Player()->Abilities() |= BIT(ability);
        }
    }
    else
    {
        kex::cSystem->Printf("give <weapon#, weapons, keys, artifact#, ability#>\n");
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

    if(!gameLocal.Player()->WeaponOwned(wpn))
    {
        return;
    }
    
    gameLocal.Player()->PendingWeapon() = wpn;
}

//
// unlockmaps
//

COMMAND(unlockmaps)
{
    for(uint i = 0; i < gameLocal.MapUnlockList().Length(); ++i)
    {
        gameLocal.MapUnlockList()[i] = true;
    }
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
// savegame
//

COMMAND(savegame)
{
    int argc = kex::cCommands->GetArgc();
    int slot;
    
    if(argc != 2)
    {
        kex::cSystem->Printf("savegame <slot>\n");
        return;
    }

    slot = atoi(kex::cCommands->GetArgv(1));

    if(gameLocal.SaveGame(slot))
    {
        kex::cSystem->Printf("Game saved\n");
    }
}

//
// loadgame
//

COMMAND(loadgame)
{
    int argc = kex::cCommands->GetArgc();
    int slot;
    
    if(argc != 2)
    {
        kex::cSystem->Printf("loadgame <slot>\n");
        return;
    }

    slot = atoi(kex::cCommands->GetArgv(1));

    if(gameLocal.LoadGame(slot))
    {
        kex::cSystem->Printf("Game loaded\n");
    }
}

//
// generic button events (client-side only)
//

COMMAND(menu_up)        { gameLocal.ButtonEvent() |= GBE_MENU_UP; }
COMMAND(menu_right)     { gameLocal.ButtonEvent() |= GBE_MENU_RIGHT; }
COMMAND(menu_down)      { gameLocal.ButtonEvent() |= GBE_MENU_DOWN; }
COMMAND(menu_left)      { gameLocal.ButtonEvent() |= GBE_MENU_LEFT; }
COMMAND(menu_cancel)    { gameLocal.ButtonEvent() |= GBE_MENU_CANCEL; }
COMMAND(menu_back)      { gameLocal.ButtonEvent() |= GBE_MENU_BACK; }
COMMAND(menu_activate)  { gameLocal.ButtonEvent() |= GBE_MENU_ACTIVATE; }
COMMAND(menu_select)
{
    int argc = kex::cCommands->GetArgc();

    if(argc == 2)
    {
        if(atoi(kex::cCommands->GetArgv(1)) == 1)
        {
            gameLocal.ButtonEvent() |= GBE_MENU_DESELECT;
            return;
        }
    }

    gameLocal.ButtonEvent() |= GBE_MENU_SELECT;
}

//=============================================================================
//
// kexGameLocal
//
//=============================================================================

//
// kexGameLocal::kexGameLocal
//

kexGameLocal::kexGameLocal(void)
{
    this->smallFont         = NULL;
    this->bigFont           = NULL;
    this->activeMap         = NULL;
    this->ticks             = 0;
    this->buttonEvent       = 0;
    this->gameState         = GS_NONE;
    this->pendingGameState  = GS_NONE;
    this->bNoMonsters       = false;
    this->gameLoop          = &this->gameLoopStub;

    this->titleScreen       = new kexTitleScreen;
    this->playLoop          = new kexPlayLoop;
    this->overWorld         = new kexOverWorld;
    this->translation       = new kexTranslation;
    this->player            = new kexPlayer;
    this->cmodel            = new kexCModel;
    this->spriteManager     = new kexSpriteManager;
    this->spriteAnimManager = new kexSpriteAnimManager;

    this->currentSaveSlot   = -1;

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
    bool bHasUserConfig;

    kex::cActions->AddAction(IA_ATTACK, "attack");
    kex::cActions->AddAction(IA_JUMP, "jump");
    kex::cActions->AddAction(IA_FORWARD, "forward");
    kex::cActions->AddAction(IA_BACKWARD, "backward");
    kex::cActions->AddAction(IA_LEFT, "turnleft");
    kex::cActions->AddAction(IA_RIGHT, "turnright");
    kex::cActions->AddAction(IA_STRAFELEFT, "strafeleft");
    kex::cActions->AddAction(IA_STRAFERIGHT, "straferight");
    kex::cActions->AddAction(IA_WEAPNEXT, "+weapnext");
    kex::cActions->AddAction(IA_WEAPPREV, "+weapprev");
    kex::cActions->AddAction(IA_USE, "+use");
    kex::cActions->AddAction(IA_MAPZOOMIN, "mapzoomin");
    kex::cActions->AddAction(IA_MAPZOOMOUT, "mapzoomout");
    
    bHasUserConfig = kex::cSystem->ReadConfigFile("config.cfg");

    kex::cPakFiles->LoadZipFile("game.kpf");
    kex::cPakFiles->LoadUserFiles();

    if(!bHasUserConfig)
    {
        char *buffer = NULL;

        if(kex::cPakFiles->OpenFile("default.cfg", (byte**)(&buffer), hb_static) > 0)
        {
            kex::cCommands->Execute(buffer);
            Mem_Free(buffer);
        }
    }

    actorDefs.LoadFilesInDirectory("defs/actors/");
    weaponDefs.LoadFile("defs/weaponInfo.txt");
    mapDefs.LoadFile("defs/mapInfo.txt");
    animPicDefs.LoadFile("defs/animPicInfo.txt");
    
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
    if(kex::cSystem->CheckParam("-skipintromovies") <= 0 &&
        cvarShowMovieIntro.GetBool())
    {
        kex::cMoviePlayer->StartVideoStream("movies/LOBOTOMY.avi");
        kex::cMoviePlayer->StartVideoStream("movies/INTRO1.avi");
    }

    bNoMonsters = (kex::cSystem->CheckParam("-nomonsters") > 0);

    smallFont   = kexFont::Alloc("smallfont");
    bigFont     = kexFont::Alloc("bigfont");

    loadingPic.LoadFromFile("gfx/loadback.png", TC_CLAMP, TF_NEAREST);

    kexRender::cBackend->ClearBuffer();
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

    StartNewGame();
}

//
// kexGameLocal::StartNewGame
//

void kexGameLocal::StartNewGame(const int slot)
{
    for(uint i = 0; i < bMapUnlockList.Length(); ++i)
    {
        bMapUnlockList[i] = false;
    }

    player->Reset();
    overWorld->SelectedMap() = 0;

    SavePersistentData();

    if(slot >= 0)
    {
        SaveGame(slot);
    }
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
        dict->GetBool("disableUnderwater", weapon->bDisableUnderwater, false);
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
        dict->GetString("saveTitle", mapInfo->saveTitle);
        dict->GetString("musicTrack", mapInfo->musicTrack);
        dict->GetString("script", mapInfo->script);
        dict->GetFloat("overworld_x", mapInfo->overworldX);
        dict->GetFloat("overworld_y", mapInfo->overworldY);
        dict->GetInt("transmitter", mapInfo->transmitterBit, -1);
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
    
    kexGame::cWorld->UnloadMap();
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

            case GS_ENDING_BAD:
                kex::cMoviePlayer->StartVideoStream("movies/BADEND1.avi");
                kex::cMoviePlayer->StartVideoStream("movies/CREDU1.avi");
                gameLoop = titleScreen;
                break;

            case GS_ENDING_GOOD:
                kex::cMoviePlayer->StartVideoStream("movies/GOODEND1.avi");
                kex::cMoviePlayer->StartVideoStream("movies/CREDU1.avi");
                gameLoop = titleScreen;
                break;
                
            default:
                gameLoop = &gameLoopStub;
                break;
        }
        
        gameState = pendingGameState;
        pendingGameState = GS_NONE;
        
        kexRand::SetSeed(kex::cTimer->GetTicks() >> 4);
        
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
    buttonEvent = 0;

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

//=============================================================================
//
// Menus
//
//=============================================================================

//
// kexGameLocal::SetMenu
//

void kexGameLocal::SetMenu(const menus_t menu)
{
    if(activeMenu != NULL)
    {
        activeMenu->Reset();
    }

    activeMenu = menus[menu];
    activeMenu->Reset();
    activeMenu->OnShow();
    menuStack.Push(menu);

    if(menuStack.Length() >= 2)
    {
        return;
    }

    bCursorEnabled.Push(!kex::cInput->MouseGrabbed() && kex::cSession->CursorVisible());

    kex::cInput->ToggleMouseGrab(false);
    kex::cSession->ToggleCursor(true);
}

//
// kexGameLocal::ClearMenu
//

void kexGameLocal::ClearMenu(const bool bClearAll)
{
    unsigned int len = bCursorEnabled.Length();

    activeMenu->Reset();

    if(bClearAll)
    {
        menuStack.Empty();
        bCursorEnabled.Empty();

        kex::cInput->ToggleMouseGrab(true);
        kex::cInput->CenterMouse();
        kex::cSession->ToggleCursor(false);

        activeMenu = NULL;
        return;
    }

    menuStack.Pop();
    if(menuStack.Length() != 0)
    {
        activeMenu = menus[menuStack.GetLast()];
        return;
    }

    activeMenu = NULL;

    if(len != 0 && bCursorEnabled.GetLast() == false)
    {
        kex::cInput->ToggleMouseGrab(true);
        kex::cInput->CenterMouse();
        kex::cSession->ToggleCursor(false);
    }

    bCursorEnabled.Pop();
}

//=============================================================================
//
// Sound management
//
//=============================================================================

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

//=============================================================================
//
// Map loading/changing
//
//=============================================================================

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

    if(playLoop->RestartRequested())
    {
        RestorePersistentData();
    }

    if(!kexGame::cWorld->LoadMap(pendingMap.c_str()))
    {
        SetGameState(GS_TITLE);
        return;
    }
    
    SetGameState(GS_LEVEL);
}

//=============================================================================
//
// Common string drawing
//
//=============================================================================

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

//=============================================================================
//
// Game object management
//
//=============================================================================

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

#ifdef _DEBUG
    assert(go->RefCount() == 0);
#endif

    if(go->RefCount() != 0)
    {
        return;
    }

    delete go;
}

//
// kexGameLocal::RemoveAllGameObjects
//

void kexGameLocal::RemoveAllGameObjects(void)
{
    kexGameObject *go;
    kexGameObject *next;
    
    // de-reference all targets
    for(go = gameObjects.Next(); go != NULL; go = go->Link().Next())
    {
        go->SetTarget(NULL);
    }
    
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

//
// kexGameLocal::SpawnDynamicLight
//

kexDLight *kexGameLocal::SpawnDynamicLight(kexActor *source, const float radius,
                                           const kexVec3 &color, const float fadeTime, const int passes)
{
    if(!source || source->IsStale() || playLoop->RenderScene().DLights().MaxedOutLights())
    {
        return NULL;
    }

    kexDLight *obj = static_cast<kexDLight*>(ConstructObject("kexDLight"));

    obj->Origin() = source->Origin();
    obj->Sector() = source->Sector();

    obj->Radius() = radius;
    obj->FadeTime() = fadeTime;
    obj->Passes() = passes;
    obj->Color()[0] = (byte)(color.x * 255.0f);
    obj->Color()[1] = (byte)(color.y * 255.0f);
    obj->Color()[2] = (byte)(color.z * 255.0f);

    obj->CallSpawn();
    obj->SetTarget(source);

    return obj;
}

//=============================================================================
//
// Game saving/loading/restoring
//
//=============================================================================

//
// kexGameLocal::SavePersistentData
//

void kexGameLocal::SavePersistentData(void)
{
    for(int i = 0; i < NUMPLAYERWEAPONS; ++i)
    {
        persistentData.ammo[i] = player->GetAmmo(i);
        persistentData.weapons[i] = player->WeaponOwned(i);
    }

    persistentData.ankahs = player->Ankahs();
    persistentData.ankahFlags = player->AnkahFlags();
    persistentData.artifacts = player->Artifacts();
    persistentData.questItems = player->QuestItems();
    persistentData.teamDolls = player->TeamDolls();
    persistentData.abilities = player->Abilities();
    persistentData.health = player->Actor() ? player->Actor()->Health() : player->Health();
    persistentData.currentWeapon = player->CurrentWeapon();
}

//
// kexGameLocal::RestorePersistentData
//

void kexGameLocal::RestorePersistentData(void)
{
    for(int i = 0; i < NUMPLAYERWEAPONS; ++i)
    {
        player->SetWeapon(persistentData.weapons[i], i);
        player->SetAmmo(persistentData.ammo[i], i);
    }

    player->Ankahs() = persistentData.ankahs;
    player->AnkahFlags() = persistentData.ankahFlags;
    player->Artifacts() = persistentData.artifacts;
    player->QuestItems() = persistentData.questItems;
    player->TeamDolls() = persistentData.teamDolls;
    player->Abilities() = persistentData.abilities;
    player->Health() = persistentData.health;

    player->PendingWeapon() = persistentData.currentWeapon;
    player->ChangeWeapon();
}

//
// kexGameLocal::SaveGame
//

bool kexGameLocal::SaveGame(const int slot)
{
    kexStr filepath;
    kexBinFile saveFile;
    int padLength;

    assert(slot >= 0);

    filepath = kexStr::Format("%s\\saves\\save_%03d.sav", kex::cvarBasePath.GetValue(), slot);
    filepath.NormalizeSlashes();

    if(!saveFile.Create(filepath.c_str()))
    {
        return false;
    }

    saveFile.Write32(GAME_VERSION);
    saveFile.Write32(GAME_SUBVERSION);

    for(int i = 0; i < NUMPLAYERWEAPONS; ++i)
    {
        saveFile.Write16(player->GetAmmo(i));
    }

    for(int i = 0; i < NUMPLAYERWEAPONS; ++i)
    {
        saveFile.Write8(player->WeaponOwned(i));
    }

    saveFile.Write16(player->Ankahs());
    saveFile.Write16(player->AnkahFlags());
    saveFile.Write16(player->Artifacts());
    saveFile.Write16(player->QuestItems());
    saveFile.Write16(player->Abilities());
    saveFile.Write32(player->TeamDolls());
    saveFile.Write16(player->Actor() ? player->Actor()->Health() : player->Health());
    saveFile.Write16(static_cast<int16_t>(player->CurrentWeapon()));
    saveFile.Write16(overWorld->SelectedMap());
    saveFile.Write32(bMapUnlockList.Length());

    for(uint i = 0; i < bMapUnlockList.Length(); ++i)
    {
        saveFile.Write8(bMapUnlockList[i]);
    }

    padLength = ((saveFile.Length() + 0xF) & ~0xF) - saveFile.Length();

    for(int i = 0; i < padLength; ++i)
    {
        saveFile.Write8(0xFF);
    }

    currentSaveSlot = slot;
    return true;
}

//
// kexGameLocal::SaveGame
//

bool kexGameLocal::SaveGame(void)
{
    if(currentSaveSlot < 0)
    {
        return false;
    }

    return SaveGame(currentSaveSlot);
}

//
// kexGameLocal::LoadPersistentData
//

bool kexGameLocal::LoadPersistentData(persistentData_t *data, int &currentMap, const int slot)
{
    kexStr filepath;
    kexBinFile loadFile;
    int version, subVersion;

    assert(slot >= 0);

    filepath = kexStr::Format("saves\\save_%03d.sav", slot);
    filepath.NormalizeSlashes();

    if(!loadFile.OpenExternal(filepath.c_str()))
    {
        return false;
    }

    version = loadFile.Read32();
    subVersion = loadFile.Read32();

    if(version != GAME_VERSION || subVersion != GAME_SUBVERSION)
    {
        return false;
    }

    for(int i = 0; i < NUMPLAYERWEAPONS; ++i)
    {
        data->ammo[i] = loadFile.Read16();
    }

    for(int i = 0; i < NUMPLAYERWEAPONS; ++i)
    {
        data->weapons[i] = (loadFile.Read8() == 1);
    }

    data->ankahs = loadFile.Read16();
    data->ankahFlags = loadFile.Read16();
    data->artifacts = loadFile.Read16();
    data->questItems = loadFile.Read16();
    data->abilities = loadFile.Read16();
    data->teamDolls = loadFile.Read32();
    data->health = loadFile.Read16();
    data->currentWeapon = static_cast<playerWeapons_t>(loadFile.Read16());
    currentMap = loadFile.Read16();

    return true;
}

//
// kexGameLocal::LoadGame
//

bool kexGameLocal::LoadGame(const int slot)
{
    kexStr filepath;
    kexBinFile loadFile;
    int version, subVersion;
    uint numMaps;

    assert(slot >= 0);

    filepath = kexStr::Format("saves\\save_%03d.sav", slot);
    filepath.NormalizeSlashes();

    if(!loadFile.OpenExternal(filepath.c_str()))
    {
        return false;
    }

    version = loadFile.Read32();
    subVersion = loadFile.Read32();

    if(version != GAME_VERSION || subVersion != GAME_SUBVERSION)
    {
        return false;
    }

    for(int i = 0; i < NUMPLAYERWEAPONS; ++i)
    {
        persistentData.ammo[i] = loadFile.Read16();
    }

    for(int i = 0; i < NUMPLAYERWEAPONS; ++i)
    {
        persistentData.weapons[i] = (loadFile.Read8() == 1);
    }

    persistentData.ankahs = loadFile.Read16();
    persistentData.ankahFlags = loadFile.Read16();
    persistentData.artifacts = loadFile.Read16();
    persistentData.questItems = loadFile.Read16();
    persistentData.abilities = loadFile.Read16();
    persistentData.teamDolls = loadFile.Read32();
    persistentData.health = loadFile.Read16();
    persistentData.currentWeapon = static_cast<playerWeapons_t>(loadFile.Read16());

    overWorld->SelectedMap() = loadFile.Read16();
    numMaps = loadFile.Read32();

    assert(numMaps == bMapUnlockList.Length());

    for(uint i = 0; i < bMapUnlockList.Length(); ++i)
    {
        bMapUnlockList[i] = (loadFile.Read8() == 1);
    }

    currentSaveSlot = slot;

    RestorePersistentData();
    SetGameState(GS_OVERWORLD);
    return true;
}
