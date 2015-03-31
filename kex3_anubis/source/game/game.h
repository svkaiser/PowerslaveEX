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

class kexFont;
class kexTitleScreen;
class kexTranslation;
class kexPlayLoop;
class kexRenderView;
class kexMover;
class kexMenuPanel;

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
    GS_NONE     = 0,
    GS_TITLE,
    GS_LEVEL,
    GS_CHANGELEVEL,
    GS_MAPEDITOR,
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

    NUMWEAPONSTATES
} weaponState_t;

typedef enum
{
    PA_SANDALS      = BIT(0),
    PA_MASK         = BIT(1),
    PA_SHAWL        = BIT(2),
    PA_ANKLETS      = BIT(3),
    PA_SCEPTER      = BIT(4),
    PA_FEATHER      = BIT(5),
    PA_DOLPHIN      = BIT(6),
    PA_EAGLE        = BIT(7)
} playerArtifacts_t;

typedef enum
{
    PT_TRANSMITTER1 = BIT(0),
    PT_TRANSMITTER2 = BIT(1),
    PT_TRANSMITTER3 = BIT(2),
    PT_TRANSMITTER4 = BIT(3),
    PT_TRANSMITTER5 = BIT(4),
    PT_TRANSMITTER6 = BIT(5),
    PT_TRANSMITTER7 = BIT(6),
    PT_TRANSMITTER8 = BIT(7)
} playerTransmitter_t;

typedef enum
{
    PK_TIME         = BIT(0),
    PK_WAR          = BIT(1),
    PK_POWER        = BIT(2),
    PK_EARTH        = BIT(3)
} playerKeys_t;

typedef enum
{
    TD_DOLL01       = BIT(0),
    TD_DOLL02       = BIT(1),
    TD_DOLL03       = BIT(2),
    TD_DOLL04       = BIT(3),
    TD_DOLL05       = BIT(4),
    TD_DOLL06       = BIT(5),
    TD_DOLL07       = BIT(6),
    TD_DOLL08       = BIT(7),
    TD_DOLL09       = BIT(8),
    TD_DOLL10       = BIT(9),
    TD_DOLL11       = BIT(10),
    TD_DOLL12       = BIT(11),
    TD_DOLL13       = BIT(12),
    TD_DOLL14       = BIT(13),
    TD_DOLL15       = BIT(14),
    TD_DOLL16       = BIT(15),
    TD_DOLL17       = BIT(16),
    TD_DOLL18       = BIT(17),
    TD_DOLL19       = BIT(18),
    TD_DOLL20       = BIT(19),
    TD_DOLL21       = BIT(20),
    TD_DOLL22       = BIT(21),
    TD_DOLL23       = BIT(22)
} teamDolls_t;

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
    void                            UpdateGameObjects(void);
    void                            RemoveGameObject(kexGameObject *go);
    void                            RemoveAllGameObjects(void);
    void                            ChangeMap(const char *name);
    void                            PlaySound(const char *name);
    void                            ToggleQuitConfirm(const bool bToggle);

    typedef struct
    {
        int             maxAmmo;
        bool            bPersistent;
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
    
    kexTitleScreen                  *TitleScreen(void) { return titleScreen; }
    kexPlayLoop                     *PlayLoop(void) { return playLoop; }
    kexTranslation                  *Translation(void) { return translation; }
    kexWorld                        *World(void) { return world; }
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

    kexObject                       *ConstructObject(const char *className);
    kexActor                        *SpawnActor(const int type, const float x, const float y, const float z,
                                                const float yaw, const int sector = -1);
    kexActor                        *SpawnActor(const kexStr &name, const float x, const float y, const float z,
                                                const float yaw, const int sector = -1);
    void                            DrawSmallString(const char *string, float x, float y, float scale, bool center,
                                                    byte r = 0xff, byte g = 0xff, byte b = 0xff);
    void                            DrawBigString(const char *string, float x, float y, float scale, bool center,
                                                  byte r = 0xff, byte g = 0xff, byte b = 0xff);

private:
    void                            LoadNewMap(void);
    void                            InitWeaponDefs(void);
    void                            StopSounds(void);
    void                            UpdateSounds(void);
    void                            DrawQuitConfirm(void);
    
    kexFont                         *smallFont;
    kexFont                         *bigFont;
    kexTitleScreen                  *titleScreen;
    kexPlayLoop                     *playLoop;
    kexTranslation                  *translation;
    kexWorld                        *world;
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
    kexIndexDefManager              weaponDef;
    kexStr                          pendingMap;
    weaponInfo_t                    weaponInfo[NUMPLAYERWEAPONS];
    kexTexture                      *loadingPic;
    bool                            bQuitConfirm;
    kexMenuPanel::selectButton_t    quitYesButton;
    kexMenuPanel::selectButton_t    quitNoButton;
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
    static kexScriptManager     *cScriptManager;
    static kexActionDefManager  *cActionDefManager;
    static kexActorFactory      *cActorFactory;
    static kexMenuPanel         *cMenuPanel;
};

#endif
