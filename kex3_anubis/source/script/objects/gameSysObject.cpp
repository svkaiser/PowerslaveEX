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
//      Game Object
//

#include "kexlib.h"
#include "game.h"
#include "scriptSystem.h"
#include "gameSysObject.h"

//
// kexScriptObjGame::Init
//

void kexScriptObjGame::Init(void)
{
    asIScriptEngine *e = kexGame::cScriptManager->Engine();

    e->RegisterObjectType("kGame", sizeof(kexGameLocal), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS);
    e->RegisterObjectType("kPlayLoop", sizeof(kexPlayLoop), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS);
    e->RegisterObjectType("kPlayer", sizeof(kexPlayer), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS);

    e->RegisterGlobalProperty("kGame Game", kexGame::cLocal);
    e->RegisterObjectMethod("kGame", "kActor @SpawnActor(const int, const float, const float, const float, const float, const int sector = -1)", asMETHODPR(kexGameLocal, SpawnActor, (const int, const float, const float, const float, const float, const int), kexActor*), asCALL_THISCALL);
    e->RegisterObjectMethod("kGame", "kActor @SpawnActor(const kStr &in, const float, const float, const float, const float, const int sector = -1)", asMETHODPR(kexGameLocal, SpawnActor, (const kexStr&, const float, const float, const float, const float, const int), kexActor*), asCALL_THISCALL);
    e->RegisterObjectMethod("kGame", "void CallDelayedMapScript(const kStr &in, kActor@, const float)", asMETHODPR(kexScriptObjGame, CallDelayedMapScript, (const kexStr&, kexActor*, const float), void), asCALL_THISCALL);
    e->RegisterObjectMethod("kGame", "void CallDelayedMapScript(const int, kActor@, const float)", asMETHODPR(kexScriptObjGame, CallDelayedMapScript, (const int, kexActor*, const float), void), asCALL_THISCALL);
    e->RegisterObjectMethod("kGame", "void HaltMapScript(const int)", asMETHODPR(kexScriptObjGame, HaltMapScript, (const int), void), asCALL_THISCALL);
    e->RegisterObjectMethod("kGame", "void FireRemoteEventFromTag(const int)", asMETHODPR(kexScriptObjGame, FireRemoteEventFromTag, (const int), void), asCALL_THISCALL);
    e->RegisterObjectMethod("kGame", "void FireActorEventFromTag(const int)", asMETHODPR(kexScriptObjGame, FireActorEventFromTag, (const int), void), asCALL_THISCALL);
    e->RegisterObjectMethod("kGame", "void PlaySound(const kStr &in)", asMETHODPR(kexScriptObjGame, PlaySound, (const kexStr&), void), asCALL_THISCALL);
    e->RegisterObjectMethod("kGame", "void PlayMusic(const kStr &in, const bool)", asMETHODPR(kexScriptObjGame, PlayMusic, (const kexStr&, const bool), void), asCALL_THISCALL);
    e->RegisterObjectMethod("kGame", "void StopMusic(void)", asMETHODPR(kexScriptObjGame, StopMusic, (void), void), asCALL_THISCALL);
    e->RegisterObjectMethod("kGame", "void MoveScriptedSector(const int, const float, const float, const bool)", asMETHODPR(kexScriptObjGame, MoveScriptedSector, (const int, const float, const float, const bool), void), asCALL_THISCALL);
    e->RegisterObjectMethod("kGame", "void ChangeMap(const kStr &in)", asMETHODPR(kexScriptObjGame, ChangeMap, (const kexStr&), void), asCALL_THISCALL);
    e->RegisterObjectMethod("kGame", "void EndGame(const bool)", asMETHODPR(kexScriptObjGame, EndGame, (const bool), void), asCALL_THISCALL);
    e->RegisterObjectMethod("kGame", "void SpawnLight(kActor@, const float, const kVec3 &in, const float, const int)", asMETHODPR(kexScriptObjGame, SpawnLight, (kexActor *source, const float radius, const kexVec3 &color, const float fadeTime, const int passes), void), asCALL_THISCALL);
    e->RegisterObjectMethod("kGame", "const bool LevelIsMapped(void) const", asMETHODPR(kexScriptObjGame, LevelIsMapped, (void) const, const bool), asCALL_THISCALL);

    e->RegisterGlobalProperty("kPlayLoop PlayLoop", kexGame::cLocal->PlayLoop());
    e->RegisterObjectMethod("kPlayLoop", "const int Ticks(void) const", asMETHODPR(kexPlayLoop, Ticks, (void) const, const int), asCALL_THISCALL);

    e->RegisterGlobalProperty("kPlayer Player", kexGame::cLocal->Player());
    e->RegisterObjectMethod("kPlayer", "int &LockTime(void)", asMETHODPR(kexPlayer, LockTime, (void), int&), asCALL_THISCALL);
    e->RegisterObjectMethod("kPlayer", "void HoldsterWeapon(void)", asMETHODPR(kexPlayer, HoldsterWeapon, (void), void), asCALL_THISCALL);
    e->RegisterObjectMethod("kPlayer", "const int16 Buttons(void) const", asMETHODPR(kexPlayer, Buttons, (void) const, const uint16_t), asCALL_THISCALL);
    e->RegisterObjectMethod("kPlayer", "int16 &Artifacts(void)", asMETHODPR(kexPlayer, Artifacts, (void), int16_t&), asCALL_THISCALL);
    e->RegisterObjectMethod("kPlayer", "int16 &QuestItems(void)", asMETHODPR(kexPlayer, QuestItems, (void), int16_t&), asCALL_THISCALL);
    e->RegisterObjectMethod("kPlayer", "int16 &Abilities(void)", asMETHODPR(kexPlayer, Abilities, (void), int16_t&), asCALL_THISCALL);
    e->RegisterObjectMethod("kPlayer", "uint &TeamDolls(void)", asMETHODPR(kexPlayer, TeamDolls, (void), uint&), asCALL_THISCALL);
    e->RegisterObjectMethod("kPlayer", "int &ShakeTime(void)", asMETHODPR(kexPlayer, ShakeTime, (void), int&), asCALL_THISCALL);

    e->RegisterEnum("EnumPlayerButtons");
    e->RegisterEnumValue("EnumPlayerButtons", "BC_ATTACK", BC_ATTACK);
    e->RegisterEnumValue("EnumPlayerButtons", "BC_JUMP", BC_JUMP);
    e->RegisterEnumValue("EnumPlayerButtons", "BC_FORWARD", BC_FORWARD);
    e->RegisterEnumValue("EnumPlayerButtons", "BC_BACKWARD", BC_BACKWARD);
    e->RegisterEnumValue("EnumPlayerButtons", "BC_LEFT", BC_LEFT);
    e->RegisterEnumValue("EnumPlayerButtons", "BC_RIGHT", BC_RIGHT);
    e->RegisterEnumValue("EnumPlayerButtons", "BC_STRAFELEFT", BC_STRAFELEFT);
    e->RegisterEnumValue("EnumPlayerButtons", "BC_STRAFERIGHT", BC_STRAFERIGHT);
    e->RegisterEnumValue("EnumPlayerButtons", "BC_WEAPONRIGHT", BC_WEAPONRIGHT);
    e->RegisterEnumValue("EnumPlayerButtons", "BC_WEAPONLEFT", BC_WEAPONLEFT);
    e->RegisterEnumValue("EnumPlayerButtons", "BC_USE", BC_USE);
    e->RegisterEnumValue("EnumPlayerButtons", "BC_MAPZOOMIN", BC_MAPZOOMIN);
    e->RegisterEnumValue("EnumPlayerButtons", "BC_MAPZOOMOUT", BC_MAPZOOMOUT);
}

