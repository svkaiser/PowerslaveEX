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

#include <math.h>

#include "SDL.h"
#include "kexlib.h"

static kexInput inputSystem;
kexInput *kex::cInput = &inputSystem;

kexCvar cvarMSensitivityX("cl_msensitivityx", CVF_FLOAT|CVF_CONFIG, "5", "Mouse-X sensitivity");
kexCvar cvarMSensitivityY("cl_msensitivityy", CVF_FLOAT|CVF_CONFIG, "5", "Mouse-Y sensitivity");
kexCvar cvarMAcceleration("cl_macceleration", CVF_FLOAT|CVF_CONFIG, "0", "Mouse acceleration");
kexCvar cvarInvertLook("cl_mlookinvert", CVF_BOOL|CVF_CONFIG, "0", "Invert mouse-look");

//
// kexInput::kexInput
//

kexInput::kexInput()
{
    lastmbtn        = 0;
    bGrabbed        = false;
    bWindowFocused  = false;
    bEnabled        = true;
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
    SDL_PumpEvents();
    CenterMouse();

    kex::cSystem->Printf("Input Initialized\n");
}

//
// kexInput::ReadMouse
//

void kexInput::ReadMouse(void)
{
    int x, y;
    Uint8 btn;
    event_t ev;

    if(kex::cSystem->IsWindowed() /*&& console.IsActive()*/)
    {
        return;
    }

    SDL_GetRelativeMouseState(&x, &y);
    btn = SDL_GetMouseState(&mouse_x, &mouse_y);

    if(x != 0 || y != 0 || btn || (lastmbtn != btn))
    {
        ev.type = ev_mouse;
        ev.data1 = GetButtonState(btn);
        ev.data2 = x << 5;
        ev.data3 = (-y) << 5;
        ev.data4 = 0;
        //client.PostEvent(&ev);
    }

    lastmbtn = btn;

    if(MouseShouldBeGrabbed())
    {
        CenterMouse();
    }
}

//
// kexInput::GetButtonState
//

int kexInput::GetButtonState(Uint8 buttonstate) const
{
    return 0
           | (buttonstate & SDL_BUTTON(SDL_BUTTON_LEFT)      ? 1 : 0)
           | (buttonstate & SDL_BUTTON(SDL_BUTTON_MIDDLE)    ? 2 : 0)
           | (buttonstate & SDL_BUTTON(SDL_BUTTON_RIGHT)     ? 4 : 0);
}

//
// kexInput::UpdateFocus
//

void kexInput::UpdateFocus(void)
{
    Uint32 flags;
    flags = kex::cSystem->GetWindowFlags();

    // We should have input (keyboard) focus and be visible
    // (not minimised)
    bWindowFocused = (flags & SDL_WINDOW_INPUT_FOCUS) || (flags & SDL_WINDOW_MOUSE_FOCUS);
}

//
// kexInput::IsShiftDown
//

bool kexInput::IsShiftDown(int c) const
{
    return(c == SDLK_RSHIFT || c == SDLK_LSHIFT);
}

//
// kexInput::IsCtrlDown
//

bool kexInput::IsCtrlDown(int c) const
{
    return(c == SDLK_RCTRL || c == SDLK_LCTRL);
}

//
// kexInput::IsAltDown
//

bool kexInput::IsAltDown(int c) const
{
    return(c == SDLK_RALT || c == SDLK_LALT);
}

//
// kexInput::GetEvent
//

