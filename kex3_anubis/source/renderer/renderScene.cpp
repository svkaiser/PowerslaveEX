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
//      Scene Rendering
//

#include "kexlib.h"
#include "game.h"
#include "renderMain.h"
#include "renderView.h"
#include "renderScene.h"

kexCvar kexRenderScene::cvarRenderUseVBO("r_usevertexbuffers", CVF_BOOL|CVF_CONFIG, "1", "Enables vertex buffer objects for rendering the scene");
kexCvar kexRenderScene::cvarRenderWireframe("r_wireframe", CVF_BOOL, "0", "Renders scene in wireframe mode");

bufferUpdateList_t kexRenderScene::bufferUpdateList;

#define RENDERSCENE_DEFINE_DEBUG_COMMAND(var, cmd)  \
    bool kexRenderScene:: var = false;  \
    COMMAND(cmd)    \
    {   \
        if(kex::cCommands->GetArgc() < 1)   \
        {   \
            return; \
        }   \
        \
        kexRenderScene:: var ^= 1;   \
    }

RENDERSCENE_DEFINE_DEBUG_COMMAND(bPrintStats, statscene);
RENDERSCENE_DEFINE_DEBUG_COMMAND(bShowPortals, showportals);
RENDERSCENE_DEFINE_DEBUG_COMMAND(bShowWaterPortals, showwaterportals);
RENDERSCENE_DEFINE_DEBUG_COMMAND(bShowCollision, showcollision);
RENDERSCENE_DEFINE_DEBUG_COMMAND(bShowBounds, showbounds);

//
// kexRenderScene::kexRenderScene
//

kexRenderScene::kexRenderScene(void)
{
    this->world = NULL;
    this->clipY = 0;
    this->polyList.Init(512);
}

//
// kexRenderScene::~kexRenderScene
//

kexRenderScene::~kexRenderScene(void)
{
}

//
// kexRenderScene::InitVertexBuffer
//

void kexRenderScene::InitVertexBuffer(void)
{
    vertexCount = 0;
    indiceCount = 0;
    drawTris = 0;
    drawVerts = NULL;
    drawIndices = NULL;

    bufferUpdateList.Reset();

    vertexBufferLookup = new kexArray<int>[world->NumVertices()];

    // thanks to the unfortunate requirement of treating
    // shared vertices as a unique instance, we need to add
    // extra padding to the total number of vertices in the level
    worldVertexBuffer.Allocate(NULL, world->NumVertices()*3, kexVertBuffer::RBU_STATIC,
                               NULL, world->NumPolys()*8, kexVertBuffer::RBU_STATIC);
}

//
// kexRenderScene::DestroyVertexBuffer
//

void kexRenderScene::DestroyVertexBuffer(void)
{
    delete[] vertexBufferLookup;
    vertexBufferLookup = NULL;
    worldVertexBuffer.Delete();
}

//
// kexRenderScene::DrawSky
//

