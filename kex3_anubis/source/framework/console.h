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

#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#define CON_MAX_HISTORY     16
#define CON_BUFFER_SIZE     256
#define CON_STATE_DOWN      0
#define CON_STATE_UP        1
#define CON_BLINK_TIME      350
#define CON_INPUT_LENGTH    512
#define CON_LINE_LENGTH     512

#include "renderFont.h"

class kexConsole
{
public:
    kexConsole(void);
    ~kexConsole(void);

    void                Init(void);
    void                Clear(void);
    void                Print(rcolor color, const char *text);
    bool                ProcessInput(const inputEvent_t *ev);
    void                Tick(void);
    void                Draw(void);

    const bool          IsActive(void) const { return (state == CON_STATE_DOWN); }
    kexFont             *Font(void) { return font; }

private:
    void                ClearOutput(void);
    void                OutputTextLine(rcolor color, const char *text);
    void                AddToHistory(void);
    void                GetHistory(bool bPrev);
    void                LineScroll(bool dir);
    void                MoveTypePos(bool dir);
    void                BackSpace(void);
    void                DeleteChar(void);
    void                CheckShift(const inputEvent_t *ev);
    void                CheckStickyKeys(const inputEvent_t *ev);
    void                StickyKeyTick(void);
    void                UpdateBlink(void);
    void                ParseKey(int c);
    void                ParseInput(void);
    void                HandlePaste(void);
    
    void                SetInputText(const char *string) { textBuffer = string; }
    void                ResetInputText(void) { typeStrPos = 0; scrollBackPos = 0; textBuffer.Clear(); }
    
    char                scrollBackStr[CON_BUFFER_SIZE][CON_LINE_LENGTH];
    unsigned int        scrollBackPos;
    unsigned int        scrollBackLines;
    rcolor              lineColor[CON_BUFFER_SIZE];
    int                 historyTop;
    int                 historyCur;
    char                history[CON_MAX_HISTORY][CON_INPUT_LENGTH];
    kexStr              textBuffer;
    int                 typeStrPos;
    bool                bShiftDown;
    bool                bCapsDown;
    bool                bCtrlDown;
    int                 state;
    int                 blinkTime;
    bool                bKeyHeld;
    bool                bStickyActive;
    int                 lastKeyPressed;
    int                 timePressed;
    bool                bShowPrompt;
    int                 outputLength;
    kexFont             *font;
    char                shiftcode[256];
};

#endif
