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
//      Internal level editor (a really lame one)
//

#include "kexlib.h"
#include "game.h"
#include "renderMain.h"
#include "renderView.h"
#include "mapEditor.h"

kexMapEditor mapEditorLocal;

kexCvar cvarEditorGridSize("editor_gridsize", CVF_FLOAT|CVF_CONFIG, "16", 1, 1024, "Map editor grid size");
kexCvar cvarEditorGridSnap("editor_gridsnap", CVF_BOOL|CVF_CONFIG, "1", "Snap draggable objects to grid");

const float kexMapEditor::CAM_GRID_SCALE = 16384.0f;

//
// editor
//

COMMAND(editor)
{
    if(kex::cCommands->GetArgc() < 1)
    {
        return;
    }
    
    kexGame::cLocal->SetGameState(GS_MAPEDITOR);
}

//
// kexMapEditor::kexMapEditor
//

kexMapEditor::kexMapEditor(void)
{
    this->gridScale = 0.0f;
    this->bShowGrid = true;
    this->bGridSnap = true;
    this->bCameraMove = false;
    this->mouse_x = 0;
    this->mouse_y = 0;
}

//
// kexMapEditor::~kexMapEditor
//

kexMapEditor::~kexMapEditor(void)
{
}

//
// kexMapEditor::Init
//

void kexMapEditor::Init(void)
{
}

//
// kexMapEditor::Start
//

void kexMapEditor::Start(void)
{
    renderView.Origin().Set(0, 0, 512);
    kex::cInput->ToggleMouseGrab(false);
    kex::cSession->ToggleCursor(true);
    
    if(!kexGame::cWorld->MapLoaded())
    {
        kexGame::cWorld->LoadMap("maps/TOMB.MAP");
    }
}

//
// kexMapEditor::Stop
//

void kexMapEditor::Stop(void)
{
    kex::cInput->ToggleMouseGrab(true);
    kex::cSession->ToggleCursor(false);
    
    if(kexGame::cWorld->MapLoaded())
    {
        kexGame::cWorld->UnloadMap();
    }
}

//
// kexMapEditor::Draw
//

void kexMapEditor::Draw(void)
{
    dglClearColor(0.125f, 0.125f, 0.125f, 1);
    kexRender::cBackend->ClearBuffer();
    
    renderView.Pitch() = kexMath::Deg2Rad(90);
    renderView.Setup();
    
    renderView.ModelView().Scale(1, 1, 0);
    kexRender::cBackend->LoadProjectionMatrix(renderView.ProjectionView());
    kexRender::cBackend->LoadModelViewMatrix(renderView.ModelView());
    
    DrawXYGrid(cvarEditorGridSize.GetFloat(), 64);
    DrawXYGrid(1024.0f, 128);
    
    kexRender::cUtils->DrawOrigin(0, 0, 0, 32);
    DrawWorld();
}

//
// kexMapEditor::Tick
//

void kexMapEditor::Tick(void)
{
    gridScale = 64 * renderView.Origin().z / kexMapEditor::CAM_GRID_SCALE;
    bGridSnap = cvarEditorGridSnap.GetBool();
}

//
// kexMapEditor::ProcessInput
//

bool kexMapEditor::ProcessInput(inputEvent_t *ev)
{
    bool ok = false;
    
    mouse_x = kex::cInput->MouseX();
    mouse_y = kex::cInput->MouseY();
    
    if(ev->type == ev_mouse)
    {
        OnMouse(ev);
    }
    
    if(ev->type == ev_keydown)
    {
        switch(ev->data1)
        {
        case KKEY_SPACE:
            bCameraMove = true;
            kex::cInput->ToggleMouseGrab(true);
            kex::cInput->CenterMouse();
            kex::cSession->ToggleCursor(false);
            ok = true;
            break;
        }
    }
    
    if(ev->type == ev_mousedown)
    {
        if(ev->data1 == KMSB_WHEEL_UP)
        {
            renderView.Origin().z -= (gridScale * 32);
        }
        else if(ev->data1 == KMSB_WHEEL_DOWN)
        {
            renderView.Origin().z += (gridScale * 32);
        }
    }
    
    if(ev->type == ev_keyup)
    {
        switch(ev->data1)
        {
        case KKEY_SPACE:
            bCameraMove = false;
            kex::cInput->ToggleMouseGrab(false);
            kex::cSession->ToggleCursor(true);
            ok = true;
            break;
        }
    }
    
    return ok;
}

