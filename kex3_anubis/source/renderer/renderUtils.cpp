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
//    Drawing utilities
//

#include "renderMain.h"

static kexRenderUtils renderUtilLocal;
kexRenderUtils *kexRender::cUtils = &renderUtilLocal;

float kexRenderUtils::debugLineNum = 0;

//
// kexRenderUtils::DrawTexturedQuad
//

void kexRenderUtils::DrawTexturedQuad(const kexVec2 &start, const kexVec2 &end,
                                      const float height1, const float height2,
                                      const byte r, const byte g, const byte b)
{
    kexRender::cVertList->BindDrawPointers();
    kexRender::cVertList->AddVertex(start.x, start.y, height1, 0, 0, r, g, b, 255);
    kexRender::cVertList->AddVertex(end.x, end.y, height1, 1, 0, r, g, b, 255);
    kexRender::cVertList->AddVertex(start.x, start.y, height2, 0, 1, r, g, b, 255);
    kexRender::cVertList->AddVertex(end.x, end.y, height2, 1, 1, r, g, b, 255);
    kexRender::cVertList->AddTriangle(0, 1, 2);
    kexRender::cVertList->AddTriangle(2, 1, 3);
    kexRender::cVertList->DrawElements();
}

//
// kexRenderUtils::DrawQuad
//

void kexRenderUtils::DrawQuad(const kexVec2 &start, const kexVec2 &end,
                              const float height1, const float height2,
                              const byte r, const byte g, const byte b)
{

    kexRender::cTextures->whiteTexture->Bind();
    DrawTexturedQuad(start, end, height1, height2, r, g, b);
}

//
// kexRenderUtils::DrawQuad
//

void kexRenderUtils::DrawQuad(const kexVec3 &p1, const kexVec3 &p2, const kexVec3 &p3, const kexVec3 &p4,
                              const byte r, const byte g, const byte b, const byte a)
{
    kexRender::cTextures->whiteTexture->Bind();

    kexRender::cVertList->BindDrawPointers();
    kexRender::cVertList->AddVertex(p1, 0, 0, r, g, b, a);
    kexRender::cVertList->AddVertex(p2, 1, 0, r, g, b, a);
    kexRender::cVertList->AddVertex(p3, 0, 1, r, g, b, a);
    kexRender::cVertList->AddVertex(p4, 1, 1, r, g, b, a);
    kexRender::cVertList->AddTriangle(0, 1, 2);
    kexRender::cVertList->AddTriangle(2, 1, 3);
    kexRender::cVertList->DrawElements();
}

//
// kexRenderUtils::DrawBoundingBox
//

void kexRenderUtils::DrawBoundingBox(const kexBBox &bbox, const byte r, const byte g, const byte b)
{
    kexRender::cTextures->whiteTexture->Bind();

#define ADD_LINE(ba1, ba2, ba3, bb1, bb2, bb3)                      \
    kexRender::cVertList->AddLine(bbox[ba1][0], bbox[ba2][1], bbox[ba3][2],   \
                         bbox[bb1][0], bbox[bb2][1], bbox[bb3][2],  \
                         r, g, b, 255)

    kexRender::cVertList->BindDrawPointers();
    ADD_LINE(0, 0, 0, 1, 0, 0); // 0 1
    ADD_LINE(1, 0, 1, 0, 0, 1); // 2 3
    ADD_LINE(0, 0, 0, 0, 1, 0); // 0 4
    ADD_LINE(0, 1, 1, 0, 0, 1); // 5 3
    ADD_LINE(0, 0, 1, 0, 0, 0); // 3 0
    ADD_LINE(0, 1, 0, 1, 1, 0); // 4 6
    ADD_LINE(1, 1, 1, 0, 1, 1); // 7 5
    ADD_LINE(0, 1, 1, 0, 1, 0); // 5 4
    ADD_LINE(1, 0, 0, 1, 1, 0); // 1 6
    ADD_LINE(1, 1, 0, 1, 1, 1); // 6 7
    ADD_LINE(1, 1, 1, 1, 0, 1); // 7 2
    ADD_LINE(1, 0, 1, 1, 0, 0); // 2 1
    kexRender::cVertList->DrawLineElements();
#undef ADD_LINE
}

//
// kexRenderUtils::DrawFilledBoundingBox
//

