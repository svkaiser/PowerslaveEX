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

#include "keyboard.h"
#include "mouse.h"

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
} eventType_t;

// Event structure.
typedef struct
{
    int         type;
    int         data1;  // keys / mouse/joystick buttons
    int         data2;  // mouse/joystick x move
    int         data3;  // mouse/joystick y move
    int         data4;  // misc data
} inputEvent_t;

class kexInput
{
public:
    kexInput();
    ~kexInput();

    virtual bool    IsShiftDown(int c) const;
    virtual bool    IsCtrlDown(int c) const;
    virtual bool    IsAltDown(int c) const;
    virtual void    PollInput(void);
    virtual void    UpdateGrab(void);
    virtual void    CenterMouse(void);
    virtual void    Init(void);
    virtual void    ActivateMouse(void);
    virtual void    DeactivateMouse(void);
    virtual int     TranslateKeyboard(const int val);
    virtual int     TranslateMouse(const int val);
    virtual bool    CapslockOn(void);

    const bool      MouseGrabbed(void) const { return bGrabMouse; }
    void            SetEnabled(bool enable) { bEnabled = enable; }
    void            ToggleMouseGrab(bool enable) { bGrabMouse = enable; }
    const int       MouseX(void) const { return mouse_x; }
    const int       MouseY(void) const { return mouse_y; }

protected:
    bool            bEnabled;
    bool            bGrabMouse;
    int             mouse_x;
    int             mouse_y;
};

#endif