void kexRenderScene::DrawSky(kexRenderView &view)
{
    kexCpuVertList *vl = kexRender::cVertList;
    float s, c;
    float x = view.Origin().x;
    float y = view.Origin().y;
    float z = view.Origin().z + 1280;
    int t = 0;
    int u = 0;
    float radius = 8192;
    float height = 6144;
    float px[10], py[10], pz[10];
    float lx[10], ly[10], lz[10];
    int tris = 0;
    float ang = 0;
    
    kexRender::cTextures->whiteTexture->Bind();
    
    if(cvarRenderWireframe.GetBool())
    {
        for(uint i = 0; i < visibleSkyFaces.CurrentLength(); ++i)
        {
            mapFace_t *face = &world->Faces()[visibleSkyFaces[i]];
            mapVertex_t *v = &world->Vertices()[face->vertexStart];

            vl->AddVertex(v[0].origin, 0, 0);
            vl->AddVertex(v[1].origin, 0, 0);
            vl->AddVertex(v[2].origin, 0, 0);
            vl->AddVertex(v[3].origin, 0, 0);
            
            vl->AddTriangle(tris+0, tris+2, tris+1);
            vl->AddTriangle(tris+0, tris+3, tris+2);
            tris += 4;
        }
    }
    
    if(visibleSkyFaces.CurrentLength() == 0)
    {
        return;
    }
    
    if(cvarRenderWireframe.GetBool())
    {
        kexRender::cBackend->SetColorMask(0);
        vl->DrawElements();
    }
    
    kexTexture *skyTexture;

    if(world->SkyTexture() == NULL)
    {
        skyTexture = kexRender::cTextures->defaultTexture;
    }
    else
    {
        skyTexture = world->SkyTexture();
    }

    skyTexture->Bind();
    tris = 0;
    
    if(cvarRenderWireframe.GetBool())
    {
        kexRender::cBackend->SetColorMask(1);
        kexRender::cBackend->SetDepth(GLFUNC_GEQUAL);
    }
    
    for(int i = 0; i < 17; i++)
    {
        float z1;
        float ang2;
        
        s = kexMath::Sin(kexMath::Deg2Rad(ang));
        c = kexMath::Cos(kexMath::Deg2Rad(ang));
        
        z1 = z + height * c;
        ang += 22.5f;
        
        if(!(i % 8))
        {
            px[0] = x;
            py[0] = y + radius * s;
            pz[0] = z1;
            
            if(i != 0)
            {
                for(int j = 0; j < 8; j++)
                {
                    vl->AddVertex(px[0], py[0], pz[0], 0, 0);
                    vl->AddVertex(lx[j], ly[j], lz[j], 0, 0);
                    vl->AddVertex(lx[1+j], ly[1+j], lz[1+j], 0, 0);
                    if(i == 8)
                    {
                        vl->AddTriangle(tris+2, tris+1, tris+0);
                        tris += 3;
                    }
                    else
                    {
                        vl->AddTriangle(tris+0, tris+1, tris+2);
                        tris += 3;
                    }
                }
            }
            
            continue;
        }
        
        ang2 = 45;
        
        for(int j = 0; j < 9; j++)
        {
            float x2, y2, z2;
            
            x2 = x + kexMath::Sin(kexMath::Deg2Rad(ang2)) * radius * s;
            y2 = y + kexMath::Cos(kexMath::Deg2Rad(ang2)) * radius * s;
            z2 = z1;
            
            ang2 += 22.5f;
            
            px[1+j] = x2;
            py[1+j] = y2;
            pz[1+j] = z2;
        }
        
        if(i == 1 || i == 9)
        {
            for(int j = 0; j < 8; j++)
            {
                vl->AddVertex(px[0], py[0], pz[0], 0, 0);
                vl->AddVertex(px[1+j], py[1+j], pz[1+j], 0, 0);
                vl->AddVertex(px[2+j], py[2+j], pz[2+j], 0, 0);
                
                if(i >= 9)
                {
                    vl->AddTriangle(tris+2, tris+1, tris+0);
                    tris += 3;
                }
                else
                {
                    vl->AddTriangle(tris+0, tris+1, tris+2);
                    tris += 3;
                }
            }
        }
        else
        {
            float tv1 = (float)t / 6;
            float tv2 = ((float)t + 1) / 6;
            
            for(int j = 0; j < 8; j++)
            {
                float tu1 = (float)u / 4;
                float tu2 = ((float)u + 1) / 4;
                
                if(i >= 9)
                {
                    vl->AddVertex(lx[1+j], ly[1+j], lz[1+j], tu2, 1.0f - tv1);
                    vl->AddVertex(lx[j], ly[j], lz[j], tu1, 1.0f - tv1);
                    vl->AddVertex(px[2+j], py[2+j], pz[2+j], tu2, 1.0f - tv2);
                    vl->AddVertex(px[1+j], py[1+j], pz[1+j], tu1, 1.0f - tv2);
                }
                else
                {
                    vl->AddVertex(lx[j], ly[j], lz[j], tu1, tv1);
                    vl->AddVertex(lx[1+j], ly[1+j], lz[1+j], tu2, tv1);
                    vl->AddVertex(px[1+j], py[1+j], pz[1+j], tu1, tv2);
                    vl->AddVertex(px[2+j], py[2+j], pz[2+j], tu2, tv2);
                }
                
                vl->AddTriangle(tris+0, tris+2, tris+1);
                vl->AddTriangle(tris+1, tris+2, tris+3);
                tris += 4;
                
                u++;
            }
            
            t = (t + 1) % 6;
        }
        
        for(int j = 0; j < 9; j++)
        {
            lx[j] = px[1+j];
            ly[j] = py[1+j];
            lz[j] = pz[1+j];
        }
    }
    
    vertCount += vl->VertexCount();
    triCount += vl->IndiceCount();
    
    vl->DrawElements();

    if(cvarRenderWireframe.GetBool())
    {
        kexRender::cBackend->ClearBuffer(GLCB_DEPTH);
        kexRender::cBackend->SetDepth(GLFUNC_LEQUAL);
    }
}

//
// kexRenderScene::BuildSectorBuffer
//
// Builds a batch of triangles per unique texture that will
// be used for the vertex buffer
//

