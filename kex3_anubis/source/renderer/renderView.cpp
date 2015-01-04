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
    kexMatrix transform;

    fov = cvarFOV.GetFloat();

    // setup projection matrix
    projectionView.SetViewProjection(kex::cSystem->VideoRatio(), fov, Z_NEAR, -1);

    // setup rotation matrix
    // start off with the matrix on it's z-axis and then rotate it along the x-axis
    kexMatrix rotMatrix(-yaw + kexMath::Deg2Rad(90), 2);
    rotMatrix.RotateZ(-pitch - kexMath::Deg2Rad(90));

    // setup modelview matrix
    transform.SetTranslation(-origin);
    modelView = transform * rotMatrix;
}

//
// kexRenderView::Render
//

void kexRenderView::Render(void)
{
    SetupMatrices();
    frustum.MakeClipPlanes(projectionView, modelView);
}
