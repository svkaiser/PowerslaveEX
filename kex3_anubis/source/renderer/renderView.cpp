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
    kexQuat qyaw(-yaw, kexVec3::vecUp);
    kexQuat qpitch(pitch - kexMath::Deg2Rad(90), kexVec3::vecRight);
    
    rotation = qyaw * (qpitch * qroll);
    
    // setup model view matrix
    modelView = kexMatrix(rotation);
    modelView.AddTranslation(-(origin * modelView));
}

//
// kexRenderView::Render
//

void kexRenderView::Render(void)
{
    SetupMatrices();
    frustum.MakeClipPlanes(projectionView, modelView);
}
