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
//      Console System
//

#include "kexlib.h"
#include "renderMain.h"
#include "renderFont.h"

kexCvar cvarDisplayConsole("con_alwaysShowConsole", CVF_BOOL|CVF_CONFIG, "0", "TODO");
kexCvar cvarShowFPS("con_showfps", CVF_BOOL|CVF_CONFIG, "0", "Displays current FPS");
kexCvar cvarStickyKeySpeed("con_stickySpeed", CVF_BOOL|CVF_CONFIG, "500", "TODO");

static kexConsole consoleLocal;
kexConsole *kex::cConsole = &consoleLocal;

//
// clear
//

COMMAND(clear)
{
    consoleLocal.Clear();
}

//
// help
//

COMMAND(help)
{
    kex::cCommands->Execute("listcmds");
    kex::cSystem->Printf("\n");
    kex::cCommands->Execute("listvars");
}

//
// kexConsole::kexConsole
//

kexConsole::kexConsole(void)
{
    this->scrollBackPos     = 0;
    this->historyTop        = 0;
    this->historyCur        = -1;
    this->typeStrPos        = 0;
    this->scrollBackLines   = 0;
    this->bShiftDown        = false;
    this->bCapsDown         = false;
    this->bCtrlDown         = false;
    this->state             = CON_STATE_UP;
    this->blinkTime         = 0;
    this->bKeyHeld          = false;
    this->bStickyActive     = false;
    this->lastKeyPressed    = 0;
    this->timePressed       = 0;
    this->bShowPrompt       = true;
    this->outputLength      = 0;

    ClearOutput();
}

//
// kexConsole::~kexConsole
//

kexConsole::~kexConsole(void)
{
    ClearOutput();
}

//
// kexConsole::Init
//

void kexConsole::Init(void)
{
    font = kexFont::Alloc("confont");
    memset(shiftcode, ' ', 256);

    shiftcode['1']  = '!';
    shiftcode['2']  = '@';
    shiftcode['3']  = '#';
    shiftcode['4']  = '$';
    shiftcode['5']  = '%';
    shiftcode['6']  = '^';
    shiftcode['7']  = '&';
    shiftcode['8']  = '*';
    shiftcode['9']  = '(';
    shiftcode['0']  = ')';
    shiftcode['-']  = '_';
    shiftcode['=']  = '+';
    shiftcode['[']  = '{';
    shiftcode[']']  = '}';
    shiftcode['\\'] = '|';
    shiftcode[';']  = ':';
    shiftcode['\''] = '"';
    shiftcode[',']  = '<';
    shiftcode['.']  = '>';
    shiftcode['/']  = '?';
    shiftcode['`']  = '~';

    for(int c = 'a'; c <= 'z'; c++)
    {
        shiftcode[c] = toupper(c);
    }
}

//
// kexConsole::ClearOutput
//

void kexConsole::ClearOutput(void)
{
    for(int i = 0; i < CON_BUFFER_SIZE; i++)
    {
        memset(this->scrollBackStr[i], 0, CON_LINE_LENGTH);
        lineColor[i] = COLOR_WHITE;
    }

    scrollBackLines = 0;
}

//
// kexConsole::Clear
//

void kexConsole::Clear(void)
{
    ClearOutput();
    ResetInputText();
}

//
// kexConsole::OutputTextLine
//

void kexConsole::OutputTextLine(rcolor color, const char *text)
{
    if(scrollBackLines >= CON_BUFFER_SIZE)
    {
        for(unsigned int i = 0; i < CON_BUFFER_SIZE-1; i++)
        {
            memset(scrollBackStr[i], 0, CON_LINE_LENGTH);
            strcpy(scrollBackStr[i], scrollBackStr[i+1]);
            lineColor[i] = lineColor[i+1];
        }

        scrollBackLines = CON_BUFFER_SIZE-1;
    }

    unsigned int len = strlen(text);
    if(len >= CON_LINE_LENGTH)
    {
        len = CON_LINE_LENGTH-1;
    }

    strncpy(scrollBackStr[scrollBackLines], text, len);
    scrollBackStr[scrollBackLines][len] = '\0';
    lineColor[scrollBackLines] = color;

    scrollBackLines++;
}

