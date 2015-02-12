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

#ifndef __SCRIPT_MATH_OBJ_H__
#define __SCRIPT_MATH_OBJ_H__

class kexScriptObjMath
{
public:
    static void         Init(void);
};

class kexScriptObjVec3
{
public:
    static void         Init(void);
    static void         ObjectConstruct1(kexVec3 *thisvec);
    static void         ObjectConstruct2(float x, float y, float z, kexVec3 *thisvec);
    static void         ObjectConstructCopy(const kexVec3 &in, kexVec3 *thisvec);
};

class kexScriptObjQuat
{
public:
    static void         Init(void);
    static void         ObjectConstruct1(kexQuat *thisq);
    static void         ObjectConstruct2(float a, float x, float y, float z, kexVec3 *thisq);
    static void         ObjectConstruct3(float a, kexVec3 &in, kexQuat *thisq);
    static void         ObjectConstructCopy(const kexQuat &in, kexQuat *thisq);
};

class kexScriptObjAngle
{
public:
    static void         Init(void);
    static void         ObjectConstruct1(kexAngle *thisang);
    static void         ObjectConstruct2(float an, kexAngle *thisang);
    static void         ObjectConstructCopy(const kexAngle &in, kexAngle *thisang);
};

#endif