void kexRenderScene::BuildSectorBuffer(mapSector_t *sector)
{
    static kexStack<int> scanTextures;

    bufferIndex_t *bufferIndex = NULL;
    mapTexCoords_t *tcoord = world->TexCoords();
    mapVertex_t *vertex;
    uint scanTexturesIdx = 0;
    int curTexture = 0;

    scanTextures.Reset();

    // get the initial texture
    for(int j = sector->faceStart; j < sector->faceEnd+3; ++j)
    {
        mapFace_t *face = &world->Faces()[j];

        if(face->polyStart == -1 || face->polyEnd == -1)
        {
            continue;
        }

        if(face->flags & FF_DYNAMIC)
        {
            continue;
        }

        // found one
        scanTextures.Set(world->Polys()[face->polyStart].texture);
        sector->bufferIndex.Resize(sector->bufferIndex.Length()+1);
        break;
    }

    if(scanTextures.CurrentLength() == 0)
    {
        // no textures at all? what?
        return;
    }

    // start off with the initial texture and batch up all polygons
    // containing that same texture. if there are more unique textures
    // found, then add them to scanTextures for the next iteration until
    // there are no more unique textures to scan
    do
    {
        curTexture = scanTextures[scanTexturesIdx];
        bufferIndex = &sector->bufferIndex[scanTexturesIdx];

        // setup new buffer
        bufferIndex->triStart = indiceCount*sizeof(uint);
        bufferIndex->vertStart = vertexCount;
        bufferIndex->count = 0;
        bufferIndex->numVert = 0;
        bufferIndex->numTris = 0;
        bufferIndex->texture = curTexture;
        bufferIndex->sector = sector - world->Sectors();

        // scan all walls
        for(int j = sector->faceStart; j < sector->faceEnd+3; ++j)
        {
            mapFace_t *face = &world->Faces()[j];

            if(face->polyStart == -1 || face->polyEnd == -1)
            {
                continue;
            }

            if(face->flags & FF_DYNAMIC)
            {
                continue;
            }

            // scan all polygons
            for(int k = face->polyStart; k <= face->polyEnd; ++k)
            {
                mapPoly_t *poly = &world->Polys()[k];
                bool bHasTexture = false;
                int indices[4] = { 0, 0, 0, 0 };
                int tcoords[4] = { 0, 0, 0, 0 };
                int curIdx = 0;

                for(uint i = 0; i < scanTextures.CurrentLength(); ++i)
                {
                    if(scanTextures[i] == poly->texture)
                    {
                        bHasTexture = true;
                        break;
                    }
                }

                if(!bHasTexture)
                {
                    // found a new texture. add it to the list
                    sector->bufferIndex.Resize(sector->bufferIndex.Length()+1);
                    bufferIndex = &sector->bufferIndex[scanTexturesIdx];
                    scanTextures.Set(poly->texture);
                }

                if(poly->texture != curTexture)
                {
                    // doesn't match the texture we're scanning for
                    continue;
                }

                // build triangles/vertices from polygon data
                for(int idx = 0; idx < 4; idx++)
                {
                    if(poly->indices[idx] == 0xff || poly->tcoords[idx] == -1)
                    {
                        continue;
                    }
                    
                    indices[curIdx] = poly->indices[idx];
                    tcoords[curIdx] = poly->tcoords[idx];
                    curIdx++;
                }

                if(curIdx <= 2)
                {
                    // bad polygon?
                    continue;
                }

                // setup vertices
                for(int idx = (curIdx-1); idx >= 0; idx--)
                {
                    kexVec3 vPoint;
                    int lookup;
                    int r, g, b;

                    lookup = face->vertStart + indices[idx];
                    vertex = &world->Vertices()[lookup];

                    vPoint = vertex->origin;
                    r = vertex->rgba[0];
                    g = vertex->rgba[1];
                    b = vertex->rgba[2];

                    //
                    // TODO
                    // vertices that are shared with other quads may not be
                    // pointing to the same texture coordinate index, so
                    // each vertex will need to be a unqiue instance.
                    // If there was a way to process the shared vertices, then
                    // I could of easily cut the amount of processed vertices by half
                    //
                    vertexBufferLookup[lookup].Push(vertexCount);

                    drawVerts[vertexCount].vertex = vPoint;
                    drawVerts[vertexCount].texCoords.x = tcoord->uv[tcoords[idx]].s;
                    drawVerts[vertexCount].texCoords.y = 1.0f - tcoord->uv[tcoords[idx]].t;
                    drawVerts[vertexCount].rgba[0] = r;
                    drawVerts[vertexCount].rgba[1] = g;
                    drawVerts[vertexCount].rgba[2] = b;
                    drawVerts[vertexCount].rgba[3] = 255;

                    vertexCount++;
                    bufferIndex->numVert++;
                }

                // mark down triangle indices
                drawIndices[indiceCount++] = drawTris+0;
                drawIndices[indiceCount++] = drawTris+2;
                drawIndices[indiceCount++] = drawTris+1;

                bufferIndex->count += 3;
                bufferIndex->numTris++;

                if(curIdx == 4)
                {
                    drawIndices[indiceCount++] = drawTris+0;
                    drawIndices[indiceCount++] = drawTris+3;
                    drawIndices[indiceCount++] = drawTris+2;

                    bufferIndex->count += 3;
                    bufferIndex->numTris++;
                }

                drawTris += curIdx;

                if(drawTris >= INT_MAX)
                {
                    kex::cSystem->Error("kexRenderScene::BuildSectorBuffer: Triangle Indice Overflow\n");
                    return;
                }
            } 
        }
    } while(++scanTexturesIdx < sector->bufferIndex.Length());
}

//
// kexRenderScene::UpdateBuffer
//
// Used to update moving sectors
//

void kexRenderScene::UpdateBuffer(void)
{
    if(vertexBufferLookup == NULL)
    {
        bufferUpdateList.Reset();
        return;
    }

    for(uint i = 0; i < bufferUpdateList.CurrentLength(); ++i)
    {
        kexArray<int> *list = &vertexBufferLookup[bufferUpdateList[i].index];

        for(uint j = 0; j < list->Length(); ++j)
        {
            uint idx = (*list)[j];

            if(idx >= vertexCount)
            {
                continue;
            }

            drawVerts[idx].vertex = bufferUpdateList[i].newVec;
            drawVerts[idx].rgba[0] = bufferUpdateList[i].newColor[0];
            drawVerts[idx].rgba[1] = bufferUpdateList[i].newColor[1];
            drawVerts[idx].rgba[2] = bufferUpdateList[i].newColor[2];
            drawVerts[idx].rgba[3] = bufferUpdateList[i].newColor[3];
        }
    }

    bufferUpdateList.Reset();
}

//
// kexRenderScene::DrawSector
//

void kexRenderScene::DrawSector(kexRenderView &view, mapSector_t *sector)
{
    int start = sector->faceStart;
    int end = sector->faceEnd;
    
    if(!view.TestBoundingBox(sector->bounds))
    {
        return;
    }
    
    if(sector->flags & SF_DEBUG)
    {
        sector->flags &= ~SF_DEBUG;
    }

    if(bShowBounds)
    {
        kexRender::cUtils->DrawBoundingBox(sector->bounds, 255, 64, 64);
    }

    if(cvarRenderUseVBO.GetBool())
    {
        if(sector->bufferIndex.Length() == 0)
        {
            BuildSectorBuffer(sector);
        }

        for(uint i = 0; i < sector->bufferIndex.Length(); ++i)
        {
            bufferList.Set(sector->bufferIndex[i]);
        }
    }
    
    for(int j = start; j < end+3; ++j)
    {
        DrawFace(view, sector, j);
    }
}

