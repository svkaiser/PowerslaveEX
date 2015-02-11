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

#ifndef __ACTIONDEF_H__
#define __ACTIONDEF_H__

typedef union
{
    int i;
    float f;
    char *s;
} actionDefArgs_t;

typedef enum
{
    AAT_INVALID     = -1,
    AAT_INTEGER     =  0,
    AAT_FLOAT,
    AAT_STRING
} actionArgType_t;

#define MAX_ACTION_DEF_ARGS     8

typedef struct
{
    kexStr              name;
    kexObject           *(*Create)(void);
    int                 argTypes[MAX_ACTION_DEF_ARGS];
    int                 numArgs;
} actionDefInfo_t;

class kexActionDefManager
{
public:
    kexActionDefManager(void);
    ~kexActionDefManager(void);

    void                            RegisterActions(void);
    void                            RegisterAction(const char *name, kexObject *(*Create)(void),
                                                   const int t1 = AAT_INVALID, const int t2 = AAT_INVALID,
                                                   const int t3 = AAT_INVALID, const int t4 = AAT_INVALID,
                                                   const int t5 = AAT_INVALID, const int t6 = AAT_INVALID,
                                                   const int t7 = AAT_INVALID, const int t8 = AAT_INVALID);
    void                            RegisterScriptAction(const char *name, kexStrList &argTypes,
                                                         unsigned int numArgs);
    kexActionDef                    *CreateInstance(const char *name);

    kexHashList<actionDefInfo_t>    actionDefInfos;
};

BEGIN_EXTENDED_KEX_CLASS(kexActionDef, kexObject);
public:
    kexActionDef(void);
    ~kexActionDef(void);

    virtual void            Execute(kexActor *actor) = 0;
    virtual void            Parse(kexLexer *lexer);

    actionDefInfo_t         *defInfo;
    actionDefArgs_t         *args;
    int                     *argTypes;
    int                     numArgs;
END_KEX_CLASS();

#endif
