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
    
private:
    void                ReadMouse(void);
    bool                MouseShouldBeGrabbed(void) const;
    void                GetEvent(const SDL_Event *Event);
    void                UpdateFocus(void);
    
    bool                bWindowFocused;
    bool                bGrabbed;
    Uint8               lastmbtn;
};

static kexInputSDL inputSystem;
kexInput *kex::cInput = &inputSystem;

//
// kexInputSDL::kexInputSDL
//

kexInputSDL::kexInputSDL()
{
    lastmbtn        = 0;
    bGrabbed        = false;
    bWindowFocused  = false;
    bEnabled        = true;
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

    kex::cSystem->Printf("Input Initialized\n");
}

//
// kexInputSDL::ReadMouse
//

void kexInputSDL::ReadMouse(void)
{
    int x, y;
    Uint8 btn;
    inputEvent_t ev;

    if(kex::cSystem->IsWindowed() && kex::cConsole->IsActive())
    {
        return;
    }

    SDL_GetRelativeMouseState(&x, &y);
    btn = SDL_GetMouseState(&mouse_x, &mouse_y);

    if(x != 0 || y != 0 || btn || (lastmbtn != btn))
    {
        ev.type = ev_mouse;
        ev.data1 = x << 5;
        ev.data2 = (-y) << 5;
        ev.data3 = 0;
        ev.data4 = 0;

        kex::cSession->EventQueue().Push(&ev);
    }

    lastmbtn = btn;

    if(MouseShouldBeGrabbed())
    {
        CenterMouse();
    }
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
        return KKEY_CAPSLOCK + (code - SDL_SCANCODE_KP_PERIOD);
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
        event.type = ev_mousewheel;
        event.data1 = Event->wheel.y > 0 ? 1 : -1;
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
    }

    ReadMouse();
}

//
// kexInputSDL::MouseShouldBeGrabbed
//

bool kexInputSDL::MouseShouldBeGrabbed(void) const
{
    // if the window doesnt have focus, never grab it
    if(!bWindowFocused)
    {
        return false;
    }

    if(kex::cSystem->IsWindowed())
    {
        if(kex::cConsole->IsActive())
        {
            return false;
        }
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
