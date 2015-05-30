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

#ifndef __GAME_H__
#define __GAME_H__

#define GAME_VERSION        KEX_GAME_VERSION
#define GAME_SUBVERSION     KEX_GAME_SUBVERSION

class kexFont;
class kexTitleScreen;
class kexTranslation;
class kexPlayLoop;
class kexOverWorld;
class kexRenderView;
class kexMover;
class kexMenuPanel;
class kexDLight;

typedef enum
{
    IA_ATTACK   = 0,
    IA_JUMP,
    IA_FORWARD,
    IA_BACKWARD,
    IA_LEFT,
    IA_RIGHT,
    IA_STRAFELEFT,
    IA_STRAFERIGHT,
    IA_WEAPNEXT,
    IA_WEAPPREV,
    IA_USE,
    IA_MAPZOOMIN,
    IA_MAPZOOMOUT,
    
    NUMINPUTACTIONS
} inputActions_t;

typedef enum
{
    GBE_MENU_UP         = BIT(0),
    GBE_MENU_RIGHT      = BIT(1),
    GBE_MENU_DOWN       = BIT(2),
    GBE_MENU_LEFT       = BIT(3),
    GBE_MENU_SELECT     = BIT(4),
    GBE_MENU_CANCEL     = BIT(5),
    GBE_MENU_BACK       = BIT(6),
    GBE_MENU_ACTIVATE   = BIT(7),
    GBE_MENU_DESELECT   = BIT(8)
} gameButtonEvents_t;

typedef enum
{
    GS_NONE     = 0,
    GS_TITLE,
    GS_OVERWORLD,
    GS_LEVEL,
    GS_CHANGELEVEL,
    GS_MAPEDITOR,
    GS_ENDING_BAD,
    GS_ENDING_GOOD,
    NUMGAMESTATES
} gameState_t;

typedef enum
{
    PW_MACHETE      =  0,
    PW_PISTOL,
    PW_M60,
    PW_BOMBS,
    PW_FLAMETHROWER,
    PW_COBRASTAFF,
    PW_RINGOFRA,
    PW_BRACELET,

    NUMPLAYERWEAPONS
} playerWeapons_t;

typedef enum
{
    WS_IDLE         = 0,
    WS_RAISE,
    WS_LOWER,
    WS_FIRE,
    WS_HOLDSTER,

    NUMWEAPONSTATES
} weaponState_t;

typedef enum
{
    PA_SANDALS      = BIT(0),
    PA_MASK         = BIT(1),
    PA_SHAWL        = BIT(2),
    PA_ANKLETS      = BIT(3),
    PA_SCEPTER      = BIT(4),
    PA_FEATHER      = BIT(5)
} playerArtifacts_t;

#include "object.h"
#include "actor.h"
#include "world.h"
#include "cmodel.h"
#include "playLoop.h"
#include "sprite.h"
#include "spriteAnim.h"
#include "pWeapon.h"
#include "player.h"
#include "actionDef.h"
#include "actorFactory.h"
#include "menu.h"
#include "textureObject.h"

//-----------------------------------------------------------------------------
//
// Local Game
//
//-----------------------------------------------------------------------------

class kexGameLocal : public kexGameLoop
{
public:
    kexGameLocal(void);
    ~kexGameLocal(void);

    void                            Init(void);
    void                            Start(void);
    void                            Stop(void);
    void                            Tick(void);
    void                            Draw(void);
    bool                            ProcessInput(inputEvent_t *ev);
    void                            StartNewGame(const int slot = -1);
    void                            UpdateGameObjects(void);
    void                            RemoveGameObject(kexGameObject *go);
    void                            RemoveAllGameObjects(void);
    void                            ChangeMap(const char *name);
    void                            PlaySound(const char *name);
    void                            SavePersistentData(void);
    void                            RestorePersistentData(void);
    bool                            SaveGame(const int slot);
    bool                            SaveGame(void);
    bool                            LoadGame(const int slot);

    typedef struct
    {
        int             maxAmmo;
        bool            bPersistent;
        bool            bDisableUnderwater;
        float           offsetX;
        float           offsetY;
        spriteAnim_t    *raise;
        spriteAnim_t    *lower;
        spriteAnim_t    *idle;
        spriteAnim_t    *fire;
        spriteAnim_t    *flame;
        spriteAnim_t    *ammoIdle[3];
        spriteAnim_t    *ammoRaise[3];
        spriteAnim_t    *ammoLower[3];
        spriteAnim_t    *ammoFire[3];
    } weaponInfo_t;
    
    typedef struct
    {
        kexStr              title;
        kexStr              saveTitle;
        kexStr              musicTrack;
        kexStr              map;
        kexStr              script;
        float               overworldX;
        float               overworldY;
        int16_t             transmitterBit;
        int16_t             refID;
        int16_t             nextMap[4];
    } mapInfo_t;

    typedef struct
    {
        int16_t             ankahs;
        int16_t             ankahFlags;
        bool                weapons[NUMPLAYERWEAPONS];
        int16_t             ammo[NUMPLAYERWEAPONS];
        int16_t             artifacts;
        int16_t             questItems;
        int16_t             abilities;
        uint                teamDolls;
        int16_t             health;
        playerWeapons_t     currentWeapon;
    } persistentData_t;