//
// kexRenderScene::DrawPortal
//

void kexRenderScene::DrawPortal(kexRenderView &view, mapFace_t *face, byte r, byte g, byte b)
{
    mapVertex_t *v = &world->Vertices()[face->vertexStart];
    mapSector_t *sector = &world->Sectors()[face->sectorOwner];

    kexRender::cBackend->SetScissorRect((int)sector->x1, (int)sector->y1,
                                        (int)sector->x2, (int)sector->y2);
    
    kexRender::cUtils->DrawLine(v[0].origin, v[1].origin, r, g, b);
    kexRender::cUtils->DrawLine(v[1].origin, v[2].origin, r, g, b);
    kexRender::cUtils->DrawLine(v[2].origin, v[3].origin, r, g, b);
    kexRender::cUtils->DrawLine(v[3].origin, v[0].origin, r, g, b);
}

//
// kexRenderScene::DrawFace
//

void kexRenderScene::DrawFace(kexRenderView &view, mapSector_t *sector, int faceID)
{
    mapFace_t *face = &world->Faces()[faceID];
    
    if(face->flags & FF_OCCLUDED && face->sector <= -1)
    {
        if(faceID <= sector->faceEnd)
        {
            face->flags &= ~FF_OCCLUDED;
            return;
        }
    }
    
    face->validcount = 0;

    if(face->flags & FF_WATER && bShowWaterPortals)
    {
        DrawPortal(view, face, 0, 0, 255);
    }
    
    if(face->flags & FF_PORTAL && bShowPortals)
    {
        DrawPortal(view, face, 255, 0, 255);
    }
    
    if(face->polyStart == -1 || face->polyEnd == -1)
    {
        return;
    }
    
    if(!view.TestBoundingBox(face->bounds))
    {
        return;
    }
    
    if(faceID <= sector->faceEnd && face->plane.PointOnSide(view.Origin()) == kexPlane::PSIDE_BACK)
    {
        return;
    }

    if(bShowBounds)
    {
        kexRender::cUtils->DrawBoundingBox(face->bounds, 64, 255, 0);
    }
    
    if(face->flags & FF_WATER)
    {
        waterFaces.Set(faceID);
        return;
    }
    
    face->flags |= FF_MAPPED;
    face->flags &= ~FF_HIDDEN;

    if(cvarRenderUseVBO.GetBool() && sector->flags & SF_WATER)
    {
        kexVec3 vPoint;
        int r, g, b;

        for(int k = face->vertStart; k <= face->vertEnd; ++k)
        {
            bufferUpdate_t *bufUpdate = kexRenderScene::bufferUpdateList.Get();
            mapVertex_t *vtx = &world->Vertices()[k];

            vPoint = vtx->origin;

            r = vtx->rgba[0];
            g = vtx->rgba[1];
            b = vtx->rgba[2];

            int v = kexGame::cLocal->PlayLoop()->GetWaterVelocityPoint(vPoint.x + vPoint.z, vPoint.y + vPoint.z);
            float max = (((float)r + (float)g + (float)b) / 3) / 3;
            float c = ((float)v / (float)kexGame::cLocal->PlayLoop()->MaxWaterMagnitude()) * max;

            kexMath::Clamp(c, -max, max);

            r += (int)c;
            g += (int)c;
            b += (int)c;

            kexMath::Clamp(r, 0, 255);
            kexMath::Clamp(g, 0, 255);
            kexMath::Clamp(b, 0, 255);

            bufUpdate->index = k;
            bufUpdate->newVec = vtx->origin;
            bufUpdate->newColor[0] = r;
            bufUpdate->newColor[1] = g;
            bufUpdate->newColor[2] = b;
            bufUpdate->newColor[3] = vtx->rgba[3];
        }
    }
    
    for(int k = face->polyStart; k <= face->polyEnd; ++k)
    {
        polyList.Set(k);

        if(cvarRenderUseVBO.GetBool() == true &&
            face->flags & FF_DYNAMIC && !(face->flags & FF_WATER))
        {
            dynamicPolyList.Set(k);
        }
    }
}

//
// kexRenderScene::DrawPolygon
//

