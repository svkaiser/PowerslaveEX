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
//      Play loop (in-game) logic
//

#include "kexlib.h"
#include "renderMain.h"
#include "renderView.h"
#include "game.h"
#include "playLoop.h"

//
// kexPlayLoop::kexPlayLoop
//

kexPlayLoop::kexPlayLoop(void)
{
}

//
// kexPlayLoop::~kexPlayLoop
//

kexPlayLoop::~kexPlayLoop(void)
{
}

//
// kexPlayLoop::Init
//

void kexPlayLoop::Init(void)
{
}

//
// kexPlayLoop::Start
//

void kexPlayLoop::Start(void)
{
}

//
// kexPlayLoop::Stop
//

void kexPlayLoop::Stop(void)
{
}

//
// kexPlayLoop::Draw
//

void kexPlayLoop::Draw(void)
{
    // TEMP
    kex::cGame->RenderView()->Render();
    kexRender::cBackend->LoadProjectionMatrix(kex::cGame->RenderView()->ProjectionView());
    kexRender::cBackend->LoadModelViewMatrix(kex::cGame->RenderView()->ModelView());
    kexRender::cUtils->DrawBoundingBox(kexBBox(
        kexVec3(-64, -128, -32),
        kexVec3(64, 128, 32)), 255, 0, 0);
}

//
// kexPlayLoop::Tick
//

void kexPlayLoop::Tick(void)
{
    UpdateActors();
}

//
// kexPlayLoop::ProcessInput
//

bool kexPlayLoop::ProcessInput(inputEvent_t *ev)
{
    return false;
}

//
// kexPlayLoop::UpdateActors
//

void kexPlayLoop::UpdateActors(void)
{
    kexActor *next = NULL;

    for(actorRover = actors.Next(); actorRover != NULL; actorRover = next)
    {
        next = actorRover->Link().Next();
        actorRover->Tick();

        if(actorRover->Removing())
        {
            RemoveActor(actorRover);
        }
    }
}

//
// kexPlayLoop::AddActor
//

void kexPlayLoop::AddActor(kexActor *actor)
{
    actor->Link().Add(actors);
    actor->CallSpawn();
}

//
// kexPlayLoop::RemoveActor
//

void kexPlayLoop::RemoveActor(kexActor *actor)
{
    actor->SetTarget(NULL);
    actor->Link().Remove();
    actor->UnlinkArea();
    delete actor;
}
