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
#define CON_BUFFER_SIZE     64
#define CON_STATE_DOWN      0
#define CON_STATE_UP        1
#define CON_BLINK_TIME      350
#define CON_INPUT_LENGTH    512
#define CON_LINE_LENGTH     512

class kexConsole
{
public:
    kexConsole(void);
    ~kexConsole(void);

    void                ClearOutput(void);
    void                Clear(void);
    void                OutputTextLine(rcolor color, const char *text);
    void                AddToHistory(void);
    void                GetHistory(bool bPrev);
    void                Print(rcolor color, const char *text);
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
    bool                ProcessInput(const inputEvent_t *ev);
    void                Tick(void);
    void                Draw(void);

    void                SetInputText(const char *string) { strcpy(typeStr, string); }
    void                ResetInputText(void) { typeStr[0] = '\0'; typeStrPos = 0; }
    const bool          IsActive(void) const { return (state == CON_STATE_DOWN); }

private:
    char                scrollBackStr[CON_BUFFER_SIZE][CON_LINE_LENGTH];
    unsigned int        scrollBackPos;
    unsigned int        scrollBackLines;
    rcolor              lineColor[CON_BUFFER_SIZE];
    int                 historyTop;
    int                 historyCur;
    char                history[CON_MAX_HISTORY][CON_INPUT_LENGTH];
    char                typeStr[CON_INPUT_LENGTH];
    int                 typeStrPos;
    bool                bShiftDown;
    bool                bCtrlDown;
    int                 state;
    int                 blinkTime;
    bool                bKeyHeld;
    bool                bStickyActive;
    int                 lastKeyPressed;
    int                 timePressed;
    bool                bShowPrompt;
    int                 outputLength;
};

#endif
