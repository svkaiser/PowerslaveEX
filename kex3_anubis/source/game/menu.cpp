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
//      Menu logic
//

#include "kexlib.h"
#include "game.h"
#include "renderMain.h"
#include "menu.h"
#include "localization.h"

//-----------------------------------------------------------------------------
//
// kexMenuObject
//
//-----------------------------------------------------------------------------

DECLARE_ABSTRACT_KEX_CLASS(kexMenuObject, kexObject)

//
// kexMenuObject::kexMenuObject
//

kexMenuObject::kexMenuObject(void)
{
    this->x = 0;
    this->y = 0;
    this->label = " ";
    this->textAlignment = MITA_CENTER;
    this->index = 0;
    this->bSelected = false;
    this->Callback = NULL;
}

//-----------------------------------------------------------------------------
//
// kexMenuObjectPanelSelect
//
//-----------------------------------------------------------------------------

BEGIN_EXTENDED_KEX_CLASS(kexMenuObjectPanelSelect, kexMenuObject);
public:
    kexMenuObjectPanelSelect(void);

    virtual void                    Draw(void);
    virtual void                    Tick(void);
    virtual void                    Select(const bool b);
    virtual bool                    CheckMouseSelect(const float mx, const float my);

    float                           w;
    float                           h;
END_KEX_CLASS();

DECLARE_KEX_CLASS(kexMenuObjectPanelSelect, kexMenuObject)

//
// kexMenuObjectPanelSelect::kexMenuObjectPanelSelect
//

kexMenuObjectPanelSelect::kexMenuObjectPanelSelect(void)
{
    this->w = 0;
    this->h = 0;
}

//
// kexMenuObjectPanelSelect::Draw
//

void kexMenuObjectPanelSelect::Draw(void)
{
    static kexFont *font = NULL;
    float len;

    kexGame::cMenuPanel->DrawInset(x, y, w, h);

    if(bSelected)
    {
        kexRender::cScreen->DrawTexture(kexRender::cTextures->whiteTexture,
                                        x+2, y+2, 192, 32, 32, 128, w-3, h-4);
    }

    if(font == NULL)
    {
        if(!(font = kexFont::Get("smallfont")))
        {
            return;
        }
    }

    switch(textAlignment)
    {
    case MITA_LEFT:
        kexGame::cLocal->DrawSmallString(label.c_str(), x+8, y, 1, false);
        break;

    case MITA_RIGHT:
        len = font->StringWidth(label.c_str(), 1, 0);
        kexGame::cLocal->DrawSmallString(label.c_str(), (w-len)+10, y, 1, false);
        break;

    case MITA_CENTER:
        kexGame::cLocal->DrawSmallString(label.c_str(),
                                         (w + (x * 2)) * 0.5f,
                                         ((h + (y * 2)) * 0.5f) - 4, 1, true);
        break;

    default:
        break;
    }
}

//
// kexMenuObjectPanelSelect::CheckMouseSelect
//

bool kexMenuObjectPanelSelect::CheckMouseSelect(const float mx, const float my)
{
    if(mx >= x+1 && mx <= (x+1) + (w-1) &&
       my >= y-1 && my <= y + (h-1))
    {
        bSelected = true;
        return true;
    }

    bSelected = false;
    return false;
}

//
// kexMenuObjectPanelSelect::Tick
//

void kexMenuObjectPanelSelect::Tick(void)
{
    if(bSelected && kexGame::cLocal->ButtonEvent() & GBE_MENU_SELECT)
    {
        if(Callback)
        {
            (reinterpret_cast<kexMenu*const>(this)->*Callback)(this);
        }
    }
}

//
// kexMenuObjectPanelSelect::Select
//

void kexMenuObjectPanelSelect::Select(const bool b)
{
}

//-----------------------------------------------------------------------------
//
// kexMenuObjectButton
//
//-----------------------------------------------------------------------------

BEGIN_EXTENDED_KEX_CLASS(kexMenuObjectButton, kexMenuObject);
public:
    kexMenuObjectButton(void);

    virtual void                    Draw(void);
    virtual void                    Tick(void);
    virtual void                    Select(const bool b);
    virtual bool                    CheckMouseSelect(const float mx, const float my);

    float                           w;
    float                           h;

    bool                            bPressed;
END_KEX_CLASS();

DECLARE_KEX_CLASS(kexMenuObjectButton, kexMenuObject)

//
// kexMenuObjectButton::kexMenuObjectButton
//

kexMenuObjectButton::kexMenuObjectButton(void)
{
    this->w = 0;
    this->h = 0;
    this->bPressed = false;
}

//
// kexMenuObjectButton::Draw
//

void kexMenuObjectButton::Draw(void)
{
    static kexFont *font = NULL;
    kexCpuVertList *vl = kexRender::cVertList;
    byte r, g, b;

    kexRender::cTextures->whiteTexture->Bind();
    kexRender::cBackend->SetBlend(GLSRC_SRC_ALPHA, GLDST_ONE_MINUS_SRC_ALPHA);

    vl->AddQuad(x, y, w, 1, 0, 0, 0, 255);
    vl->AddQuad(x, y, 1, h, 0, 0, 0, 255);
    vl->AddQuad(x+w, y, 1, h+1, 0, 0, 0, 255);
    vl->AddQuad(x, y+h, w, 1, 0, 0, 0, 255);

    r = 96;
    g = 96;
    b = 120;

    if(bPressed)
    {
        r /= 2;
        g /= 2;
        b /= 2;
    }
    else if(bSelected)
    {
        r *= 2;
        g *= 2;
        b *= 2;
    }

    vl->AddQuad(x+1, y+1, w-1, 1, r, g, b, 255);
    vl->AddQuad(x+1, y+1, 1, h-1, r, g, b, 255);

    r = 16;
    g = 16;
    b = 80;

    if(bPressed)
    {
        r /= 2;
        g /= 2;
        b /= 2;
    }
    else if(bSelected)
    {
        r *= 2;
        g *= 2;
        b *= 2;
    }

    vl->AddQuad(x+w-1, y+1, 1, h-1, r, g, b, 255);
    vl->AddQuad(x+1, y+h-1, w-1, 1, r, g, b, 255);

    r = 48;
    g = 48;
    b = 120;

    if(bPressed)
    {
        r /= 2;
        g /= 2;
        b /= 2;
    }
    else if(bSelected)
    {
        r *= 2;
        g *= 2;
        b *= 2;
    }

    vl->AddQuad(x+2, y+2, w-3, h-3, r, g, b, 255);
    vl->DrawElements();

    if(font == NULL)
    {
        if(!(font = kexFont::Get("smallfont")))
        {
            return;
        }
    }

    font->DrawString(label, x+(w*0.5f), y+(h*0.5f)-2, 1, true);
}

//
// kexMenuObjectButton::CheckMouseSelect
//

bool kexMenuObjectButton::CheckMouseSelect(const float mx, const float my)
{
    bSelected = (mx >= x && mx <= x + w &&
                 my >= y && my <= y + h);

    if(!bSelected && bPressed)
    {
        bPressed = false;
    }

    return bSelected;
}

