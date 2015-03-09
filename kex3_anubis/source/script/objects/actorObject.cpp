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

//
// kexScriptObjActor::Init
//

void kexScriptObjActor::Init(void)
{
    asIScriptEngine *e = kexGame::cScriptManager->Engine();
    
    e->RegisterObjectType("kActor", sizeof(kexActor), asOBJ_REF | asOBJ_NOCOUNT);
    e->RegisterObjectMethod("kActor", "kAngle &Yaw(void)", asMETHODPR(kexActor, Yaw, (void), kexAngle&), asCALL_THISCALL);
    e->RegisterObjectMethod("kActor", "kAngle &Pitch(void)", asMETHODPR(kexActor, Pitch, (void), kexAngle&), asCALL_THISCALL);
    e->RegisterObjectMethod("kActor", "kAngle &Roll(void)", asMETHODPR(kexActor, Roll, (void), kexAngle&), asCALL_THISCALL);
    e->RegisterObjectMethod("kActor", "kVec3 &Origin(void)", asMETHODPR(kexActor, Origin, (void), kexVec3&), asCALL_THISCALL);
    e->RegisterObjectMethod("kActor", "void SetTarget(kActor@)", asMETHODPR(kexActor, SetTarget, (kexGameObject*), void), asCALL_THISCALL);
    e->RegisterObjectMethod("kActor", "kActor @GetTarget(void)", asMETHODPR(kexActor, Target, (void), kexGameObject*), asCALL_THISCALL);
    e->RegisterObjectMethod("kActor", "void SetTaggedActor(kActor@)", asMETHODPR(kexActor, SetTaggedActor, (kexActor*), void), asCALL_THISCALL);
    e->RegisterObjectMethod("kActor", "kActor @GetTaggedActor(void)", asMETHODPR(kexActor, GetTaggedActor, (void), kexActor*), asCALL_THISCALL);
    e->RegisterObjectMethod("kActor", "void SetSector(const uint)", asMETHODPR(kexActor, SetSector, (const unsigned int), void), asCALL_THISCALL);
    e->RegisterObjectMethod("kActor", "void Remove(void)", asMETHODPR(kexActor, Remove, (void), void), asCALL_THISCALL);
    e->RegisterObjectMethod("kActor", "kVec3 &Velocity(void)", asMETHODPR(kexActor, Velocity, (void), kexVec3&), asCALL_THISCALL);
    e->RegisterObjectMethod("kActor", "kVec3 &Movement(void)", asMETHODPR(kexActor, Movement, (void), kexVec3&), asCALL_THISCALL);
    e->RegisterObjectMethod("kActor", "int16 &Health(void)", asMETHODPR(kexActor, Health, (void), int16_t&), asCALL_THISCALL);
    e->RegisterObjectMethod("kActor", "int &Type(void)", asMETHODPR(kexActor, Type, (void), int&), asCALL_THISCALL);
    e->RegisterObjectMethod("kActor", "uint &Flags(void)", asMETHODPR(kexActor, Flags, (void), unsigned int&), asCALL_THISCALL);
    e->RegisterObjectMethod("kActor", "float &Radius(void)", asMETHODPR(kexActor, Radius, (void), float&), asCALL_THISCALL);
    e->RegisterObjectMethod("kActor", "float &Height(void)", asMETHODPR(kexActor, Height, (void), float&), asCALL_THISCALL);
    e->RegisterObjectMethod("kActor", "float &StepHeight(void)", asMETHODPR(kexActor, StepHeight, (void), float&), asCALL_THISCALL);
    e->RegisterObjectMethod("kActor", "float &Scale(void)", asMETHODPR(kexActor, Scale, (void), float&), asCALL_THISCALL);
    e->RegisterObjectMethod("kActor", "float &Ticks(void)", asMETHODPR(kexActor, Ticks, (void), float&), asCALL_THISCALL);
    e->RegisterObjectMethod("kActor", "float &FloorHeight(void)", asMETHODPR(kexActor, FloorHeight, (void), float&), asCALL_THISCALL);
    e->RegisterObjectMethod("kActor", "float &CeilingHeight(void)", asMETHODPR(kexActor, CeilingHeight, (void), float&), asCALL_THISCALL);
    e->RegisterObjectMethod("kActor", "const int FrameID(void) const", asMETHODPR(kexActor, FrameID, (void) const, const int), asCALL_THISCALL);
    e->RegisterObjectMethod("kActor", "const int SectorIndex(void)", asMETHODPR(kexActor, SectorIndex, (void), const int), asCALL_THISCALL);
    e->RegisterObjectMethod("kActor", "bool InstanceOf(const kStr &in) const", asMETHODPR(kexActor, InstanceOf, (const kexStr&) const, bool), asCALL_THISCALL);
    e->RegisterObjectMethod("kActor", "void PlaySound(const kStr &in)", asMETHODPR(kexActor, PlaySound, (const kexStr&), void), asCALL_THISCALL);
    e->RegisterObjectMethod("kActor", "void ChangeAnim(const kStr &in)", asMETHODPR(kexActor, ChangeAnim, (const kexStr&), void), asCALL_THISCALL);
    e->RegisterObjectMethod("kActor", "const int GameTicks(void) const", asMETHODPR(kexActor, GameTicks, (void) const, const int), asCALL_THISCALL);
    e->RegisterObjectMethod("kActor", "kVec3 &Color(void)", asMETHODPR(kexActor, Color, (void), kexVec3&), asCALL_THISCALL);

    e->RegisterEnum("EnumActorFlags");
    e->RegisterEnumValue("EnumActorFlags", "AF_FLOORFRICTION", AF_FLOORFRICTION);
    e->RegisterEnumValue("EnumActorFlags", "AF_CEILINGFRICTION", AF_CEILINGFRICTION);
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
}
