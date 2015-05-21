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
//      Actor Script Object
//

#include "kexlib.h"
#include "game.h"
#include "scriptSystem.h"
#include "actorObject.h"

#define REGISTER_ACTOR_CLASS(asName, clsName)   \
    e->RegisterObjectType(asName, sizeof(kexActor), asOBJ_REF | asOBJ_NOCOUNT);   \
    e->RegisterObjectMethod(asName, "kAngle &Yaw(void)", asMETHODPR(clsName, Yaw, (void), kexAngle&), asCALL_THISCALL);  \
    e->RegisterObjectMethod(asName, "kAngle &Pitch(void)", asMETHODPR(clsName, Pitch, (void), kexAngle&), asCALL_THISCALL);  \
    e->RegisterObjectMethod(asName, "kAngle &Roll(void)", asMETHODPR(clsName, Roll, (void), kexAngle&), asCALL_THISCALL);    \
    e->RegisterObjectMethod(asName, "kVec3 &Origin(void)", asMETHODPR(clsName, Origin, (void), kexVec3&), asCALL_THISCALL);  \
    e->RegisterObjectMethod(asName, "void SetTarget(kActor@)", asMETHODPR(clsName, SetTarget, (kexGameObject*), void), asCALL_THISCALL); \
    e->RegisterObjectMethod(asName, "kActor @GetTarget(void)", asMETHODPR(clsName, Target, (void), kexGameObject*), asCALL_THISCALL);    \
    e->RegisterObjectMethod(asName, "void SetTaggedActor(kActor@)", asMETHODPR(clsName, SetTaggedActor, (kexActor*), void), asCALL_THISCALL);    \
    e->RegisterObjectMethod(asName, "kActor @GetTaggedActor(void)", asMETHODPR(clsName, GetTaggedActor, (void), kexActor*), asCALL_THISCALL);    \
    e->RegisterObjectMethod(asName, "void SetSector(const uint)", asMETHODPR(clsName, SetSector, (const unsigned int), void), asCALL_THISCALL);  \
    e->RegisterObjectMethod(asName, "void Remove(void)", asMETHODPR(clsName, Remove, (void), void), asCALL_THISCALL);    \
    e->RegisterObjectMethod(asName, "kVec3 &Velocity(void)", asMETHODPR(clsName, Velocity, (void), kexVec3&), asCALL_THISCALL);  \
    e->RegisterObjectMethod(asName, "kVec3 &Movement(void)", asMETHODPR(clsName, Movement, (void), kexVec3&), asCALL_THISCALL);  \
    e->RegisterObjectMethod(asName, "int16 &Health(void)", asMETHODPR(clsName, Health, (void), int16_t&), asCALL_THISCALL);  \
    e->RegisterObjectMethod(asName, "int &Type(void)", asMETHODPR(clsName, Type, (void), int&), asCALL_THISCALL);    \
    e->RegisterObjectMethod(asName, "uint &Flags(void)", asMETHODPR(clsName, Flags, (void), unsigned int&), asCALL_THISCALL);    \
    e->RegisterObjectMethod(asName, "float &Radius(void)", asMETHODPR(clsName, Radius, (void), float&), asCALL_THISCALL);    \
    e->RegisterObjectMethod(asName, "float &Height(void)", asMETHODPR(clsName, Height, (void), float&), asCALL_THISCALL);    \
    e->RegisterObjectMethod(asName, "float &StepHeight(void)", asMETHODPR(clsName, StepHeight, (void), float&), asCALL_THISCALL);    \
    e->RegisterObjectMethod(asName, "float &FallHeight(void)", asMETHODPR(clsName, FallHeight, (void), float&), asCALL_THISCALL);    \
    e->RegisterObjectMethod(asName, "float &Friction(void)", asMETHODPR(clsName, Friction, (void), float&), asCALL_THISCALL);    \
    e->RegisterObjectMethod(asName, "float &Scale(void)", asMETHODPR(clsName, Scale, (void), float&), asCALL_THISCALL);  \
    e->RegisterObjectMethod(asName, "float &Ticks(void)", asMETHODPR(clsName, Ticks, (void), float&), asCALL_THISCALL);  \
    e->RegisterObjectMethod(asName, "float &FloorHeight(void)", asMETHODPR(clsName, FloorHeight, (void), float&), asCALL_THISCALL);  \
    e->RegisterObjectMethod(asName, "float &CeilingHeight(void)", asMETHODPR(clsName, CeilingHeight, (void), float&), asCALL_THISCALL);  \
    e->RegisterObjectMethod(asName, "const int FrameID(void) const", asMETHODPR(clsName, FrameID, (void) const, const int), asCALL_THISCALL);    \
    e->RegisterObjectMethod(asName, "void SetFrameID(const int)", asMETHODPR(clsName, SetFrameID, (const int), void), asCALL_THISCALL); \
    e->RegisterObjectMethod(asName, "const int SectorIndex(void)", asMETHODPR(clsName, SectorIndex, (void), const int), asCALL_THISCALL);    \
    e->RegisterObjectMethod(asName, "bool InstanceOf(const kStr &in) const", asMETHODPR(clsName, InstanceOf, (const kexStr&) const, bool), asCALL_THISCALL); \
    e->RegisterObjectMethod(asName, "void PlaySound(const kStr &in)", asMETHODPR(clsName, PlaySound, (const kexStr&), void), asCALL_THISCALL);   \
    e->RegisterObjectMethod(asName, "void ChangeAnim(const kStr &in)", asMETHODPR(clsName, ChangeAnim, (const kexStr&), void), asCALL_THISCALL); \
    e->RegisterObjectMethod(asName, "const int GameTicks(void) const", asMETHODPR(clsName, GameTicks, (void) const, const int), asCALL_THISCALL);    \
    e->RegisterObjectMethod(asName, "kVec3 &Color(void)", asMETHODPR(clsName, Color, (void), kexVec3&), asCALL_THISCALL);    \
    e->RegisterObjectMethod(asName, "kActor @SpawnActor(const kStr &in, const float, const float, const float)", asMETHODPR(clsName, SpawnActor, (const kexStr&, const float, const float, const float), kexActor*), asCALL_THISCALL);   \
    e->RegisterObjectMethod(asName, "kAngle &CollidedWallAngle(void)", asMETHODPR(clsName, CollidedWallAngle, (void), kexAngle&), asCALL_THISCALL);  \
    e->RegisterObjectMethod(asName, "kVec3 &CollidedWallNormal(void)", asMETHODPR(clsName, CollidedWallNormal, (void), kexVec3&), asCALL_THISCALL);  \
    e->RegisterObjectMethod(asName, "bool CanSee(const kVec3 &in, const float)", asMETHODPR(clsName, CanSee, (kexVec3&, const float), bool), asCALL_THISCALL);  \
    e->RegisterObjectMethod(asName, "bool RandomDecision(const int)", asMETHODPR(clsName, RandomDecision, (const int), bool), asCALL_THISCALL); \
    e->RegisterObjectMethod(asName, "void InflictDamage(kActor@, const int)", asMETHODPR(clsName, InflictDamage, (kexActor*, const int), void), asCALL_THISCALL)