//
// kexMenuObjectButton::Tick
//

void kexMenuObjectButton::Tick(void)
{
    if(bSelected == false)
    {
        return;
    }

    if(kexGame::cLocal->ButtonEvent() & GBE_MENU_SELECT)
    {
        bPressed = true;
    }
    else if(bPressed == true)
    {
        bPressed = false;

        if(Callback)
        {
            (reinterpret_cast<kexMenu*const>(this)->*Callback)(this);
        }
    }
}

//
// kexMenuObjectButton::Select
//

void kexMenuObjectButton::Select(const bool b)
{
}

//-----------------------------------------------------------------------------
//
// kexMenu
//
//-----------------------------------------------------------------------------

DECLARE_KEX_CLASS(kexMenu, kexObject)

//
// kexMenu::kexMenu
//

kexMenu::kexMenu(void)
{
    this->itemIndex = 0;
    this->selectedItem = -1;
}

//
// kexMenu::Init
//

void kexMenu::Init(void)
{
}

//
// kexMenu::Update
//

void kexMenu::Update(void)
{
}

//
// kexMenu::Display
//

void kexMenu::Display(void)
{
}

//
// kexMenu::Reset
//

void kexMenu::Reset(void)
{
}

//
// kexMenu::ProcessInput
//

bool kexMenu::ProcessInput(inputEvent_t *ev)
{
    return false;
}

//
// kexMenu::AllocateMenuObject
//

kexMenuObject *kexMenu::AllocateMenuObject(const char *className)
{
    kexObject *obj;
    kexMenuObject *mObj;
    kexRTTI *objType;
    
    if(!(objType = kexObject::Get(className)))
    {
        kex::cSystem->Error("kexMenu::AllocateMenuObject: unknown class (\"%s\")\n", className);
        return NULL;
    }
        
    if(!(obj = objType->Create()))
    {
        kex::cSystem->Error("kexMenu::AllocateMenuObject: could not spawn (\"%s\")\n", className);
        return NULL;
    }

    mObj = static_cast<kexMenuObject*>(obj);
    mObj->index = itemIndex;

    menuObjects.Push(mObj);
    itemIndex++;

    return mObj;
}

//
// kexMenu::DrawItems
//

void kexMenu::DrawItems(void)
{
    for(uint i = 0; i < menuObjects.Length(); ++i)
    {
        menuObjects[i]->Draw();
    }
}

//
// kexMenu::UpdateItems
//

void kexMenu::UpdateItems(void)
{
    float mx = (float)kex::cInput->MouseX();
    float my = (float)kex::cInput->MouseY();

    kexRender::cScreen->CoordsToRenderScreenCoords(mx, my);
    selectedItem = -1;

    for(uint i = 0; i < menuObjects.Length(); ++i)
    {
        menuObjects[i]->Tick();
        if(menuObjects[i]->CheckMouseSelect(mx, my))
        {
            selectedItem = (int)i;
        }
    }
}

//-----------------------------------------------------------------------------
//
// kexMenuQuitConfirm
//
//-----------------------------------------------------------------------------

DEFINE_MENU_CLASS(kexMenuQuitConfirm);
public:
    virtual void                    Init(void);
    virtual void                    Display(void);
    virtual void                    Update(void);
    virtual bool                    ProcessInput(inputEvent_t *ev);

private:
    kexMenuPanel::selectButton_t    quitYesButton;
    kexMenuPanel::selectButton_t    quitNoButton;
END_MENU_CLASS();

DECLARE_MENU_CLASS(kexMenuQuitConfirm, MENU_QUITCONFIRM);

//
// kexMenuQuitConfirm::Init
//

void kexMenuQuitConfirm::Init(void)
{
    quitYesButton.x = 40;
    quitYesButton.y = 120;
    quitYesButton.w = 96;
    quitYesButton.h = 24;
    quitYesButton.label = "Yes";
    
    quitNoButton.x = 182;
    quitNoButton.y = 120;
    quitNoButton.w = 96;
    quitNoButton.h = 24;
    quitNoButton.label = "No";
}

//
// kexMenuQuitConfirm::Update
//

void kexMenuQuitConfirm::Update(void)
{
    kexGame::cMenuPanel->UpdateSelectButton(&quitYesButton);
    kexGame::cMenuPanel->UpdateSelectButton(&quitNoButton);
}

//
// kexMenuQuitConfirm::Display
//

void kexMenuQuitConfirm::Display(void)
{
    kexRender::cScreen->SetOrtho();
    
    kexRender::cScreen->DrawStretchPic(kexRender::cTextures->whiteTexture, 0, 0,
        (float)kexRender::cScreen->SCREEN_WIDTH,
        (float)kexRender::cScreen->SCREEN_HEIGHT, 0, 0, 0, 128);
    
    kexGame::cMenuPanel->DrawPanel(32, 64, 256, 96, 4);
    kexGame::cMenuPanel->DrawInset(40, 72, 238, 32);
    kexGame::cLocal->DrawSmallString("Are you sure you want to quit?", 160, 82, 1, true);
    
    kexGame::cMenuPanel->DrawSelectButton(&quitYesButton);
    kexGame::cMenuPanel->DrawSelectButton(&quitNoButton);
}

//
// kexMenuQuitConfirm::ProcessInput
//

bool kexMenuQuitConfirm::ProcessInput(inputEvent_t *ev)
{
    if(kexGame::cMenuPanel->TestSelectButtonInput(&quitYesButton, ev))
    {
        kex::cCommands->Execute("quit");
        return true;
    }
    
    if(kexGame::cMenuPanel->TestSelectButtonInput(&quitNoButton, ev))
    {
        kexGame::cLocal->ClearMenu();
        kexGame::cLocal->PlaySound("sounds/select.wav");
        return true;
    }
    
    return false;
}

//-----------------------------------------------------------------------------
//
// kexMenuBindings
//
//-----------------------------------------------------------------------------

DEFINE_MENU_CLASS(kexMenuBindings);
public:
    virtual void                    Init(void);
    virtual void                    Display(void);
    virtual void                    Update(void);
    virtual bool                    ProcessInput(inputEvent_t *ev);

    typedef struct
    {
        const char  *action;
        int         actionID;
        const char  *command;
    } inputBinds_t;

    static const inputBinds_t       inputBinds1[];
    static const inputBinds_t       inputBinds2[];
    static const inputBinds_t       inputBinds3[];
    static const inputBinds_t       inputBinds4[];

    static const float              bindXOffset;
    static const float              bindYOffset;
    static const float              bindRowHeight;
    static const float              bindRowWidth;

private:
    void                            DrawBinds(const kexMenuBindings::inputBinds_t binds[]);
    bool                            SetBind(const inputEvent_t *ev);

    kexMenuPanel::selectButton_t    exitButton;
    int                             pageNum;
    const inputBinds_t              *selectedBind;
    int                             bindTime;
END_MENU_CLASS();

DECLARE_MENU_CLASS(kexMenuBindings, MENU_BINDINGS);

