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
//      Game session
//

#include "kexlib.h"
#include "renderMain.h"

static kexSession sessionLocal;
kexSession *kex::cSession = &sessionLocal;

kexCvar cvarClientFPS("cl_maxfps", CVF_INT|CVF_CONFIG, "60", 1, 60, "Game render FPS");

//
// pausegame
//

COMMAND(pausegame)
{
    if(kex::cCommands->GetArgc() < 1)
    {
        return;
    }

    sessionLocal.TogglePause(sessionLocal.IsPaused() ^ 1);
}

//
// kexSession::kexSession
//

kexSession::kexSession(void)
{
    this->gameTimeMS    = 0;
    this->fps           = 0;
    this->time          = 0;
    this->curtime       = 0;
    this->deltaTime     = 0;
    this->ticks         = 0;
    this->bPaused       = false;
    this->bShowCursor   = false;
}

//
// kexSession::~kexSession
//

kexSession::~kexSession(void)
{
}

//
// kexSession::ProcessEvents
//

void kexSession::ProcessEvents(void)
{
    inputEvent_t *ev;

    kex::cInput->PollInput();

    while((ev = eventQueue.Pop()) != NULL)
    {
        if(kex::cConsole->ProcessInput(ev))
        {
            continue;
        }

        if(kex::cGame->ProcessInput(ev))
        {
            continue;
        }
    }
}

//
// kexSession::RunFrame
//

void kexSession::RunFrame(void)
{
    kex::cConsole->Tick();
    kex::cGame->Tick();
}

//
// kexSession::DrawFrame
//

void kexSession::DrawFrame(void)
{
    kexRender::cBackend->ClearBuffer();
    
    kex::cGame->Draw();
    kex::cConsole->Draw();

    DrawCursor();
    
    kexRender::cBackend->SwapBuffers();
}

//
// kexSession::DrawCursor
//

void kexSession::DrawCursor(void)
{
    float mx;
    float my;
    float mw;
    float mh;
    
    if(!bShowCursor)
    {
        return;
    }
    
    mx = (float)kex::cInput->MouseX();
    my = (float)kex::cInput->MouseY();
    mw = (float)cursorTexture->Width();
    mh = (float)cursorTexture->Height();
    
    kexRender::cBackend->SetOrtho();
    kexRender::cScreen->DrawStretchPic(cursorTexture, mx, my, mw, mh);
}

//
// kexSession::InitCursor
//

void kexSession::InitCursor(void)
{
    cursorTexture = kexRender::cTextures->Cache("gfx/cursor.png", TC_CLAMP, TF_NEAREST);
    bShowCursor = true;
}

//
// kexSession::Shutdown
//

void kexSession::Shutdown(void)
{
    kex::cGame->Shutdown();
}

//
// kexSession::RunGame
//

void kexSession::RunGame(void)
{
    static int clockspeed = kexMath::FrameSec(60);
    int msec;
    int framemsec = 0;
    int prevmsec;
    int nextmsec;
    int ticsToRun = 0;

    // setup mouse cursor
    InitCursor();

    // begin the core game logic
    kex::cGame->Start();

    prevmsec = kex::cTimer->GetMS();

    while(1)
    {
        do
        {
            nextmsec = kex::cTimer->GetMS();
            msec = nextmsec - prevmsec;

            if(msec < 1 && ticsToRun <= 0 && fps >= 60)
            {
                kex::cTimer->Sleep(1);
            }
        }
        while(msec < 1);

        curtime += msec;

        if(curtime >= kexMath::FrameSec(cvarClientFPS.GetInt()))
        {
            deltaTime = kexMath::MSec2Sec((float)curtime);
            kexMath::Clamp(deltaTime, 0.0f, 1.0f);

            fps = (int)kexMath::FrameSec(kexMath::Sec2MSec(deltaTime));

            time += curtime;
            curtime = 0;

            do
            {
                // check for new inputs
                ProcessEvents();

                // process game logic
                RunFrame();
            } while(--ticsToRun > 0);

            // draw scene
            DrawFrame();

            int afterms = kexMath::Sec2MSec(kex::cTimer->GetMS());
            ticsToRun = kexMath::MSec2Sec((afterms - framemsec) / clockspeed);

            // update ticks
            UpdateTicks();
            framemsec = kexMath::Sec2MSec(kex::cTimer->GetMS());
        }

        prevmsec = nextmsec;
        Mem_GC();
    }
}
