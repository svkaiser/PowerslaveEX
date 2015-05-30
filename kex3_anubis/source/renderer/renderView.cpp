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
#include "renderView.h"

kexCvar kexRenderView::cvarFOV("r_fov", CVF_FLOAT|CVF_CONFIG, "74.0", "Field of view");

const float kexRenderView::Z_NEAR = 0.1f;
const float kexRenderView::Z_FAR = -0.999f;

//
// kexRenderView::ProjectPoint
//

kexVec3 kexRenderView::ProjectPoint(const kexVec3 &point, kexVec4 *projVector)
{
    kexVec4 proj;
    float w, h;
    float x, y, z;
    
    proj.Set(point.x, point.y, point.z, 0);
    proj *= clipMatrix;
    
    if(proj.w < 0.1f)
    {
        proj.w = 0.1f;
    }
    
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
    
    x = (proj.x * 0.5f + 0.5f) * w;
    y = (-proj.y * 0.5f + 0.5f) * h;
    z = (1.0f + proj.z) * 0.5f;
    
    kexMath::Clamp(x, 0, w);
    kexMath::Clamp(y, 0, h);
    kexMath::Clamp(z, 0, 1);
    
    return kexVec3(x, y, z);
}

//
// kexRenderView::UnProjectPoint
//

kexVec3 kexRenderView::UnProjectPoint(const kexVec3 &point)
{
    kexMatrix inv = kexMatrix::Invert(clipMatrix);
    kexVec4 pt;

    pt.x = point.x / (float)kex::cSystem->VideoWidth() * 2.0f - 1.0f;
    pt.y = point.y / (float)kex::cSystem->VideoHeight() * 2.0f - 1.0f;
    pt.z = 2.0f * point.z - 1.0f;
    pt.w = 1.0f;

    pt = pt * inv;
    pt.w = 1.0f / pt.w;

    kexVec3 out = pt.ToVec3();
    out /= pt.w;

    return out;
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
    projectionView.SetViewProjection(kex::cSystem->VideoRatio(), fov, Z_NEAR, Z_FAR);

    // setup rotation quaternion
    kexQuat qyaw(yaw, kexVec3::vecUp);
    kexQuat qpitch(pitch, kexVec3::vecRight);
    
    rotation = qyaw * qpitch;
    
    // setup model view matrix
    modelView = kexMatrix(rotation);
    rotationMatrix = kexMatrix(rotation);
    
    // pitch of 0 is treated in-game as being centered but
    // the renderer sees it as looking straight down so rotate
    // the modelview matrix relative to that
    modelView.RotateX(-kexMath::Deg2Rad(90));
    
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

    if(player->ShakeTime() > 0 && (int)player->Actor()->Velocity().z == 0)
    {
        yaw += kexMath::Deg2Rad(player->ShakeVector().x * 0.125f);
        origin.z += player->ShakeVector().y * 0.5f;
    }

    kexVec3::ToAxis(&forward, 0, 0, yaw, pitch, 0);
    
    SetupMatrices();

    MakeClipPlanes();
    TransformPoints(origin, forward, fov, kex::cSystem->VideoRatio(), Z_NEAR, 8192);
}

//
// kexRenderView::Setup
//
// Generic routine for setting up the render view
//

void kexRenderView::Setup(void)
{
    SetupMatrices();
    MakeClipPlanes();
}

//
// kexRenderView::MakeClipPlanes
//

void kexRenderView::MakeClipPlanes(void)
{
    clipMatrix = modelView * projectionView;

    for(int i = 0; i < 4; i++)
    {
        p[FP_RIGHT][i]  = clipMatrix.vectors[i].w - clipMatrix.vectors[i].x;
        p[FP_LEFT][i]   = clipMatrix.vectors[i].w + clipMatrix.vectors[i].x;
        p[FP_TOP][i]    = clipMatrix.vectors[i].w - clipMatrix.vectors[i].y;
        p[FP_BOTTOM][i] = clipMatrix.vectors[i].w + clipMatrix.vectors[i].y;
        p[FP_FAR][i]    = clipMatrix.vectors[i].w - clipMatrix.vectors[i].z;
        p[FP_NEAR][i]   = clipMatrix.vectors[i].w + clipMatrix.vectors[i].z;
    }
}

//
// kexRenderView::TransformPoints
//
// Transforms the corner points of the frustum
//

