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


#ifndef __SCRIPT_STRING_OBJ_H__
#define __SCRIPT_STRING_OBJ_H__

class kexScriptObjString
{
public:
    static void         Init(void);
    static void         ObjectConstruct(kexStr *thisstring);
    static void         ObjectConstructCopy(const kexStr &in, kexStr *thisstring);
    static kexStr       ObjectFactory(unsigned int byteLength, const char *s);
    static void         ObjectDeconstruct(kexStr *thisstring);
};

#endif
