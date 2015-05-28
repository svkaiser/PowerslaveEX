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
//      Input System (SDL)
//

#include <math.h>

#include "SDL.h"
#include "kexlib.h"

#define DEAD_ZONE (32768 / 3)
#define NUM_VIRTUAL_BUTTONS 15
#define NUM_JOY_AXIS 6

// If this bit is set in a configuration file axis value, the axis is
// not actually a joystick axis, but instead is a "button axis". This
// means that instead of reading an SDL joystick axis, we read the
// state of two buttons to get the axis value. This is needed for eg.
// the PS3 SIXAXIS controller, where the D-pad buttons register as
// buttons, not as two axes.
#define BUTTON_AXIS 0x10000

// Query whether a given axis value describes a button axis.
#define IS_BUTTON_AXIS(axis) ((axis) >= 0 && ((axis) & BUTTON_AXIS) != 0)

// Get the individual buttons from a button axis value.
#define BUTTON_AXIS_NEG(axis)  ((axis) & 0xff)
#define BUTTON_AXIS_POS(axis)  (((axis) >> 8) & 0xff)

// Create a button axis value from two button values.
#define CREATE_BUTTON_AXIS(neg, pos) (BUTTON_AXIS | (neg) | ((pos) << 8))

// If this bit is set in an axis value, the axis is not actually a
// joystick axis, but is a "hat" axis. This means that we read (one of)
// the hats on the joystick.
#define HAT_AXIS    0x20000

#define IS_HAT_AXIS(axis) ((axis) >= 0 && ((axis) & HAT_AXIS) != 0)

// Get the hat number from a hat axis value.
#define HAT_AXIS_HAT(axis)         ((axis) & 0xff)
// Which axis of the hat? (horizonal or vertical)
#define HAT_AXIS_DIRECTION(axis)   (((axis) >> 8) & 0xff)

#define CREATE_HAT_AXIS(hat, direction) \
    (HAT_AXIS | (hat) | ((direction) << 8))

#define HAT_AXIS_HORIZONTAL 1
#define HAT_AXIS_VERTICAL   2

class kexInputSDL : public kexInput
{
public:
    kexInputSDL();
    ~kexInputSDL();
    
    virtual void        PollInput(void);
    virtual void        UpdateGrab(void);
    virtual void        CenterMouse(void);
    virtual void        Init(void);
    virtual void        ActivateMouse(void);
    virtual void        DeactivateMouse(void);
    virtual int         TranslateKeyboard(const int val);
    virtual int         TranslateMouse(const int val);
    virtual bool        CapslockOn(void);
    virtual bool        IsShiftDown(int c) const;
    virtual bool        IsCtrlDown(int c) const;
    virtual bool        IsAltDown(int c) const;
    virtual void        Shutdown(void);
    virtual void        ActivateJoystickDevice(int index);
    
private:
    virtual void        CloseJoystickDevice(void);

    static kexCvar      cvarProfileJoyButtons;

    void                CalibrateJoystickAxis(void);
    const bool          IsJoystickAxisButton(int physbutton);
    const int           GetJoystickAxis(const int index);
    const int           GetJoystickAxisState(int axis, int invert);
    const int           GetJoystickButtonsState(void);
    const int           ReadJoystickButtonState(int vbutton);
    void                JoystickButtonEvent(void);
    void                ReadMouse(void);
    void                ReadJoystick(void);
    bool                MouseShouldBeGrabbed(void) const;
    void                GetEvent(const SDL_Event *Event);
    void                UpdateFocus(void);

    static int          joystick_physical_buttons[NUM_VIRTUAL_BUTTONS];
    
    bool                bWindowFocused;
    bool                bGrabbed;
    Uint8               lastmbtn;
    SDL_Joystick        *joystick;
    int                 joystick_index;
    int                 joystick_axisoffset[NUM_JOY_AXIS];
    int                 joystick_oldbuttons;
};

static kexInputSDL inputSystem;
kexInput *kex::cInput = &inputSystem;

// Virtual to physical button joystick button mapping. By default this
// is a straight mapping.
int kexInputSDL::joystick_physical_buttons[NUM_VIRTUAL_BUTTONS] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14
};

