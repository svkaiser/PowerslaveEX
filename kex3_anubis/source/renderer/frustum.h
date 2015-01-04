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

#ifndef __FRUSTUM_H__
#define __FRUSTUM_H__

typedef enum
{
    FP_RIGHT    = 0,
    FP_LEFT,
    FP_BOTTOM,
    FP_TOP,
    FP_FAR,
    FP_NEAR,
    NUMFRUSTUMPLANES
} frustumPlane_t;

#define FRUSTUM_CLIPPED     BIT(NUMFRUSTUMPLANES)
#define NUMFRUSTUMPOINTS    8

class kexFrustum
{
public:
    kexFrustum(void);

    void                MakeClipPlanes(kexMatrix &proj, kexMatrix &model);
    void                TransformPoints(const kexVec3 &center, const kexVec3 &dir,
                                        const float fov, const float aspect,
                                        const float near, const float far);
    bool                TestBoundingBox(const kexBBox &bbox);
    bool                TestSphere(const kexVec3 &org, const float radius);
    bool                BoxDistance(const kexBBox &box, const float distance);
    bool                ClipSegment(kexVec3 &out1, kexVec3 &out2,
                                    int &clipbits1, int &clipbits2,
                                    const kexVec3 &start, const kexVec3 &end);

    kexPlane            &Right(void) { return p[FP_RIGHT]; }
    kexPlane            &Left(void) { return p[FP_LEFT]; }
    kexPlane            &Bottom(void) { return p[FP_BOTTOM]; }
    kexPlane            &Top(void) { return p[FP_TOP]; }
    kexPlane            &Far(void) { return p[FP_FAR]; }
    kexPlane            &Near(void) { return p[FP_NEAR]; }

    kexVec3             *Points(void) { return points; }

private:
    kexPlane            p[NUMFRUSTUMPLANES];
    kexVec3             points[NUMFRUSTUMPOINTS];
};

#endif
