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
//      Vertex buffer objects
//
//      If not available or disabled by the user then
//      vertex pointers will be used instead
//
//      Overall, VBOs has a ~0.5ms performance gain
//      compared to vertex pointers
//

#include "renderMain.h"

kexCvar kexVertBuffer::cvarRenderUseVBO("r_usevertexbuffers", CVF_BOOL|CVF_CONFIG, "1", "Enables vertex buffer objects for rendering the scene");
bool kexVertBuffer::bUseVertexBuffers = true;

#define VERT_OFFSET(n)  reinterpret_cast<void*>(OFFSETOF(drawVert_t, n))

//
// kexVertBuffer::kexVertBuffer
//

kexVertBuffer::kexVertBuffer(void)
{
    this->vboID = 0;
    this->iboID = 0;
    this->vertexSize = 0;
    this->indiceSize = 0;
    this->clientVertex = NULL;
    this->clientIndices = NULL;
    this->bClientVertexAllocated = false;
    this->bClientIndicesAllocated = false;
}

//
// kexVertBuffer::~kexVertBuffer
//

kexVertBuffer::~kexVertBuffer(void)
{
    Delete();
}

//
// kexVertBuffer::Allocate
//

void kexVertBuffer::Allocate(drawVert_t *drawVerts, uint size, const bufferUsage_t vertexUsage,
                             uint *indices, uint indiceSize, const bufferUsage_t indiceUsage)
{
    GLenum v, i;
    uint vertSize;
    uint newSize;

    vertSize = sizeof(drawVert_t) - 1;
    newSize = (size + vertSize) & ~vertSize;

    if(!Available())
    {
        if(drawVerts == NULL)
        {
            clientVertex = new drawVert_t[newSize];
            bClientVertexAllocated = true;
        }
        else
        {
            clientVertex = drawVerts;
            bClientVertexAllocated = false;
        }

        if(indices == NULL)
        {
            clientIndices = new uint[indiceSize];
            bClientIndicesAllocated = true;
        }
        else
        {
            clientIndices = indices;
            bClientIndicesAllocated = false;
        }

        this->vertexSize = size;
        this->indiceSize = indiceSize;
        return;
    }

    switch(vertexUsage)
    {
    case RBU_STATIC:
        v = GL_STATIC_DRAW_ARB;
        break;

    case RBU_DYNAMIC:
        v = GL_DYNAMIC_DRAW_ARB;
        break;

    default:
        return;
    }

    switch(indiceUsage)
    {
    case RBU_STATIC:
        i = GL_STATIC_DRAW_ARB;
        break;

    case RBU_DYNAMIC:
        i = GL_DYNAMIC_DRAW_ARB;
        break;

    default:
        return;
    }
    
    dglGenBuffersARB(1, &vboID);
    dglGenBuffersARB(1, &iboID);
    
    dglGetError();

    dglBindBufferARB(GL_ARRAY_BUFFER_ARB, vboID);
    dglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, iboID);
    
    dglBufferDataARB(GL_ARRAY_BUFFER_ARB, sizeof(drawVert_t) * newSize, drawVerts, v);
    this->vertexSize = size;

    if(dglGetError() == GL_OUT_OF_MEMORY)
    {
        Delete();
        kex::cSystem->Error("kexVertBuffer::Allocate: Failed to allocate vertex buffer\n");
        return;
    }
    
    dglBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, sizeof(uint) * indiceSize, indices, i);
    this->indiceSize = indiceSize;
    
    if(dglGetError() == GL_OUT_OF_MEMORY)
    {
        Delete();
        kex::cSystem->Error("kexVertBuffer::Allocate: Failed to allocate element buffer\n");
        return;
    }

    dglFinish();

    dglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    dglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
}

//
// kexVertBuffer::Bind
//

void kexVertBuffer::Bind(void)
{
    kexRender::cVertList->ClearBinded();

    if(!Available())
    {
        return;
    }
    
    dglBindBufferARB(GL_ARRAY_BUFFER_ARB, vboID);
    dglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, iboID);
}

//
// kexVertBuffer::UnBind
//

void kexVertBuffer::UnBind(void)
{
    if(!Available())
    {
        // switch back to array pointers
        kexRender::cVertList->BindDrawPointers();
        return;
    }
    
    dglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    dglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);

    // switch back to array pointers
    kexRender::cVertList->BindDrawPointers();
}

//
// kexVertBuffer::Delete
//

