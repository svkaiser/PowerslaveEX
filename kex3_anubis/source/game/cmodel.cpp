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
//      Collision Model
//

#include "kexlib.h"
#include "game.h"
#include "world.h"
#include "actor.h"
#include "cmodel.h"
#include "playLoop.h"

#define CONTACT_COUNT   64

int kexCModel::validcount = 1;

//
// kexCModel::kexCModel
//

kexCModel::kexCModel(void)
{
    Reset();
    sectorList.Init(64);
}

//
// kexCModel::~kexCModel
//

kexCModel::~kexCModel(void)
{
}

//
// kexCModel::Setup
//

void kexCModel::Setup(kexWorld *world)
{
    vertices    = world->Vertices();
    sectors     = world->Sectors();
    faces       = world->Faces();
    polys       = world->Polys();
}

//
// kexCModel::Rese
//

void kexCModel::Reset(void)
{
    vertices    = NULL;
    sectors     = NULL;
    faces       = NULL;
    polys       = NULL;
}

//
// kexCModel::PointInsideFace
//

bool kexCModel::PointInsideFace(const kexVec3 &origin, mapFace_t *face, const float extent)
{
    kexVec3 points[4];
    int vstart;
    float rSq, eSq;
    kexVec3 cp;
    kexVec3 pt1, pt2, pt3, dp1, dp2;
    kexVec3 edge;
    kexVec3 org;

    vstart = face->vertexStart;
    rSq = (extent * extent);

    points[0] = vertices[vstart+3].origin; // bottom
    points[1] = vertices[vstart+2].origin; // bottom
    points[2] = vertices[vstart+1].origin; // top
    points[3] = vertices[vstart+0].origin; // top

    // adjust origin so it lies exactly on the plane
    org = origin - (face->plane.Normal() * PointOnFaceSide(origin, face));

    for(int i = 0; i < 4; ++i)
    {
        pt1 = points[(i+0)&3];
        pt2 = points[(i+1)&3];
        pt3 = points[(i+2)&3];

        dp1 = pt1 - org;
        dp2 = pt2 - org;

        cp = dp1.Cross(dp2);

        if(face->plane.Normal().Dot(cp) >= 0)
        {
            continue;
        }

        if(actorRadius == 0)
        {
            return false;
        }

        // handle radius range checks here
        edge = pt1 - pt2;
        eSq = edge.UnitSq();

        if(cp.UnitSq() > eSq * rSq)
        {
            return false;
        }

        float d = edge.Dot(dp1);

        if(d < 0)
        {
            // check left edge
            edge = pt1 - pt3;
            if(edge.Dot(dp1) < 0 && dp1.UnitSq() > rSq)
            {
                return false;
            }
        }
        else if(d > eSq)
        {
            // check right edge
            edge = pt2 - pt3;
            if(edge.Dot(dp2) < 0 && dp2.UnitSq() > rSq)
            {
                return false;
            }
        }
    }

    return true;
}

//
// kexCModel::TraceFacePlane
//

bool kexCModel::TraceFacePlane(mapFace_t *face, const float extent1, const float extent2)
{
    float d1, d2;
    kexVec3 hit;

    d1 = PointOnFaceSide(start, face, extent1);
    d2 = PointOnFaceSide(end, face, extent1);

    if(d1 <= d2 || d1 < 0 || d2 > 0)
    {
        // no intersection
        return false;
    }

    float frac = (d1 / (d1 - d2));

    if(frac > 1 || frac < 0)
    {
        // not a valid contact
        return false;
    }

    if(frac >= fraction)
    {
        // farther than the current contact
        return false;
    }

    hit.Lerp(start, end, frac);

    if(!PointInsideFace(hit, face, extent2))
    {
        // intersect point not on the face
        return false;
    }

    fraction = frac;
    interceptVector = hit;
    contactNormal = face->plane.Normal();
    contactFace = face;
    return true;
}

//
// kexCModel::TraceFaceVertex
//

