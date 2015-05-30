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

kexCvar kexPlayerCmd::cvarMSensitivityX("cl_msensitivityx", CVF_FLOAT|CVF_CONFIG, "5", 1, 10, "Mouse-X sensitivity");
kexCvar kexPlayerCmd::cvarMSensitivityY("cl_msensitivityy", CVF_FLOAT|CVF_CONFIG, "5", 1, 10, "Mouse-Y sensitivity");
kexCvar kexPlayerCmd::cvarInvertLook("cl_mlookinvert", CVF_BOOL|CVF_CONFIG, "0", "Invert mouse-look");
kexCvar kexPlayerCmd::cvarMSmooth("cl_mousesmooth", CVF_INT|CVF_CONFIG, "4", 1, 4, "Set smooth mouse threshold");
kexCvar kexPlayerCmd::cvarJoyStickLookSensitivityX("cl_joylooksensitivity_x", CVF_FLOAT|CVF_CONFIG, "0.5", 0.1f, 1, "Joystick Look sensitivity (x-axis)");
kexCvar kexPlayerCmd::cvarJoyStickLookSensitivityY("cl_joylooksensitivity_y", CVF_FLOAT|CVF_CONFIG, "0.25", 0.1f, 1, "Joystick Look sensitivity (y-axis)");
kexCvar kexPlayerCmd::cvarJoyStickMoveSensitivity("cl_joymovesensitivity", CVF_FLOAT|CVF_CONFIG, "1", 0.1f, 5, "Joystick Move sensitivity");
kexCvar kexPlayerCmd::cvarJoyStickThreshold("cl_joystickthreshold", CVF_FLOAT|CVF_CONFIG, "4000", 500, 10000, " ");

//
// kexPlayerCmd::kexPlayerCmd
//

kexPlayerCmd::kexPlayerCmd(void)
{
    Reset();
    joyturnthreshold = 0;
    joylookthreshold = 0;

    memset(buttonHeldTime, 0, sizeof(uint) * NUMINPUTACTIONS);
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
    movement[0] = movement[1] = 0;
    turnx = turny = 0;
    joyturn[0] = joyturn[1] = 0;
    joymove[0] = joymove[1] = 0;
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
            buttonHeldTime[i]++;
        }
        else
        {
            buttonHeldTime[i] = 0;
        }
    }
}

//
// kexPlayerCmd::ButtonHeldTime
//

uint kexPlayerCmd::ButtonHeldTime(const int btn)
{
    return buttonHeldTime[btn];
}

//
// kexPlayerCmd::BuildTurning
//

void kexPlayerCmd::BuildTurning(void)
{
    static float history[4][2];
    static float turnLerp[2];
    static int historyCount = 0;
    int smooth;
    
    if(buttons & BC_LEFT)
    {
        turnLerp[0] = (0.078125f - turnLerp[0]) * 0.05f + turnLerp[0];
    }
    else
    {
        turnLerp[0] = (0 - turnLerp[0]) * 0.15f + turnLerp[0];

        if(kexMath::Fabs(turnLerp[0]) <= 0.005f)
        {
            turnLerp[0] = 0;
        }
    }

    if(buttons & BC_RIGHT)
    {
        turnLerp[1] = (0.078125f - turnLerp[1]) * 0.05f + turnLerp[1];
    }
    else
    {
        turnLerp[1] = (0 - turnLerp[1]) * 0.15f + turnLerp[1];

        if(kexMath::Fabs(turnLerp[1]) <= 0.005f)
        {
            turnLerp[1] = 0;
        }
    }

    angles[0] = ((float)turnx * cvarMSensitivityX.GetFloat()) / 2048.0f;
    angles[1] = ((float)turny * cvarMSensitivityY.GetFloat()) / 2048.0f;

    if(cvarInvertLook.GetBool())
    {
        angles[1] = -angles[1];
    }
    
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

    angles[0] -= turnLerp[0];
    angles[0] += turnLerp[1];
    
    historyCount++;
}

//
// kexPlayerCmd::BuildJoy
//
// And there was rejoice...
//
 
void kexPlayerCmd::BuildJoy(void)
{
    float m1 = (float)joymove[0];
    float m2 = (float)joymove[1];

    if(joyturn[0] == 0) joyturnthreshold = 0;
    if(joyturn[1] == 0) joylookthreshold = 0;

    angles[0] += ((float)joyturn[0] * joyturnthreshold) / 2048.0f;
    angles[1] += ((float)joyturn[1] * joylookthreshold) / 2048.0f;

    if(m1 >= 1 || m1 <= -1)
    {
        movement[0] = (m1 * cvarJoyStickMoveSensitivity.GetFloat()) / 32768.0f;
        kexMath::Clamp(movement[0], -1, 1);
    }

    if(m2 >= 1 || m2 <= -1)
    {
        movement[1] = (m2 * cvarJoyStickMoveSensitivity.GetFloat()) / 32768.0f;
        kexMath::Clamp(movement[1], -1, 1);
    }
}

//
// kexPlayerCmd::SetJoy
//

void kexPlayerCmd::SetJoy(inputEvent_t *ev)
{
    joyturn[0] = ev->data2;
    joyturn[1] = ev->data1;
    joymove[1] = ev->data4;
    joymove[0] = ev->data3;

    SetJoyTurnThreshold(ev->data2, ev->data1);
}

//
// kexPlayerCmd::SetJoyTurnThreshold
//

void kexPlayerCmd::SetJoyTurnThreshold(const int turn, const int look)
{
    float turnspeed = 1.0f / cvarJoyStickThreshold.GetFloat();
    float x = cvarJoyStickLookSensitivityX.GetFloat() / 128.0f;
    float y = cvarJoyStickLookSensitivityY.GetFloat() / 128.0f;

    if(turn == 0)
    {
        joyturnthreshold = 0;
    }
    else
    {
        joyturnthreshold += turnspeed;

        if(joyturnthreshold >= x)
        {
            joyturnthreshold = x;
        }
    }

    if(look == 0)
    {
        joylookthreshold = 0;
    }
    else
    {
        joylookthreshold += turnspeed;

        if(joylookthreshold >= y)
        {
            joylookthreshold = y;
        }
    }
}

//
// kexPlayerCmd::BuildCommands
//

void kexPlayerCmd::BuildCommands(void)
{
    BuildButtons();
    BuildTurning();
    BuildJoy();
}