void kexVertBuffer::Delete(void)
{
    if(!Available())
    {
        if(clientVertex && bClientVertexAllocated)
        {
            delete[] clientVertex;
            clientVertex = NULL;
        }
        if(clientIndices && bClientIndicesAllocated)
        {
            delete[] clientIndices;
            clientIndices = NULL;
        }
        return;
    }
    
    if(vboID != 0)
    {
        dglDeleteBuffersARB(1, &vboID);
        vboID = 0;
    }
    
    if(iboID != 0)
    {
        dglDeleteBuffersARB(1, &iboID);
        iboID = 0;
    }
}

//
// kexVertBuffer::Latch
//

void kexVertBuffer::Latch(void)
{
    if(!Available())
    {
        // vertex buffers not available. use vertex pointers instead
        dglTexCoordPointer(2, GL_FLOAT, sizeof(drawVert_t), clientVertex->texCoords.ToFloatPtr());
        dglVertexPointer(3, GL_FLOAT, sizeof(drawVert_t), clientVertex->vertex.ToFloatPtr());
        dglColorPointer(4, GL_UNSIGNED_BYTE, sizeof(drawVert_t), &clientVertex->rgba);
        return;
    }
    
    dglTexCoordPointer(2, GL_FLOAT, sizeof(drawVert_t), VERT_OFFSET(texCoords));
    dglVertexPointer(3, GL_FLOAT, sizeof(drawVert_t), VERT_OFFSET(vertex));
    dglColorPointer(4, GL_UNSIGNED_BYTE, sizeof(drawVert_t), VERT_OFFSET(rgba));
}

//
// kexVertBuffer::Draw
//

void kexVertBuffer::Draw(void)
{
    if(indiceSize == 0)
    {
        return;
    }

    if(!Available())
    {
        dglDrawElements(GL_TRIANGLES, indiceSize, GL_UNSIGNED_INT, clientIndices);
        return;
    }

    dglDrawElements(GL_TRIANGLES, indiceSize, GL_UNSIGNED_INT, 0);
}

//
// kexVertBuffer::Draw
//

void kexVertBuffer::Draw(const uint count, const uint offset)
{
    if(!Available())
    {
        dglDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, &clientIndices[offset/sizeof(uint)]);
        return;
    }

    dglDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, reinterpret_cast<void*>(offset));
}

//
// kexVertBuffer::MapVertexBuffer
//

kexVertBuffer::drawVert_t *kexVertBuffer::MapVertexBuffer(void)
{
    if(!Available())
    {
        return clientVertex;
    }
    
    return (drawVert_t*)dglMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_READ_WRITE_ARB);
}

//
// kexVertBuffer::UnMapVertexBuffer
//

void kexVertBuffer::UnMapVertexBuffer(void)
{
    if(!Available())
    {
        return;
    }

    dglFlush();
    dglUnmapBufferARB(GL_ARRAY_BUFFER_ARB);
}

//
// kexVertBuffer::MapIndiceBuffer
//

uint *kexVertBuffer::MapIndiceBuffer(void)
{
    if(!Available())
    {
        return clientIndices;
    }
    
    return (uint*)dglMapBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, GL_READ_WRITE_ARB);
}

//
// kexVertBuffer::UnMapIndiceBuffer
//

void kexVertBuffer::UnMapIndiceBuffer(void)
{
    if(!Available())
    {
        return;
    }

    dglFlush();
    dglUnmapBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB);
}

//
// kexVertBuffer::GetVertexBufferSize
//

const uint kexVertBuffer::GetVertexBufferSize(void)
{
    GLint val = 0;

    if(!Available())
    {
        return vertexSize * sizeof(drawVert_t);
    }

    dglGetBufferParameterivARB(GL_ARRAY_BUFFER_ARB, GL_BUFFER_SIZE, &val);
    return (uint)val;
}

//
// kexVertBuffer::GetIndiceBufferSize
//

const uint kexVertBuffer::GetIndiceBufferSize(void)
{
    GLint val = 0;

    if(!Available())
    {
        return indiceSize * sizeof(uint);
    }

    dglGetBufferParameterivARB(GL_ELEMENT_ARRAY_BUFFER_ARB, GL_BUFFER_SIZE, &val);
    return (uint)val;
}

//
// kexVertBuffer::Available
//

bool kexVertBuffer::Available(void)
{
    return (has_GL_ARB_vertex_buffer_object && bUseVertexBuffers);
}
