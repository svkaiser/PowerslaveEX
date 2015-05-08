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

#ifndef __RENDERVIEW_H__
#define __RENDERVIEW_H__

class kexPlayer;

typedef enum
{
    FP_RIGHT    = 0,
    FP_LEFT,
    FP_TOP,
    FP_BOTTOM,
    FP_FAR,
    FP_NEAR,
    NUMFRUSTUMPLANES
} frustumPlane_t;

#define NUMFRUSTUMPOINTS    8

class kexRenderView
{
public:
    kexVec3             &Origin(void) { return origin; }
    kexAngle            &Yaw(void) { return yaw; }
    kexAngle            &Pitch(void) { return pitch; }
    kexAngle            &Roll(void) { return roll; }
    kexMatrix           &ProjectionView(void) { return projectionView; }
    kexMatrix           &ModelView(void) { return modelView; }
    kexMatrix           &RotationMatrix(void) { return rotationMatrix; }
    kexMatrix           &ClipMatrix(void) { return clipMatrix; }
    kexQuat             &Rotation(void) { return rotation; }
    float               &Fov(void) { return fov; }

    kexPlane            &RightPlane(void) { return p[FP_RIGHT]; }
    kexPlane            &LeftPlane(void) { return p[FP_LEFT]; }
    kexPlane            &BottomPlane(void) { return p[FP_BOTTOM]; }
    kexPlane            &TopPlane(void) { return p[FP_TOP]; }
    kexPlane            &FarPlane(void) { return p[FP_FAR]; }
    kexPlane            &NearPlane(void) { return p[FP_NEAR]; }

    kexVec3             *Points(void) { return points; }

    void                SetupFromPlayer(kexPlayer *player);
    void                Setup(void);
    kexVec3             ProjectPoint(const kexVec3 &point, kexVec4 *projVector = NULL);
    kexVec3             UnProjectPoint(const kexVec3 &point);
    bool                TestBoundingBox(const kexBBox &bbox);
    bool                TestSphere(const kexVec3 &org, const float radius);
    bool                TestPointNearPlane(const kexVec3 &org);
    byte                SphereBits(const kexVec3 &org, const float radius);
    bool                BoxDistance(const kexBBox &box, const float distance);

    static const float  Z_NEAR;
    static const float  Z_FAR;

    static kexCvar      cvarFOV;

private:
    void                SetupMatrices(void);
    void                MakeClipPlanes(void);
    void                TransformPoints(const kexVec3 &center, const kexVec3 &dir,
                                        const float fov, const float aspect,
                                        const float _near, const float _far);

    kexPlane            p[NUMFRUSTUMPLANES];
    kexVec3             points[NUMFRUSTUMPOINTS];
    kexVec3             origin;
    kexAngle            yaw;
    kexAngle            pitch;
    kexAngle            roll;
    kexMatrix           projectionView;
    kexMatrix           modelView;
    kexMatrix           rotationMatrix;
    kexMatrix           clipMatrix;
    kexQuat             rotation;
    float               fov;
};

#endif
