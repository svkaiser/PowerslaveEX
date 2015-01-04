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
//      Player commands
//

#include "kexlib.h"
#include "game.h"
#include "playerCmd.h"

//
// kexPlayerCmd::kexPlayerCmd
//

kexPlayerCmd::kexPlayerCmd(void)
{
    Reset();
}

//
// kexPlayerCmd::~kexPlayerCmd
//

kexPlayerCmd::~kexPlayerCmd(void)
{
}

//
// kexPlayerCmd::Reset
//

void kexPlayerCmd::Reset(void)
{
    buttons = 0;
    angles[0] = angles[1] = 0;
    mousex = mousey = 0;
}

//
// kexPlayerCmd::BuildButtons
//

void kexPlayerCmd::BuildButtons(void)
{
    for(int i = 0; i < NUMINPUTACTIONS; ++i)
    {
        if(kex::cActions->GetAction(i) != 0)
        {
            buttons |= (1 << i);
        }
    }
}

//
// kexPlayerCmd::BuildCommands
//

void kexPlayerCmd::BuildCommands(void)
{
    Reset();
    BuildButtons();
}
