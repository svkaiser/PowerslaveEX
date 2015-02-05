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
//      String Object
//

#include "kexlib.h"
#include "game.h"
#include "scriptSystem.h"
#include "stringObject.h"

//
// kexScriptObjString::Init
//

void kexScriptObjString::Init(void)
{
    asIScriptEngine *e = kexGame::cScriptManager->Engine();

    e->RegisterObjectType("kStr", sizeof(kexStr), asOBJ_VALUE | asOBJ_APP_CLASS_CA);
    e->RegisterStringFactory("kStr", asFUNCTION(ObjectFactory), asCALL_CDECL);
    e->RegisterObjectBehaviour("kStr", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(ObjectConstruct), asCALL_CDECL_OBJLAST);
    e->RegisterObjectBehaviour("kStr", asBEHAVE_CONSTRUCT, "void f(const kStr &in)", asFUNCTION(ObjectConstructCopy), asCALL_CDECL_OBJLAST);
    e->RegisterObjectBehaviour("kStr", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(ObjectDeconstruct), asCALL_CDECL_OBJLAST);
    e->RegisterObjectMethod("kStr", "int IndexOf(const kStr &in) const", asMETHODPR(kexStr, IndexOf, (const kexStr&)const, int), asCALL_THISCALL);
    e->RegisterObjectMethod("kStr", "int Hash(void)", asMETHODPR(kexStr, Hash, (void), int), asCALL_THISCALL);
    e->RegisterObjectMethod("kStr", "int Atoi(void)", asMETHODPR(kexStr, Atoi, (void), int), asCALL_THISCALL);
    e->RegisterObjectMethod("kStr", "kStr &ToUpper(void)", asMETHODPR(kexStr, ToUpper, (void), kexStr&), asCALL_THISCALL);
    e->RegisterObjectMethod("kStr", "kStr &ToLower(void)", asMETHODPR(kexStr, ToLower, (void), kexStr&), asCALL_THISCALL);
    e->RegisterObjectMethod("kStr", "kStr &opAssign(const kStr &in)", asMETHODPR(kexStr, operator=, (const kexStr&), kexStr&), asCALL_THISCALL);
    e->RegisterObjectMethod("kStr", "kStr opAdd(const kStr &in)", asMETHODPR(kexStr, operator+, (const kexStr&), kexStr), asCALL_THISCALL);
    e->RegisterObjectMethod("kStr", "int8 opIndex(const int)", asMETHODPR(kexStr, operator[], (const int) const, const char), asCALL_THISCALL);
    e->RegisterObjectMethod("kStr", "kStr opAdd(bool)", asMETHODPR(kexStr, operator+, (bool), kexStr), asCALL_THISCALL);
    e->RegisterObjectMethod("kStr", "kStr opAdd(int)", asMETHODPR(kexStr, operator+, (int), kexStr), asCALL_THISCALL);
    e->RegisterObjectMethod("kStr", "kStr opAdd(float)", asMETHODPR(kexStr, operator+, (float), kexStr), asCALL_THISCALL);
    e->RegisterObjectMethod("kStr", "kStr &opAddAssign(const kStr &in)", asMETHODPR(kexStr, operator+=, (const kexStr&), kexStr&), asCALL_THISCALL);
    e->RegisterObjectMethod("kStr", "kStr &opAddAssign(bool)", asMETHODPR(kexStr, operator+=, (bool), kexStr&), asCALL_THISCALL);
}

//
// kexScriptObjString::ObjectConstruct
//

void kexScriptObjString::ObjectConstruct(kexStr *thisstring)
{
    new(thisstring)kexStr();
}

//
// kexScriptObjString::ObjectConstructCopy
//

void kexScriptObjString::ObjectConstructCopy(const kexStr &in, kexStr *thisstring)
{
    new(thisstring)kexStr(in);
}

//
// kexScriptObjString::ObjectFactory
//

kexStr kexScriptObjString::ObjectFactory(unsigned int byteLength, const char *s)
{
    return kexStr(s);
}

//
// kexScriptObjString::ObjectDeconstruct
//

void kexScriptObjString::ObjectDeconstruct(kexStr *thisstring)
{
    thisstring->~kexStr();
}
