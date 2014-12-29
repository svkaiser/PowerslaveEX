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

#ifndef __INPUT_H__
#define __INPUT_H__

#include "SDL.h"

// Input event types.
typedef enum
{
    ev_keydown,
    ev_keyup,
    ev_mouse,
    ev_mousedown,
    ev_mouseup,
    ev_mousewheel,
    ev_gamepad
} evtype_t;

// Event structure.
typedef struct
{
    int         type;
    int         data1;  // keys / mouse/joystick buttons
    int         data2;  // mouse/joystick x move
    int         data3;  // mouse/joystick y move
    int         data4;  // misc data
} event_t;

#define SDL_BUTTON_WHEELUP SDL_BUTTON_X1
#define SDL_BUTTON_WHEELDOWN SDL_BUTTON_X2

class kexInput
{
public:
    kexInput();
    ~kexInput();

    bool            IsShiftDown(int c) const;
    bool            IsCtrlDown(int c) const;
    bool            IsAltDown(int c) const;
    void            PollInput(void);
    void            MoveMouse(int x, int y);
    void            UpdateGrab(void);
    void            CenterMouse(void);
    void            Init(void);
    void            ActivateMouse(void);
    void            DeactivateMouse(void);
    float           AccelerateMouse(int val) const;

    void            SetEnabled(bool enable) { bEnabled = enable; }

private:
    void            ReadMouse(void);
    bool            MouseShouldBeGrabbed(void) const;
    void            GetEvent(const SDL_Event *Event);
    void            UpdateFocus(void);
    int             GetButtonState(Uint8 buttonstate) const;

    bool            bWindowFocused;
    bool            bGrabbed;
    bool            bEnabled;
    int             mouse_x;
    int             mouse_y;
    Uint8           lastmbtn;
};

#endif
