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

#ifndef __GAME_H__
#define __GAME_H__

class kexFont;
class kexTitleScreen;
class kexTranslation;

class kexGame
{
public:
    kexGame(void);
    ~kexGame(void);

    void                Init(void);
    void                Tick(void);
    void                Draw(void);
    bool                ProcessInput(inputEvent_t *ev);
    void                ProcessActions(inputEvent_t *ev);

    kexTitleScreen      *TitleScreen(void) { return titleScreen; }
    kexTranslation      *Translation(void) { return translation; }
    kexFont             *SmallFont(void) { return smallFont; }
    kexFont             *BigFont(void) { return bigFont; }
    const int           GetTicks(void) const { return ticks; }

    void                DrawSmallString(const char *string, float x, float y, float scale, bool center,
                                        byte r = 0xff, byte g = 0xff, byte b = 0xff);
    void                DrawBigString(const char *string, float x, float y, float scale, bool center,
                                        byte r = 0xff, byte g = 0xff, byte b = 0xff);

private:
    kexFont             *smallFont;
    kexFont             *bigFont;
    kexTitleScreen      *titleScreen;
    kexTranslation      *translation;
    int                 ticks;
};

#endif
