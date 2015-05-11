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

#ifndef __VERTEX_BUFFER_H__
#define __VERTEX_BUFFER_H__

#include "renderMain.h"

extern bool has_GL_ARB_vertex_buffer_object;

class kexVertBuffer
{
public:
    kexVertBuffer(void);
    ~kexVertBuffer(void);

    typedef enum
    {
        RBU_STATIC  = 0,
        RBU_DYNAMIC,

        NUMBUFFERUSAGES
    } bufferUsage_t;
    
    typedef struct
    {
        kexVec3 vertex;
        kexVec2 texCoords;
        byte rgba[4];
        byte padding[8];
    } drawVert_t;

    typedef struct
    {
        kexArray<drawVert_t> drawVerts;

        void AddVertex(const float x, const float y, const float z,
                       const float t, const float u,
                       const byte r, const byte g, const byte b, const byte a)
        {
            drawVert_t *dv = drawVerts.Grow();
            dv->vertex.Set(x, y, z);
            dv->texCoords.Set(t, u);
            dv->rgba[0] = r;
            dv->rgba[1] = g;
            dv->rgba[2] = b;
            dv->rgba[3] = a;
        }

        void AddVertex(const float x, const float y, const float z,
                       const float t, const float u)
        {
            AddVertex(x, y, z, t, u, 255, 255, 255, 255);
        }

    } drawVertList_t;

    typedef struct
    {
        kexArray<uint> drawIndices;

        void AddTriangle(const int t0, const int t1, const int t2)
        {
            drawIndices.Push(t0);
            drawIndices.Push(t1);
            drawIndices.Push(t2);
        }

    } drawIndiceList_t;
    
    static kexCvar      cvarRenderUseVBO;
    static bool         bUseVertexBuffers;

    void                Allocate(drawVert_t *drawVerts, uint size, const bufferUsage_t vertexUsage,
                                 uint *indices, uint indiceSize, const bufferUsage_t indiceUsage);
    void                Bind(void);
    void                UnBind(void);
    void                Latch(void);
    void                Draw(void);
    void                Draw(const uint count, const uint offset);
    void                Delete(void);
    drawVert_t          *MapVertexBuffer(void);
    void                UnMapVertexBuffer(void);
    uint                *MapIndiceBuffer(void);
    void                UnMapIndiceBuffer(void);
    const uint          GetVertexBufferSize(void);
    const uint          GetIndiceBufferSize(void);

    static bool         Available(void);

    const uint          VertexSize(void) { return vertexSize; }
    const uint          IndiceSize(void) { return indiceSize; }

private:
    uint                vboID;
    uint                iboID;
    uint                vertexSize;
    uint                indiceSize;
    drawVert_t          *clientVertex;
    uint                *clientIndices;
    bool                bClientVertexAllocated;
    bool                bClientIndicesAllocated;
};

#endif
