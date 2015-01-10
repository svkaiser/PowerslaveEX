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

#ifndef __RENDER_UTILS_H__
#define __RENDER_UTILS_H__

#define DEBUG_LINE_TOP  32

class kexRenderUtils
{
public:
    static void     DrawTexturedQuad(const kexVec2 &start, const kexVec2 &end,
                                     const float height1, const float height2,
                                     const byte r, const byte g, const byte b);
    static void     DrawQuad(const kexVec2 &start, const kexVec2 &end,
                             const float height1, const float height2,
                             const byte r, const byte g, const byte b);
    static void     DrawQuad(const kexVec3 &p1, const kexVec3 &p2, const kexVec3 &p3, const kexVec3 &p4,
                             const byte r, const byte g, const byte b, const byte a);
    static void     DrawBoundingBox(const kexBBox &bbox,
                                    const byte r, const byte g, const byte b);
    static void     DrawFilledBoundingBox(const kexBBox &bbox,
                                          const byte r, const byte g, const byte b);
    static void     DrawRadius(float x, float y, float z,
                               float radius, float height,
                               const byte r, const byte g, const byte b);
    static void     DrawOrigin(float x, float y, float z, float size);
    static void     DrawSphere(float x, float y, float z, float radius,
                               const byte r, const byte g, const byte b);
    static void     DrawLine(const kexVec3 &p1, const kexVec3 &p2,
                             const byte r, const byte g, const byte b);
    static void     DrawArrow(const kexVec3 &p1, const kexVec3 &p2, const int size,
                              const byte r, const byte g, const byte b);
    static void     PrintStatsText(const char *title, const char *s, ...);

    static void     ClearDebugLine(void) { debugLineNum = DEBUG_LINE_TOP; }
    static void     AddDebugLineSpacing(void) { debugLineNum += 16; }

    static float    debugLineNum;
};

#endif