const kexMenuBindings::inputBinds_t kexMenuBindings::inputBinds1[] =
{
    { "Attack",         IA_ATTACK,          "attack" },
    { "Jump",           IA_JUMP,            "jump" },
    { "Forward",        IA_FORWARD,         "forward" },
    { "Backward",       IA_BACKWARD,        "backward" },
    { "Turn-L",         IA_LEFT,            "left" },
    { "Turn-R",         IA_RIGHT,           "right" },
    { "Strafe-L",       IA_STRAFELEFT,      "strafeleft" },
    { "Strafe-R",       IA_STRAFERIGHT,     "straferight" },
    { "Interact",       IA_USE,             "+use" },
    { NULL,             -1,                 NULL }
};

const kexMenuBindings::inputBinds_t kexMenuBindings::inputBinds2[] =
{
    { "Machete",        -1,                 "weapon 0" },
    { "Pistol",         -1,                 "weapon 1" },
    { "M-60",           -1,                 "weapon 2" },
    { "Amun Bombs",     -1,                 "weapon 3" },
    { "Flame Thrower",  -1,                 "weapon 4" },
    { "Cobra Staff",    -1,                 "weapon 5" },
    { "Ring Of Ra",     -1,                 "weapon 6" },
    { "Manacle",        -1,                 "weapon 7" },
    { "Next Weapon",    IA_WEAPNEXT,        "+weapnext" },
    { "Prev Weapon",    IA_WEAPPREV,        "+weapprev" },
    { NULL,             -1,                 NULL }
};

const kexMenuBindings::inputBinds_t kexMenuBindings::inputBinds3[] =
{
    { "Map Zoom-In",    IA_MAPZOOMIN,       "mapzoomin" },
    { "Map Zoom-Out",   IA_MAPZOOMOUT,      "mapzoomout" },
    { "Automap",        -1,                 "automap" },
    { "Inventory Menu", -1,                 "inventorymenu" },
    { "Screenshot",     -1,                 "screenshot" },
    { NULL,             -1,                 NULL }
};

const kexMenuBindings::inputBinds_t kexMenuBindings::inputBinds4[] =
{
    { "Menu Activate",  -1,                 "menu_activate" },
    { "Menu Up",        -1,                 "menu_up" },
    { "Menu Right",     -1,                 "menu_right" },
    { "Menu Down",      -1,                 "menu_down" },
    { "Menu Left",      -1,                 "menu_left" },
    { "Menu Select",    -1,                 "menu_select" },
    { "Menu Cancel",    -1,                 "menu_cancel" },
    { "Menu Back",      -1,                 "menu_back" },
    { NULL,             -1,                 NULL }
};

const float kexMenuBindings::bindXOffset   = 16;
const float kexMenuBindings::bindYOffset   = 44;
const float kexMenuBindings::bindRowHeight = 16;
const float kexMenuBindings::bindRowWidth  = 288;

//
// kexMenuBindings::Init
//

void kexMenuBindings::Init(void)
{
    pageNum = 0;
    
    exitButton.x = 112;
    exitButton.y = 204;
    exitButton.w = 96;
    exitButton.h = 24;
    exitButton.label = "Exit";
}

//
// kexMenuBindings::Update
//

void kexMenuBindings::Update(void)
{
    const kexMenuBindings::inputBinds_t *bind;
    const kexMenuBindings::inputBinds_t *binds;
    float mx = (float)kex::cInput->MouseX();
    float my = (float)kex::cInput->MouseY();
    float y;
    
    if(bindTime > 0)
    {
        if(selectedBind == NULL)
        {
            bindTime = 0;
        }
        else
        {
            bindTime--;
            return;
        }
    }

    kexGame::cMenuPanel->UpdateSelectButton(&exitButton);
    kexRender::cScreen->CoordsToRenderScreenCoords(mx, my);

    y = bindYOffset;

    switch(pageNum)
    {
    case 0:
        binds = inputBinds1;
        break;
    case 1:
        binds = inputBinds2;
        break;
    case 2:
        binds = inputBinds3;
        break;
    case 3:
        binds = inputBinds4;
        break;
    default:
        return;
    }

    selectedBind = NULL;

    for(bind = binds; bind->action != NULL; bind++)
    {
        if(mx >= bindXOffset+3 && mx <= (bindXOffset+3) + (bindRowWidth-3) &&
           my >= y-2 && my <= y + (bindRowHeight-6))
        {
            selectedBind = bind;
            break;
        }

        y += bindRowHeight;
    }
}

//
// kexMenuBindings::DrawBinds
//

void kexMenuBindings::DrawBinds(const kexMenuBindings::inputBinds_t binds[])
{
    static kexFont *font = NULL;
    kexStrList bindList;
    const kexMenuBindings::inputBinds_t *bind;
    float len;
    float y;
    int which;

    if(font == NULL)
    {
        if(!(font = kexFont::Get("smallfont")))
        {
            return;
        }
    }

    y = bindYOffset;
    which = kex::cSession->GetTicks() >> 6;

    for(bind = binds; bind->action != NULL; bind++)
    {
        if(bind->actionID >= 0)
        {
            kex::cActions->GetActionBinds(bindList, bind->actionID);
        }
        else
        {
            kex::cActions->GetCommandBinds(bindList, bind->command);
        }

        kexGame::cMenuPanel->DrawInset(bindXOffset, y-4, bindRowWidth, bindRowHeight);

        if(selectedBind == bind)
        {
            kexRender::cScreen->DrawTexture(kexRender::cTextures->whiteTexture, (bindXOffset+2), y-2,
                                            192, 32, 32, 128, (bindRowWidth-3), (bindRowHeight-4));
        }

        kexGame::cLocal->DrawSmallString(bind->action, bindXOffset+8, y, 1, false);

        if(bindList.Length() != 0)
        {
            int i = which % bindList.Length();

            len = font->StringWidth(bindList[i].c_str(), 1, 0);
            kexGame::cLocal->DrawSmallString(bindList[i].c_str(), (bindRowWidth-len)+10, y, 1, false);
            bindList.Empty();
        }
        else
        {
            const char *tmp = "Unbound";

            len = font->StringWidth(tmp, 1, 0);
            kexGame::cLocal->DrawSmallString(tmp, (bindRowWidth-len)+10, y, 1, false);
        }

        y += bindRowHeight;
    }

    if(pageNum < 3)
    {
        kexGame::cMenuPanel->DrawRightArrow(bindRowWidth, 21);
    }

    if(pageNum > 0)
    {
        kexGame::cMenuPanel->DrawLeftArrow(24, 21);
    }
}

//
// kexMenuBindings::Display
//

