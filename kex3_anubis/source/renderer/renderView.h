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

#include "frustum.h"

class kexPlayer;

class kexRenderView
{
public:
    kexVec3             &Origin(void) { return origin; }
    kexAngle            &Yaw(void) { return yaw; }
    kexAngle            &Pitch(void) { return pitch; }
    kexAngle            &Roll(void) { return roll; }
    kexMatrix           &ProjectionView(void) { return projectionView; }
    kexMatrix           &ModelView(void) { return modelView; }
    kexFrustum          &Frustum(void) { return frustum; }
    kexQuat             &Rotation(void) { return rotation; }
    float               &Fov(void) { return fov; }

    void                SetupFromPlayer(kexPlayer *player);
    void                Setup(void);
    kexVec3             ProjectPoint(const kexVec3 &point, kexVec4 *projVector = NULL);

    static const float  Z_NEAR;

private:
    void                SetupMatrices(void);

    kexVec3             origin;
    kexAngle            yaw;
    kexAngle            pitch;
    kexAngle            roll;
    kexMatrix           projectionView;
    kexMatrix           modelView;
    kexQuat             rotation;
    kexFrustum          frustum;
    float               fov;
};

#endif