void kexRenderScene::DrawPolygon(mapFace_t *face, mapPoly_t *poly)
{
    int tris = 0;
    kexCpuVertList *vl = kexRender::cVertList;
    mapTexCoords_t *tcoord = world->TexCoords();
    mapVertex_t *vertex;
    
    int indices[4] = { 0, 0, 0, 0 };
    int tcoords[4] = { 0, 0, 0, 0 };
    int curIdx = 0;
    
    if(cvarRenderWireframe.GetBool())
    {
        kexRender::cTextures->whiteTexture->Bind();
    }
    else if(world->Textures()[poly->texture])
    {
        world->Textures()[poly->texture]->Bind();
    }
    
    for(int idx = 0; idx < 4; idx++)
    {
        if(poly->indices[idx] == 0xff || poly->tcoords[idx] == -1)
        {
            continue;
        }
        
        indices[curIdx] = poly->indices[idx];
        tcoords[curIdx] = poly->tcoords[idx];
        curIdx++;
    }

    if(curIdx <= 2)
    {
        return;
    }
    
    for(int idx = (curIdx-1); idx >= 0; idx--)
    {
        kexVec3 vPoint;
        int r, g, b;
        vertex = &world->Vertices()[face->vertStart + indices[idx]];

        vPoint = vertex->origin;
        r = vertex->rgba[0];
        g = vertex->rgba[1];
        b = vertex->rgba[2];

        if(world->Sectors()[face->sectorOwner].flags & SF_WATER || face->flags & FF_WATER)
        {
            int v = kexGame::cLocal->PlayLoop()->GetWaterVelocityPoint(vPoint.x + vPoint.z, vPoint.y + vPoint.z);
            float max = (((float)r + (float)g + (float)b) / 3) / 3;
            float c = ((float)v / (float)kexGame::cLocal->PlayLoop()->MaxWaterMagnitude()) * max;

            kexMath::Clamp(c, -max, max);

            r += (int)c;
            g += (int)c;
            b += (int)c;

            kexMath::Clamp(r, 0, 255);
            kexMath::Clamp(g, 0, 255);
            kexMath::Clamp(b, 0, 255);
        }

        if(face->flags & FF_WATER && face->sector >= 0)
        {
            int v = kexGame::cLocal->PlayLoop()->GetWaterVelocityPoint(vPoint.x, vPoint.y);
            vPoint.z += (float)v / 32768.0f;
            
        }
        
        vl->AddVertex(vPoint,
                      tcoord->uv[tcoords[idx]].s, 1.0f - tcoord->uv[tcoords[idx]].t,
                      r, g, b,
                      (face->flags & FF_WATER) ? 128 : 255);
    }
    
    vl->AddTriangle(tris+0, tris+2, tris+1);
    if(curIdx == 4)
    {
        vl->AddTriangle(tris+0, tris+3, tris+2);
    }
    
    tris += curIdx;
    
    vertCount += vl->VertexCount();
    triCount += vl->IndiceCount();
    
    vl->DrawElements();
}

//
// kexRenderScene::DrawWater
//

void kexRenderScene::DrawWater(kexRenderView &view)
{
    kexRender::cBackend->SetBlend(GLSRC_SRC_ALPHA, GLDST_ONE_MINUS_SRC_ALPHA);
    kexRender::cBackend->SetState(GLSTATE_CULL, true);
    kexRender::cBackend->SetDepthMask(0);
    
    for(uint i = 0; i < waterFaces.CurrentLength(); ++i)
    {
        mapFace_t *face = &world->Faces()[waterFaces[i]];
        
        for(int k = face->polyStart; k <= face->polyEnd; ++k)
        {
            DrawPolygon(face, &world->Polys()[k]);
        }
    }
    
    kexRender::cBackend->SetDepthMask(1);
}

//
// kexRenderScene::DrawSprite
//

void kexRenderScene::DrawSprite(kexRenderView &view, mapSector_t *sector, kexActor *actor)
{
    kexCpuVertList  *vl = kexRender::cVertList;
    spriteFrame_t   *frame;
    spriteSet_t     *spriteSet;
    kexSprite       *sprite;
    spriteInfo_t    *info;
    int             rotation;
    kexMatrix       scale;
    kexVec3         org;

    org = actor->Origin();
    scale.Identity(actor->Scale(), actor->Scale(), actor->Scale());

    frame = actor->Frame();
    rotation = 0;

    if(frame->flags & SFF_HASROTATIONS)
    {
        float an = actor->Yaw() - kexMath::ATan2(actor->Origin().x - view.Origin().x,
                                                 actor->Origin().y - view.Origin().y);
        
        kexAngle::Clamp360(an);
        rotation = (int)((an + ((45 / 2) * 9)) / 45);

        if(rotation >= 8) rotation -= 8;
        if(rotation <  0) rotation += 8;
    }

    for(uint i = 0; i < frame->spriteSet[rotation].Length(); ++i)
    {
        int c = 0xff;
        int r, g, b;

        spriteSet = &frame->spriteSet[rotation][i];
        sprite = spriteSet->sprite;
        info = &sprite->InfoList()[spriteSet->index];

        float x = (float)spriteSet->x;
        float y = (float)spriteSet->y;
        float w = (float)info->atlas.w;
        float h = (float)info->atlas.h;

        float u1, u2, v1, v2;
        
        u1 = info->u[0 ^ spriteSet->bFlipped];
        u2 = info->u[1 ^ spriteSet->bFlipped];
        v1 = info->v[0];
        v2 = info->v[1];

        sprite->Texture()->Bind();

        kexVec3 p1 = kexVec3(x, 0, y);
        kexVec3 p2 = kexVec3(x+w, 0, y);
        kexVec3 p3 = kexVec3(x, 0, y+h);
        kexVec3 p4 = kexVec3(x+w, 0, y+h);

        p1 *= spriteMatrix;
        p2 *= spriteMatrix;
        p3 *= spriteMatrix;
        p4 *= spriteMatrix;

        p1 *= scale;
        p2 *= scale;
        p3 *= scale;
        p4 *= scale;

        p1 += org;
        p2 += org;
        p3 += org;
        p4 += org;

        if(!(actor->Flags() & AF_FULLBRIGHT))
        {
            c = (sector->lightLevel << 1) + 32;

            if(c > 255)
            {
                c = 255;
            }
        }

        r = (int)((float)c * actor->Color().x * 2);
        g = (int)((float)c * actor->Color().y * 2);
        b = (int)((float)c * actor->Color().z * 2);

        kexMath::Clamp(r, 0, 255);
        kexMath::Clamp(g, 0, 255);
        kexMath::Clamp(b, 0, 255);

        vl->AddVertex(p1, u1, v1, r, g, b, 255);
        vl->AddVertex(p2, u2, v1, r, g, b, 255);
        vl->AddVertex(p3, u1, v2, r, g, b, 255);
        vl->AddVertex(p4, u2, v2, r, g, b, 255);

        vl->AddTriangle(0, 2, 1);
        vl->AddTriangle(1, 2, 3);

        if(actor->Flags() & AF_FLASH)
        {
            vl->DrawElements(false);
            kexRender::cBackend->SetBlend(GLSRC_ONE, GLDST_ONE);
            vl->DrawElements();
            kexRender::cBackend->SetBlend(GLSRC_SRC_ALPHA, GLDST_ONE_MINUS_SRC_ALPHA);
        }
        else
        {
            vl->DrawElements();
        }
    }

    if(bShowCollision && actor->Flags() & AF_SOLID)
    {
        kexRender::cUtils->DrawRadius(org.x, org.y, org.z - (actor->Height()*0.5f),
                                      actor->Radius(), actor->Height() + actor->StepHeight(),
                                      255, 128, 64);
        kexRender::cUtils->DrawSphere(org.x, org.y, org.z + actor->StepHeight(),
                                      actor->Radius(), 255, 32, 32);
    }

    if(bShowBounds)
    {
        kexRender::cUtils->DrawBoundingBox(actor->Bounds() + actor->Origin(), 32, 128, 255);
    }
}