void kexRenderUtils::DrawFilledBoundingBox(const kexBBox &bbox,
                                           const byte r, const byte g, const byte b)
{

    word indices[36];
    float points[24];
    byte colors[96];

    kexRender::cTextures->whiteTexture->Bind();

    indices[ 0] = 0;
    indices[ 1] = 1;
    indices[ 2] = 3;
    indices[ 3] = 4;
    indices[ 4] = 7;
    indices[ 5] = 5;
    indices[ 6] = 0;
    indices[ 7] = 4;
    indices[ 8] = 1;
    indices[ 9] = 1;
    indices[10] = 5;
    indices[11] = 6;
    indices[12] = 2;
    indices[13] = 6;
    indices[14] = 7;
    indices[15] = 4;
    indices[16] = 0;
    indices[17] = 3;
    indices[18] = 1;
    indices[19] = 2;
    indices[20] = 3;
    indices[21] = 7;
    indices[22] = 6;
    indices[23] = 5;
    indices[24] = 2;
    indices[25] = 1;
    indices[26] = 6;
    indices[27] = 3;
    indices[28] = 2;
    indices[29] = 7;
    indices[30] = 7;
    indices[31] = 4;
    indices[32] = 3;
    indices[33] = 4;
    indices[34] = 5;
    indices[35] = 1;

    bbox.ToPoints(points);

    for(int i = 0; i < 96 / 4; i++)
    {
        colors[i * 4 + 0] = r;
        colors[i * 4 + 1] = g;
        colors[i * 4 + 2] = b;
        colors[i * 4 + 3] = 255;
    }

    dglDisableClientState(GL_TEXTURE_COORD_ARRAY);
    dglVertexPointer(3, GL_FLOAT, sizeof(float)*3, points);
    dglColorPointer(4, GL_UNSIGNED_BYTE, sizeof(byte)*4, colors);

    dglDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, indices);

    dglEnableClientState(GL_TEXTURE_COORD_ARRAY);
}

//
// kexRenderUtils::DrawRadius
//

void kexRenderUtils::DrawRadius(float x, float y, float z,
                                float radius, float height,
                                const byte r, const byte g, const byte b)
{
    float an;
    int i;

    kexRender::cTextures->whiteTexture->Bind();
    kexRender::cVertList->BindDrawPointers();

    an = kexMath::Deg2Rad(360 / 32);

    for(i = 0; i < 32; i++)
    {
        float s1 = kexMath::Sin(an * i);
        float c1 = kexMath::Cos(an * i);
        float s2 = kexMath::Sin(an * ((i+1)%31));
        float c2 = kexMath::Cos(an * ((i+1)%31));
        float x1 = x + (radius * s1);
        float x2 = x + (radius * s2);
        float y1 = y + (radius * c1);
        float y2 = y + (radius * c2);
        float z1 = z;
        float z2 = z + height;

        kexRender::cVertList->AddLine(x1, y1, z1, x1, y1, z2, r, g, b, 255);
        kexRender::cVertList->AddLine(x1, y1, z1, x2, y2, z1, r, g, b, 255);
        kexRender::cVertList->AddLine(x1, y1, z2, x2, y2, z2, r, g, b, 255);
    }

    kexRender::cVertList->DrawLineElements();
}

//
// kexRenderUtils::DrawOrigin
//

void kexRenderUtils::DrawOrigin(float x, float y, float z, float size)
{
    kexRender::cTextures->whiteTexture->Bind();

    dglDepthRange(0.0f, 0.0f);
    dglLineWidth(2.0f);

    kexRender::cVertList->BindDrawPointers();
    kexRender::cVertList->AddLine(x, y, z, x + size, y, z, 255, 0, 0, 255); // x
    kexRender::cVertList->AddLine(x, y, z, x, y + size, z, 0, 255, 0, 255); // y
    kexRender::cVertList->AddLine(x, y, z, x, y, z + size, 0, 0, 255, 255); // z
    kexRender::cVertList->DrawLineElements();

    dglLineWidth(1.0f);
    dglDepthRange(0.0f, 1.0f);
}

//
// kexRenderUtils::DrawSphere
//

void kexRenderUtils::DrawSphere(float x, float y, float z, float radius,
                                const byte r, const byte g, const byte b)
{
    float points[72];
    int count;
    int i;
    int j;
    int k;
    float s;
    float c;
    float v1[3];
    float v2[3];

    kexRender::cTextures->whiteTexture->Bind();
    kexRender::cVertList->BindDrawPointers();

    count = (360 / 15);
    points[0 * 3 + 0] = x;
    points[0 * 3 + 1] = y;
    points[0 * 3 + 2] = z + radius;

    for(i = 1; i < count; i++)
    {
        points[i * 3 + 0] = points[0 * 3 + 0];
        points[i * 3 + 1] = points[0 * 3 + 1];
        points[i * 3 + 2] = points[0 * 3 + 2];
    }

    for(i = 15; i <= 360; i += 15)
    {
        s = kexMath::Sin(kexMath::Deg2Rad((float)i));
        c = kexMath::Cos(kexMath::Deg2Rad((float)i));

        v1[0] = x;
        v1[1] = y + radius * s;
        v1[2] = z + radius * c;

        for(k = 0, j = 15; j <= 360; j += 15, k++)
        {
            v2[0] = x + kexMath::Sin(kexMath::Deg2Rad((float)j)) * radius * s;
            v2[1] = y + kexMath::Cos(kexMath::Deg2Rad((float)j)) * radius * s;
            v2[2] = v1[2];

            kexRender::cVertList->AddLine(v1[0], v1[1], v2[2], v2[0], v2[1], v2[2], r, g, b, 255);
            kexRender::cVertList->AddLine(v1[0], v1[1], v2[2],
                                points[k * 3 + 0],
                                points[k * 3 + 1],
                                points[k * 3 + 2],
                                r, g, b, 255);

            points[k * 3 + 0] = v1[0];
            points[k * 3 + 1] = v1[1];
            points[k * 3 + 2] = v1[2];

            v1[0] = v2[0];
            v1[1] = v2[1];
            v1[2] = v2[2];
        }
    }

    kexRender::cVertList->DrawLineElements();
}