//
// kexConsole::AddToHistory
//

void kexConsole::AddToHistory(void)
{
    strncpy(history[historyTop], textBuffer, CON_INPUT_LENGTH);
    historyTop = (historyTop+1) % CON_MAX_HISTORY;
}

//
// kexConsole::GetHistory
//

void kexConsole::GetHistory(bool bPrev)
{
    const char *hist;

    if(!bPrev)
    {
        historyCur--;
        if(historyCur < 0)
        {
            historyCur = historyTop-1;
        }
    }
    else
    {
        historyCur++;
        if(historyCur >= historyTop)
        {
            historyCur = 0;
        }
    }

    ResetInputText();
    hist = history[historyCur];

    textBuffer = hist;
    typeStrPos = strlen(hist);
}

//
// kexConsole::Print
//

void kexConsole::Print(rcolor color, const char *text)
{
    int strLength = strlen(text);
    char *curText = (char*)text;
    char tmpChar[CON_LINE_LENGTH];

    while(strLength > 0)
    {
        int lineLength = kexStr::IndexOf(curText, "\n");

        if(lineLength == -1)
        {
            lineLength = strLength;
        }

        strncpy(tmpChar, curText, lineLength);
        tmpChar[lineLength] = '\0';
        OutputTextLine(color, tmpChar);

        curText = (char*)&text[lineLength+1];
        strLength -= (lineLength+1);
    }
}

//
// kexConsole::LineScroll
//

void kexConsole::LineScroll(bool dir)
{
    if(dir)
    {
        if(scrollBackPos < scrollBackLines)
        {
            scrollBackPos++;
        }
    }
    else
    {
        if(scrollBackPos > 0)
        {
            scrollBackPos--;
        }
    }
}

//
// kexConsole::BackSpace
//

void kexConsole::BackSpace(void)
{
    if(textBuffer.Length() <= 0 || typeStrPos <= 0)
    {
        return;
    }

    typeStrPos--;
    if(typeStrPos < 0)
    {
        typeStrPos = 0;
    }
    
    textBuffer.Remove(typeStrPos, typeStrPos+1);
}

//
// kexConsole::DeleteChar
//

void kexConsole::DeleteChar(void)
{
    if(textBuffer.Length() <= 0 || typeStrPos >= textBuffer.Length())
    {
        return;
    }
    
    textBuffer.Remove(typeStrPos, typeStrPos+1);
}

//
// kexConsole::MoveTypePos
//

void kexConsole::MoveTypePos(bool dir)
{
    if(dir)
    {
        int len = textBuffer.Length();
        typeStrPos++;
        if(typeStrPos >= len)
        {
            typeStrPos = len;
        }
    }
    else
    {
        typeStrPos--;
        if(typeStrPos <= 0)
        {
            typeStrPos = 0;
        }
    }
}

//
// kexConsole::CheckShift
//

void kexConsole::CheckShift(const inputEvent_t *ev)
{
    bShiftDown = kex::cInput->IsShiftDown(0);
}

//
// kexConsole::CheckStickyKeys
//

void kexConsole::CheckStickyKeys(const inputEvent_t *ev)
{
    if(kex::cInput->IsShiftDown(ev->data1) ||
        ev->data1 == KKEY_RETURN ||
        ev->data1 == KKEY_TAB)
    {
        return;
    }

    lastKeyPressed = ev->data1;

    switch(ev->type)
    {
    case ev_keydown:
        if(!bKeyHeld)
        {
            bKeyHeld = true;
            timePressed = kex::cTimer->GetMS();
        }
        break;
    case ev_keyup:
        bKeyHeld = false;
        timePressed = 0;
        bStickyActive = false;
        break;
    default:
        break;
    }
}

