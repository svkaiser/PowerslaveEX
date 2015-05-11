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
//      Math Object
//

#include "kexlib.h"
#include "game.h"
#include "scriptSystem.h"
#include "mathObject.h"

//
// kexScriptObjMath::Init
//

void kexScriptObjMath::Init(void)
{
    asIScriptEngine *e = kexGame::cScriptManager->Engine();

    e->SetDefaultNamespace("Math");
    e->RegisterGlobalFunction("float Sin(float)", asFUNCTION(kexMath::Sin), asCALL_CDECL);
    e->RegisterGlobalFunction("float Cos(float)", asFUNCTION(kexMath::Cos), asCALL_CDECL);
    e->RegisterGlobalFunction("float Tan(float)", asFUNCTION(kexMath::Tan), asCALL_CDECL);
    e->RegisterGlobalFunction("float ATan2(float)", asFUNCTION(kexMath::ATan2), asCALL_CDECL);
    e->RegisterGlobalFunction("float Fabs(float)", asFUNCTION(kexMath::Fabs), asCALL_CDECL);
    e->RegisterGlobalFunction("float Acos(float)", asFUNCTION(kexMath::ACos), asCALL_CDECL);
    e->RegisterGlobalFunction("float Sqrt(float)", asFUNCTION(kexMath::Sqrt), asCALL_CDECL);
    e->RegisterGlobalFunction("int Abs(int)", asFUNCTION(kexMath::Abs), asCALL_CDECL);
    e->RegisterGlobalFunction("float Ceil(float)", asFUNCTION(kexMath::Ceil), asCALL_CDECL);
    e->RegisterGlobalFunction("float Floor(float)", asFUNCTION(kexMath::Floor), asCALL_CDECL);
    e->RegisterGlobalFunction("float Log(float)", asFUNCTION(kexMath::Log), asCALL_CDECL);
    e->RegisterGlobalFunction("float Pow(float, float)", asFUNCTION(kexMath::Pow), asCALL_CDECL);
    e->RegisterGlobalFunction("float Deg2Rad(float)", asFUNCTION(kexMath::Deg2Rad), asCALL_CDECL);
    e->RegisterGlobalFunction("float Rad2Deg(float)", asFUNCTION(kexMath::Rad2Deg), asCALL_CDECL);
    e->RegisterGlobalFunction("float InvSqrt(float)", asFUNCTION(kexMath::InvSqrt), asCALL_CDECL);
    e->RegisterGlobalFunction("int SysRand(void)", asFUNCTION(kexRand::SysRand), asCALL_CDECL);
    e->RegisterGlobalFunction("int Rand(void)", asFUNCTION(kexRand::Int), asCALL_CDECL);
    e->RegisterGlobalFunction("uint8 RandByte(void)", asFUNCTION(kexRand::Byte), asCALL_CDECL);
    e->RegisterGlobalFunction("int RandMax(const int)", asFUNCTION(kexRand::Max), asCALL_CDECL);
    e->RegisterGlobalFunction("float RandFloat(void)", asFUNCTION(kexRand::Float), asCALL_CDECL);
    e->RegisterGlobalFunction("float RandCFloat(void)", asFUNCTION(kexRand::CFloat), asCALL_CDECL);
    e->RegisterGlobalFunction("float Range(const float, const float)", asFUNCTION(kexRand::Range), asCALL_CDECL);
    e->SetDefaultNamespace("");
}

//
// kexScriptObjVec3::Init
//