    bool                            LoadPersistentData(persistentData_t *data, int &currentMap, const int slot);
    
    kexTitleScreen                  *TitleScreen(void) { return titleScreen; }
    kexPlayLoop                     *PlayLoop(void) { return playLoop; }
    kexTranslation                  *Translation(void) { return translation; }
    kexOverWorld                    *OverWorld(void) { return overWorld; }
    kexFont                         *SmallFont(void) { return smallFont; }
    kexFont                         *BigFont(void) { return bigFont; }
    const int                       GetTicks(void) const { return ticks; }
    const gameState_t               GameState(void) const { return gameState; }
    void                            SetGameState(const gameState_t state) { pendingGameState = state; }
    kexPlayer                       *Player(void) { return player; }
    kexLinklist<kexGameObject>      &GameObjects(void) { return gameObjects; }
    kexCModel                       *CModel(void) { return cmodel; }
    kexSpriteManager                *SpriteManager(void) { return spriteManager; }
    kexSpriteAnimManager            *SpriteAnimManager(void) { return spriteAnimManager; }
    const weaponInfo_t              *WeaponInfo(const int id) const { return &weaponInfo[id]; }
    kexIndexDefManager              &ActorDefs(void) { return actorDefs; }
    kexDefManager                   &AnimPicDefs(void) { return animPicDefs; }
    kexArray<mapInfo_t>             &MapInfoList(void) { return mapInfoList; }
    kexArray<bool>                  &MapUnlockList(void) { return bMapUnlockList; }
    kexMenu                         *ActiveMenu(void) { return activeMenu; }
    mapInfo_t                       *ActiveMap(void) { return activeMap; }
    unsigned int                    &ButtonEvent(void) { return buttonEvent; }
    const int                       CurrentSaveSlot(void) const { return currentSaveSlot; }
    const bool                      NoMonstersEnabled(void) const { return bNoMonsters; }

    void                            SetMenu(const menus_t menu);
    void                            ClearMenu(const bool bClearAll = false);
    kexObject                       *ConstructObject(const char *className);
    kexActor                        *SpawnActor(const int type, const float x, const float y, const float z,
                                                const float yaw, const int sector = -1);
    kexActor                        *SpawnActor(const kexStr &name, const float x, const float y, const float z,
                                                const float yaw, const int sector = -1);

    void                            DrawSmallString(const char *string, float x, float y, float scale, bool center,
                                                    byte r = 0xff, byte g = 0xff, byte b = 0xff);
    void                            DrawBigString(const char *string, float x, float y, float scale, bool center,
                                                  byte r = 0xff, byte g = 0xff, byte b = 0xff);
    kexDLight                       *SpawnDynamicLight(kexActor *source, const float radius,
                                                       const kexVec3 &color, const float fadeTime = -1, const int passes = 2);
    static kexMenu                  *menus[NUMMENUS];
    static bool                     bShowSoundStats;
    static kexCvar                  cvarShowMovieIntro;
    
private:
    void                            LoadNewMap(void);
    void                            InitWeaponDefs(void);
    void                            InitMapDefs(void);
    void                            StopSounds(void);
    void                            UpdateSounds(void);
    void                            PrintSoundStats(void);

    bool                            bNoMonsters;
    
    kexFont                         *smallFont;
    kexFont                         *bigFont;
    kexTitleScreen                  *titleScreen;
    kexPlayLoop                     *playLoop;
    kexOverWorld                    *overWorld;
    kexTranslation                  *translation;
    kexCModel                       *cmodel;
    kexSpriteManager                *spriteManager;
    kexSpriteAnimManager            *spriteAnimManager;

    int                             ticks;
    gameState_t                     gameState;
    gameState_t                     pendingGameState;
    kexPlayer                       *player;
    kexGameLoop                     gameLoopStub;
    kexGameLoop                     *gameLoop;
    kexGameObject                   *goRover;
    kexLinklist<kexGameObject>      gameObjects;
    kexIndexDefManager              actorDefs;
    kexIndexDefManager              weaponDefs;
    kexIndexDefManager              mapDefs;
    kexDefManager                   animPicDefs;
    kexStr                          pendingMap;
    weaponInfo_t                    weaponInfo[NUMPLAYERWEAPONS];
    kexTexture                      loadingPic;
    kexMenu                         *activeMenu;
    kexArray<mapInfo_t>             mapInfoList;
    mapInfo_t                       *activeMap;
    kexArray<bool>                  bMapUnlockList;
    kexArray<bool>                  bCursorEnabled;
    kexArray<menus_t>               menuStack;
    unsigned int                    buttonEvent;
    persistentData_t                persistentData;
    int                             currentSaveSlot;
};

//-----------------------------------------------------------------------------
//
// Public Game Class
//
//-----------------------------------------------------------------------------

#include "scriptSystem.h"

class kexGame
{
public:
    static kexGameLocal         *cLocal;
    static kexWorld             *cWorld;
    static kexScriptManager     *cScriptManager;
    static kexActionDefManager  *cActionDefManager;
    static kexActorFactory      *cActorFactory;
    static kexMenuPanel         *cMenuPanel;
};

#endif