//
// kexConsole::HandlePaste
//

void kexConsole::HandlePaste(void)
{
    const char *buf = kex::cSystem->GetClipboardText();
    textBuffer.Insert(buf, typeStrPos);
    typeStrPos = textBuffer.Length();
}

//
// kexConsole::ParseKey
//

void kexConsole::ParseKey(int c)
{
    switch(c)
    {
    case KKEY_BACKSPACE:
        BackSpace();
        return;
    case KKEY_DELETE:
        DeleteChar();
        return;
    case KKEY_LEFT:
        MoveTypePos(0);
        return;
    case KKEY_RIGHT:
        MoveTypePos(1);
        return;
    case KKEY_PAGEUP:
        LineScroll(1);
        return;
    case KKEY_PAGEDOWN:
        LineScroll(0);
        return;
    case KKEY_END:
        typeStrPos = textBuffer.Length();
        return;
    case KKEY_HOME:
        typeStrPos = 0;
        return;
    case KKEY_v:
        if(kex::cInput->IsCtrlDown(0))
        {
            HandlePaste();
            return;
        }
        break;
    }
    
    if(typeStrPos+1 >= CON_INPUT_LENGTH)
    {
        return;
    }

    if(c >= KKEY_SPACE && c <= KKEY_z)
    {
        bool bCaps;

        bCaps = (bCapsDown && (c >= KKEY_a && c <= KKEY_z));

        if(bShiftDown ^ bCaps)
        {
            c = shiftcode[c];
        }
        
        textBuffer.Insert((const char*)&c, typeStrPos);
        typeStrPos++;
    }
}

//
// kexConsole::StickyKeyTick
//

void kexConsole::StickyKeyTick(void)
{
    if(!bStickyActive)
    {
        int stickyTime = cvarStickyKeySpeed.GetInt();
        if(stickyTime < 0)
        {
            stickyTime = 0;
        }

        if(bKeyHeld && ((kex::cTimer->GetMS() - timePressed) >= stickyTime))
        {
            bStickyActive = true;
        }
    }
    else
    {
        ParseKey(lastKeyPressed);
    }
}

//
// kexConsole::UpdateBlink
//

void kexConsole::UpdateBlink(void)
{
    if(blinkTime >= kex::cSession->GetTime())
    {
        return;
    }

    bShowPrompt = !bShowPrompt;
    blinkTime = kex::cSession->GetTime() + CON_BLINK_TIME;
}

//
// kexConsole::ParseInput
//

void kexConsole::ParseInput(void)
{
    if(typeStrPos <= 0 || textBuffer.Length() <= 0)
    {
        return;
    }

    OutputTextLine(RGBA(192, 192, 192, 255), textBuffer);
    kex::cCommands->Execute(textBuffer);
    AddToHistory();
    ResetInputText();

    historyCur = 0;
}

//
// kexConsole::ProcessInput
//

