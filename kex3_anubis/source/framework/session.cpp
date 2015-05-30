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

static const float clockspeed = kexMath::FrameSec(60.0f);

kexCvar cvarClientFPS("cl_maxfps", CVF_INT|CVF_CONFIG, "60", 1, 60, "Game render FPS");

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
    this->bShowCursor   = false;

    this->eventQueue.Init(4096);
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

    kexHeap::DrawHeapInfo();

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
    kexRender::cBackend->SetState(GLSTATE_BLEND, true);
    kexRender::cBackend->SetBlend(GLSRC_SRC_ALPHA, GLDST_ONE_MINUS_SRC_ALPHA);
    kexRender::cScreen->DrawTexture(cursorTexture, mx, my, mw, mh);
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
    kex::cGame->Stop();
}

//
// kexSession::GetNextTickCount
//
// Determines how many game ticks to run for
// the next frame
//

int kexSession::GetNextTickCount(void)
{
    static int framemsec = 0;
    static float leftOverTime = 0;
    int afterms, ticsToRun;
    float t;

    afterms = kexMath::Sec2MSec(kex::cTimer->GetMS()) - framemsec;
    t = kexMath::MSec2Sec((float)afterms) / clockspeed;

    ticsToRun = (int)t;

    if(kexMath::Sec2MSec(deltaTime) <= clockspeed)
    {
        leftOverTime = 0;
        ticsToRun = 0;
    }
    else
    {
        // accumulate the remainder
        leftOverTime += (t - ticsToRun);

        if(leftOverTime < 0)
        {
            // don't go under
            leftOverTime = 0;
        }

        // add another game tick when the remainder is equal
        // to a full tick
        if(leftOverTime >= 1.0f)
        {
            ticsToRun++;
            leftOverTime -= 1.0f;
        }
    }

    framemsec = kexMath::Sec2MSec(kex::cTimer->GetMS());
    if(ticsToRun > 60)
    {
        ticsToRun = 1;
    }
    return ticsToRun;
}

//
// kexSession::RunGame
//

void kexSession::RunGame(void)
{
    int msec = 0;
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
        }
        while(msec < 1);

        curtime += msec;

        if(cvarClientFPS.GetInt() > 60)
        {
            cvarClientFPS.Set(60);
        }

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

                // update ticks
                UpdateTicks();
            } while(--ticsToRun > 0);

            // draw scene
            DrawFrame();

            // decide how many game ticks to run for next loop

            if(!bForceSingleFrame)
            {
                ticsToRun = GetNextTickCount();
            }
            else
            {
                ticsToRun = 1;
                bForceSingleFrame = false;
            }

            // handle garbage collection
            Mem_GC();

            kex::cSound->Update();
        }

        prevmsec = nextmsec;
    }
}