//
// kexRenderScene::DrawStretchSprite
//

void kexRenderScene::DrawStretchSprite(kexRenderView &view, mapSector_t *sector, kexActor *actor)
{
    kexMatrix       mtx, scale;
    kexVec3         org;
    int             count, tris;
    spriteFrame_t   *frame;
    spriteSet_t     *spriteSet;
    kexSprite       *sprite;
    spriteInfo_t    *info;
    kexCpuVertList  *vl = kexRender::cVertList;

    if(!actor->GetTaggedActor())
    {
        return;
    }

    frame = actor->Frame();

    if(frame->spriteSet[0].Length() == 0)
    {
        return;
    }

    spriteSet = &frame->spriteSet[0][0];
    sprite = spriteSet->sprite;
    info = &sprite->InfoList()[spriteSet->index];

    scale.Identity(actor->Scale(), actor->Scale(), actor->Scale());
    count = -1;

    for(kexActor *child = actor->GetTaggedActor(); ; child = static_cast<kexActor*>(child->Target()))
    {
        int c = 0xff;
        int r, g, b;

        org = child->Origin();

        mtx = kexMatrix(-child->Pitch(), 1) * kexMatrix(child->Yaw() + 1.57f, 2);
        mtx.RotateX(kexMath::pi);

        float x = (float)spriteSet->x;
        float y = (float)spriteSet->y;
        float h = (float)info->atlas.h;

        float u1, u2, v1, v2;
        
        u1 = info->u[0];
        u2 = info->u[1];
        v1 = info->v[0];
        v2 = info->v[1];

        kexVec3 p1 = kexVec3(x, 0, y);
        kexVec3 p2 = kexVec3(x, 0, y+h);

        p1 *= (mtx * scale);
        p2 *= (mtx * scale);

        p1 += org;
        p2 += org;

        if(!(child->Flags() & AF_FULLBRIGHT))
        {
            c = (sector->lightLevel << 1) + 32;

            if(c > 255)
            {
                c = 255;
            }
        }

        r = (int)((float)c * child->Color().x * 2);
        g = (int)((float)c * child->Color().y * 2);
        b = (int)((float)c * child->Color().z * 2);

        kexMath::Clamp(r, 0, 255);
        kexMath::Clamp(g, 0, 255);
        kexMath::Clamp(b, 0, 255);

        vl->AddVertex(p1, u1, v1, r, g, b, 255);
        vl->AddVertex(p2, u1, v2, r, g, b, 255);

        count++;

        if(child == actor)
        {
            break;
        }
    }

    tris = 0;

    for(int i = 0; i < count; ++i)
    {
        vl->AddTriangle(tris+0, tris+2, tris+1);
        vl->AddTriangle(tris+1, tris+2, tris+3);
        tris += 2;
    }

    sprite->Texture()->Bind();
    vl->DrawElements();
}

//
// kexRenderScene::SortPolys
//

int kexRenderScene::SortPolys(const int *p1, const int *p2)
{
    mapPoly_t *poly1 = &kexGame::cLocal->World()->Polys()[*p1];
    mapPoly_t *poly2 = &kexGame::cLocal->World()->Polys()[*p2];

    if(poly1->texture < poly2->texture) return  1;
    if(poly1->texture > poly2->texture) return -1;

    return 0;
}

//
// kexRenderScene::SortBufferLists
//

int kexRenderScene::SortBufferLists(const bufferIndex_t *p1, const bufferIndex_t *p2)
{
    if(p1->texture < p2->texture) return  1;
    if(p1->texture > p2->texture) return -1;

    return 0;
}

//
// kexRenderScene::SortSprites
//

int kexRenderScene::SortSprites(const visSprite_t *vis1, const visSprite_t *vis2)
{
    if(vis1->dist < vis2->dist) return  1;
    if(vis1->dist > vis2->dist) return -1;
    return 0;
}

//
// kexRenderScene::SetSectorScissor
//