bool kexCModel::TraceFaceVertex(mapFace_t *face, const kexVec2 &point)
{
    kexVec2 org;
    kexVec2 dir;
    kexVec2 cDist;
    float cp;
    float rd;
    float r;
    
    org = (point - end.ToVec2());
    dir = moveDir;
    dir.Normalize();
    
    if(dir.Dot(org) < 0)
    {
        return false;
    }
    
    float len = (end - start).Unit();
    
    if(len == 0)
    {
        return false;
    }
    
    cp      = dir.Dot(org);
    cDist   = (org - (dir * cp));
    r       = actorRadius + 8.192f;
    rd      = r * r - cDist.UnitSq();
    
    if(rd < 0)
    {
        return false;
    }
    
    float frac = (cp - kexMath::Sqrt(rd)) * (1.0f / len);
    
    if(frac <= 1 && frac < fraction)
    {
        if(frac < 0)
        {
            frac = 0;
        }
        
        fraction = frac;
        interceptVector = start;
        interceptVector.Lerp(end, frac);
        
        contactNormal.x = interceptVector.x - point.x;
        contactNormal.y = interceptVector.y - point.y;
        contactNormal.z = 0;
        contactNormal.Normalize();
        contactFace = face;
        return true;
    }
    
    return false;
}

//
// kexCModel::CollideFace
//

bool kexCModel::CollideFace(mapFace_t *face)
{
    // try colliding with the end points of the face segment
    if(PointOnFaceSide(end, face) >= 0)
    {
        kexVec3 points[4];
        int vstart = face->vertexStart;
        
        points[0] = vertices[vstart+3].origin; // bottom
        points[1] = vertices[vstart+2].origin; // bottom
        points[2] = vertices[vstart+1].origin; // top
        points[3] = vertices[vstart+0].origin; // top
        
        float rSq = (actorRadius * actorRadius) + (points[3] - points[2]).UnitSq();
        
        if((points[3].z >= end.z || points[2].z >= end.z) &&
           (points[0].z <= end.z || points[1].z <= end.z))
        {
            kexVec2 Q = end.ToVec2();
            kexVec2 A = points[3].ToVec2();
            kexVec2 B = points[2].ToVec2();
            kexVec2 e = B - A;
            
            float u = e.Dot(B - Q);
            float v = e.Dot(Q - A);
            
            // left side
            if(v <= 0)
            {
                kexVec2 P = A;
                kexVec2 d = Q - P;
                float dd = d.UnitSq();
                
                if(dd > rSq)
                {
                    return false;
                }
                
                return TraceFaceVertex(face, P);
            }
            
            // right side
            if(u <= 0)
            {
                kexVec2 P = B;
                kexVec2 d = Q - P;
                float dd = d.UnitSq();
                
                if(dd > rSq)
                {
                    return false;
                }
                
                return TraceFaceVertex(face, P);
            }
        }
    }
    
    // collide with face plane
    return TraceFacePlane(face, actorRadius);
}

//
// kexCModel::SlideAgainstFaces
//

void kexCModel::SlideAgainstFaces(mapSector_t *sector)
{
    for(int i = sector->faceStart; i < sector->faceEnd+3; ++i)
    {
        mapFace_t *face = &faces[i];

        if(face->flags & FF_PORTAL || !(face->flags & FF_SOLID))
        {
            continue;
        }

        if(i <= sector->faceEnd)
        {
            CollideFace(face);
        }
        else
        {
            TraceFacePlane(face, (i == sector->faceEnd+1) ? actorHeight : 0);
        }
    }
}

//
// kexCModel::CheckActorPosition
//

bool kexCModel::CheckActorPosition(kexActor *actor)
{
    mapFace_t *face;
    mapSector_t *sector = actor->Sector();

    for(int i = sector->faceStart; i < sector->faceEnd+1; ++i)
    {
        float d;
        face = &faces[i];

        if(!(face->flags & FF_SOLID))
        {
            continue;
        }

        d = PointOnFaceSide(actor->Origin(), face);

        if(d < actorRadius)
        {
            kexVec3 pt = actor->Origin() + (face->plane.Normal() * kexMath::Fabs(actorRadius - d));

            if(PointInsideFace(pt, face, actorRadius))
            {
                actor->Origin() = pt;
                return false;
            }
        }
    }

    return true;
}

