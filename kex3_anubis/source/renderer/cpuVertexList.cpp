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
//      Vertex pointers
//

#include "renderMain.h"

static kexCpuVertList cpuVertList;
kexCpuVertList *kexRender::cVertList = &cpuVertList;

//
// kexCpuVertList::kexCpuVertList
//

kexCpuVertList::kexCpuVertList(void)
{
    this->bBinded = false;
    Reset();
}

//
// kexCpuVertList::Reset
//

void kexCpuVertList::Reset(void)
{
    indiceCount = 0;
    vertexCount = 0;
    roverIndices = drawIndices;
    roverVertices = drawVertices;
    roverTexCoords = drawTexCoords;
    roverRGB = drawRGB;
}

//
// kexCpuVertList::BindDrawPointers
//

void kexCpuVertList::BindDrawPointers(void)
{
    if(!bBinded)
    {
        dglTexCoordPointer(2, GL_FLOAT, sizeof(float)*2, drawTexCoords);
        dglVertexPointer(3, GL_FLOAT, sizeof(float)*3, drawVertices);
        dglColorPointer(4, GL_UNSIGNED_BYTE, sizeof(byte)*4, drawRGB);

        bBinded = true;
    }

    Reset();
}

//
// kexCpuVertList::AddTriangle
//

void kexCpuVertList::AddTriangle(int v0, int v1, int v2)
{
    if((vertexCount * 3) >= CPU_VERT_NUM_XYZ)
    {
        return;
    }

    *(roverIndices++) = v0;
    *(roverIndices++) = v1;
    *(roverIndices++) = v2;

    indiceCount += 3;
}

//
// kexCpuVertList::AddVertex
//

void kexCpuVertList::AddVertex(float x, float y, float z, float s, float t,
                               byte r, byte g, byte b, byte a)
{
    if((vertexCount * 3) >= CPU_VERT_NUM_XYZ)
    {
        return;
    }

    *(roverVertices++)   = x;
    *(roverVertices++)   = y;
    *(roverVertices++)   = z;
    *(roverTexCoords++)  = s;
    *(roverTexCoords++)  = t;
    *(roverRGB++)        = r;
    *(roverRGB++)        = g;
    *(roverRGB++)        = b;
    *(roverRGB++)        = a;

    vertexCount++;
}

//
// kexCpuVertList::AddVertex
//

void kexCpuVertList::AddVertex(float x, float y, float z, float s, float t)
{
    AddVertex(x, y, z, s, t, 255, 255, 255, 255);
}

//
// kexCpuVertList::AddVertex
//

void kexCpuVertList::AddVertex(float x, float y, float z, float s, float t, byte *rgba)
{
    AddVertex(x, y, z, s, t, rgba[0], rgba[1], rgba[2], rgba[3]);
}

//
// kexCpuVertList::AddVertex
//

void kexCpuVertList::AddVertex(float x, float y, float z, float s, float t, const byte *rgba)
{
    AddVertex(x, y, z, s, t, rgba[0], rgba[1], rgba[2], rgba[3]);
}

//
// kexCpuVertList::AddVertex
//

void kexCpuVertList::AddVertex(const kexVec3 &vec, float s, float t,
                               byte r, byte g, byte b, byte a)
{
    AddVertex(vec.x, vec.y, vec.z, s, t, r, g, b, a);
}

//
// kexCpuVertList::AddVertex
//

void kexCpuVertList::AddVertex(const kexVec3 &vec, float s, float t, byte *rgba)
{
    AddVertex(vec.x, vec.y, vec.z, s, t, rgba[0], rgba[1], rgba[2], rgba[3]);
}

//
// kexCpuVertList::AddVertex
//

void kexCpuVertList::AddVertex(const kexVec3 &vec, float s, float t)
{
    AddVertex(vec, s, t, 255, 255, 255, 255);
}

//
// kexCpuVertList::AddLine
//

void kexCpuVertList::AddLine(float x1, float y1, float z1,
                             float x2, float y2, float z2,
                             byte r, byte g, byte b, byte a)
{

    *(roverIndices++) = vertexCount;
    indiceCount++;
    AddVertex(x1, y1, z1, 0, 0, r, g, b, a);
    *(roverIndices++) = vertexCount;
    indiceCount++;
    AddVertex(x2, y2, z2, 0, 0, r, g, b, a);
}

//
// kexCpuVertList::AddLine
//

void kexCpuVertList::AddLine(float x1, float y1, float z1,
                             float x2, float y2, float z2,
                             byte r1, byte g1, byte b1, byte a1,
                             byte r2, byte g2, byte b2, byte a2)
{

    *(roverIndices++) = vertexCount;
    indiceCount++;
    AddVertex(x1, y1, z1, 0, 0, r1, g1, b1, a1);
    *(roverIndices++) = vertexCount;
    indiceCount++;
    AddVertex(x2, y2, z2, 0, 0, r2, g2, b2, a2);
}

//
// kexCpuVertList::AddQuad
//

void kexCpuVertList::AddQuad(float x, float y, float z, float w, float h,
                             float s1, float t1, float s2, float t2,
                             byte r, byte g, byte b, byte a)
{
    int cnt = vertexCount;
    
    AddVertex(x, y, z, s1, t1, r, g, b, a);
    AddVertex(x+w, y, z, s2, t1, r, g, b, a);
    AddVertex(x, y+h, z, s1, t2, r, g, b, a);
    AddVertex(x+w, y+h, z, s2, t2, r, g, b, a);
    
    AddTriangle(cnt+0, cnt+2, cnt+1);
    AddTriangle(cnt+1, cnt+2, cnt+3);
}

//
// kexCpuVertList::AddQuad
//

void kexCpuVertList::AddQuad(float x, float y, float z, float w, float h,
                             byte r, byte g, byte b, byte a)
{
    AddQuad(x, y, z, w, h, 0, 0, 1, 1, r, g, b, a);
}

//
// kexCpuVertList::AddQuad
//

void kexCpuVertList::AddQuad(float x, float y, float w, float h,
                             byte r, byte g, byte b, byte a)
{
    AddQuad(x, y, 0, w, h, 0, 0, 1, 1, r, g, b, a);
}

//
// kexCpuVertList::AddQuad
//

void kexCpuVertList::AddQuad(float x, float y, float z, float w, float h)
{
    AddQuad(x, y, z, w, h, 0, 0, 1, 1, 255, 255, 255, 255);
}

//
// kexCpuVertList::AddQuad
//

void kexCpuVertList::AddQuad(float x, float y, float w, float h)
{
    AddQuad(x, y, 0, w, h, 0, 0, 1, 1);
}

//
// kexCpuVertList::DrawElements
//

void kexCpuVertList::DrawElements(const bool bClearCount)
{
    dglDrawElements(GL_TRIANGLES, indiceCount, GL_UNSIGNED_SHORT, drawIndices);

    if(bClearCount)
    {
        Reset();
    }
}

//
// kexCpuVertList::DrawLineElements
//

void kexCpuVertList::DrawLineElements(void)
{
    dglDrawElements(GL_LINES, indiceCount, GL_UNSIGNED_SHORT, drawIndices);
    Reset();
}
