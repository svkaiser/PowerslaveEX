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
//      Input System
//

#include "kexlib.h"

kexCvar kexInput::cvarJoystick_XAxis("in_joystick_xaxis", CVF_INT|CVF_CONFIG, "-1", " ");
kexCvar kexInput::cvarJoystick_XInvert("in_joystick_xinvert", CVF_INT|CVF_CONFIG, "0", " ");
kexCvar kexInput::cvarJoystick_YAxis("in_joystick_yaxis", CVF_INT|CVF_CONFIG, "-1", " ");
kexCvar kexInput::cvarJoystick_YInvert("in_joystick_yinvert", CVF_INT|CVF_CONFIG, "0", " ");
kexCvar kexInput::cvarJoystick_StrafeAxis("in_joystick_strafeaxis", CVF_INT|CVF_CONFIG, "-1", " ");
kexCvar kexInput::cvarJoystick_StrafeInvert("in_joystick_strafeinvert", CVF_INT|CVF_CONFIG, "0", " ");
kexCvar kexInput::cvarJoystick_LookAxis("in_joystick_lookaxis", CVF_INT|CVF_CONFIG, "-1", " ");
kexCvar kexInput::cvarJoystick_LookInvert("in_joystick_lookinvert", CVF_INT|CVF_CONFIG, "0", " ");

//
// kexInput::kexInput
//

kexInput::kexInput()
{
    bEnabled = true;
    bJoystickEnabled = false;
}

//
// kexInput::~kexInput
//

kexInput::~kexInput()
{
}

//
// kexInput::Init
//

void kexInput::Init(void)
{
}

//
// kexInput::Shutdown
//

void kexInput::Shutdown(void)
{
}

//
// kexInput::IsShiftDown
//

bool kexInput::IsShiftDown(int c) const
{
    return(c == KKEY_RSHIFT || c == KKEY_LSHIFT);
}

//
// kexInput::IsCtrlDown
//

bool kexInput::IsCtrlDown(int c) const
{
    return(c == KKEY_RCTRL || c == KKEY_LCTRL);
}

//
// kexInput::IsAltDown
//

bool kexInput::IsAltDown(int c) const
{
    return(c == KKEY_RALT || c == KKEY_LALT);
}

//
// kexInput::PollInput
//

void kexInput::PollInput(void)
{
}

//
// kexInput::TranslateKeyboard
//

int kexInput::TranslateKeyboard(const int val)
{
    return val;
}

//
// kexInput::TranslateMouse
//

int kexInput::TranslateMouse(const int val)
{
    return val;
}

//
// kexInput::ActivateMouse
//

void kexInput::ActivateMouse(void)
{
}

//
// kexInput::DeactivateMouse
//

void kexInput::DeactivateMouse(void)
{
}

//
// kexInput::UpdateGrab
//

void kexInput::UpdateGrab(void)
{
}

//
// kexInput::CapsLockOn
//

bool kexInput::CapslockOn(void)
{
    return false;
}

//
// kexInput::CenterMouse
// Warp the mouse back to the middle of the screen
//

void kexInput::CenterMouse(void)
{
}

//
// kexInput::CloseJoystickDevice
//

void kexInput::CloseJoystickDevice(void)
{
}

//
// kexInput::ActivateJoystickDevice
//

void kexInput::ActivateJoystickDevice(int index)
{
}