kexCvar kexInputSDL::cvarProfileJoyButtons("in_profilejoybuttons", CVF_BOOL, "0",
                                           "Display joystick button ID in console when pressed");

//
// kexInputSDL::kexInputSDL
//

kexInputSDL::kexInputSDL()
{
    lastmbtn                = 0;
    bGrabbed                = false;
    bWindowFocused          = false;
    bEnabled                = true;
    bGrabMouse              = false;
    joystick_oldbuttons     =  0;
}

//
// kexInputSDL::~kexInputSDL
//

kexInputSDL::~kexInputSDL()
{
}

//
// kexInputSDL::Init
//

void kexInputSDL::Init(void)
{
    SDL_PumpEvents();
    CenterMouse();

    bJoystickEnabled = (SDL_Init(SDL_INIT_JOYSTICK|SDL_INIT_GAMECONTROLLER) >= 0);

    if(!bJoystickEnabled)
    {
        kex::cSystem->Warning("Joystick not supported on this machine\n");
    }
    else
    {
        // auto open first device if one is available and nothing is configured
        if(joystick_index == -1 && SDL_NumJoysticks() >= 1)
        {
            joystick_index = 0;
        }

        ActivateJoystickDevice(joystick_index);
    }

    kex::cSystem->Printf("Input Initialized\n");
}

//
// kexInputSDL::Shutdown
//

void kexInputSDL::Shutdown(void)
{
    CloseJoystickDevice();

    if(bJoystickEnabled)
    {
        SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
    }
}

//
// kexInputSDL::CloseJoystickDevice
//

void kexInputSDL::CloseJoystickDevice(void)
{
    if(joystick != NULL)
    {
        SDL_JoystickClose(joystick);
        joystick = NULL;
    }
}

//
// kexInputSDL::CalibrateJoystickAxis
//

void kexInputSDL::CalibrateJoystickAxis(void)
{
    int numaxis;
    int i;

    if(!joystick)
    {
        return;
    }

    numaxis = MIN(SDL_JoystickNumAxes(joystick), NUM_JOY_AXIS);

    for(i = 0; i < numaxis; ++i)
    {
        joystick_axisoffset[i] = SDL_JoystickGetAxis(joystick, i);
    }
}

//
// kexInputSDL::ActivateJoystickDevice
//

void kexInputSDL::ActivateJoystickDevice(int index)
{
    // close any currently open device
    CloseJoystickDevice();

    // set new index
    joystick_index = index;

    // validate
    if(joystick_index < 0 || joystick_index >= SDL_NumJoysticks())
    {
        return;
    }

    // open the device
    if(!(joystick = SDL_JoystickOpen(joystick_index)))
    {
        return;
    }

    // allow event polling
    SDL_JoystickEventState(SDL_ENABLE);
    CalibrateJoystickAxis();
}

//
// kexInputSDL::ReadMouse
//

void kexInputSDL::ReadMouse(void)
{
    int x, y;
    Uint8 btn;
    inputEvent_t ev;

    btn = SDL_GetMouseState(&mouse_x, &mouse_y);
    
    if(!bGrabMouse)
    {
        return;
    }

    SDL_GetRelativeMouseState(&x, &y);

    if(x != 0 || y != 0 || btn || (lastmbtn != btn))
    {
        ev.type = ev_mouse;
        ev.data1 = x;
        ev.data2 = y;
        ev.data3 = mouse_x;
        ev.data4 = mouse_y;

        kex::cSession->EventQueue().Push(&ev);
    }

    lastmbtn = btn;

    if(MouseShouldBeGrabbed())
    {
        CenterMouse();
    }
}

//
// kexInputSDL::GetJoystickAxis
//

const int kexInputSDL::GetJoystickAxis(const int index)
{
    if(index < 0 || index >= NUM_JOY_AXIS)
    {
        return 0;
    }

    return MAX(MIN(SDL_JoystickGetAxis(joystick, index) -
        joystick_axisoffset[index], 32767), -32768);
}

//
// kexInputSDL::IsJoystickAxisButton
//

