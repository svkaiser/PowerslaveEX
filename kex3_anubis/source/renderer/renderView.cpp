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
#include "frustum.h"
#include "renderView.h"

kexCvar cvarFOV("r_fov", CVF_FLOAT|CVF_CONFIG, "74.0", "Field of view");

const float kexRenderView::Z_NEAR = 0.1f;

//
// kexRenderView::kexRenderView
//

kexRenderView::kexRenderView(void)
{
}

//
// kexRenderView::~kexRenderView
//

kexRenderView::~kexRenderView(void)
{
}

//
// kexRenderView::ProjectPoint
//

kexVec3 kexRenderView::ProjectPoint(const kexVec3 &point)
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
    
    return kexVec3( (proj.x * 0.5f + 0.5f) * w,
                   (-proj.y * 0.5f + 0.5f) * h,
                   (1.0f + proj.z) * 0.5f);
}

//
// kexRenderView::SetupMatrices
//

void kexRenderView::SetupMatrices(void)
{
    projectionView.Identity();
    modelView.Identity();
    
    fov = cvarFOV.GetFloat();

    // setup projection matrix
    projectionView.SetViewProjection(kex::cSystem->VideoRatio(), fov, Z_NEAR, -1);

    // setup rotation quaternion
    kexQuat qroll(roll, kexVec3::vecForward);
    kexQuat qyaw(yaw, kexVec3::vecUp);
    kexQuat qpitch(-kexMath::Deg2Rad(90) + pitch, kexVec3::vecRight);
    
    rotation = qyaw * (qpitch * qroll);
    
    // setup model view matrix
    modelView = kexMatrix(rotation);
    modelView.AddTranslation(-origin * modelView);
}

//
// kexRenderView::Setup
//

void kexRenderView::Setup(void)
{
    SetupMatrices();
    frustum.MakeClipPlanes(projectionView, modelView);
}