//
// kexCModel::PointOnFaceSide
//

float kexCModel::PointOnFaceSide(const kexVec3 &origin, mapFace_t *face, const float extent)
{
    return face->plane.Distance(origin) - (face->plane.d + extent);
}

//
// kexCModel::PointWithinSectorEdges
//

bool kexCModel::PointWithinSectorEdges(const kexVec3 &origin, mapSector_t *sector, const float extent)
{
    for(int i = sector->faceStart; i < sector->faceEnd+1; ++i)
    {
        mapFace_t *face = &faces[i];
        
        if(PointOnFaceSide(origin, face, extent) >= 0)
        {
            continue;
        }
        
        return false;
    }
    
    return true;
}

//
// kexCModel::PointInsideSector
//

bool kexCModel::PointInsideSector(const kexVec3 &origin, mapSector_t *sector, const float extent)
{
    if(origin.z > GetCeilingHeight(origin, sector))
    {
        return false;
    }

    if(origin.z < GetFloorHeight(origin, sector))
    {
        return false;
    }

    return PointWithinSectorEdges(origin, sector, extent);
}

//
// kexCModel::SectorLinksToSector
//

bool kexCModel::SectorLinksToSector(const kexVec3 &origin, mapSector_t *source, mapSector_t *dest,
                                    const float extent)
{
    for(int i = source->faceStart; i < source->faceEnd+3; ++i)
    {
        mapFace_t *face = &faces[i];
        mapSector_t *s;

        if(!(face->flags & FF_PORTAL) || face->sector <= -1)
        {
            continue;
        }

        s = &sectors[face->sector];

        if(s != dest || s == source)
        {
            continue;
        }

        if(PointOnFaceSide(origin, face, extent) < 0)
        {
            return true;
        }
    }

    return false;
}

//
// kexCModel::GetFloorHeight
//

float kexCModel::GetFloorHeight(const kexVec3 &origin, mapSector_t *sector)
{
    mapVertex_t *v;
    mapFace_t *face;
    float dist;

    face = &faces[sector->faceEnd+2];
    v = &vertices[face->vertexStart];

    dist = kexVec3::Dot(kexVec3(v->origin.x - origin.x,
                                v->origin.y - origin.y,
                                v->origin.z), face->plane.Normal()) / face->plane.c;

    return dist;
}

//
// kexCModel::GetCeilingHeight
//

float kexCModel::GetCeilingHeight(const kexVec3 &origin, mapSector_t *sector)
{
    mapVertex_t *v;
    mapFace_t *face;
    float dist;

    face = &faces[sector->faceEnd+1];
    v = &vertices[face->vertexStart];

    dist = kexVec3::Dot(kexVec3(v->origin.x - origin.x,
                                v->origin.y - origin.y,
                                v->origin.z), face->plane.Normal()) / face->plane.c;

    return dist;
}

//
// kexCModel::RecursiveFindSectors
//

void kexCModel::RecursiveFindSectors(mapSector_t *sector)
{
    if(sector->validcount == validcount)
    {
        return;
    }

    sector->validcount = validcount;
    sector->flags |= SF_DEBUG;
    sectorList.Set(sector);

    for(int i = sector->faceStart; i < sector->faceEnd+3; ++i)
    {
        mapSector_t *s;
        mapFace_t *face;

        face = &faces[i];

        if(!(face->flags & FF_PORTAL) || face->sector <= -1)
        {
            continue;
        }

        s = &sectors[face->sector];

        if(actorBounds.IntersectingBox(s->bounds))
        {
            RecursiveFindSectors(s);
        }
    }
}

//
// kexCModel::GetSurroundingSectors
//