//
// kexScriptObjActor::Init
//

void kexScriptObjActor::Init(void)
{
    asIScriptEngine *e = kexGame::cScriptManager->Engine();
    
    REGISTER_ACTOR_CLASS("kActor", kexActor);
    REGISTER_ACTOR_CLASS("kAI", kexAI);

    e->RegisterObjectMethod("kAI", "float &MoveSpeed(void)", asMETHODPR(kexAI, MoveSpeed, (void), float&), asCALL_THISCALL);
    e->RegisterObjectMethod("kAI", "float &TurnSpeed(void)", asMETHODPR(kexAI, TurnSpeed, (void), float&), asCALL_THISCALL);
    e->RegisterObjectMethod("kAI", "int &PainChance(void)", asMETHODPR(kexAI, PainChance, (void), int&), asCALL_THISCALL);
    e->RegisterObjectMethod("kAI", "void Ignite(void)", asMETHODPR(kexAI, Ignite, (void), void), asCALL_THISCALL);
    e->RegisterObjectMethod("kAI", "void ClearBurn(void)", asMETHODPR(kexAI, ClearBurn, (void), void), asCALL_THISCALL);
    e->RegisterObjectMethod("kAI", "uint &AIFlags(void)", asMETHODPR(kexAI, AIFlags, (void), unsigned int&), asCALL_THISCALL);

    e->RegisterEnum("EnumActorFlags");
    e->RegisterEnumValue("EnumActorFlags", "AF_NOENTERWATER", AF_NOENTERWATER);
    e->RegisterEnumValue("EnumActorFlags", "AF_SOLID", AF_SOLID);
    e->RegisterEnumValue("EnumActorFlags", "AF_NOADVANCEFRAMES", AF_NOADVANCEFRAMES);
    e->RegisterEnumValue("EnumActorFlags", "AF_RANDOMIZATION", AF_RANDOMIZATION);
    e->RegisterEnumValue("EnumActorFlags", "AF_FLASH", AF_FLASH);
    e->RegisterEnumValue("EnumActorFlags", "AF_SHOOTABLE", AF_SHOOTABLE);
    e->RegisterEnumValue("EnumActorFlags", "AF_FULLBRIGHT", AF_FULLBRIGHT);
    e->RegisterEnumValue("EnumActorFlags", "AF_MOVEABLE", AF_MOVEABLE);
    e->RegisterEnumValue("EnumActorFlags", "AF_TOUCHABLE", AF_TOUCHABLE);
    e->RegisterEnumValue("EnumActorFlags", "AF_BOUNCY", AF_BOUNCY);
    e->RegisterEnumValue("EnumActorFlags", "AF_INWATER", AF_INWATER);
    e->RegisterEnumValue("EnumActorFlags", "AF_NODROPOFF", AF_NODROPOFF);
    e->RegisterEnumValue("EnumActorFlags", "AF_EXPIRES", AF_EXPIRES);
    e->RegisterEnumValue("EnumActorFlags", "AF_HIDDEN", AF_HIDDEN);
    e->RegisterEnumValue("EnumActorFlags", "AF_NOEXITWATER", AF_NOEXITWATER);
    e->RegisterEnumValue("EnumActorFlags", "AF_COLLIDEDWALL", AF_COLLIDEDWALL);

    e->RegisterEnum("EnumAIFlags");
    e->RegisterEnumValue("EnumAIFlags", "AIF_TURNING", AIF_TURNING);
    e->RegisterEnumValue("EnumAIFlags", "AIF_LOOKALLAROUND", AIF_LOOKALLAROUND);
    e->RegisterEnumValue("EnumAIFlags", "AIF_ALWAYSRANGEATTACK", AIF_ALWAYSRANGEATTACK);
    e->RegisterEnumValue("EnumAIFlags", "AIF_ONFIRE", AIF_ONFIRE);
    e->RegisterEnumValue("EnumAIFlags", "AIF_FLYING", AIF_FLYING);
    e->RegisterEnumValue("EnumAIFlags", "AIF_RETREATAFTERMELEE", AIF_RETREATAFTERMELEE);
    e->RegisterEnumValue("EnumAIFlags", "AIF_RETREATTURN", AIF_RETREATTURN);
    e->RegisterEnumValue("EnumAIFlags", "AIF_FLYADJUSTVIEWLEVEL", AIF_FLYADJUSTVIEWLEVEL);
    e->RegisterEnumValue("EnumAIFlags", "AIF_NOLAVADAMAGE", AIF_NOLAVADAMAGE);
}
