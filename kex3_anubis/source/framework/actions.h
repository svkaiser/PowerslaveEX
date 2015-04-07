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

#ifndef __ACTIONS_H__
#define __ACTIONS_H__

#include "keyboard.h"
#include "mouse.h"
#include "joystick.h"

#define MAX_KEYS    (NUMKEYBOARDKEYS + NUMMOUSEBUTTONS + NUMJOYSTICKBUTTONS)

typedef kexLinklist<struct cmdlist_s> cmdLink_t;

typedef struct keyaction_s
{
    byte                keyid;
    kexStr              name;
} keyaction_t;

typedef struct cmdlist_s
{
    char                *command;
    keyaction_t         *action;
    cmdLink_t           link;
} cmdlist_t;

class kexInputAction
{
public:
    void                    Clear(void);
    int                     FindAction(const char *name);
    void                    AddAction(byte id, const char *name);
    bool                    ActionExists(const char *name);
    void                    WriteBindings(FILE *file);
    void                    ExecuteCommand(int key, bool up, const int eventType);
    void                    ExecuteMouseCommand(int button, bool up);
    int                     GetKeyCode(char *key);
    char                    *GetKeyName(int key);
    const char              *GetKeyboardKey(const int key);
    const char              *GetMouseKey(const int key);
    const char              *GetJoystickKey(const int key);
    const int               GetKeyboardCode(const int key);
    const int               GetMouseCode(const int key);
    const int               GetJoystickCode(const int key);
    void                    BindCommand(int key, const char *string);
    void                    UnBindCommand(int key, const char *string);
    void                    ListBindings(void);
    const int               GetAction(const int id);
    void                    GetActionBinds(kexStrList &bindList, const int id);
    void                    GetCommandBinds(kexStrList &bindList, const char *command);
    bool                    IsKeyBindedToAction(const int key, const char *action);
    
    cmdLink_t               *KeyCommands(void) { return keycmds; }

private:
    cmdLink_t               keycmds[MAX_KEYS];
    kexArray<keyaction_t>   keyActions;
    kexArray<int>           heldActions;
};

#endif
