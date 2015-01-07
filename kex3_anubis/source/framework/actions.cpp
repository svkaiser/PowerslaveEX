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
//      Key input handling and binding
//

#include "kexlib.h"

static kexInputAction actions;
kexInputAction *kex::cActions = &actions;

typedef struct
{
    int         code;
    const char  *name;
} keyinfo_t;

static keyinfo_t keynames[] =
{
    { KKEY_RETURN,          "enter" },
    { KKEY_ESCAPE,          "escape" },
    { KKEY_BACKSPACE,       "backspace" },
    { KKEY_TAB,             "tab" },
    { KKEY_SPACE,           "space" },
    { KKEY_0,               "0" },
    { KKEY_1,               "1" },
    { KKEY_2,               "2" },
    { KKEY_3,               "3" },
    { KKEY_4,               "4" },
    { KKEY_5,               "5" },
    { KKEY_6,               "6" },
    { KKEY_7,               "7" },
    { KKEY_8,               "8" },
    { KKEY_9,               "9" },
    { KKEY_SEMICOLON,       ";" },
    { KKEY_LESS,            "<" },
    { KKEY_EQUALS,          "=" },
    { KKEY_GREATER,         ">" },
    { KKEY_QUESTION,        "?" },
    { KKEY_LEFTBRACKET,     "[" },
    { KKEY_BACKSLASH,       "backslash" },
    { KKEY_RIGHTBRACKET,    "]" },
    { KKEY_CARET,           "^" },
    { KKEY_a,               "a" },
    { KKEY_b,               "b" },
    { KKEY_c,               "c" },
    { KKEY_d,               "d" },
    { KKEY_e,               "e" },
    { KKEY_f,               "f" },
    { KKEY_g,               "g" },
    { KKEY_h,               "h" },
    { KKEY_i,               "i" },
    { KKEY_j,               "j" },
    { KKEY_k,               "k" },
    { KKEY_l,               "l" },
    { KKEY_m,               "m" },
    { KKEY_n,               "n" },
    { KKEY_o,               "o" },
    { KKEY_p,               "p" },
    { KKEY_q,               "q" },
    { KKEY_r,               "r" },
    { KKEY_s,               "s" },
    { KKEY_t,               "t" },
    { KKEY_u,               "u" },
    { KKEY_v,               "v" },
    { KKEY_w,               "w" },
    { KKEY_x,               "x" },
    { KKEY_y,               "y" },
    { KKEY_z,               "z" },
    { KKEY_CAPSLOCK,        "caps_lock" },
    { KKEY_F1,              "f1" },
    { KKEY_F2,              "f2" },
    { KKEY_F3,              "f3" },
    { KKEY_F4,              "f4" },
    { KKEY_F5,              "f5" },
    { KKEY_F6,              "f6" },
    { KKEY_F7,              "f7" },
    { KKEY_F8,              "f8" },
    { KKEY_F9,              "f9" },
    { KKEY_F10,             "f10" },
    { KKEY_F11,             "f11" },
    { KKEY_F12,             "f12" },
    { KKEY_PRINTSCREEN,     "prnscreen" },
    { KKEY_SCROLLLOCK,      "scrlock" },
    { KKEY_PAUSE,           "pause" },
    { KKEY_INSERT,          "insert" },
    { KKEY_HOME,            "home" },
    { KKEY_PAGEUP,          "pageup" },
    { KKEY_DELETE,          "delete" },
    { KKEY_END,             "end" },
    { KKEY_PAGEDOWN,        "pagedown" },
    { KKEY_RIGHT,           "right" },
    { KKEY_LEFT,            "left" },
    { KKEY_DOWN,            "down" },
    { KKEY_UP,              "up" },
    { KKEY_NUMLOCKCLEAR,    "numlock" },
    { KKEY_KP_DIVIDE,       "kp_divide" },
    { KKEY_KP_MULTIPLY,     "kp_mul" },
    { KKEY_KP_MINUS,        "kp_minus" },
    { KKEY_KP_PLUS,         "kp_plus" },
    { KKEY_KP_ENTER,        "kp_enter" },
    { KKEY_KP_1,            "kp_1" },
    { KKEY_KP_2,            "kp_2" },
    { KKEY_KP_3,            "kp_3" },
    { KKEY_KP_4,            "kp_4" },
    { KKEY_KP_5,            "kp_5" },
    { KKEY_KP_6,            "kp_6" },
    { KKEY_KP_7,            "kp_7" },
    { KKEY_KP_8,            "kp_8" },
    { KKEY_KP_9,            "kp_9" },
    { KKEY_KP_0,            "kp_0" },
    { KKEY_KP_PERIOD,       "kp_period" },
    { KKEY_LCTRL,           "lctrl" },
    { KKEY_LSHIFT,          "lshift" },
    { KKEY_LALT,            "lalt" },
    { KKEY_RCTRL,           "rctrl" },
    { KKEY_RSHIFT,          "rshift" },
    { KKEY_RALT,            "ralt" },
    { 0,                    NULL }
};

