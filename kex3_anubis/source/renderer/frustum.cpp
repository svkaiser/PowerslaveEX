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
//      Frustum class
//

#include "kexlib.h"
#include "renderMain.h"
#include "frustum.h"

#undef near
#undef far

//
// kexFrustum::kexFrustum
//

kexFrustum::kexFrustum(void)
{
}

//
// kexFrustum::MakeClipPlanes
//

void kexFrustum::MakeClipPlanes(kexMatrix &proj, kexMatrix &model)
{
    kexMatrix clip  = model * proj;

    for(int i = 0; i < 4; i++)
    {
        p[FP_RIGHT][i]  = clip.vectors[i].w - clip.vectors[i].x;
        p[FP_LEFT][i]   = clip.vectors[i].w + clip.vectors[i].x;
        p[FP_BOTTOM][i] = clip.vectors[i].w - clip.vectors[i].y;
        p[FP_TOP][i]    = clip.vectors[i].w + clip.vectors[i].y;
        p[FP_FAR][i]    = clip.vectors[i].w - clip.vectors[i].z;
        p[FP_NEAR][i]   = clip.vectors[i].w + clip.vectors[i].z;
    }
}

//
// kexFrustum::TransformPoints
//
// Transforms the corner points of the frustum
//

void kexFrustum::TransformPoints(const kexVec3 &center, const kexVec3 &dir,
                                 const float fov, const float aspect,
                                 const float near, const float far)
{
    kexVec3 right = dir.Cross(kexVec3::vecUp);
    right.Normalize();

    kexVec3 up = right.Cross(dir);
    up.Normalize();

    const float fovAngle    = kexMath::Deg2Rad(fov) + 0.2f;

    const kexVec3 vFar      = center + dir * far;
    const kexVec3 vNear     = center + dir * near;

    const float nearHeight  = kexMath::Tan(fovAngle / 2.0f) * near;
    const float nearWidth   = nearHeight * aspect;
    const float farHeight   = kexMath::Tan(fovAngle / 2.0f) * far;
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
// kexFrustum::TestBoundingBox
//

bool kexFrustum::TestBoundingBox(const kexBBox &bbox)
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
// kexFrustum::TestSphere
//

bool kexFrustum::TestSphere(const kexVec3 &org, const float radius)
{
    for(int i = 0; i < NUMFRUSTUMPLANES; i++)
    {
        if(p[i].Distance(org) + p[i].d <= -radius)
        {
            return false;
        }

    }
    return true;
}

//
// kexFrustum::SphereBits
//

byte kexFrustum::SphereBits(const kexVec3 &org, const float radius)
{
    byte bits = 0x3f;
    
    for(int i = 0; i < NUMFRUSTUMPLANES; i++)
    {
        if(p[i].Distance(org) + p[i].d <= -radius)
        {
            bits &= ~(1 << i);
        }
        
    }
    
    return bits;
}

//
// kexFrustum::BoxDistance
//

bool kexFrustum::BoxDistance(const kexBBox &box, const float distance)
{
    kexPlane nearPlane = Near();

    return (nearPlane.Distance(box.Center()) + nearPlane.d) > distance;
}
