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
//      View bounds
//

#include "kexlib.h"
#include "renderMain.h"
#include "renderView.h"
#include "viewBounds.h"

//
// kexViewBounds::kexViewBounds
//

kexViewBounds::kexViewBounds(void)
{
    Clear();
}

//
// kexViewBounds::Clear
//

void kexViewBounds::Clear(void)
{
    max[0]  = -kexMath::infinity;
    max[1]  = -kexMath::infinity;
    min[0]  =  kexMath::infinity;
    min[1]  =  kexMath::infinity;
    zmin    = 0.0f;
    zfar    = 1.0f;
}

//
// kexViewBounds::IsClosed
//

const bool kexViewBounds::IsClosed(void) const
{
    return (min[0] > max[0] || min[1] > max[1]);
}

//
// kexViewBounds::Fill
//

void kexViewBounds::Fill(void)
{
    min[0] = 0;
    min[1] = 0;
    max[0] = (float)kex::cSystem->VideoWidth();
    max[1] = (float)kex::cSystem->VideoHeight();
    zmin = 0.0f;
    zfar = 1.0f;
}

//
// kexViewBounds::AddPoint
//

void kexViewBounds::AddPoint(const float x, const float y, const float z)
{
    if(x < min[0]) { min[0] = x; }
    if(x > max[0]) { max[0] = x; }
    if(y < min[1]) { min[1] = y; }
    if(y > max[1]) { max[1] = y; }

    // get closest z-clip
    if(z < zfar) { zfar = z; }
}

//
// kexViewBounds::AddVector
//

void kexViewBounds::AddVector(kexRenderView *view, kexVec3 &vector)
{
    int bits;
    float w, h;
    kexFrustum frustum;
    kexVec3 pmin;

    frustum = view->Frustum();
    bits = frustum.SphereBits(vector, 1);

    pmin = view->ProjectPoint(vector);

    w = (float)kex::cSystem->VideoWidth();
    h = (float)kex::cSystem->VideoHeight();

    if(!(bits & BIT(FP_RIGHT)))
    {
        pmin[0] = w;
    }

    if(!(bits & BIT(FP_LEFT)))
    {
        pmin[0] = 0;
    }

    if(!(bits & BIT(FP_BOTTOM)))
    {
        pmin[1] = 0;
    }

    if(!(bits & BIT(FP_TOP)))
    {
        pmin[1] = h;
    }

    AddPoint(pmin[0], pmin[1], pmin[2]);
}

//
// kexViewBounds::AddBox
//

void kexViewBounds::AddBox(kexRenderView *view, kexBBox &box)
{
    kexVec3 points[8];
    int i;

    box.ToVectors(points);

    for(i = 0; i < 8; i++)
    {
        AddVector(view, points[i]);
    }
}

//
// kexViewBounds::ViewBoundInside
//

bool kexViewBounds::ViewBoundInside(const kexViewBounds &viewBounds)
{
    if((viewBounds.max[0] < min[0] || viewBounds.max[1] < min[1] ||
        viewBounds.min[0] > max[0] || viewBounds.min[1] > max[1]))
    {
        return false;
    }
    
    return true;
}

//
// kexViewBounds::operator=
//

kexViewBounds &kexViewBounds::operator=(const kexViewBounds &viewBounds)
{
    min[0] = viewBounds.min[0];
    min[1] = viewBounds.min[1];
    max[0] = viewBounds.max[0];
    max[1] = viewBounds.max[1];

    zmin = viewBounds.zmin;
    zfar = viewBounds.zfar;

    return *this;
}

//
// kexViewBounds::DebugDraw
//

void kexViewBounds::DebugDraw(void)
{
    kexRender::cTextures->whiteTexture->Bind();

    kexRender::cVertList->BindDrawPointers();
    kexRender::cVertList->AddLine(min[0], min[1], 0, min[0], max[1], 0, 255, 0, 255, 255);
    kexRender::cVertList->AddLine(min[0], max[1], 0, max[0], max[1], 0, 255, 0, 255, 255);
    kexRender::cVertList->AddLine(max[0], max[1], 0, max[0], min[1], 0, 255, 0, 255, 255);
    kexRender::cVertList->AddLine(max[0], min[1], 0, min[0], min[1], 0, 255, 0, 255, 255);
    kexRender::cVertList->DrawLineElements();
}