static keyinfo_t mousenames[] =
{
    { KMSB_LEFT+NUMKEYBOARDKEYS,        "mouse_left" },
    { KMSB_MIDDLE+NUMKEYBOARDKEYS,      "mouse_middle" },
    { KMSB_RIGHT+NUMKEYBOARDKEYS,       "mouse_right" },
    { KMSB_MISC1+NUMKEYBOARDKEYS,       "mouse_misc1" },
    { KMSB_MISC2+NUMKEYBOARDKEYS,       "mouse_misc2" },
    { KMSB_MISC3+NUMKEYBOARDKEYS,       "mouse_misc3" },
    { KMSB_MISC4+NUMKEYBOARDKEYS,       "mouse_misc4" },
    { KMSB_MISC5+NUMKEYBOARDKEYS,       "mouse_misc5" },
    { 0,                                NULL }
};

//
// bind
//

COMMAND(bind)
{
    int argc;
    int key;
    int i;
    char cmd[1024];
    keycmd_t *keycmd;
    cmdlist_t *cmdlist;

    argc = kex::cCommands->GetArgc();

    if(argc < 3)
    {
        kex::cSystem->Printf("bind <key> <command>\n");
        return;
    }

    if((key = actions.GetKeyCode(kex::cCommands->GetArgv(1))) <= -1)
    {
        kex::cSystem->Warning("\"%s\" isn't a valid key\n", kex::cCommands->GetArgv(1));
        return;
    }

    cmd[0] = 0;
    for(i = 2; i < argc; i++)
    {
        strcat(cmd, kex::cCommands->GetArgv(i));
        if(i != (argc - 1))
        {
            strcat(cmd, " ");
        }
    }
    
    keycmd = &kex::cActions->KeyCommands()[key];
    
    for(cmdlist = keycmd->cmds; cmdlist; cmdlist = cmdlist->next)
    {
        if(!strcmp(cmdlist->command, cmd))
        {
            kex::cSystem->Warning("\"%s\" is already binded to %s\n",
                                  kex::cCommands->GetArgv(1), cmdlist->command);
            return;
        }
    }

    actions.BindCommand(key, cmd);
}

//
// unbind
//

COMMAND(unbind)
{
}

//
// listbinds
//

COMMAND(listbinds)
{
    actions.ListBindings();
}

//
// kexInputAction::GetKeyCode
//

int kexInputAction::GetKeyCode(char *key)
{
    keyinfo_t *pkey;
    kexStr tmp(key);

    tmp.ToLower();

    for(pkey = keynames; pkey->name; pkey++)
    {
        if(!strcmp(key, pkey->name))
        {
            return pkey->code;
        }
    }

    for(pkey = mousenames; pkey->name; pkey++)
    {
        if(!strcmp(key, pkey->name))
        {
            return pkey->code;
        }
    }

    return -1;
}

//
// kexInputAction::GetKeyName
//

char *kexInputAction::GetKeyName(int key)
{
    keyinfo_t *pkey;

    for(pkey = keynames; pkey->name; pkey++)
    {
        if(key == pkey->code)
        {
            return (char*)pkey->name;
        }
    }

    for(pkey = mousenames; pkey->name; pkey++)
    {
        if(key == pkey->code)
        {
            return (char*)pkey->name;
        }
    }

    return NULL;
}

