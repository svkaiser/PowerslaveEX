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

class kexRenderView
{
public:
    kexRenderView(void);
    ~kexRenderView(void);

    kexVec3         &Origin(void) { return origin; }
    kexAngle        &Angle(void) { return angle; }
    kexVec3         &Forward(void) { return forward; }
    kexVec3         &Right(void) { return right; }
    kexVec3         &Up(void) { return up; }
    kexMatrix       &ProjectionView(void) { return projectionView; }
    kexMatrix       &ModelView(void) { return modelView; }
    kexFrustum      &Frustum(void) { return frustum; }

private:
    kexVec3         origin;
    kexAngle        angle;
    kexVec3         forward;
    kexVec3         right;
    kexVec3         up;
    kexMatrix       projectionView;
    kexMatrix       modelView;
    kexFrustum      frustum;
    float           fov;
};

#endif
