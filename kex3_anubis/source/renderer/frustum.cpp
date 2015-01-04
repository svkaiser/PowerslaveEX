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
// kexFrustum::BoxDistance
//

bool kexFrustum::BoxDistance(const kexBBox &box, const float distance)
{
    kexPlane nearPlane = Near();

    return (nearPlane.Distance(box.Center()) + nearPlane.d) > distance;
}

//
// kexFrustum::ClipSegment
//
//

bool kexFrustum::ClipSegment(kexVec3 &out1, kexVec3 &out2,
                             int &clipbits1, int &clipbits2,
                             const kexVec3 &start, const kexVec3 &end)
{
    float d1;
    float d2;
    float frac;
    float f1;
    float f2;
    int bit1;
    int bit2;
    kexVec3 hit;

    clipbits1 = 0;
    clipbits2 = 0;
    f1 = 1.0f;
    f2 = 1.0f;

    out1 = start;
    out2 = end;

    for(int i = 0; i < NUMFRUSTUMPLANES; i++)
    {
        d1 = p[i].Distance(start) + p[i].d;
        d2 = p[i].Distance(end) + p[i].d;

        bit1 = FLOATSIGNBIT(d1);
        bit2 = FLOATSIGNBIT(d2);

        if(d1 <= 0 && d2 <= 0)
        {
            clipbits1 |= FRUSTUM_CLIPPED | (bit1 << i);
            clipbits2 |= FRUSTUM_CLIPPED | (bit2 << i);
            return false;
        }

        if(bit1 ^ bit2)
        {
            if(d2 < d1)
            {
                frac = (d1 / (d1 - d2));

                if(frac > 1 || frac > f1)
                {
                    continue;
                }
                clipbits2 |= (bit2 << i);
                out2 = start.Lerp(end, frac);
                f1 = frac;
            }
            else
            {
                frac = (d2 / (d2 - d1));

                if(frac > 1 || frac > f2)
                {
                    continue;
                }
                clipbits1 |= (bit1 << i);
                out1 = end.Lerp(start, frac);
                f2 = frac;
            }
        }
    }

    return true;
}