void kexMenuBindings::Display(void)
{
    kexRender::cScreen->SetOrtho();
    
    kexRender::cScreen->DrawStretchPic(kexRender::cTextures->whiteTexture, 0, 0,
        (float)kexRender::cScreen->SCREEN_WIDTH,
        (float)kexRender::cScreen->SCREEN_HEIGHT, 0, 0, 0, 128);

    kexGame::cMenuPanel->DrawPanel(0, 0, 320, 240, 4);
    kexGame::cMenuPanel->DrawInset(bindXOffset, 16, bindRowWidth, 16);
    
    kexGame::cMenuPanel->DrawSelectButton(&exitButton);
    
    kexGame::cLocal->DrawSmallString(kexStr::Format("Page %i/4", pageNum+1), 160, 20, 1, true);
    
    switch(pageNum)
    {
    case 0:
        DrawBinds(inputBinds1);
        break;
    case 1:
        DrawBinds(inputBinds2);
        break;
    case 2:
        DrawBinds(inputBinds3);
        break;
    case 3:
        DrawBinds(inputBinds4);
        break;
    default:
        break;
    }
    
    if(bindTime > 0 && selectedBind)
    {
        kexRender::cScreen->DrawStretchPic(kexRender::cTextures->whiteTexture, 0, 0,
            (float)kexRender::cScreen->SCREEN_WIDTH,
            (float)kexRender::cScreen->SCREEN_HEIGHT, 0, 0, 0, 128);
        
        kexGame::cMenuPanel->DrawPanel(32, 64, 256, 96, 4);
        kexGame::cMenuPanel->DrawInset(40, 72, 238, 80);
        kexGame::cLocal->DrawSmallString(kexStr::Format("Set bind for %s", selectedBind->action),
                                         160, 82, 1, true);
        kexGame::cLocal->DrawSmallString("Wait 5 seconds to cancel", 160, 92, 1, true);
    }
}

//
// kexMenuBindings::SetBind
//

bool kexMenuBindings::SetBind(const inputEvent_t *ev)
{
    int key;
    
    if(!selectedBind)
    {
        return false;
    }
    
    switch(ev->type)
    {
    case ev_keydown:
        key = kex::cActions->GetKeyboardCode(ev->data1);
        break;
        
    case ev_mousedown:
        key = kex::cActions->GetMouseCode(ev->data1);
        break;
        
    case ev_joybtndown:
        key = kex::cActions->GetJoystickCode(ev->data1);
        break;
        
    default:
        return false;
    }
    
    if(key <= -1)
    {
        return false;
    }
    
    if(kex::cActions->IsKeyBindedToAction(key, selectedBind->command))
    {
        kex::cActions->UnBindCommand(key, selectedBind->command);
    }
    else
    {
        kex::cActions->BindCommand(key, selectedBind->command);
    }
    
    bindTime = 0;
    return true;
}

//
// kexMenuBindings::ProcessInput
//

bool kexMenuBindings::ProcessInput(inputEvent_t *ev)
{
    if(bindTime > 0)
    {
        return SetBind(ev);
    }
    
    if(kexGame::cMenuPanel->TestSelectButtonInput(&exitButton, ev))
    {
        kexGame::cLocal->ClearMenu();
        kexGame::cLocal->PlaySound("sounds/select.wav");
        return true;
    }

    switch(ev->type)
    {
    case ev_mousedown:
        if(ev->data1 == KMSB_LEFT)
        {
            if(pageNum < 3 && kexGame::cMenuPanel->CursorOnRightArrow(288, 21))
            {
                kexGame::cLocal->PlaySound("sounds/select.wav");
                pageNum++;
                return true;
            }
            if(pageNum > 0 && kexGame::cMenuPanel->CursorOnLeftArrow(24, 21))
            {
                kexGame::cLocal->PlaySound("sounds/select.wav");
                pageNum--;
                return true;
            }
            if(selectedBind != NULL)
            {
                bindTime = 300;
            }
        }
        break;

    case ev_mouseup:
        if(ev->data1 == KMSB_LEFT)
        {
        }
        break;
    }
    
    return false;
}

//-----------------------------------------------------------------------------
//
// kexMenuTravel
//
//-----------------------------------------------------------------------------

DEFINE_MENU_CLASS(kexMenuTravel);
public:
    virtual void                    Init(void);
    virtual void                    Display(void);
    virtual void                    Update(void);
    virtual bool                    ProcessInput(inputEvent_t *ev);

private:
    kexMenuPanel::selectButton_t    travelYesButton;
    kexMenuPanel::selectButton_t    travelNoButton;
END_MENU_CLASS();

DECLARE_MENU_CLASS(kexMenuTravel, MENU_TRAVEL);

//
// kexMenuTravel::Init
//

void kexMenuTravel::Init(void)
{
    travelYesButton.x = 40;
    travelYesButton.y = 120;
    travelYesButton.w = 96;
    travelYesButton.h = 24;
    travelYesButton.label = "Travel";
    
    travelNoButton.x = 182;
    travelNoButton.y = 120;
    travelNoButton.w = 96;
    travelNoButton.h = 24;
    travelNoButton.label = "Remain";
}

//
// kexMenuTravel::Update
//

void kexMenuTravel::Update(void)
{
    kexGame::cMenuPanel->UpdateSelectButton(&travelYesButton);
    kexGame::cMenuPanel->UpdateSelectButton(&travelNoButton);
}

//
// kexMenuTravel::Display
//

void kexMenuTravel::Display(void)
{
    kexRender::cScreen->SetOrtho();
    
    kexRender::cScreen->DrawStretchPic(kexRender::cTextures->whiteTexture, 0, 0,
        (float)kexRender::cScreen->SCREEN_WIDTH,
        (float)kexRender::cScreen->SCREEN_HEIGHT, 0, 0, 0, 255);
    
    kexGame::cMenuPanel->DrawPanel(32, 64, 256, 96, 4);
    kexGame::cMenuPanel->DrawInset(40, 72, 238, 32);
    kexGame::cLocal->DrawSmallString(kexGame::cLocal->Translation()->GetString(127), 160, 82, 1, true);
    
    kexGame::cMenuPanel->DrawSelectButton(&travelYesButton);
    kexGame::cMenuPanel->DrawSelectButton(&travelNoButton);
}

//
// kexMenuTravel::ProcessInput
//

bool kexMenuTravel::ProcessInput(inputEvent_t *ev)
{
    if(kexGame::cMenuPanel->TestSelectButtonInput(&travelYesButton, ev))
    {
        int angBit;
        int nextMap;
        float ang;

        if(kexTravelObject::currentObject == NULL)
        {
            return true;
        }

        ang = kexTravelObject::currentObject->Yaw().an;
        kexAngle::Clamp360(ang);

        angBit = ((int)kexMath::Ceil(ang * (4096.0f / 360.0f)) >> 10) & 3;
        nextMap = kexGame::cLocal->ActiveMap()->nextMap[angBit];

        if(nextMap <= -1)
        {
            kexGame::cLocal->ClearMenu();
            kex::cSystem->Warning("No linked map has been found for %s\n",
                                  kexGame::cLocal->ActiveMap()->title.c_str());
            return true;
        }

        kexGame::cLocal->MapUnlockList()[kexGame::cLocal->ActiveMap()->refID] = true;
        kexGame::cLocal->MapUnlockList()[nextMap] = true;
        kexGame::cLocal->SetGameState(GS_OVERWORLD);
        return true;
    }
    
    if(kexGame::cMenuPanel->TestSelectButtonInput(&travelNoButton, ev))
    {
        kexGame::cLocal->ClearMenu();
        kexGame::cLocal->PlaySound("sounds/select.wav");
        return true;
    }
    
    return false;
}