void kexScriptObjVec3::Init(void)
{
    asIScriptEngine *e = kexGame::cScriptManager->Engine();
    
    e->RegisterObjectType("kVec3", sizeof(kexVec3), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CA | asOBJ_APP_CLASS_ALLFLOATS);
    e->RegisterObjectBehaviour("kVec3", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(ObjectConstruct1), asCALL_CDECL_OBJLAST);
    e->RegisterObjectBehaviour("kVec3", asBEHAVE_CONSTRUCT, "void f(float, float, float)", asFUNCTION(ObjectConstruct2), asCALL_CDECL_OBJLAST);
    e->RegisterObjectBehaviour("kVec3", asBEHAVE_CONSTRUCT, "void f(const kVec3 &in)", asFUNCTION(ObjectConstructCopy), asCALL_CDECL_OBJLAST);
    e->RegisterObjectMethod("kVec3", "kVec3 &Normalize(void)", asMETHODPR(kexVec3, Normalize, (void), kexVec3&), asCALL_THISCALL);
    e->RegisterObjectMethod("kVec3", "float Dot(const kVec3 &in) const", asMETHODPR(kexVec3, Dot, (const kexVec3 &vec)const, float), asCALL_THISCALL);
    e->RegisterObjectMethod("kVec3", "float Unit(void) const", asMETHODPR(kexVec3, Unit, (void) const, float), asCALL_THISCALL);
    e->RegisterObjectMethod("kVec3", "float UnitSq(void) const", asMETHODPR(kexVec3, UnitSq, (void) const, float), asCALL_THISCALL);
    e->RegisterObjectMethod("kVec3", "float Distance(const kVec3 &in) const", asMETHODPR(kexVec3, Distance, (const kexVec3 &vec) const, float), asCALL_THISCALL);
    e->RegisterObjectMethod("kVec3", "kStr ToString(void)", asMETHODPR(kexVec3, ToString, (void)const, kexStr), asCALL_THISCALL);
    e->RegisterObjectMethod("kVec3", "float ToYaw(void)", asMETHODPR(kexVec3, ToYaw, (void)const, float), asCALL_THISCALL);
    e->RegisterObjectMethod("kVec3", "float ToPitch(void)", asMETHODPR(kexVec3, ToPitch, (void)const, float), asCALL_THISCALL);
    e->RegisterObjectMethod("kVec3", "void Clear(void)", asMETHODPR(kexVec3, Clear, (void), void), asCALL_THISCALL);
    e->RegisterObjectMethod("kVec3", "void Set(const float, const float, const float)", asMETHODPR(kexVec3, Set, (const float, const float, const float), void), asCALL_THISCALL);
    e->RegisterObjectMethod("kVec3", "kVec3 Lerp(const kVec3 &in, const float) const", asMETHODPR(kexVec3, Lerp, (const kexVec3 &next, const float movement) const, kexVec3), asCALL_THISCALL);
    e->RegisterObjectMethod("kVec3", "kVec3 &Lerp(const kVec3 &in, const float)", asMETHODPR(kexVec3, Lerp, (const kexVec3 &next, const float movement), kexVec3&), asCALL_THISCALL);
    e->RegisterObjectMethod("kVec3", "kVec3 opAdd(const kVec3 &in)", asMETHODPR(kexVec3, operator+, (const kexVec3&), kexVec3), asCALL_THISCALL);
    e->RegisterObjectMethod("kVec3", "kVec3 &opAddAssign(const kVec3 &in)", asMETHODPR(kexVec3, operator+=, (const kexVec3&), kexVec3&), asCALL_THISCALL);
    e->RegisterObjectMethod("kVec3", "kVec3 opNeg(void)", asMETHODPR(kexVec3, operator-, (void)const, kexVec3), asCALL_THISCALL);
    e->RegisterObjectMethod("kVec3", "kVec3 opSub(const kVec3 &in)", asMETHODPR(kexVec3, operator-, (const kexVec3&)const, kexVec3), asCALL_THISCALL);
    e->RegisterObjectMethod("kVec3", "kVec3 &opSubAssign(const kVec3 &in)", asMETHODPR(kexVec3, operator-=, (const kexVec3&), kexVec3&), asCALL_THISCALL);
    e->RegisterObjectMethod("kVec3", "kVec3 opMul(const kVec3 &in)", asMETHODPR(kexVec3, operator*, (const kexVec3&), kexVec3), asCALL_THISCALL);
    e->RegisterObjectMethod("kVec3", "kVec3 opMul(const float val)", asMETHODPR(kexVec3, operator*, (const float), kexVec3), asCALL_THISCALL);
    e->RegisterObjectMethod("kVec3", "kVec3 &opMulAssign(const kVec3 &in)", asMETHODPR(kexVec3, operator*=, (const kexVec3&), kexVec3&), asCALL_THISCALL);
    e->RegisterObjectMethod("kVec3", "kVec3 &opMulAssign(const float)", asMETHODPR(kexVec3, operator*=, (const float), kexVec3&), asCALL_THISCALL);
    e->RegisterObjectMethod("kVec3", "kVec3 opDiv(const kVec3 &in)", asMETHODPR(kexVec3, operator/, (const kexVec3&), kexVec3), asCALL_THISCALL);
    e->RegisterObjectMethod("kVec3", "kVec3 opDiv(const float val)", asMETHODPR(kexVec3, operator/, (const float), kexVec3), asCALL_THISCALL);
    e->RegisterObjectMethod("kVec3", "kVec3 &opDivAssign(const kVec3 &in)", asMETHODPR(kexVec3, operator/=, (const kexVec3&), kexVec3&), asCALL_THISCALL);
    e->RegisterObjectMethod("kVec3", "kVec3 &opAssign(const kVec3 &in)", asMETHODPR(kexVec3, operator=, (const kexVec3&), kexVec3&), asCALL_THISCALL);
    e->RegisterObjectMethod("kVec3", "float opIndex(uint)const", asMETHODPR(kexVec3, operator[], (int index)const, float), asCALL_THISCALL);
    e->RegisterObjectMethod("kVec3", "float opIndex(uint)", asMETHODPR(kexVec3, operator[], (int index), float&), asCALL_THISCALL);
    
    e->RegisterObjectProperty("kVec3", "float x", asOFFSET(kexVec3, x));
    e->RegisterObjectProperty("kVec3", "float y", asOFFSET(kexVec3, y));
    e->RegisterObjectProperty("kVec3", "float z", asOFFSET(kexVec3, z));
}

