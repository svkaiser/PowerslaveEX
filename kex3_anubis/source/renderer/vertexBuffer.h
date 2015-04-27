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

class kexVertBuffer
{
public:
    kexVertBuffer(void);
    ~kexVertBuffer(void);
    
    typedef struct
    {
        kexVec3 vertex;
        kexVec2 texCoords;
        byte rgba[4];
    } drawVert_t;
    
    void                Allocate(drawVert_t *drawVerts, uint size, word *indices, uint indiceSize);
    void                Bind(void);
    void                UnBind(void);
    void                Latch(void);
    void                Delete(void);
    drawVert_t          *MapBuffer(void);
    void                UnMapBuffer(void);

private:
    uint                vboID;
    uint                iboID;
};

#endif