//-----------------------------------------------------------------------------
//
// kexMenuStartupNotice
//
//-----------------------------------------------------------------------------

DEFINE_MENU_CLASS(kexMenuStartupNotice);
public:
    virtual void                    Init(void);
    virtual void                    Display(void);
    virtual void                    Update(void);
    virtual bool                    ProcessInput(inputEvent_t *ev);

private:
    kexMenuPanel::selectButton_t    okButton;
END_MENU_CLASS();

DECLARE_MENU_CLASS(kexMenuStartupNotice, MENU_STARTUP_NOTICE);

//
// kexMenuStartupNotice::Init
//

void kexMenuStartupNotice::Init(void)
{
    okButton.x = 112;
    okButton.y = 168;
    okButton.w = 96;
    okButton.h = 24;
    okButton.label = "Ok";
}

//
// kexMenuStartupNotice::Update
//

void kexMenuStartupNotice::Update(void)
{
    kexGame::cMenuPanel->UpdateSelectButton(&okButton);
}

//
// kexMenuStartupNotice::Display
//

void kexMenuStartupNotice::Display(void)
{
    float y;
    static const char *noticeStrings[] =
    {
        "--NOTICE--",
        " ",
        "This build of PowerslaveEX is",
        "still in early development",
        "and may be unstable. Some",
        "features may also be",
        "unavailable.",
        " ",
        "Be sure to report any issues",
        "to svkaiser-at-gmail.com",
        NULL
    };
    
    kexRender::cBackend->SetState(GLSTATE_ALPHATEST, true);
    kexRender::cBackend->SetState(GLSTATE_BLEND, true);

    kexRender::cScreen->SetOrtho();
    
    y = 50;
    
    kexRender::cScreen->DrawStretchPic(kexRender::cTextures->whiteTexture, 0, 0,
        (float)kexRender::cScreen->SCREEN_WIDTH,
        (float)kexRender::cScreen->SCREEN_HEIGHT, 0, 0, 0, 128);
    
    kexGame::cMenuPanel->DrawPanel(32, 32, 256, 176, 4);
    kexGame::cMenuPanel->DrawInset(40, 40, 238, 120);

    for(int i = 0; noticeStrings[i]; ++i)
    {
        const char *text = noticeStrings[i];

        float height = kexFont::Get("smallfont")->StringHeight(text, 1, 0);
        kexGame::cLocal->DrawSmallString(text, 160, y, 1, true);

        y += (height * 1.25f);
    }
    
    kexGame::cMenuPanel->DrawSelectButton(&okButton);
}

//
// kexMenuStartupNotice::ProcessInput
//

bool kexMenuStartupNotice::ProcessInput(inputEvent_t *ev)
{
    if(kexGame::cMenuPanel->TestSelectButtonInput(&okButton, ev))
    {
        kexGame::cLocal->ClearMenu();
        kexGame::cLocal->PlaySound("sounds/select.wav");
        return true;
    }
    
    return false;
}

//-----------------------------------------------------------------------------
//
// kexMenuPause
//
//-----------------------------------------------------------------------------

DEFINE_MENU_CLASS(kexMenuPause);
public:
    virtual void                    Init(void);
    virtual void                    Display(void);
    virtual void                    Update(void);
    virtual bool                    ProcessInput(inputEvent_t *ev);

private:
    void                            OnResume(kexMenuObject *menuObject);
    void                            OnOptions(kexMenuObject *menuObject);
    void                            OnMainMenu(kexMenuObject *menuObject);
    void                            OnQuit(kexMenuObject *menuObject);
END_MENU_CLASS();

DECLARE_MENU_CLASS(kexMenuPause, MENU_PAUSE);

//
// kexMenuPause::Init
//

void kexMenuPause::Init(void)
{
    kexMenuObjectPanelSelect *item;
    const char *labels[] =
    {
        "Resume",
        "Options",
        "Exit to Main Menu",
        "Restart Level",
        "Quit"
    };

    for(int i = 0; i < ARRLEN(labels); ++i)
    {
        item = static_cast<kexMenuObjectPanelSelect*>(AllocateMenuObject("kexMenuObjectPanelSelect"));
        item->x = 88;
        item->y = 88 + (16 * (float)i);
        item->w = 142;
        item->h = 16;
        item->label = labels[i];
        item->textAlignment = kexMenuObject::MITA_CENTER;
    }

    menuObjects[0]->Callback = static_cast<selectCallback_t>(&kexMenuPause::OnResume);
    menuObjects[1]->Callback = static_cast<selectCallback_t>(&kexMenuPause::OnOptions);
    menuObjects[2]->Callback = static_cast<selectCallback_t>(&kexMenuPause::OnMainMenu);
    menuObjects[4]->Callback = static_cast<selectCallback_t>(&kexMenuPause::OnQuit);
}

//
// kexMenuPause::Update
//

void kexMenuPause::Update(void)
{
    UpdateItems();
}

//
// kexMenuPause::OnResume
//

void kexMenuPause::OnResume(kexMenuObject *menuObject)
{
    kexGame::cLocal->ClearMenu();
    kexGame::cLocal->PlaySound("sounds/select.wav");
}

//
// kexMenuPause::OnMainMenu
//

void kexMenuPause::OnMainMenu(kexMenuObject *menuObject)
{
    kexGame::cLocal->SetMenu(MENU_MAINCONFIRM);
    kexGame::cLocal->PlaySound("sounds/select.wav");
}

//
// kexMenuPause::OnOptions
//

void kexMenuPause::OnOptions(kexMenuObject *menuObject)
{
    kexGame::cLocal->SetMenu(MENU_OPTIONS);
    kexGame::cLocal->PlaySound("sounds/select.wav");
}

//
// kexMenuPause::OnQuit
//

void kexMenuPause::OnQuit(kexMenuObject *menuObject)
{
    kexGame::cLocal->SetMenu(MENU_QUITCONFIRM);
    kexGame::cLocal->PlaySound("sounds/select.wav");
}

//
// kexMenuPause::Display
//

void kexMenuPause::Display(void)
{
    kexRender::cScreen->SetOrtho();
    
    kexRender::cScreen->DrawStretchPic(kexRender::cTextures->whiteTexture, 0, 0,
        (float)kexRender::cScreen->SCREEN_WIDTH,
        (float)kexRender::cScreen->SCREEN_HEIGHT, 0, 0, 0, 128);

    kexGame::cMenuPanel->DrawPanel(80, 48, 160, 128, 4);
    kexGame::cMenuPanel->DrawInset(88, 56, 142, 16);

    DrawItems();

    kexGame::cLocal->DrawSmallString(kexGame::cLocal->Translation()->GetString(48), 160, 60, 1, true);
}

//
// kexMenuPause::ProcessInput
//

bool kexMenuPause::ProcessInput(inputEvent_t *ev)
{
    if(kexGame::cLocal->ButtonEvent() & GBE_MENU_BACK)
    {
        kexGame::cLocal->ButtonEvent() = 0;
        kexGame::cLocal->ClearMenu();
        return true;
    }
    
    return false;
}

//-----------------------------------------------------------------------------
//
// kexMenuOptions
//
//-----------------------------------------------------------------------------