//
// kexScriptObjVec3::ObjectConstruct1
//

void kexScriptObjVec3::ObjectConstruct1(kexVec3 *thisvec)
{
    new(thisvec)kexVec3();
}

//
// kexScriptObjVec3::ObjectConstruct2
//

void kexScriptObjVec3::ObjectConstruct2(float x, float y, float z, kexVec3 *thisvec)
{
    new(thisvec)kexVec3(x, y, z);
}

//
// kexScriptObjVec3::ObjectConstructCopy
//

void kexScriptObjVec3::ObjectConstructCopy(const kexVec3 &in, kexVec3 *thisvec)
{
    new(thisvec)kexVec3(in);
}

//
// kexScriptObjQuat::Init
//

void kexScriptObjQuat::Init(void)
{
    asIScriptEngine *e = kexGame::cScriptManager->Engine();
    
    e->RegisterObjectType("kQuat", sizeof(kexQuat), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CA | asOBJ_APP_CLASS_ALLFLOATS);
    e->RegisterObjectBehaviour("kQuat", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(ObjectConstruct1), asCALL_CDECL_OBJLAST);
    e->RegisterObjectBehaviour("kQuat", asBEHAVE_CONSTRUCT, "void f(float, float, float, float)", asFUNCTION(ObjectConstruct2), asCALL_CDECL_OBJLAST);
    e->RegisterObjectBehaviour("kQuat", asBEHAVE_CONSTRUCT, "void f(float, kVec3 &in)", asFUNCTION(ObjectConstruct3), asCALL_CDECL_OBJLAST);
    e->RegisterObjectBehaviour("kQuat", asBEHAVE_CONSTRUCT, "void f(const kQuat &in)", asFUNCTION(ObjectConstructCopy), asCALL_CDECL_OBJLAST);
    e->RegisterObjectMethod("kQuat", "kQuat &Normalize(void)", asMETHODPR(kexQuat, Normalize, (void), kexQuat&), asCALL_THISCALL);
    e->RegisterObjectMethod("kQuat", "kQuat RotateFrom(const kVec3 &in, const kVec3 &in, float)", asMETHODPR(kexQuat, RotateFrom, (const kexVec3 &location, const kexVec3 &target, float maxAngle), kexQuat), asCALL_THISCALL);
    e->RegisterObjectMethod("kQuat", "kQuat opAdd(const kQuat &in)", asMETHODPR(kexQuat, operator+, (const kexQuat &in), kexQuat), asCALL_THISCALL);
    e->RegisterObjectMethod("kQuat", "kQuat opSub(const kQuat &in)", asMETHODPR(kexQuat, operator-, (const kexQuat &in), kexQuat), asCALL_THISCALL);
    e->RegisterObjectMethod("kQuat", "kQuat opMul(const kQuat &in)", asMETHODPR(kexQuat, operator*, (const kexQuat &in), kexQuat), asCALL_THISCALL);
    e->RegisterObjectMethod("kQuat", "kQuat &opAssign(const kQuat &in)", asMETHODPR(kexQuat, operator=, (const kexQuat&), kexQuat&), asCALL_THISCALL);
    e->RegisterObjectMethod("kVec3", "kVec3 opMul(const kQuat &in)", asMETHODPR(kexVec3, operator*, (const kexQuat&), kexVec3), asCALL_THISCALL);
    e->RegisterObjectMethod("kVec3", "kVec3 &opMulAssign(const kQuat &in)", asMETHODPR(kexVec3, operator*=, (const kexQuat&), kexVec3&), asCALL_THISCALL);
    e->RegisterObjectMethod("kVec3", "kQuat ToQuaternion(void)", asMETHODPR(kexVec3, ToQuat, (void), kexQuat), asCALL_THISCALL);
    e->RegisterObjectProperty("kQuat", "float x", asOFFSET(kexQuat, x));
    e->RegisterObjectProperty("kQuat", "float y", asOFFSET(kexQuat, y));
    e->RegisterObjectProperty("kQuat", "float z", asOFFSET(kexQuat, z));
    e->RegisterObjectProperty("kQuat", "float w", asOFFSET(kexQuat, w));
}

//
// kexScriptObjQuat::ObjectConstruct1
//

void kexScriptObjQuat::ObjectConstruct1(kexQuat *thisq)
{
    new(thisq)kexQuat();
}

//
// kexScriptObjQuat::ObjectConstruct2
//