//
// kexInputAction::BindCommand
//

void kexInputAction::BindCommand(int key, const char *string)
{
    keycmd_t *keycmd;
    cmdlist_t *newcmd;
    int actionid;

    keycmd = &keycmds[key];
    newcmd = (cmdlist_t*)Mem_Malloc(sizeof(cmdlist_t), hb_static);
    newcmd->command = Mem_Strdup(string, hb_static);
    newcmd->next = keycmd->cmds;
    newcmd->action = NULL;

    if((actionid = FindAction(string)) != -1)
    {
        newcmd->action = &keyActions[actionid];
    }

    keycmd->cmds = newcmd;
}

//
// kexInputAction::ListBindings
//

void kexInputAction::ListBindings(void)
{
    keycmd_t *keycmd;
    cmdlist_t *cmd;
    char *tmp;

    kex::cSystem->Printf("\n");

    for(int i = 0; i < MAX_KEYS; i++)
    {
        keycmd = &keycmds[i];

        if((tmp = GetKeyName(i)) == NULL)
        {
            continue;
        }

        for(cmd = keycmd->cmds; cmd; cmd = cmd->next)
        {
            kex::cSystem->CPrintf(COLOR_GREEN, "%s : \"%s\"\n", tmp, cmd->command);
        }
    }
}

//
// kexInputAction::FindAction
//

int kexInputAction::FindAction(const char *name)
{
    for(unsigned int i = 0; i < keyActions.Length(); ++i)
    {
        if(!strcmp(name, keyActions[i].name))
        {
            return keyActions[i].keyid;
        }
    }

    return -1;
}

//
// kexInputAction::ActionExists
//

bool kexInputAction::ActionExists(const char *name)
{
    for(unsigned int i = 0; i < keyActions.Length(); ++i)
    {
        if(!strcmp(name, keyActions[i].name))
        {
            return true;
        }
    }

    return false;
}

//
// kexInputAction::ExecuteCommand
//

void kexInputAction::ExecuteCommand(int key, bool keyup, const int eventType)
{
    keycmd_t *keycmd;
    cmdlist_t *cmd;

    if(eventType == ev_mousedown || eventType == ev_mouseup)
    {
        key += NUMKEYBOARDKEYS;
    }

    keycmd = &keycmds[key];

    for(cmd = keycmd->cmds; cmd; cmd = cmd->next)
    {
        if(cmd->action != NULL)
        {
            if(cmd->command[0] == '+' || cmd->command[0] == '-')
            {
                if(keyup && cmd->command[0] == '+')
                {
                    cmd->command[0] = '-';
                }

                heldActions[cmd->action->keyid] = (cmd->command[0] != '-');

                if(keyup && cmd->command[0] == '-')
                {
                    cmd->command[0] = '+';
                }
            }
            else
            {
                heldActions[cmd->action->keyid] = !keyup;
            }
        }
        else if(!keyup)
        {
            kex::cCommands->Execute(cmd->command);
        }
    }
}

//
// kexInputAction::AddAction
//

void kexInputAction::AddAction(byte id, const char *name)
{
    keyaction_t keyaction;

    keyaction.keyid = id;
    keyaction.name = name;

    keyActions.Push(keyaction);
    heldActions.Push(0);
}

//
// kexInputAction::WriteBindings
//

void kexInputAction::WriteBindings(FILE *file)
{
    keycmd_t *keycmd;
    cmdlist_t *cmd;
    char *tmp;

    for(int i = 0; i < MAX_KEYS; i++)
    {
        keycmd = &keycmds[i];

        if((tmp = GetKeyName(i)) == NULL)
        {
            continue;
        }

        for(cmd = keycmd->cmds; cmd; cmd = cmd->next)
        {
            fprintf(file, "bind %s \"%s\"\n", tmp,
                (cmd->action == NULL) ? cmd->command : cmd->action->name.c_str());
        }
    }
}