DEFINE_MENU_CLASS(kexMenuOptions);
public:
    virtual void                    Init(void);
    virtual void                    Display(void);
    virtual void                    Update(void);
    virtual bool                    ProcessInput(inputEvent_t *ev);

private:
    void                            OnBack(kexMenuObject *menuObject);
    void                            OnInput(kexMenuObject *menuObject);
END_MENU_CLASS();

DECLARE_MENU_CLASS(kexMenuOptions, MENU_OPTIONS);

//
// kexMenuOptions::Init
//

void kexMenuOptions::Init(void)
{
    kexMenuObjectPanelSelect *item;
    kexMenuObjectButton *button;
    const char *labels[] =
    {
        "Gameplay",
        "Input",
        "Graphics",
        "Audio"
    };

    for(int i = 0; i < ARRLEN(labels); ++i)
    {
        item = static_cast<kexMenuObjectPanelSelect*>(AllocateMenuObject("kexMenuObjectPanelSelect"));
        item->x = 88;
        item->y = 88 + (16 * (float)i);
        item->w = 142;
        item->h = 16;
        item->label = labels[i];
        item->textAlignment = kexMenuObject::MITA_CENTER;
    }

    menuObjects[1]->Callback = static_cast<selectCallback_t>(&kexMenuOptions::OnInput);

    button = static_cast<kexMenuObjectButton*>(AllocateMenuObject("kexMenuObjectButton"));

    button->x = 112;
    button->y = 160;
    button->w = 96;
    button->h = 24;
    button->label = "Back";
    button->textAlignment = kexMenuObject::MITA_CENTER;
    button->Callback = static_cast<selectCallback_t>(&kexMenuOptions::OnBack);
}

//
// kexMenuOptions::Update
//

void kexMenuOptions::Update(void)
{
    UpdateItems();
}

//
// kexMenuOptions::OnBack
//

void kexMenuOptions::OnBack(kexMenuObject *menuObject)
{
    kexGame::cLocal->ClearMenu();
    kexGame::cLocal->PlaySound("sounds/select.wav");
}

//
// kexMenuOptions::OnInput
//

void kexMenuOptions::OnInput(kexMenuObject *menuObject)
{
    kexGame::cLocal->SetMenu(MENU_INPUT);
    kexGame::cLocal->PlaySound("sounds/select.wav");
}

//
// kexMenuOptions::Display
//

void kexMenuOptions::Display(void)
{
    kexRender::cScreen->SetOrtho();
    
    kexRender::cScreen->DrawStretchPic(kexRender::cTextures->whiteTexture, 0, 0,
        (float)kexRender::cScreen->SCREEN_WIDTH,
        (float)kexRender::cScreen->SCREEN_HEIGHT, 0, 0, 0, 128);

    kexGame::cMenuPanel->DrawPanel(80, 48, 160, 148, 4);
    kexGame::cMenuPanel->DrawInset(88, 56, 142, 16);

    DrawItems();

    kexGame::cLocal->DrawSmallString("Options", 160, 60, 1, true);
}

//
// kexMenuOptions::ProcessInput
//

bool kexMenuOptions::ProcessInput(inputEvent_t *ev)
{
    if(kexGame::cLocal->ButtonEvent() & GBE_MENU_BACK)
    {
        kexGame::cLocal->ButtonEvent() = 0;
        kexGame::cLocal->ClearMenu();
        return true;
    }
    
    return false;
}

//-----------------------------------------------------------------------------
//
// kexMenuInput
//
//-----------------------------------------------------------------------------

DEFINE_MENU_CLASS(kexMenuInput);
public:
    virtual void                    Init(void);
    virtual void                    Display(void);
    virtual void                    Update(void);
    virtual bool                    ProcessInput(inputEvent_t *ev);

private:
    void                            OnBack(kexMenuObject *menuObject);
    void                            OnBindings(kexMenuObject *menuObject);
END_MENU_CLASS();

DECLARE_MENU_CLASS(kexMenuInput, MENU_INPUT);

//
// kexMenuInput::Init
//

void kexMenuInput::Init(void)
{
    kexMenuObjectPanelSelect *item;
    kexMenuObjectButton *button;
    const char *labels[] =
    {
        "Bindings",
        "Mouse",
        "Gamepad"
    };

    for(int i = 0; i < ARRLEN(labels); ++i)
    {
        item = static_cast<kexMenuObjectPanelSelect*>(AllocateMenuObject("kexMenuObjectPanelSelect"));
        item->x = 88;
        item->y = 88 + (16 * (float)i);
        item->w = 142;
        item->h = 16;
        item->label = labels[i];
        item->textAlignment = kexMenuObject::MITA_CENTER;
    }

    menuObjects[0]->Callback = static_cast<selectCallback_t>(&kexMenuInput::OnBindings);

    button = static_cast<kexMenuObjectButton*>(AllocateMenuObject("kexMenuObjectButton"));

    button->x = 112;
    button->y = 160;
    button->w = 96;
    button->h = 24;
    button->label = "Back";
    button->textAlignment = kexMenuObject::MITA_CENTER;
    button->Callback = static_cast<selectCallback_t>(&kexMenuInput::OnBack);
}

//
// kexMenuInput::Update
//

void kexMenuInput::Update(void)
{
    UpdateItems();
}

//
// kexMenuInput::OnBack
//

void kexMenuInput::OnBack(kexMenuObject *menuObject)
{
    kexGame::cLocal->ClearMenu();
    kexGame::cLocal->PlaySound("sounds/select.wav");
}

//
// kexMenuInput::OnBindings
//

void kexMenuInput::OnBindings(kexMenuObject *menuObject)
{
    kexGame::cLocal->SetMenu(MENU_BINDINGS);
    kexGame::cLocal->PlaySound("sounds/select.wav");
}

//
// kexMenuInput::Display
//

void kexMenuInput::Display(void)
{
    kexRender::cScreen->SetOrtho();
    
    kexRender::cScreen->DrawStretchPic(kexRender::cTextures->whiteTexture, 0, 0,
        (float)kexRender::cScreen->SCREEN_WIDTH,
        (float)kexRender::cScreen->SCREEN_HEIGHT, 0, 0, 0, 128);

    kexGame::cMenuPanel->DrawPanel(80, 48, 160, 148, 4);
    kexGame::cMenuPanel->DrawInset(88, 56, 142, 16);

    DrawItems();

    kexGame::cLocal->DrawSmallString("Input", 160, 60, 1, true);
}

//
// kexMenuInput::ProcessInput
//

bool kexMenuInput::ProcessInput(inputEvent_t *ev)
{
    if(kexGame::cLocal->ButtonEvent() & GBE_MENU_BACK)
    {
        kexGame::cLocal->ButtonEvent() = 0;
        kexGame::cLocal->ClearMenu();
        return true;
    }
    
    return false;
}

//-----------------------------------------------------------------------------
//
// kexMenuMainMenuConfirm
//
//-----------------------------------------------------------------------------

DEFINE_MENU_CLASS(kexMenuMainMenuConfirm);
public:
    virtual void                    Init(void);
    virtual void                    Display(void);
    virtual void                    Update(void);
    virtual bool                    ProcessInput(inputEvent_t *ev);