void kexRenderScene::SetSectorScissor(mapSector_t *sector)
{
    int rectY = (int)sector->y2;
            
    if(rectY > clipY)
    {
        rectY = clipY;
    }
    
    // backend accepts the values as integers so there's precision loss when
    // converting from float to ints. add -/+1 to the rect to avoid getting
    // tiny seams between walls
    kexRender::cBackend->SetScissorRect((int)sector->x1-1, (int)sector->y1-1,
                                        (int)sector->x2+1, rectY+1);
}

//
// kexRenderScene::DrawIndividualPolygons
//

void kexRenderScene::DrawIndividualPolygons(kexStack<int> &polys)
{
    mapSector_t *prevSector = NULL;

    for(uint i = 0; i < polys.CurrentLength(); ++i)
    {
        mapPoly_t *poly = &world->Polys()[polys[i]];
        mapFace_t *face = &world->Faces()[poly->faceRef];
        mapSector_t *sector = &world->Sectors()[face->sectorOwner];

        if(sector != prevSector)
        {
            SetSectorScissor(sector);
            prevSector = sector;
        }

        DrawPolygon(face, poly);
    }
}

//
// kexRenderScene::DrawGroupedPolygons
//

void kexRenderScene::DrawGroupedPolygons(void)
{
    mapSector_t *prevSector = NULL;

    if(bufferList.CurrentLength() == 0)
    {
        return;
    }

    if(cvarRenderWireframe.GetBool())
    {
        kexRender::cTextures->whiteTexture->Bind();
    }

    for(uint i = 0; i < bufferList.CurrentLength(); ++i)
    {
        bufferIndex_t *bufIndex = &bufferList[i];
        mapSector_t *sector = &world->Sectors()[bufIndex->sector];

        if(sector != prevSector)
        {
            SetSectorScissor(sector);
            prevSector = sector;
        }

        if(!cvarRenderWireframe.GetBool())
        {
            world->Textures()[bufIndex->texture]->Bind();
        }

        worldVertexBuffer.Draw(bufIndex->count, bufIndex->triStart);

        vertCount += bufIndex->numVert;
        triCount += bufIndex->numTris;
    }
}

//
// kexRenderScene::DrawSectors
//

void kexRenderScene::DrawSectors(kexRenderView &view)
{
    if(bPrintStats)
    {
        drawSectorTime = kex::cTimer->GetPerformanceCounter();
    }

    // bind and map vertex buffer for writing
    if(cvarRenderUseVBO.GetBool() == true)
    {
        worldVertexBuffer.Bind();

        drawVerts = worldVertexBuffer.MapVertexBuffer();
        drawIndices = worldVertexBuffer.MapIndiceBuffer();

        assert(drawVerts != NULL);
        assert(drawIndices != NULL);
    }

    bufferList.Reset();
    polyList.Reset();
    dynamicPolyList.Reset();

    // process sectors here
    for(uint i = 0; i < visibleSectors.CurrentLength(); ++i)
    {
        DrawSector(view, &world->Sectors()[visibleSectors[i]]);
    }

    if(cvarRenderUseVBO.GetBool() == true)
    {
        // did any sectors moved? if so then we need to update the vertex buffer
        if(bufferUpdateList.CurrentLength() != 0)
        {
            UpdateBuffer();
        }

        worldVertexBuffer.UnMapVertexBuffer();
        worldVertexBuffer.UnMapIndiceBuffer();

        drawVerts = NULL;
        drawIndices = NULL;
    }

    if(bPrintStats)
    {
        polySortTime = kex::cTimer->GetPerformanceCounter();
    }
    
    if(cvarRenderUseVBO.GetBool() == true)
    {
        bufferList.Sort(kexRenderScene::SortBufferLists);
    }
    else
    {
        polyList.Sort(kexRenderScene::SortPolys);
    }

    dynamicPolyList.Sort(kexRenderScene::SortPolys);
    
    if(bPrintStats)
    {
        polySortTime = kex::cTimer->GetPerformanceCounter() - polySortTime;
    }

    if(cvarRenderWireframe.GetBool())
    {
        kexRender::cBackend->SetPolyMode(GLPOLY_LINE);
    }

    // do the actual drawing here
    if(cvarRenderUseVBO.GetBool() == false)
    {
        DrawIndividualPolygons(polyList);
    }
    else
    {
        worldVertexBuffer.Latch();
        DrawGroupedPolygons();
        worldVertexBuffer.UnBind();
    }

    // draw dynamic geometry
    DrawIndividualPolygons(dynamicPolyList);

    kexRender::cBackend->SetScissorRect(0, 0, kex::cSystem->VideoWidth(), clipY);

    if(cvarRenderWireframe.GetBool())
    {
        kexRender::cBackend->SetPolyMode(GLPOLY_FILL);
    }
    
    if(bPrintStats)
    {
        drawSectorTime = kex::cTimer->GetPerformanceCounter() - drawSectorTime;
    }
}

//
// kexRenderScene::DrawActors
//

