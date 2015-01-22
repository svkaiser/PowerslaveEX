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

#ifndef __VIEWBOUNDS_H__
#define __VIEWBOUNDS_H__

class kexRenderView;

class kexViewBounds
{
public:
    kexViewBounds(void);

    void                Clear(void);
    const bool          IsClosed(void) const;
    void                Fill(void);
    void                AddPoint(const float x, const float y, const float z);
    void                AddVector(kexRenderView *view, kexVec3 &vector);
    void                AddBox(kexRenderView *view, kexBBox &box);
    bool                ViewBoundInside(const kexViewBounds &viewBounds);
    void                DebugDraw(void);

    kexViewBounds       &operator=(const kexViewBounds &viewBounds);

    float               *Min(void) { return min; }
    float               *Max(void) { return max; }
    const float         ZFar(void) const { return zfar; }

private:
    float               min[2];
    float               max[2];
    float               zmin;
    float               zfar;
};

#endif