private:
    void                            OnYes(kexMenuObject *menuObject);
    void                            OnNo(kexMenuObject *menuObject);
END_MENU_CLASS();

DECLARE_MENU_CLASS(kexMenuMainMenuConfirm, MENU_MAINCONFIRM);

//
// kexMenuMainMenuConfirm::Init
//

void kexMenuMainMenuConfirm::Init(void)
{
    kexMenuObjectButton *item1, *item2;
    
    item1 = static_cast<kexMenuObjectButton*>(AllocateMenuObject("kexMenuObjectButton"));
    item2 = static_cast<kexMenuObjectButton*>(AllocateMenuObject("kexMenuObjectButton"));

    item1->x = 40;
    item1->y = 120;
    item1->w = 96;
    item1->h = 24;
    item1->label = "Yes";
    item1->textAlignment = kexMenuObject::MITA_CENTER;
    item1->Callback = static_cast<selectCallback_t>(&kexMenuMainMenuConfirm::OnYes);

    item2->x = 182;
    item2->y = 120;
    item2->w = 96;
    item2->h = 24;
    item2->label = "No";
    item2->textAlignment = kexMenuObject::MITA_CENTER;
    item2->Callback = static_cast<selectCallback_t>(&kexMenuMainMenuConfirm::OnNo);
}

//
// kexMenuMainMenuConfirm::Update
//

void kexMenuMainMenuConfirm::Update(void)
{
    UpdateItems();
}

//
// kexMenuMainMenuConfirm::Display
//

void kexMenuMainMenuConfirm::Display(void)
{
    kexRender::cScreen->SetOrtho();
    
    kexRender::cScreen->DrawStretchPic(kexRender::cTextures->whiteTexture, 0, 0,
        (float)kexRender::cScreen->SCREEN_WIDTH,
        (float)kexRender::cScreen->SCREEN_HEIGHT, 0, 0, 0, 128);
    
    kexGame::cMenuPanel->DrawPanel(32, 64, 256, 96, 4);
    kexGame::cMenuPanel->DrawInset(40, 72, 238, 32);
    kexGame::cLocal->DrawSmallString("Quit to Main Menu?", 160, 82, 1, true);

    DrawItems();
}

//
// kexMenuMainMenuConfirm::ProcessInput
//

bool kexMenuMainMenuConfirm::ProcessInput(inputEvent_t *ev)
{
    if(kexGame::cLocal->ButtonEvent() & GBE_MENU_BACK)
    {
        kexGame::cLocal->ButtonEvent() = 0;
        kexGame::cLocal->ClearMenu();
        return true;
    }

    return false;
}

//
// kexMenuMainMenuConfirm::OnYes
//

void kexMenuMainMenuConfirm::OnYes(kexMenuObject *menuObject)
{
    kexGame::cLocal->PlaySound("sounds/select.wav");
    kexGame::cLocal->ClearMenu(true);
    kexGame::cLocal->SetGameState(GS_TITLE);
}

//
// kexMenuMainMenuConfirm::OnNo
//

void kexMenuMainMenuConfirm::OnNo(kexMenuObject *menuObject)
{
    kexGame::cLocal->ClearMenu();
    kexGame::cLocal->PlaySound("sounds/select.wav");
}

//-----------------------------------------------------------------------------
//
// kexMenuItem
//
//-----------------------------------------------------------------------------

DECLARE_KEX_CLASS(kexMenuItem, kexObject)

//
// kexMenuItem::kexMenuItem
//

kexMenuItem::kexMenuItem(void)
{
    this->x             = 0;
    this->y             = 0;
    this->scale         = 1;
    this->bLerping      = false;
    this->bHighLighted  = false;
    this->bInteract     = true;
    this->bDisabled     = false;
    this->destX         = 0;
    this->destY         = 0;
    this->time          = 0;
    this->highlightTime = 0;
    this->lerpCallback  = NULL;
}

//
// kexMenuItem::kexMenuItem
//

kexMenuItem::kexMenuItem(const char *label, const float x, const float y,
                         const float scale, menuItemLerpDone_t callback)
{
    this->x             = x;
    this->y             = y;
    this->scale         = scale;
    this->bLerping      = false;
    this->bInteract     = true;
    this->destX         = 0;
    this->destY         = 0;
    this->time          = 0;
    this->label         = label;
    this->highlightTime = 0;
    this->lerpCallback  = callback;
}

//
// kexMenuItem::~kexMenuItem
//

kexMenuItem::~kexMenuItem(void)
{
}

//
// kexMenuItem::LerpTo
//

void kexMenuItem::LerpTo(const float destx, const float desty)
{
    this->bLerping = true;
    this->startX = x;
    this->startY = y;
    this->destX = destx;
    this->destY = desty;
    this->time = 1;
    this->bInteract = false;
    this->bHighLighted = false;
    this->highlightTime = 0;
}

//
// kexMenuItem::LerpTo
//

void kexMenuItem::LerpTo(const float destx)
{
    LerpTo(destx, y);
}

//
// kexMenuItem::Move
//

void kexMenuItem::Move(void)
{
    float c = 1.0f - time;

    x = c * (destX - startX) + startX;
    y = c * (destY - startY) + startY;

    time -= 0.03f;

    if(time <= 0)
    {
        bLerping = false;
        bInteract = true;
        x = destX;
        y = destY;

        if(lerpCallback)
        {
            lerpCallback(this);
        }
    }
}

//
// kexMenuItem::OnCursor
//

bool kexMenuItem::OnCursor(void)
{
    float width = kexGame::cLocal->BigFont()->StringWidth(label.c_str(), scale, 0) * 0.5f;
    float height = kexGame::cLocal->BigFont()->StringHeight(label.c_str(), scale, 0) * 0.5f;
    float sy = y + height;
    float mx = (float)kex::cInput->MouseX();
    float my = (float)kex::cInput->MouseY();

    kexRender::cScreen->CoordsToRenderScreenCoords(mx, my);

    return !(mx < (x - width) || my < (sy - height) ||
             mx > (x + width) || my > (sy + height));
}

//
// kexMenuItem::Tick
//

void kexMenuItem::Tick(void)
{
    static kexMenuItem *lastItem;

    if(bLerping)
    {
        Move();
    }

    if(bInteract && !bSelected && !bDisabled)
    {
        if(!(bHighLighted = OnCursor()))
        {
            highlightTime = 0;

            if(lastItem == this)
            {
                lastItem = NULL;
            }
        }
        else
        {
            highlightTime += 1.0f;
        }
    }

    if(bHighLighted && lastItem != this)
    {
        lastItem = this;
        kexGame::cLocal->PlaySound("sounds/menu_highlight.wav");
    }
}

//
// kexMenuItem::DrawSmallString
//