void kexRenderView::TransformPoints(const kexVec3 &center, const kexVec3 &dir,
                                 const float fov, const float aspect,
                                 const float _near, const float _far)
{
    kexVec3 right = dir.Cross(kexVec3::vecUp);
    right.Normalize();

    kexVec3 up = right.Cross(dir);
    up.Normalize();

    const float fovAngle    = kexMath::Deg2Rad(fov) + 0.2f;

    const kexVec3 vFar      = center + dir * _far;
    const kexVec3 vNear     = center + dir * _near;

    const float nearHeight  = kexMath::Tan(fovAngle / 2.0f) * _near;
    const float nearWidth   = nearHeight * aspect;
    const float farHeight   = kexMath::Tan(fovAngle / 2.0f) * _far;
    const float farWidth    = farHeight * aspect;

    points[0] = vNear - up * nearHeight - right * nearWidth;
    points[1] = vNear + up * nearHeight - right * nearWidth;
    points[2] = vNear + up * nearHeight + right * nearWidth;
    points[3] = vNear - up * nearHeight + right * nearWidth;

    points[4] = vFar - up * farHeight - right * farWidth;
    points[5] = vFar + up * farHeight - right * farWidth;
    points[6] = vFar + up * farHeight + right * farWidth;
    points[7] = vFar - up * farHeight + right * farWidth;
}

//
// kexRenderView::TestBoundingBox
//

bool kexRenderView::TestBoundingBox(const kexBBox &bbox)
{
    float d;
    for(int i = 0; i < NUMFRUSTUMPLANES; i++)
    {
        d = p[i].a * bbox.min.x + p[i].b * bbox.min.y + p[i].c * bbox.min.z + p[i].d;
        if(!FLOATSIGNBIT(d))
        {
            continue;
        }
        d = p[i].a * bbox.max.x + p[i].b * bbox.min.y + p[i].c * bbox.min.z + p[i].d;
        if(!FLOATSIGNBIT(d))
        {
            continue;
        }
        d = p[i].a * bbox.min.x + p[i].b * bbox.max.y + p[i].c * bbox.min.z + p[i].d;
        if(!FLOATSIGNBIT(d))
        {
            continue;
        }
        d = p[i].a * bbox.max.x + p[i].b * bbox.max.y + p[i].c * bbox.min.z + p[i].d;
        if(!FLOATSIGNBIT(d))
        {
            continue;
        }
        d = p[i].a * bbox.min.x + p[i].b * bbox.min.y + p[i].c * bbox.max.z + p[i].d;
        if(!FLOATSIGNBIT(d))
        {
            continue;
        }
        d = p[i].a * bbox.max.x + p[i].b * bbox.min.y + p[i].c * bbox.max.z + p[i].d;
        if(!FLOATSIGNBIT(d))
        {
            continue;
        }
        d = p[i].a * bbox.min.x + p[i].b * bbox.max.y + p[i].c * bbox.max.z + p[i].d;
        if(!FLOATSIGNBIT(d))
        {
            continue;
        }
        d = p[i].a * bbox.max.x + p[i].b * bbox.max.y + p[i].c * bbox.max.z + p[i].d;
        if(!FLOATSIGNBIT(d))
        {
            continue;
        }

        return false;
    }

    return true;
}

//
// kexRenderView::TestSphere
//

bool kexRenderView::TestSphere(const kexVec3 &org, const float radius)
{
    for(int i = 0; i < NUMFRUSTUMPLANES; i++)
    {
        if(p[i].Dot(org) + p[i].d <= -radius)
        {
            return false;
        }

    }
    return true;
}

//
// kexRenderView::TestPointNearPlane
//

bool kexRenderView::TestPointNearPlane(const kexVec3 &org)
{
    return ((p[FP_NEAR].Dot(org) + p[FP_NEAR].d) >= 0);
}

//
// kexRenderView::SphereBits
//

byte kexRenderView::SphereBits(const kexVec3 &org, const float radius)
{
    byte bits = 0x3f;
    
    for(int i = 0; i < NUMFRUSTUMPLANES; i++)
    {
        if(p[i].Dot(org) + p[i].d <= -radius)
        {
            bits &= ~(1 << i);
        }
        
    }
    
    return bits;
}

//
// kexRenderView::BoxDistance
//

bool kexRenderView::BoxDistance(const kexBBox &box, const float distance)
{
    kexPlane nearPlane = NearPlane();

    return (nearPlane.Dot(box.Center()) + nearPlane.d) > distance;
}
