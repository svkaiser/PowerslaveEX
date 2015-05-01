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

#include "renderMain.h"

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

    if(!has_GL_ARB_vertex_buffer_object)
    {
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
    
    dglBufferDataARB(GL_ARRAY_BUFFER_ARB, sizeof(drawVert_t) * size, drawVerts, v);
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

    dglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    dglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
}

//
// kexVertBuffer::Bind
//

void kexVertBuffer::Bind(void)
{
    if(!has_GL_ARB_vertex_buffer_object)
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
    if(!has_GL_ARB_vertex_buffer_object)
    {
        return;
    }

    dglDisableClientState(GL_INDEX_ARRAY);
    
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
    if(!has_GL_ARB_vertex_buffer_object)
    {
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
    if(!has_GL_ARB_vertex_buffer_object)
    {
        return;
    }

    dglEnableClientState(GL_INDEX_ARRAY);
    
    dglIndexPointer(GL_UNSIGNED_INT, sizeof(uint), 0);
    dglTexCoordPointer(2, GL_FLOAT, sizeof(drawVert_t), VERT_OFFSET(texCoords));
    dglVertexPointer(3, GL_FLOAT, sizeof(drawVert_t), VERT_OFFSET(vertex));
    dglColorPointer(4, GL_UNSIGNED_BYTE, sizeof(drawVert_t), VERT_OFFSET(rgba));
}

//
// kexVertBuffer::Draw
//

void kexVertBuffer::Draw(void)
{
    if(!has_GL_ARB_vertex_buffer_object || indiceSize == 0)
    {
        return;
    }

    dglDrawElements(GL_TRIANGLES, indiceSize, GL_UNSIGNED_INT, 0);
}

//
// kexVertBuffer::Draw
//

void kexVertBuffer::Draw(const uint count, const uint offset)
{
    if(!has_GL_ARB_vertex_buffer_object)
    {
        return;
    }

    dglDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, reinterpret_cast<void*>(offset));
}

//
// kexVertBuffer::MapVertexBuffer
//

kexVertBuffer::drawVert_t *kexVertBuffer::MapVertexBuffer(void)
{
    if(!has_GL_ARB_vertex_buffer_object)
    {
        return NULL;
    }
    
    return (drawVert_t*)dglMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_WRITE_ONLY_ARB);
}

//
// kexVertBuffer::UnMapVertexBuffer
//

void kexVertBuffer::UnMapVertexBuffer(void)
{
    if(!has_GL_ARB_vertex_buffer_object)
    {
        return;
    }

    dglUnmapBufferARB(GL_ARRAY_BUFFER_ARB);
}

//
// kexVertBuffer::MapIndiceBuffer
//

uint *kexVertBuffer::MapIndiceBuffer(void)
{
    if(!has_GL_ARB_vertex_buffer_object)
    {
        return NULL;
    }
    
    return (uint*)dglMapBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, GL_WRITE_ONLY_ARB);
}

//
// kexVertBuffer::UnMapIndiceBuffer
//

void kexVertBuffer::UnMapIndiceBuffer(void)
{
    if(!has_GL_ARB_vertex_buffer_object)
    {
        return;
    }

    dglUnmapBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB);
}

//
// kexVertBuffer::GetVertexBufferSize
//

const uint kexVertBuffer::GetVertexBufferSize(void)
{
    GLint val = 0;

    dglGetBufferParameterivARB(GL_ARRAY_BUFFER_ARB, GL_BUFFER_SIZE, &val);
    return (uint)val;
}

//
// kexVertBuffer::GetIndiceBufferSize
//

const uint kexVertBuffer::GetIndiceBufferSize(void)
{
    GLint val = 0;

    dglGetBufferParameterivARB(GL_ELEMENT_ARRAY_BUFFER_ARB, GL_BUFFER_SIZE, &val);
    return (uint)val;
}