const bool kexInputSDL::IsJoystickAxisButton(int physbutton)
{
    if(IS_BUTTON_AXIS(cvarJoystick_XAxis.GetInt()))
    {
        if (physbutton == BUTTON_AXIS_NEG(cvarJoystick_XAxis.GetInt())
         || physbutton == BUTTON_AXIS_POS(cvarJoystick_XAxis.GetInt()))
        {
            return true;
        }
    }
    if(IS_BUTTON_AXIS(cvarJoystick_YAxis.GetInt()))
    {
        if (physbutton == BUTTON_AXIS_NEG(cvarJoystick_YAxis.GetInt())
         || physbutton == BUTTON_AXIS_POS(cvarJoystick_YAxis.GetInt()))
        {
            return true;
        }
    }
    if(IS_BUTTON_AXIS(cvarJoystick_StrafeAxis.GetInt()))
    {
        if (physbutton == BUTTON_AXIS_NEG(cvarJoystick_StrafeAxis.GetInt())
         || physbutton == BUTTON_AXIS_POS(cvarJoystick_StrafeAxis.GetInt()))
        {
            return true;
        }
    }
    if(IS_BUTTON_AXIS(cvarJoystick_LookAxis.GetInt()))
    {
        if (physbutton == BUTTON_AXIS_NEG(cvarJoystick_LookAxis.GetInt())
         || physbutton == BUTTON_AXIS_POS(cvarJoystick_LookAxis.GetInt()))
        {
            return true;
        }
    }

    return false;
}

//
// kexInputSDL::ReadJoystickButtonState
//
// Get the state of the given virtual button.
//

const int kexInputSDL::ReadJoystickButtonState(int vbutton)
{
    int physbutton;

    if(!joystick)
    {
        return 0;
    }

    // Map from virtual button to physical (SDL) button.
    physbutton = joystick_physical_buttons[vbutton];

    // Never read axis buttons as buttons.
    if(IsJoystickAxisButton(physbutton))
    {
        return 0;
    }

    if(physbutton < 0 || physbutton >= SDL_JoystickNumButtons(joystick))
    {
        return 0;
    }

    return SDL_JoystickGetButton(joystick, physbutton);
}

//
// kexInputSDL::GetJoystickAxisState
//

const int kexInputSDL::GetJoystickAxisState(int axis, int invert)
{
    int result;

    // Axis -1 means disabled.

    if(axis < 0)
    {
        return 0;
    }

    // Is this a button axis, or a hat axis?
    // If so, we need to handle it specially.

    result = 0;

    if(IS_BUTTON_AXIS(axis))
    {
        int button = BUTTON_AXIS_NEG(axis);

        if(button >= 0 && button < SDL_JoystickNumButtons(joystick) &&
            SDL_JoystickGetButton(joystick, button))
        {
            result -= 32767;
        }

        button = BUTTON_AXIS_POS(axis);

        if(button >= 0 && button < SDL_JoystickNumButtons(joystick) &&
            SDL_JoystickGetButton(joystick, button))
        {
            result += 32767;
        }
    }
    else if(IS_HAT_AXIS(axis))
    {
        int direction = HAT_AXIS_DIRECTION(axis);
        int hataxis   = HAT_AXIS_HAT(axis);

        if(hataxis >= 0 && hataxis < SDL_JoystickNumHats(joystick))
        {
            int hatval = SDL_JoystickGetHat(joystick, hataxis);

            if(direction == HAT_AXIS_HORIZONTAL)
            {
                if((hatval & SDL_HAT_LEFT) != 0)
                {
                    result -= 32767;
                }
                else if((hatval & SDL_HAT_RIGHT) != 0)
                {
                    result += 32767;
                }
            }
            else if(direction == HAT_AXIS_VERTICAL)
            {
                if ((hatval & SDL_HAT_UP) != 0)
                {
                    result -= 32767;
                }
                else if((hatval & SDL_HAT_DOWN) != 0)
                {
                    result += 32767;
                }
            }
        }
    }
    else if(axis >= 0 && axis < SDL_JoystickNumAxes(joystick))
    {
        result = GetJoystickAxis(axis);

        if (result < DEAD_ZONE && result > -DEAD_ZONE)
        {
            result = 0;
        }
    }

    if(invert)
    {
        result = -result;
    }

    return result;
}

