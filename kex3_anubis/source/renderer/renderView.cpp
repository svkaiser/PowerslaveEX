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
//      View/perspective Rendering
//

#include "renderMain.h"
#include "game.h"
#include "frustum.h"
#include "renderView.h"

kexCvar cvarFOV("r_fov", CVF_FLOAT|CVF_CONFIG, "74.0", "Field of view");

const float kexRenderView::Z_NEAR = 0.1f;

//
// kexRenderView::ProjectPoint
//

kexVec3 kexRenderView::ProjectPoint(const kexVec3 &point, kexVec4 *projVector)
{
    kexVec4 proj, model;
    float w, h;
    
    model.Set(point.x, point.y, point.z, 0);
    model *= modelView;
    
    proj = model;
    proj *= projectionView;
    
    proj.ToVec3() *= model.w;
    
    if(proj.w != 0)
    {
        proj.w = 1.0f / proj.w;
        proj.ToVec3() *= proj.w;
    }
    
    w = (float)kex::cSystem->VideoWidth();
    h = (float)kex::cSystem->VideoHeight();

    if(projVector)
    {
        *projVector = proj;
    }
    
    return kexVec3((proj.x * 0.5f + 0.5f) * w,
                   (-proj.y * 0.5f + 0.5f) * h,
                   (1.0f + proj.z) * 0.5f);
}

//
// kexRenderView::SetupMatrices
//

void kexRenderView::SetupMatrices(void)
{
    kexMatrix transform;

    projectionView.Identity();
    modelView.Identity();
    
    fov = cvarFOV.GetFloat();

    // setup projection matrix
    projectionView.SetViewProjection(kex::cSystem->VideoRatio(), fov, Z_NEAR, -1);

    // setup rotation quaternion
    kexQuat qyaw(yaw, kexVec3::vecUp);
    kexQuat qpitch(-kexMath::Deg2Rad(90) + pitch, kexVec3::vecRight);
    
    rotation = qyaw * qpitch;
    
    // setup model view matrix
    modelView = kexMatrix(rotation);
    
    modelView = kexMatrix(roll * kexMath::Sin(yaw), 1) * modelView;
    modelView.RotateY(roll * kexMath::Cos(yaw));

    // scale to aspect ratio
    modelView.Scale(1, 1, 1.07142f);
    modelView.AddTranslation(-origin * modelView);

    // re-adjust translation
    transform.SetTranslation(0, 0, -32);
    modelView = (transform * modelView);
}

//
// kexRenderView::SetupFromPlayer
//

void kexRenderView::SetupFromPlayer(kexPlayer *player)
{
    kexActor *actor = player->Actor();
    kexVec3 forward;
    
    yaw = actor->Yaw();
    pitch = actor->Pitch();
    roll = actor->Roll();
    origin = actor->Origin();
    origin.z += player->ViewZ() + player->Bob() + player->LandTime() + player->StepViewZ();

    kexVec3::ToAxis(&forward, 0, 0, yaw, pitch, 0);
    
    SetupMatrices();
    frustum.MakeClipPlanes(projectionView, modelView);
    frustum.TransformPoints(origin, forward, fov, kex::cSystem->VideoRatio(), 0.1f, 8192);
}

//
// kexRenderView::Setup
//
// Generic routine for setting up the render view
//

void kexRenderView::Setup(void)
{
    SetupMatrices();
    frustum.MakeClipPlanes(projectionView, modelView);
}