void kexMenuItem::DrawSmallString(const char *string, float x, float y, float scale, bool center, bool flash)
{
    byte c = (flash || bSelected || bDisabled) ? 255 : 224;
    kexFont *font = kexGame::cLocal->SmallFont();

    kexGame::cLocal->DrawSmallString(string, x, y, scale, center, c, c, c);

    if(flash)
    {
        byte pulse = (byte)(kexMath::Sin(highlightTime * 0.1f) * 64.0f) + 64;

        kexRender::cBackend->SetBlend(GLSRC_ONE, GLDST_ONE);
        font->DrawString(string, x, y, scale, center, RGBA(pulse, pulse, pulse, 0xff));
    }
}

//
// kexMenuItem::DrawBigString
//

void kexMenuItem::DrawBigString(const char *string, float x, float y, float scale, bool center, bool flash)
{
    byte c = (flash || bSelected || bDisabled) ? 255 : 224;
    kexFont *font = kexGame::cLocal->BigFont();

    kexGame::cLocal->DrawBigString(string, x, y, scale, center, c, c, c);

    if(flash)
    {
        byte pulse = (byte)(kexMath::Sin(highlightTime * 0.1f) * 64.0f) + 64;

        kexRender::cBackend->SetBlend(GLSRC_ONE, GLDST_ONE);
        font->DrawString(string, x, y, scale, center, RGBA(pulse, pulse, pulse, 0xff));
    }
}

//
// kexMenuItem::Draw
//

void kexMenuItem::Draw(void)
{
    DrawBigString(label.c_str(), x, y, scale, true, bHighLighted);
}

//
// kexMenuItem::operator=
//

kexMenuItem &kexMenuItem::operator=(const kexMenuItem &item)
{
    this->x             = item.x;
    this->y             = item.y;
    this->scale         = item.scale;
    this->label         = item.label;
    this->bInteract     = item.bInteract;
    this->bLerping      = item.bLerping;

    return *this;
}

//-----------------------------------------------------------------------------
//
// kexMenuItemLabel
//
//-----------------------------------------------------------------------------

DECLARE_KEX_CLASS(kexMenuItemLabel, kexMenuItem)

//
// kexMenuItemLabel::kexMenuItemLabel
//

kexMenuItemLabel::kexMenuItemLabel(void)
{
    this->x             = 0;
    this->y             = 0;
    this->scale         = 1;
    this->bLerping      = false;
    this->bHighLighted  = false;
    this->bInteract     = false;
    this->bDisabled     = false;
    this->destX         = 0;
    this->destY         = 0;
    this->time          = 0;
    this->highlightTime = 0;
    this->lerpCallback  = NULL;
}

//
// kexMenuItemLabel::kexMenuItemLabel
//

kexMenuItemLabel::kexMenuItemLabel(const char *label, const float x, const float y, const float scale)
{
    this->x             = x;
    this->y             = y;
    this->scale         = scale;
    this->bLerping      = false;
    this->bInteract     = false;
    this->destX         = 0;
    this->destY         = 0;
    this->time          = 0;
    this->label         = label;
    this->highlightTime = 0;
    this->lerpCallback  = NULL;
}

//
// kexMenuItemLabel::~kexMenuItemLabel
//

kexMenuItemLabel::~kexMenuItemLabel(void)
{
}

//
// kexMenuItemLabel::Tick
//

void kexMenuItemLabel::Tick(void)
{
    if(bLerping)
    {
        Move();
    }
}

//
// kexMenuItemLabel::Draw
//

void kexMenuItemLabel::Draw(void)
{
    DrawBigString(label.c_str(), x, y, scale, true, false);
}

//-----------------------------------------------------------------------------
//
// kexMenuItemSlider
//
//-----------------------------------------------------------------------------

DECLARE_KEX_CLASS(kexMenuItemSlider, kexMenuItem)
#define MAX_MENU_SLIDER_BARS    24

//
// kexMenuItemSlider::kexMenuItemSlider
//

kexMenuItemSlider::kexMenuItemSlider(void)
{
    this->x             = 0;
    this->y             = 0;
    this->scale         = 1;
    this->bLerping      = false;
    this->bHighLighted  = false;
    this->bInteract     = true;
    this->bDisabled     = false;
    this->destX         = 0;
    this->destY         = 0;
    this->time          = 0;
    this->highlightTime = 0;
    this->lerpCallback  = NULL;
}

//
// kexMenuItemSlider::kexMenuItemSlider
//

kexMenuItemSlider::kexMenuItemSlider(const float x, const float y, const float scale, kexCvar &cvar)
{
    this->x             = x;
    this->y             = y;
    this->scale         = scale;
    this->bLerping      = false;
    this->bInteract     = true;
    this->destX         = 0;
    this->destY         = 0;
    this->time          = 0;
    this->label         = "";
    this->highlightTime = 0;
    this->numBars       = 0;
    this->lerpCallback  = NULL;
    this->cvar          = &cvar;
}

//
// kexMenuItemSlider::~kexMenuItemSlider
//

kexMenuItemSlider::~kexMenuItemSlider(void)
{
}

//
// kexMenuItemSlider::Select
//

void kexMenuItemSlider::Select(const bool b)
{
    kexMenuItem::Select(b);
}

//
// kexMenuItemSlider::OnCursor
//

bool kexMenuItemSlider::OnCursor(void)
{
    float width = 2.5f * MAX_MENU_SLIDER_BARS;
    float height = kexGame::cLocal->BigFont()->StringHeight(label.c_str(), scale, 0) * 0.5f;
    float sy = y + height;
    float mx = (float)kex::cInput->MouseX();
    float my = (float)kex::cInput->MouseY();
    
    kexRender::cScreen->CoordsToRenderScreenCoords(mx, my);
    
    return !(mx < (x - width) || my < (sy - height) ||
             mx > (x + width) || my > (sy + height));
}

//
// kexMenuItemSlider::Tick
//

void kexMenuItemSlider::Tick(void)
{
    int bars = 0;
    
    kexMenuItem::Tick();
    
    if(bSelected)
    {
        float width = (kexGame::cLocal->BigFont()->StringWidth(label.c_str(), scale, 0) * 0.5f);
        float mx = (float)kex::cInput->MouseX();
        float my = (float)kex::cInput->MouseY();
        float val;
        
        kexRender::cScreen->CoordsToRenderScreenCoords(mx, my);
        
        val = ((mx-x)/width) * 0.5f + 0.5f;
        kexMath::Clamp(val, 0, 1);
        
        cvar->Set(cvar->GetMax() * val);
        
        bars = (int)kexMath::Floor(MAX_MENU_SLIDER_BARS * val);
        kexMath::Clamp(bars, 0, MAX_MENU_SLIDER_BARS);

        if(numBars != bars)
        {
            kexGame::cLocal->PlaySound("sounds/select.wav");
        }

        numBars = bars;
    }
    else
    {
        bars = (int)kexMath::Floor(MAX_MENU_SLIDER_BARS * (cvar->GetFloat() / cvar->GetMax()));
    }
    
    label.Clear();
    
    for(int i = 0; i < bars; ++i)
    {
        label += "/";
    }
    for(int i = bars; i < MAX_MENU_SLIDER_BARS; ++i)
    {
        label += ".";
    }
}

//
// kexMenuItemSlider::Draw
//

void kexMenuItemSlider::Draw(void)
{
    DrawBigString(label, x, y, scale, true, bHighLighted);
}