//
// kexInputSDL::GetJoystickButtonsState
//
// Get a bitmask of all currently-pressed buttons
//

const int kexInputSDL::GetJoystickButtonsState(void)
{
    int i;
    int axis;
    int result;

    result = 0;

    for(i = 0; i < NUM_VIRTUAL_BUTTONS; ++i)
    {
        if(ReadJoystickButtonState(i))
        {
            result |= 1 << i;
        }
    }

    // I am going to assume, at most, that the most number of axis a joystick can have is 5
    for(i = 0; i < NUM_JOY_AXIS; ++i)
    {
        if(i >= SDL_JoystickNumAxes(joystick))
        {
            break;
        }

        axis = GetJoystickAxis(i);

        if(axis > DEAD_ZONE || axis < -DEAD_ZONE)
        {
            if(axis > 0)
            {
                result |= 1 << (NUM_VIRTUAL_BUTTONS + i);
            }
            else
            {
                result |= 1 << (NUM_VIRTUAL_BUTTONS + NUM_JOY_AXIS + i);
            }
        }
    }

    if(SDL_JoystickNumHats(joystick) <= 0)
    {
        return result;
    }

    axis = SDL_JoystickGetHat(joystick, 0);

    if(axis != 0)
    {
        int j;

        for(j = 0; j < 4; ++j)
        {
            if(axis & (1 << j))
            {
                result |= 1 << (NUM_VIRTUAL_BUTTONS + (NUM_JOY_AXIS*2) + j);
            }
        }
    }

    return result;
}

//
// kexInputSDL::JoystickButtonEvent
//

void kexInputSDL::JoystickButtonEvent(void)
{
    int bits;
    int data;
    int i;
    inputEvent_t ev;

    data = GetJoystickButtonsState();

    if(data)
    {
        bits = data;

        // check for button press
        bits &= ~joystick_oldbuttons;
        for(i = 0; i < NUM_VIRTUAL_BUTTONS + 16; ++i)
        {
            if(bits & (1 << i))
            {
                if(cvarProfileJoyButtons.GetBool())
                {
                    kex::cSystem->DPrintf("%i\n", i);
                }

                ev.type = ev_joybtndown;
                ev.data1 = i;
                kex::cSession->EventQueue().Push(&ev);
            }
        }
    }

    bits = joystick_oldbuttons;
    joystick_oldbuttons = data;

    // check for button release
    bits &= ~joystick_oldbuttons;
    for(i = 0; i < NUM_VIRTUAL_BUTTONS + 16; ++i)
    {
        if(bits & (1 << i))
        {
            ev.type = ev_joybtnup;
            ev.data1 = i;
            kex::cSession->EventQueue().Push(&ev);
        }
    }
}

//
// kexInputSDL::ReadJoystick
//

void kexInputSDL::ReadJoystick(void)
{
    if(joystick == NULL)
    {
        return;
    }

    int x = GetJoystickAxisState(cvarJoystick_XAxis.GetInt(), cvarJoystick_XInvert.GetInt());
    int y = GetJoystickAxisState(cvarJoystick_YAxis.GetInt(), cvarJoystick_YInvert.GetInt());
    int s = GetJoystickAxisState(cvarJoystick_StrafeAxis.GetInt(), cvarJoystick_StrafeInvert.GetInt());
    int l = GetJoystickAxisState(cvarJoystick_LookAxis.GetInt(), cvarJoystick_LookInvert.GetInt());

    if(x || y || s || l)
    {
        inputEvent_t ev;

        ev.type = ev_joystick;
        ev.data1 = l;
        ev.data2 = x;
        ev.data3 = y;
        ev.data4 = s;

        kex::cSession->EventQueue().Push(&ev);
    }

    JoystickButtonEvent();
}

//
// kexInputSDL::UpdateFocus
//

void kexInputSDL::UpdateFocus(void)
{
    Uint32 flags;
    flags = kex::cSystem->GetWindowFlags();

    // We should have input (keyboard) focus and be visible
    // (not minimised)
    bWindowFocused = (flags & SDL_WINDOW_INPUT_FOCUS) || (flags & SDL_WINDOW_MOUSE_FOCUS);
}

