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
    e->RegisterGlobalProperty("kGame Game", kexGame::cLocal);
    e->RegisterObjectMethod("kGame", "kActor @SpawnActor(const int, const float, const float, const float, const float, const int sector = -1)", asMETHODPR(kexGameLocal, SpawnActor, (const int, const float, const float, const float, const float, const int), kexActor*), asCALL_THISCALL);
    e->RegisterObjectMethod("kGame", "kActor @SpawnActor(const kStr &in, const float, const float, const float, const float, const int sector = -1)", asMETHODPR(kexGameLocal, SpawnActor, (const kexStr&, const float, const float, const float, const float, const int), kexActor*), asCALL_THISCALL);
}
