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
//      System Object
//

#include "kexlib.h"
#include "game.h"
#include "scriptSystem.h"
#include "systemObject.h"

static kexScriptObjSystem systemObjectLocal;

//
// kexScriptObjSystem::Init
//

void kexScriptObjSystem::Init(void)
{
    asIScriptEngine *e = kexGame::cScriptManager->Engine();

    e->RegisterObjectType("kSys", sizeof(kexScriptObjSystem), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS);
    e->RegisterObjectMethod("kSys", "void Print(const kStr &in)", asMETHODPR(kexScriptObjSystem, Printf, (const kexStr &str), void), asCALL_THISCALL);
    e->RegisterObjectMethod("kSys", "void Warning(const kStr &in)", asMETHODPR(kexScriptObjSystem, Warning, (const kexStr &str), void), asCALL_THISCALL);
    e->RegisterObjectMethod("kSys", "int VideoWidth(void)", asMETHODPR(kexScriptObjSystem, VideoWidth, (void), int), asCALL_THISCALL);
    e->RegisterObjectMethod("kSys", "int VideoHeight(void)", asMETHODPR(kexScriptObjSystem, VideoHeight, (void), int), asCALL_THISCALL);
    e->RegisterObjectMethod("kSys", "int Mouse_X(void)", asMETHODPR(kexScriptObjSystem, Mouse_X, (void), int), asCALL_THISCALL);
    e->RegisterObjectMethod("kSys", "int Mouse_Y(void)", asMETHODPR(kexScriptObjSystem, Mouse_Y, (void), int), asCALL_THISCALL);

    e->RegisterGlobalProperty("kSys Sys", &systemObjectLocal);
}

//
// kexScriptObjSystem::Printf
//

void kexScriptObjSystem::Printf(const kexStr &str)
{
    kex::cSystem->Printf(kexStr::Format("%s\n", str.c_str()));
}

//
// kexScriptObjSystem::Warning
//

void kexScriptObjSystem::Warning(const kexStr &str)
{
    kex::cSystem->Warning(str.c_str());
}

//
// kexScriptObjSystem::VideoWidth
//

int kexScriptObjSystem::VideoWidth(void)
{
    return kex::cSystem->VideoWidth();
}

//
// kexScriptObjSystem::VideoHeight
//

int kexScriptObjSystem::VideoHeight(void)
{
    return kex::cSystem->VideoHeight();
}

//
// kexScriptObjSystem::Mouse_X
//

int kexScriptObjSystem::Mouse_X(void)
{
    return kex::cInput->MouseX();
}

//
// kexScriptObjSystem::Mouse_Y
//

int kexScriptObjSystem::Mouse_Y(void)
{
    return kex::cInput->MouseY();
}