//
// kexInput::TranslateKeyboard
//

int kexInputSDL::TranslateKeyboard(const int val)
{
    int code = val;
    
    if(code >= SDLK_0 && code <= SDLK_9)
    {
        return KKEY_0 - (SDLK_0 - code);
    }
    
    if(code >= SDLK_a && code <= SDLK_z)
    {
        return KKEY_z - (SDLK_z - code);
    }
    
    switch(code)
    {
        case SDLK_RETURN:           return KKEY_RETURN;
        case SDLK_ESCAPE:           return KKEY_ESCAPE;
        case SDLK_BACKSPACE:        return KKEY_BACKSPACE;
        case SDLK_TAB:              return KKEY_TAB;
        case SDLK_SPACE:            return KKEY_SPACE;
        case SDLK_EXCLAIM:          return KKEY_EXCLAIM;
        case SDLK_QUOTEDBL:         return KKEY_QUOTEDBL;
        case SDLK_HASH:             return KKEY_HASH;
        case SDLK_PERCENT:          return KKEY_PERCENT;
        case SDLK_DOLLAR:           return KKEY_DOLLAR;
        case SDLK_AMPERSAND:        return KKEY_AMPERSAND;
        case SDLK_QUOTE:            return KKEY_QUOTE;
        case SDLK_LEFTPAREN:        return KKEY_LEFTPAREN;
        case SDLK_RIGHTPAREN:       return KKEY_RIGHTPAREN;
        case SDLK_ASTERISK:         return KKEY_ASTERISK;
        case SDLK_PLUS:             return KKEY_PLUS;
        case SDLK_COMMA:            return KKEY_COMMA;
        case SDLK_MINUS:            return KKEY_MINUS;
        case SDLK_PERIOD:           return KKEY_PERIOD;
        case SDLK_SLASH:            return KKEY_SLASH;
        case SDLK_COLON:            return KKEY_COLON;
        case SDLK_SEMICOLON:        return KKEY_SEMICOLON;
        case SDLK_LESS:             return KKEY_LESS;
        case SDLK_EQUALS:           return KKEY_EQUALS;
        case SDLK_GREATER:          return KKEY_GREATER;
        case SDLK_QUESTION:         return KKEY_QUESTION;
        case SDLK_AT:               return KKEY_AT;
        case SDLK_LEFTBRACKET:      return KKEY_LEFTBRACKET;
        case SDLK_BACKSLASH:        return KKEY_BACKSLASH;
        case SDLK_RIGHTBRACKET:     return KKEY_RIGHTBRACKET;
        case SDLK_CARET:            return KKEY_CARET;
        case SDLK_UNDERSCORE:       return KKEY_UNDERSCORE;
        case SDLK_BACKQUOTE:        return KKEY_BACKQUOTE;
        case SDLK_DELETE:           return KKEY_DELETE;
    }
    
    code &= ~SDLK_SCANCODE_MASK;
    
    if(code >= SDL_SCANCODE_CAPSLOCK && code <= SDL_SCANCODE_KP_PERIOD)
    {
        return KKEY_CAPSLOCK + (code - SDL_SCANCODE_CAPSLOCK);
    }
    
    switch(code)
    {
        case SDL_SCANCODE_LCTRL:    return KKEY_LCTRL;
        case SDL_SCANCODE_LSHIFT:   return KKEY_LSHIFT;
        case SDL_SCANCODE_LALT:     return KKEY_LALT;
        case SDL_SCANCODE_RCTRL:    return KKEY_RCTRL;
        case SDL_SCANCODE_RSHIFT:   return KKEY_RSHIFT;
        case SDL_SCANCODE_RALT:     return KKEY_RALT;
    }
    
    return KKEY_UNDEFINED;
}

//
// kexInputSDL::TranslateMouse
//

int kexInputSDL::TranslateMouse(const int val)
{
    switch(val)
    {
        case SDL_BUTTON_LEFT:   return KMSB_LEFT;
        case SDL_BUTTON_MIDDLE: return KMSB_MIDDLE;
        case SDL_BUTTON_RIGHT:  return KMSB_RIGHT;
        case SDL_BUTTON_X1:     return KMSB_MISC1;
        case SDL_BUTTON_X2:     return KMSB_MISC2;
    }
    
    return KMSB_UNDEFINED;
}

