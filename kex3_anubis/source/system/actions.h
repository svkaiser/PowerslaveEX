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

//
// KEYS
//
#define MAX_KEYS        512

#define CKF_GAMEPAD     0x1
#define CKF_UP          0x4000
#define CKF_COUNTMASK   0x00ff

#define MAXACTIONS  256

typedef struct
{
    float       mousex;
    float       mousey;
    float       joyx;
    float       joyy;
    int         actions[MAXACTIONS];
    int         flags;
} control_t;

typedef struct keyaction_s
{
    byte                keyid;
    char                name[32];
    struct keyaction_s  *next;
} keyaction_t;

typedef struct cmdlist_s
{
    char *command;
    struct cmdlist_s *next;
} cmdlist_t;

typedef struct
{
    cmdlist_t *cmds;
    char *name;
} keycmd_t;

class kexInputAction
{
public:
    void            Init(void);
    void            Clear(void);
    int             FindAction(const char *name);
    void            AddAction(byte id, const char *name);
    void            AddAction(byte id, const kexStr &str);
    void            WriteBindings(FILE *file);
    void            ExecuteCommand(int key, bool up);
    void            ExecuteMouseCommand(int button, bool up);
    int             GetKeyCode(char *key);
    bool            GetName(char *buff, int key);
    void            BindCommand(char key, const char *string);
    void            ListBindings(void);
    void            HandleControl(int ctrl);

    int             GetAsciiKey(char c, bool bShift) { return keycode[bShift][c]; }
    control_t       *Controls(void) { return &control; }

private:
    char            keycode[2][MAX_KEYS];
    control_t       control;
    keycmd_t        keycmds[MAX_KEYS];
    bool            bShiftdown;
    bool            keydown[MAX_KEYS];
    keyaction_t     *keyactions[MAX_HASH];
};

#endif