void kexCModel::GetSurroundingSectors(void)
{
    mapSector_t *s;
    float floorz, diff;
    
    RecursiveFindSectors(moveActor->Sector());

    // see if any of the sectors can be stepped into
    for(unsigned int i = 0; i < sectorList.CurrentLength(); ++i)
    {
        s = sectorList[i];

        if(s == moveActor->Sector())
        {
            continue;
        }

        if(SectorLinksToSector(end, moveActor->Sector(), s, actorRadius))
        {
            floorz = GetFloorHeight(end, s);
            diff = end.z - floorz;

            if(diff < 0 && diff >= -moveActor->StepHeight())
            {
                end.z = floorz;
            }
        }
    }
}

//
// kexCModel::MoveActor
//

bool kexCModel::MoveActor(kexActor *actor)
{
    mapSector_t *sector = actor->Sector();
    mapSector_t *oldSector = sector;
    bool bInside = false;
    float r, h;
    int moves;
    kexVec3 cDir;
    float d;
    kexVec3 contactNormals[CONTACT_COUNT];

    if(sector == NULL)
    {
        return false;
    }

    moveActor = actor;
    actorRadius = actor->Radius();
    actorHeight = actor->Height();
    moveDir = actor->Velocity();
    kexVec3::ToAxis(&forwardDir, NULL, NULL, moveDir.ToYaw(), 0, 0);
    start = actor->Origin();
    end = start + moveDir;

    moves = 0;

    for(int i = 0; i < CONTACT_COUNT; ++i)
    {
        fraction = 1;
        end = start + moveDir;
        interceptVector = end;

        contactNormal.Clear();
        contactFace = NULL;

        r = actorRadius*2;
        h = actorHeight*2;

        actorBounds.min.Set(-r, -r, -h);
        actorBounds.max.Set( r,  r,  h);

        actorBounds.min += end;
        actorBounds.max += end;
        actorBounds *= moveDir;
        
        sectorList.Reset();
        RecursiveFindSectors(sector);
        validcount++;

        for(unsigned int j = 0; j < sectorList.CurrentLength(); ++j)
        {
            SlideAgainstFaces(sectorList[j]);
        }

        if(fraction >= 1)
        {
            // went the entire distance
            break;
        }

        if(moves >= CONTACT_COUNT)
        {
            break;
        }

        contactNormals[moves++] = contactNormal;

        // try all interacted normals
        for(int j = 0; j < moves; ++j)
        {
            if(moveDir.Dot(contactNormals[j]) >= 0)
            {
                continue;
            }

            moveDir.Project(contactNormals[j], 1.01f);

            // try bumping against another plane
            for(int k = 0; k < moves; ++k)
            {
                if(k == j || moveDir.Dot(contactNormals[k]) >= 0)
                {
                    continue;
                }

                // bump into second plane
                moveDir.Project(contactNormals[k], 1.01f);

                if(moveDir.Dot(contactNormals[j]) >= 0)
                {
                    continue;
                }

                // slide along the crease between two planes
                cDir = contactNormals[j].Cross(contactNormals[k]).Normalize();
                d = cDir.Dot(moveDir);
                moveDir = cDir * d;
            }
        }
    }

    // TEMP
    for(unsigned int i = 0; i < kex::cGame->World()->NumSectors(); ++i)
    {
        sector = &sectors[i];

        if(PointWithinSectorEdges(end, sector))
        {
            actor->SetSector(sector);
            bInside = true;
            break;
        }
    }
    
    if(bInside == false)
    {
        sector = oldSector;
        end = start;
    }

    float floorz = GetFloorHeight(end, sector);
    float ceilingz = GetCeilingHeight(end, sector);

    if(end.z - floorz < 0)
    {
        end.z = floorz;
    }

    if(ceilingz - end.z < actorHeight)
    {
        end.z = ceilingz - actorHeight;
    }

    actor->SetSector(sector);
    actor->Origin() = end;
    actor->Velocity() = moveDir;
    return true;
}