//
// kexRenderUtils::DrawLine
//

void kexRenderUtils::DrawLine(const kexVec3 &p1, const kexVec3 &p2,
                              const byte r, const byte g, const byte b)
{

    dglDepthRange(0.0f, 0.0f);

    kexRender::cTextures->whiteTexture->Bind();
    kexRender::cVertList->BindDrawPointers();
    kexRender::cVertList->AddLine(p1.x, p1.y, p1.z, p2.x, p2.y, p2.z, r, g, b, 255);
    kexRender::cVertList->DrawLineElements();

    dglDepthRange(0.0f, 1.0f);
}

//
// kexRenderUtils::DrawArrow
//

void kexRenderUtils::DrawArrow(const kexVec3 &p1, const kexVec3 &p2, const int size,
                               const byte r, const byte g, const byte b)
{
    kexVec3 forward, right, up, v1, v2;
    float a, s, d;
    int i;
    static float arrowCos[6];
    static float arrowSin[6];
    static int bMakeTable = false;
    
    DrawLine(p1, p2, r, g, b);
    
    if(bMakeTable == false)
    {
        bMakeTable = true;
        
        for(i = 0, a = 0; a < 360.0f; a += 60, ++i)
        {
            arrowCos[i] = kexMath::Cos(kexMath::Deg2Rad(a));
            arrowSin[i] = kexMath::Sin(kexMath::Deg2Rad(a));
        }
        
        arrowCos[i] = arrowCos[0];
        arrowSin[i] = arrowSin[0];
    }
    
    forward = p2 - p1;
    forward.Normalize();
    
    d = forward.x * forward.x + forward.y * forward.y;
    if(!d)
    {
        right.Set(1, 0, 0);
    }
    else
    {
        d = kexMath::InvSqrt(d);
        right.x = -forward.y * d;
        right.y = forward.x * d;
        right.z = 0;
    }
    
    up = right.Cross(forward);
    
    for(i = 0, a = 0; a < 360.0f; a += 60, ++i)
    {
        s = 0.5f * size * arrowCos[i];
        v1 = p2 - (forward * (float)size);
        v1 = v1 + (right * s);
        s = 0.5f * size * arrowSin[i];
        v1 = v1 + (up * s);
        
        s = 0.5f * size * arrowCos[i+1];
        v2 = p2 - (forward * (float)size);
        v2 = v2 + (right * s);
        s = 0.5f * size * arrowSin[i+1];
        v2 = v2 + (up * s);
        
        DrawLine(v1, p2, r, g, b);
        DrawLine(v1, v2, r, g, b);
    }
}

//
// kexRenderUtils::PrintStatsText
//

void kexRenderUtils::PrintStatsText(const char *title, const char *s, ...)
{
    va_list v;
    static char vastr[1024];
    unsigned int c;
    byte *cb;

    cb = (byte*)&c;
    
    dglMatrixMode(GL_PROJECTION);
    dglPushMatrix();
    dglMatrixMode(GL_MODELVIEW);
    dglPushMatrix();
    
    kexRender::cBackend->SetOrtho();

    if(title != NULL)
    {
        c = RGBA(0, 255, 0, 255);
        kex::cConsole->Font()->DrawString(title, 32, debugLineNum, 1, false, cb, cb);
    }

    if(s != NULL)
    {
        va_start(v, s);
        vsprintf(vastr, s, v);
        va_end(v);

        c = RGBA(255, 255, 0, 255);
        kex::cConsole->Font()->DrawString(vastr, 192, debugLineNum, 1, false, cb, cb);
    }
    
    dglMatrixMode(GL_PROJECTION);
    dglPopMatrix();
    dglMatrixMode(GL_MODELVIEW);
    dglPopMatrix();

    debugLineNum += 16.0f;
}

