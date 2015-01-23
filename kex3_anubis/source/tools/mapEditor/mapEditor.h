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

#ifndef __MAPEDITOR_H__
#define __MAPEDITOR_H__

class kexMapEditor : public kexGameLoop
{
public:
    kexMapEditor(void);
    ~kexMapEditor(void);
    
    void                        Init(void);
    void                        Start(void);
    void                        Stop(void);
    void                        Draw(void);
    void                        Tick(void);
    bool                        ProcessInput(inputEvent_t *ev);

private:
    bool                        OnMouse(const inputEvent_t *ev);
    void                        MoveCamera(const inputEvent_t *ev);
    void                        DrawXYGrid(const float spacing, const byte c);
    void                        DrawWorld(void);
    
    static const float          CAM_GRID_SCALE;
    
    int                         mouse_x;
    int                         mouse_y;
    
    kexRenderView               renderView;
    float                       gridScale;
    bool                        bCameraMove;
    bool                        bGridSnap;
    bool                        bShowGrid;
};

#endif