bool kexConsole::ProcessInput(const inputEvent_t *ev)
{
    if(ev->type == ev_mouseup ||
        ev->type == ev_mouse)
    {
        return false;
    }

    if(ev->type == ev_mousedown && state == CON_STATE_DOWN)
    {
        switch(ev->data1)
        {
        case KMSB_WHEEL_UP:
            LineScroll(1);
            return true;

        case KMSB_WHEEL_DOWN:
            LineScroll(0);
            return true;
        }
    }

    if(ev->type == ev_mousedown)
    {
        return false;
    }
    
    bCapsDown = kex::cInput->CapslockOn();

    CheckShift(ev);
    CheckStickyKeys(ev);

    int c = ev->data1;

    switch(state)
    {
    case CON_STATE_DOWN:
        if(ev->type == ev_keydown)
        {
            switch(c)
            {
            case KKEY_BACKQUOTE:
                state = CON_STATE_UP;
                return true;
            case KKEY_RETURN:
                ParseInput();
                return true;
            case KKEY_UP:
                GetHistory(false);
                return true;
            case KKEY_DOWN:
                GetHistory(true);
                return true;
            case KKEY_TAB:
                kex::cCvars->AutoComplete(textBuffer);
                kex::cCommands->AutoComplete(textBuffer);
                return true;
            default:
                ParseKey(c);
                return true;
            }

            return false;
        }
        break;
    case CON_STATE_UP:
        if(ev->type == ev_keydown)
        {
            switch(c)
            {
            case KKEY_BACKQUOTE:
                state = CON_STATE_DOWN;
                historyCur = 0;
                return true;
            default:
                break;
            }

            return false;
        }
        break;
    default:
        return false;
    }

    return false;
}

//
// kexConsole::Tick
//

void kexConsole::Tick(void)
{
    if(state == CON_STATE_UP)
    {
        return;
    }

    StickyKeyTick();
    UpdateBlink();
}

//
// kexConsole::Draw
//

void kexConsole::Draw(void)
{
    bool    bOverlay;
    float   w;
    float   h;
    rcolor  color;

    kexRender::cBackend->SetOrtho();
    kexRender::cBackend->SetState(GLSTATE_DEPTHTEST, false);
    kexRender::cBackend->SetBlend(GLSRC_SRC_ALPHA, GLDST_ONE_MINUS_SRC_ALPHA);

    w = (float)kex::cSystem->VideoWidth();

    if(cvarShowFPS.GetBool())
    {
        font->DrawString(kexStr::Format("fps: %i", kex::cSession->GetFPS()),
                                        w - 64, 32, 1, false);
    }

    if(state == CON_STATE_UP && !cvarDisplayConsole.GetBool())
    {
        return;
    }

    bOverlay = (state == CON_STATE_UP && cvarDisplayConsole.GetBool());

    h = (float)kex::cSystem->VideoHeight() * 0.6875f;

    if(!bOverlay)
    {
        kexCpuVertList *vl = kexRender::cVertList;
        kexStr versionStr;

        kexRender::cTextures->whiteTexture->Bind();
        vl->BindDrawPointers();

        // tint overlay
        vl->AddQuad(0, 0, w, h, 4, 8, 16, 192);
        // borders
        vl->AddQuad(0, h-17, w, 1, 0, 128, 255, 255);
        vl->AddQuad(0, h, w, 1, 0, 128, 255, 255);

        vl->DrawElements();

        versionStr = kexStr::Format("%s %i.%i (%s)",
                                    KEX_ENGINE_TITLE,
                                    KEX_ENGINE_VERSION,
                                    KEX_ENGINE_SUBVERSION,
                                    KEX_ENGINE_BRANCH);

        color = RGBA(255, 0, 0, 255);
        font->DrawString(versionStr, w - font->StringWidth(versionStr, 1, 0) - 8, h-34, 1, false, color);

        color = RGBA(255, 255, 255, 255);
        font->DrawString("> ", 0, h-15, 1, false);

        if(bShowPrompt)
        {
            float pos = 16;
            if(typeStrPos > 0)
            {
                pos += font->StringWidth(textBuffer, 1.0f, typeStrPos);
            }
            font->DrawString("_", pos, h-15, 1, false);
        }

        if(textBuffer.Length() > 0)
        {
            font->DrawString(textBuffer, 16, h-15, 1, false);
        }
    }

    if(scrollBackLines > 0)
    {
        float scy = h-34;

        for(int i = scrollBackLines-(scrollBackPos)-1; i >= 0; i--)
        {
            if(scy < 0)
            {
                break;
            }

            color = lineColor[i];
            font->DrawString(scrollBackStr[i], 0, scy, 1, false, color);
            scy -= 16;
        }
    }
}