//
// kexScriptObjGame::CallDelayedMapScript
//

void kexScriptObjGame::CallDelayedMapScript(const kexStr &str, kexActor *instigator, const float delay)
{
    kexGame::cScriptManager->CallDelayedMapScript(str.c_str(), instigator, delay);
}

//
// kexScriptObjGame::CallDelayedMapScript
//

void kexScriptObjGame::CallDelayedMapScript(const int scriptID, kexActor *instigator, const float delay)
{
    kexGame::cScriptManager->CallDelayedMapScript(scriptID, instigator, delay);
}

//
// kexScriptObjGame::HaltMapScript
//

void kexScriptObjGame::HaltMapScript(const int scriptID)
{
    kexGame::cScriptManager->HaltMapScript(scriptID);
}

//
// kexScriptObjGame::FireRemoteEventFromTag
//

void kexScriptObjGame::FireRemoteEventFromTag(const int tag)
{
    kexGame::cLocal->World()->FireRemoteEventFromTag(tag);
}

//
// kexScriptObjGame::FireActorEventFromTag
//

void kexScriptObjGame::FireActorEventFromTag(const int tag)
{
    kexGame::cLocal->World()->FireActorEventFromTag(tag);
}

//
// kexScriptObjGame::PlaySound
//

void kexScriptObjGame::PlaySound(const kexStr &str)
{
    kexGame::cLocal->PlaySound(str.c_str());
}

//
// kexScriptObjGame::PlayMusic
//

void kexScriptObjGame::PlayMusic(const kexStr &str, const bool bLoop)
{
    kex::cSound->PlayMusic(str.c_str(), bLoop);
}

//
// kexScriptObjGame::StopMusic
//

void kexScriptObjGame::StopMusic(void)
{
    kex::cSound->StopMusic();
}

//
// kexScriptObjGame::LevelIsMapped
//

const bool kexScriptObjGame::LevelIsMapped(void) const
{
    return kexGame::cLocal->PlayLoop()->LevelIsMapped();
}

//
// kexScriptObjGame::ChangeMap
//

void kexScriptObjGame::ChangeMap(const kexStr &map)
{
    kexGame::cLocal->ChangeMap(map.c_str());
}

//
// kexScriptObjGame::EndGame
//

void kexScriptObjGame::EndGame(const bool bGoodEnding)
{
    kexGame::cLocal->SetGameState(bGoodEnding ? GS_ENDING_GOOD : GS_ENDING_BAD);
}

//
// kexScriptObjGame::SpawnLight
//

void kexScriptObjGame::SpawnLight(kexActor *source, const float radius,
                                  const kexVec3 &color, const float fadeTime, const int passes)
{
    kexGame::cLocal->SpawnDynamicLight(source, radius, color, fadeTime, passes);
}

//
// kexScriptObjGame::MoveScriptedSector
//

void kexScriptObjGame::MoveScriptedSector(const int tag, const float height,
                                          const float speed, const bool bCeiling)
{
    kexGame::cLocal->World()->MoveScriptedSector(tag, height, speed, bCeiling);
}
