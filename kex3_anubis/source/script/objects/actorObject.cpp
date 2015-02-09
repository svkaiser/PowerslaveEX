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
    e->RegisterObjectMethod("kActor", "kVec3 &Origin(void)", asMETHODPR(kexActor, Origin, (void), kexVec3&), asCALL_THISCALL);
    e->RegisterObjectMethod("kActor", "void SetTarget(kActor@)", asMETHODPR(kexActor, SetTarget, (kexGameObject*), void), asCALL_THISCALL);
    e->RegisterObjectMethod("kActor", "kActor @GetTarget(void)", asMETHODPR(kexActor, Target, (void), kexGameObject*), asCALL_THISCALL);
    e->RegisterObjectMethod("kActor", "void Remove(void)", asMETHODPR(kexActor, Remove, (void), void), asCALL_THISCALL);
    e->RegisterObjectMethod("kActor", "kVec3 &Velocity(void)", asMETHODPR(kexActor, Velocity, (void), kexVec3&), asCALL_THISCALL);
    e->RegisterObjectMethod("kActor", "kVec3 &Movement(void)", asMETHODPR(kexActor, Movement, (void), kexVec3&), asCALL_THISCALL);
    e->RegisterObjectMethod("kActor", "int16 &Health(void)", asMETHODPR(kexActor, Health, (void), int16_t&), asCALL_THISCALL);
    e->RegisterObjectMethod("kActor", "float &Radius(void)", asMETHODPR(kexActor, Radius, (void), float&), asCALL_THISCALL);
    e->RegisterObjectMethod("kActor", "float &Height(void)", asMETHODPR(kexActor, Height, (void), float&), asCALL_THISCALL);
    e->RegisterObjectMethod("kActor", "float &StepHeight(void)", asMETHODPR(kexActor, StepHeight, (void), float&), asCALL_THISCALL);
    e->RegisterObjectMethod("kActor", "float &Scale(void)", asMETHODPR(kexActor, Scale, (void), float&), asCALL_THISCALL);
    e->RegisterObjectMethod("kActor", "float &Ticks(void)", asMETHODPR(kexActor, Ticks, (void), float&), asCALL_THISCALL);
    e->RegisterObjectMethod("kActor", "float &FloorHeight(void)", asMETHODPR(kexActor, FloorHeight, (void), float&), asCALL_THISCALL);
    e->RegisterObjectMethod("kActor", "float &CeilingHeight(void)", asMETHODPR(kexActor, CeilingHeight, (void), float&), asCALL_THISCALL);
    e->RegisterObjectMethod("kActor", "const int FrameID(void) const", asMETHODPR(kexActor, FrameID, (void) const, const int), asCALL_THISCALL);
}
