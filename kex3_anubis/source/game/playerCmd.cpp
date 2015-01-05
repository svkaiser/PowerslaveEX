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

kexCvar cvarMSensitivityX("cl_msensitivityx", CVF_FLOAT|CVF_CONFIG, "5", "Mouse-X sensitivity");
kexCvar cvarMSensitivityY("cl_msensitivityy", CVF_FLOAT|CVF_CONFIG, "5", "Mouse-Y sensitivity");
kexCvar cvarInvertLook("cl_mlookinvert", CVF_BOOL|CVF_CONFIG, "0", "Invert mouse-look");
kexCvar cvarMSmooth("cl_mousesmooth", CVF_INT|CVF_CONFIG, "4", 1, 4, "Set smooth mouse threshold");

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
    turnx = turny = 0;
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
// kexPlayerCmd::BuildTurning
//

void kexPlayerCmd::BuildTurning(void)
{
    static float history[4][2];
    static int historyCount = 0;
    int smooth;
    
    angles[0] = ((float)turnx * cvarMSensitivityX.GetFloat()) / 1024.0f;
    angles[1] = ((float)turny * cvarMSensitivityY.GetFloat()) / 1024.0f;
    
    history[historyCount & 3][0] = angles[0];
    history[historyCount & 3][1] = angles[1];
    
    smooth = cvarMSmooth.GetInt();
    kexMath::Clamp(smooth, 1, 4);
    cvarMSmooth.Set(smooth);
    
    for(int i = 0; i < cvarMSmooth.GetInt(); ++i)
    {
        angles[0] += (float)history[(historyCount - i + 4) & 3][0];
        angles[1] += (float)history[(historyCount - i + 4) & 3][1];
    }
    
    angles[0] /= cvarMSmooth.GetFloat();
    angles[1] /= cvarMSmooth.GetFloat();
    
    historyCount++;
}

//
// kexPlayerCmd::BuildCommands
//

void kexPlayerCmd::BuildCommands(void)
{
    BuildButtons();
    BuildTurning();
}