//
// kexInputSDL::CapslockOn
//

bool kexInputSDL::CapslockOn(void)
{
    return (SDL_GetModState() & KMOD_CAPS) != 0;
}

//
// kexInputSDL::IsShiftDown
//

bool kexInputSDL::IsShiftDown(int c) const
{
    return (SDL_GetModState() & (KMOD_LSHIFT|KMOD_RSHIFT)) != 0;
}

//
// kexInputSDL::IsCtrlDown
//

bool kexInputSDL::IsCtrlDown(int c) const
{
    return (SDL_GetModState() & (KMOD_LCTRL|KMOD_RCTRL|KMOD_LGUI|KMOD_RGUI)) != 0;
}

//
// kexInputSDL::IsAltDown
//

bool kexInputSDL::IsAltDown(int c) const
{
    return (SDL_GetModState() & (KMOD_LALT|KMOD_RALT)) != 0;
}

//
// kexInputSDL::GetEvent
//

void kexInputSDL::GetEvent(const SDL_Event *Event)
{
    inputEvent_t event;

    switch(Event->type)
    {
    case SDL_KEYDOWN:
        if(Event->key.repeat)
        {
            break;
        }
        event.type = ev_keydown;
        event.data1 = TranslateKeyboard(Event->key.keysym.sym);
        kex::cSession->EventQueue().Push(&event);
        break;

    case SDL_KEYUP:
        event.type = ev_keyup;
        event.data1 = TranslateKeyboard(Event->key.keysym.sym);
        kex::cSession->EventQueue().Push(&event);
        break;

    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
        if(!bWindowFocused)
        {
            break;
        }
        event.type = Event->type ==
                     SDL_MOUSEBUTTONUP ? ev_mouseup : ev_mousedown;
        event.data1 = TranslateMouse(Event->button.button);
        event.data2 = event.data3 = 0;
        kex::cSession->EventQueue().Push(&event);
        break;

    case SDL_MOUSEWHEEL:
        if(!bWindowFocused)
        {
            break;
        }
        event.type = ev_mousedown;
        event.data1 = Event->wheel.y > 0 ? KMSB_WHEEL_UP : KMSB_WHEEL_DOWN;
        event.data2 = event.data3 = 0;
        kex::cSession->EventQueue().Push(&event);
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
// kexInputSDL::PollInput
//

void kexInputSDL::PollInput(void)
{
    SDL_Event Event;

    while(SDL_PollEvent(&Event))
    {
        GetEvent(&Event);

        SDL_PumpEvents();
    }

    ReadMouse();
    ReadJoystick();
}

//
// kexInputSDL::MouseShouldBeGrabbed
//

bool kexInputSDL::MouseShouldBeGrabbed(void) const
{
    // if the window doesnt have focus, never grab it
    if(!bWindowFocused || !bGrabMouse)
    {
        return false;
    }

    return bEnabled;
}

//
// kexInputSDL::ActivateMouse
//

void kexInputSDL::ActivateMouse(void)
{
    SDL_ShowCursor(0);
    SDL_SetRelativeMouseMode(SDL_TRUE);
    kex::cSystem->SetWindowGrab(SDL_TRUE);
}

//
// kexInputSDL::DeactivateMouse
//

void kexInputSDL::DeactivateMouse(void)
{
    SDL_ShowCursor(1);
    SDL_SetRelativeMouseMode(SDL_FALSE);
    kex::cSystem->SetWindowGrab(SDL_FALSE);
}

//
// kexInputSDL::UpdateGrab
//

void kexInputSDL::UpdateGrab(void)
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
// kexInputSDL::CenterMouse
// Warp the mouse back to the middle of the screen
//

void kexInputSDL::CenterMouse(void)
{
    // Warp the the screen center
    kex::cSystem->WarpMouseToCenter();

    // Clear any relative movement caused by warping
    SDL_PumpEvents();
    SDL_GetRelativeMouseState(NULL, NULL);
}