void kexRenderScene::DrawActors(kexRenderView &view)
{
    kexMatrix mtx(view.Pitch(), 1);
    kexVec3 org;
    float viewPitch;

    mtx = mtx * kexMatrix(-view.Yaw()-kexMath::pi, 2);

    if(bPrintStats)
    {
        drawActorTime = kex::cTimer->GetPerformanceCounter();
    }

    visSprites.Reset();

    for(uint i = 0; i < visibleSectors.CurrentLength(); ++i)
    {
        mapSector_t *sector = &world->Sectors()[visibleSectors[i]];

        for(kexActor *actor = sector->actorList.Next();
            actor != NULL;
            actor = actor->SectorLink().Next())
        {
            visSprite_t *visSprite;

            if(actor->Anim() == NULL || actor == kexGame::cLocal->Player()->Actor())
            {
                continue;
            }

            if(actor->Flags() & AF_HIDDEN)
            {
                continue;
            }
            
            if(!view.TestBoundingBox(actor->Bounds() + actor->Origin()))
            {
                continue;
            }

            visSprite = visSprites.Get();
            visSprite->actor = actor;

            org = actor->Origin() - view.Origin();
            org *= mtx;

            visSprite->dist = org.UnitSq();
        }
    }

    visSprites.Sort(SortSprites);
    
    viewPitch = -view.Pitch().an;

    if(viewPitch < -1.565f)
    {
        viewPitch = -1.565f;
    }

    spriteMatrix = kexMatrix(viewPitch, 1) * kexMatrix(view.Yaw(), 2);
    spriteMatrix.RotateX(kexMath::pi);
    
    kexRender::cBackend->SetDepthMask(0);

    for(uint i = 0; i < visSprites.CurrentLength(); ++i)
    {
        kexActor *actor = visSprites[i].actor;

        kexRender::cBackend->SetState(GLSTATE_CULL, !(actor->Flags() & AF_STRETCHY));

        if(actor->Flags() & AF_STRETCHY)
        {
            DrawStretchSprite(view, actor->Sector(), actor);
        }
        else
        {
            DrawSprite(view, actor->Sector(), actor);
        }
    }

    kexRender::cBackend->SetDepthMask(1);
    kexRender::cBackend->SetScissorRect(0, 0, kex::cSystem->VideoWidth(), clipY);

    if(bPrintStats)
    {
        drawActorTime = kex::cTimer->GetPerformanceCounter() - drawActorTime;
    }
}

//
// kexRenderScene::Prepare
//

void kexRenderScene::Prepare(kexRenderView &view)
{
    int h;
    
    vertCount = 0;
    triCount = 0;
    
    kexRender::cBackend->LoadProjectionMatrix(view.ProjectionView());
    kexRender::cBackend->LoadModelViewMatrix(view.ModelView());
    
    kexRender::cBackend->ClearBuffer(GLCB_STENCIL);
    
    kexRender::cBackend->SetState(GLSTATE_DEPTHTEST, true);
    kexRender::cBackend->SetState(GLSTATE_ALPHATEST, true);
    kexRender::cBackend->SetState(GLSTATE_BLEND, true);
    kexRender::cBackend->SetState(GLSTATE_SCISSOR, true);
    
    h = kex::cSystem->VideoHeight();
    
    clipY = h - (int)((float)h / (240.0f / 24.0f));
    
    kexRender::cVertList->BindDrawPointers();
    waterFaces.Reset();
}

//
// kexRenderScene::PrintStats
//

void kexRenderScene::PrintStats(void)
{
    if(!bPrintStats)
    {
        return;
    }
    
    kexRender::cUtils->PrintStatsText("Vertices Drawn", "%i", vertCount);
    kexRender::cUtils->PrintStatsText("Triangles Drawn", "%i", triCount/3);
    kexRender::cUtils->PrintStatsText("Visible Sectors", "%i", visibleSectors.CurrentLength());
    kexRender::cUtils->PrintStatsText("Visible Polygons", "%i", polyList.CurrentLength());

    if(cvarRenderUseVBO.GetBool() == true)
    {
        uint vCount, iCount;

        worldVertexBuffer.Bind();
        vCount = worldVertexBuffer.GetVertexBufferSize() / sizeof(kexVertBuffer::drawVert_t);
        iCount = worldVertexBuffer.GetIndiceBufferSize() / sizeof(uint);
        worldVertexBuffer.UnBind();

        kexRender::cUtils->PrintStatsText("Total VBO (Vertices)", "%i/%i", vertexCount, vCount);
        kexRender::cUtils->PrintStatsText("Total VBO (Triangles)", "%i/%i", indiceCount, iCount);
    }
    
    kexRender::cUtils->AddDebugLineSpacing();
    
    kexRender::cUtils->PrintStatsText("Flood Fill Time", "%fms",
                                      kex::cTimer->MeasurePerformance(floodFillTime));
    kexRender::cUtils->PrintStatsText("Draw Sector Time", "%fms",
                                      kex::cTimer->MeasurePerformance(drawSectorTime));
    kexRender::cUtils->PrintStatsText("Draw Actor Time", "%fms",
                                      kex::cTimer->MeasurePerformance(drawActorTime));
    kexRender::cUtils->PrintStatsText("Polygon Sort Time", "%fms",
                                      kex::cTimer->MeasurePerformance(polySortTime));
    
    kexRender::cUtils->AddDebugLineSpacing();
}

//
// kexRenderScene::DrawView
//

void kexRenderScene::DrawView(kexRenderView &view, mapSector_t *sector)
{
    if(!world->MapLoaded())
    {
        return;
    }
    
    FindVisibleSectors(view, sector);
    
    Prepare(view);
    
    DrawSky(view);
    
    DrawSectors(view);

    dLights.Draw(this);

    DrawWater(view);
    
    DrawActors(view);
    
    kexRender::cPostProcess->RenderFXAA();
    kexRender::cPostProcess->RenderBloom();
    
    PrintStats();

    bufferUpdateList.Reset();
}