//
// kexMapEditor::OnMouse
//

bool kexMapEditor::OnMouse(const inputEvent_t *ev)
{
    if(bCameraMove)
    {
        MoveCamera(ev);
    }
    
    return true;
}

//
// kexMapEditor::MoveCamera
//

void kexMapEditor::MoveCamera(const inputEvent_t *ev)
{
    float scale = gridScale;
    
    renderView.Origin().x -= ((ev->data1 * scale) / 2.0f);
    renderView.Origin().y += ((ev->data2 * scale) / 2.0f);
    
    kexMath::Clamp(renderView.Origin().x, -kexMapEditor::CAM_GRID_SCALE, kexMapEditor::CAM_GRID_SCALE);
    kexMath::Clamp(renderView.Origin().y, -kexMapEditor::CAM_GRID_SCALE, kexMapEditor::CAM_GRID_SCALE);
}

//
// kexMapEditor::DrawXYGrid
//

void kexMapEditor::DrawXYGrid(const float spacing, const byte c)
{
    const float size = kexMapEditor::CAM_GRID_SCALE;
    
    float start = -size;
    float gridSize = spacing;
    
    while(gridScale > (gridSize * 0.5f))
    {
        gridSize *= 2.0f;
    }
    
    kexVec3 pt1, pt2, pt3, pt4;
    
    kexRender::cBackend->SetDepthMask(0);
    kexRender::cTextures->whiteTexture->Bind();
    
    for(int i = 0; i < (size * 2) / gridSize; i++)
    {
        pt1.Set(start, -size, 0.1f);
        pt2.Set(start, size, 0.1f);
        pt3.Set(-size, start, 0.1f);
        pt4.Set(size, start, 0.1f);
        
        kexRender::cUtils->DrawLine(pt1, pt2, c, c, c);
        kexRender::cUtils->DrawLine(pt3, pt4, c, c, c);
        
        start += gridSize;
    }
    
    kexRender::cBackend->SetDepthMask(1);
}

//
// kexMapEditor::DrawWorld
//

void kexMapEditor::DrawWorld(void)
{
    kexWorld *world = kexGame::cWorld;
    
    if(!world->MapLoaded())
    {
        return;
    }
    
    kexRender::cBackend->SetState(GLSTATE_DEPTHTEST, false);
    kexRender::cBackend->SetState(GLSTATE_ALPHATEST, true);
    kexRender::cBackend->SetState(GLSTATE_BLEND, true);
    
    for(unsigned int i = 0; i < world->NumSectors(); ++i)
    {
        mapSector_t *sector = &world->Sectors()[i];
        
        int start = sector->faceStart;
        int end = sector->faceEnd;
        
        sector->floodCount = 0;
        
        for(int j = start; j < end+1; ++j)
        {
            mapFace_t *face = &world->Faces()[j];
            
            if(face->sector != -1)
            {
                continue;
            }
            
            kexRender::cUtils->DrawLine(*face->LeftEdge()->v2, *face->RightEdge()->v1, 255, 255, 255);
        }
    }
    
    for(unsigned int i = 0; i < world->NumSectors(); ++i)
    {
        mapSector_t *sector = &world->Sectors()[i];
        
        int start = sector->faceStart;
        int end = sector->faceEnd;
        
        sector->floodCount = 0;
        
        for(int j = start; j < end+1; ++j)
        {
            mapFace_t *face = &world->Faces()[j];
            
            if(face->sector != -1)
            {
                mapSector_t *s = &world->Sectors()[face->sector];
                mapFace_t *f1a = &world->Faces()[s->faceEnd+1];
                mapFace_t *f2a = &world->Faces()[s->faceEnd+2];
                mapFace_t *f1b = &world->Faces()[sector->faceEnd+1];
                mapFace_t *f2b = &world->Faces()[sector->faceEnd+2];
                
                if(f1a->plane.d == f1b->plane.d && f2a->plane.d == f2b->plane.d)
                {
                    continue;
                }
                
                kexRender::cUtils->DrawLine(*face->LeftEdge()->v2, *face->RightEdge()->v1, 255, 32, 64);
            }
        }
    }
}
