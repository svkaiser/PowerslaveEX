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

//
// kexInput::kexInput
//

kexInput::kexInput()
{
    bEnabled = true;
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
// kexInput::CenterMouse
// Warp the mouse back to the middle of the screen
//

void kexInput::CenterMouse(void)
{
}
