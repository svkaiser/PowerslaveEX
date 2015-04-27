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
#include "vertexBuffer.h"

//
// kexVertBuffer::kexVertBuffer
//

kexVertBuffer::kexVertBuffer(void)
{
    this->vboID = 0;
    this->iboID = 0;
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

void kexVertBuffer::Allocate(drawVert_t *drawVerts, uint size, word *indices, uint indiceSize)
{
    if(!has_GL_ARB_vertex_buffer_object)
    {
        return;
    }
    
    dglGenBuffersARB(1, &vboID);
    dglGenBuffersARB(1, &iboID);
    
    dglGetError();
    
    dglBufferDataARB(GL_ARRAY_BUFFER_ARB, sizeof(drawVert_t) * size, drawVerts, GL_STATIC_DRAW_ARB);
    
    if(dglGetError() == GL_OUT_OF_MEMORY)
    {
        kex::cSystem->Error("kexVertBuffer::Allocate: Failed to allocate vertex buffer\n");
        return;
    }
    
    dglBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, sizeof(word) * indiceSize, indices, GL_STATIC_DRAW_ARB);
    
    if(dglGetError() == GL_OUT_OF_MEMORY)
    {
        kex::cSystem->Error("kexVertBuffer::Allocate: Failed to allocate element buffer\n");
        return;
    }
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
    
    dglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    dglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
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
    
    dglIndexPointer(GL_UNSIGNED_SHORT, 0, 0);
    dglTexCoordPointer(2, GL_FLOAT, sizeof(float)*2, 0);
    dglVertexPointer(3, GL_FLOAT, sizeof(float)*3, 0);
    dglColorPointer(4, GL_UNSIGNED_BYTE, sizeof(byte)*4, 0);
}

//
// kexVertBuffer::MapBuffer
//

kexVertBuffer::drawVert_t *kexVertBuffer::MapBuffer(void)
{
    if(!has_GL_ARB_vertex_buffer_object)
    {
        return NULL;
    }
    
    return (drawVert_t*)dglMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_WRITE_ONLY_ARB);
}