void kexInput::GetEvent(const SDL_Event *Event)
{
    event_t event;

    switch(Event->type)
    {
    case SDL_KEYDOWN:
        if(Event->key.repeat)
        {
            break;
        }
        event.type = ev_keydown;
        event.data1 = Event->key.keysym.sym;
        //client.PostEvent(&event);
        break;

    case SDL_KEYUP:
        event.type = ev_keyup;
        event.data1 = Event->key.keysym.sym;
        //client.PostEvent(&event);
        break;

    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
        if(!bWindowFocused)
        {
            break;
        }
        event.type = Event->type ==
                     SDL_MOUSEBUTTONUP ? ev_mouseup : ev_mousedown;
        event.data1 = Event->button.button;
        event.data2 = event.data3 = 0;
        //client.PostEvent(&event);
        break;

    case SDL_MOUSEWHEEL:
        if(!bWindowFocused)
        {
            break;
        }
        event.type = ev_mousewheel;
        event.data1 = Event->wheel.y > 0 ? SDL_BUTTON_WHEELUP : SDL_BUTTON_WHEELDOWN;
        event.data2 = event.data3 = 0;
        //client.PostEvent(&event);
        break;

    case SDL_WINDOWEVENT:
        switch(Event->window.event)
        {
        case SDL_WINDOWEVENT_FOCUS_GAINED:
        case SDL_WINDOWEVENT_ENTER:
            UpdateFocus();
            break;
        case SDL_WINDOWEVENT_FOCUS_LOST:
            bWindowFocused = false;
            bGrabbed = false;
            break;
        }
        break;

    case SDL_QUIT:
        kex::cSystem->Shutdown();
        break;

    default:
        break;
    }
}

//
// kexInput::PollInput
//

void kexInput::PollInput(void)
{
    SDL_Event Event;

    while(SDL_PollEvent(&Event))
    {
        GetEvent(&Event);
    }

    ReadMouse();
}

//
// kexInput::MouseShouldBeGrabbed
//

bool kexInput::MouseShouldBeGrabbed(void) const
{
    // if the window doesnt have focus, never grab it
    if(!bWindowFocused)
    {
        return false;
    }

    if(kex::cSystem->IsWindowed())
    {
        //if(console.IsActive())
        //{
        //    return false;
        //}
    }

    return bEnabled;
}

//
// kexInput::AccelerateMouse
//

float kexInput::AccelerateMouse(int val) const
{
    if(!cvarMAcceleration.GetFloat())
    {
        return (float)val;
    }

    if(val < 0)
    {
        return -AccelerateMouse(-val);
    }

    return kexMath::Pow((float)val, (cvarMAcceleration.GetFloat() / 200.0f + 1.0f));
}

//
// kexInput::MoveMouse
//

void kexInput::MoveMouse(int x, int y)
{
    control_t *ctrl;

    ctrl = kex::cActions->Controls();

    ctrl->mousex += ((AccelerateMouse(x) * cvarMSensitivityX.GetFloat()) / 128.0f);
    ctrl->mousey += ((AccelerateMouse(y) * cvarMSensitivityY.GetFloat()) / 128.0f);
}

//
// kexInput::ActivateMouse
//

void kexInput::ActivateMouse(void)
{
    SDL_ShowCursor(0);
    SDL_SetRelativeMouseMode(SDL_TRUE);
    kex::cSystem->SetWindowGrab(SDL_TRUE);
}

//
// kexInput::DeactivateMouse
//

void kexInput::DeactivateMouse(void)
{
    SDL_ShowCursor(1);
    SDL_SetRelativeMouseMode(SDL_FALSE);
    kex::cSystem->SetWindowGrab(SDL_FALSE);
}

//
// kexInput::UpdateGrab
//

void kexInput::UpdateGrab(void)
{
    bool grab;

    grab = MouseShouldBeGrabbed();
    if(grab && !bGrabbed)
    {
        ActivateMouse();
    }

    if(!grab && bGrabbed)
    {
        DeactivateMouse();
    }

    bGrabbed = grab;
}

//
// kexInput::CenterMouse
// Warp the mouse back to the middle of the screen
//

void kexInput::CenterMouse(void)
{
    // Warp the the screen center
    kex::cSystem->WarpMouseToCenter();

    // Clear any relative movement caused by warping
    SDL_PumpEvents();
    SDL_GetRelativeMouseState(NULL, NULL);
}