void kexScriptObjQuat::ObjectConstruct2(float a, float x, float y, float z, kexVec3 *thisq)
{
    new(thisq)kexQuat(a, x, y, z);
}

//
// kexScriptObjQuat::ObjectConstruct3
//

void kexScriptObjQuat::ObjectConstruct3(float a, kexVec3 &in, kexQuat *thisq)
{
    new(thisq)kexQuat(a, in);
}

//
// kexScriptObjQuat::ObjectConstructCopy
//

void kexScriptObjQuat::ObjectConstructCopy(const kexQuat &in, kexQuat *thisq)
{
    new(thisq)kexQuat(in);
}

//
// kexScriptObjAngle::Init
//

void kexScriptObjAngle::Init(void)
{
    asIScriptEngine *e = kexGame::cScriptManager->Engine();
    
    e->RegisterObjectType("kAngle", sizeof(kexAngle), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CA | asOBJ_APP_CLASS_ALLFLOATS);
    e->RegisterObjectBehaviour("kAngle", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(ObjectConstruct1), asCALL_CDECL_OBJLAST);
    e->RegisterObjectBehaviour("kAngle", asBEHAVE_CONSTRUCT, "void f(float)", asFUNCTION(ObjectConstruct2), asCALL_CDECL_OBJLAST);
    e->RegisterObjectBehaviour("kAngle", asBEHAVE_CONSTRUCT, "void f(const kAngle &in)", asFUNCTION(ObjectConstructCopy), asCALL_CDECL_OBJLAST);
    e->RegisterObjectMethod("kAngle", "float Diff(const float)", asMETHODPR(kexAngle, Diff, (const float), float), asCALL_THISCALL);
    e->RegisterObjectMethod("kAngle", "float Diff(const kAngle &in)", asMETHODPR(kexAngle, Diff, (const kexAngle&), float), asCALL_THISCALL);
    e->RegisterObjectMethod("kAngle", "kAngle opAdd(const float) const", asMETHODPR(kexAngle, operator+, (const float) const, kexAngle), asCALL_THISCALL);
    e->RegisterObjectMethod("kAngle", "kAngle &opAddAssign(const float)", asMETHODPR(kexAngle, operator+=, (const float), kexAngle&), asCALL_THISCALL);
    e->RegisterObjectMethod("kAngle", "kAngle opSub(const float) const", asMETHODPR(kexAngle, operator-, (const float) const, kexAngle), asCALL_THISCALL);
    e->RegisterObjectMethod("kAngle", "kAngle &opSubAssign(const float)", asMETHODPR(kexAngle, operator-=, (const float), kexAngle&), asCALL_THISCALL);
    e->RegisterObjectMethod("kAngle", "kAngle opAdd(const kAngle &in) const", asMETHODPR(kexAngle, operator+, (const kexAngle&) const, kexAngle), asCALL_THISCALL);
    e->RegisterObjectMethod("kAngle", "kAngle &opAddAssign(const kAngle &in)", asMETHODPR(kexAngle, operator+=, (const kexAngle&), kexAngle&), asCALL_THISCALL);
    e->RegisterObjectMethod("kAngle", "kAngle opSub(const kAngle &in) const", asMETHODPR(kexAngle, operator-, (const kexAngle&) const, kexAngle), asCALL_THISCALL);
    e->RegisterObjectMethod("kAngle", "kAngle &opSubAssign(kAngle &in)", asMETHODPR(kexAngle, operator-=, (const kexAngle&), kexAngle&), asCALL_THISCALL);
    e->RegisterObjectMethod("kAngle", "kAngle &opAssign(const float)", asMETHODPR(kexAngle, operator=, (const float), kexAngle&), asCALL_THISCALL);
    e->RegisterObjectMethod("kAngle", "kAngle &opAssign(const kAngle &in)", asMETHODPR(kexAngle, operator=, (const kexAngle&), kexAngle&), asCALL_THISCALL);
    e->RegisterObjectMethod("kAngle", "kAngle opNeg(void) const", asMETHODPR(kexAngle, operator-, (void) const, kexAngle), asCALL_THISCALL);
    e->RegisterObjectMethod("kAngle", "float opImplConv(void)", asMETHODPR(kexAngle, operator float, (void), float), asCALL_THISCALL);
}

//
// kexScriptObjAngle::ObjectConstruct1
//

void kexScriptObjAngle::ObjectConstruct1(kexAngle *thisang)
{
    new(thisang)kexAngle();
}

//
// kexScriptObjAngle::ObjectConstruct2
//

void kexScriptObjAngle::ObjectConstruct2(float an, kexAngle *thisang)
{
    new(thisang)kexAngle(an);
}

//
// kexScriptObjAngle::ObjectConstructCopy
//

void kexScriptObjAngle::ObjectConstructCopy(const kexAngle &in, kexAngle *thisang)
{
    new(thisang)kexAngle(in);
}
