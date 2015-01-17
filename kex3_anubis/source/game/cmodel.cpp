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
// kexCModel::CheckEdgeSide
//

bool kexCModel::CheckEdgeSide(mapEdge_t *edge, const kexVec3 &dir, const float heightAdjust)
{
    kexVec3 ldir = *edge->v2 - *edge->v1;
    kexVec3 pdir = (end + kexVec3(0, 0, heightAdjust)) - *edge->v1;

    return (pdir.Cross(ldir).Dot(dir) < 0);
}

//
// kexCModel::PointInsideFace
//
// Determines if the point is within the boundaries of the face polygon
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
    
    // could potentially slip through solid walls so extend the top and bottom edges of
    // the wall to prevent this
    if(!(face->BottomEdge()->flags & EGF_TOPSTEP))
    {
        points[0].z -= moveActor->StepHeight();
        points[1].z -= moveActor->StepHeight();
    }
    if(!(face->TopEdge()->flags & EGF_BOTTOMSTEP))
    {
        points[2].z += moveActor->StepHeight();
        points[3].z += moveActor->StepHeight();
    }

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
// Performs a simple intersection test on the face polygon
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
        if(!(face->BottomEdge()->flags & EGF_TOPSTEP))
        {
            // intersect point not on the face
            return false;
        }
        else
        {
            // check to see if there's enough headroom to go under this face
            if(CheckEdgeSide(face->BottomEdge(), face->plane.Normal(), actorHeight) ||
               CheckEdgeSide(face->TopEdge(), face->plane.Normal(), moveActor->StepHeight()))
            {
                // didn't contact the face
                return false;
            }
        }
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
// Performs a intersection test on a 2D-circle
//

bool kexCModel::TraceFaceVertex(mapFace_t *face, const kexVec2 &point)
{
    kexVec2 org;
    kexVec2 dir;
    kexVec2 cDist;
    float cp;
    float rd;
    float r;
    
    org = (point - start.ToVec2());
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
// Performs a collision test between a moving actor and a face
// polygon and edges. Can also check for collision between the
// end points of the face
//

bool kexCModel::CollideFace(mapFace_t *face)
{
    // if this face can be stepped over, then ignore it
    if(face->TopEdge()->flags & EGF_BOTTOMSTEP)
    {
        if(CheckEdgeSide(face->TopEdge(), face->plane.Normal(), moveActor->StepHeight()))
        {
            // walk over this face
            return false;
        }
    }
    
    // check to see if there's enough headroom to go under this face
    if(face->BottomEdge()->flags & EGF_TOPSTEP)
    {
        if(CheckEdgeSide(face->BottomEdge(), face->plane.Normal(), actorHeight))
        {
            // we're under the bottom edge of this face, so skip
            return false;
        }
    }
    
    // try colliding with the end points of the face segment
    if(PointOnFaceSide(start, face) >= 0)
    {
        kexVec3 points[4];
        float zf = start.z + moveActor->StepHeight();
        float zc = start.z + actorHeight;
        int vstart = face->vertexStart;
        
        points[0] = vertices[vstart+3].origin; // bottom
        points[1] = vertices[vstart+2].origin; // bottom
        points[2] = vertices[vstart+1].origin; // top
        points[3] = vertices[vstart+0].origin; // top
        
        float rSq = (actorRadius * actorRadius);
        
        // compute barycentric coordinates
        kexVec2 Q = end.ToVec2();
        kexVec2 A = points[3].ToVec2();
        kexVec2 B = points[2].ToVec2();
        kexVec2 e = B - A;
        
        float u = e.Dot(B - Q);
        float v = e.Dot(Q - A);
        
        // left side
        if(v <= 0 && (points[3].z >= zf && points[0].z <= zc))
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
        if(u <= 0 && (points[2].z >= zf && points[1].z <= zc))
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
            if(i == sector->faceEnd+1 && moveActor->Flags() & AF_CEILINGFRICTION)
            {
                TraceFacePlane(face, actorHeight, actorRadius);
            }
            else if(moveActor->Flags() & AF_FLOORFRICTION)
            {
                TraceFacePlane(face, 0, actorRadius);
            }
        }
    }
}

//
// kexCModel::CheckActorPosition
//

bool kexCModel::CheckActorPosition(kexActor *actor)
{
    return true;
}

//
// kexCModel::PointOnFaceSide
//
// Returns the distance from the face's plane
//

float kexCModel::PointOnFaceSide(const kexVec3 &origin, mapFace_t *face, const float extent)
{
    return face->plane.Distance(origin) - (face->plane.d + extent);
}

//
// kexCModel::PointWithinSectorEdges
//
// Returns true if the origin point is in front of
// all walls in the sector
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
// Returns true if the origin point is completely inside
// the sector
//

bool kexCModel::PointInsideSector(const kexVec3 &origin, mapSector_t *sector,
                                  const float extent, const float floorOffset)
{
    if(origin.z > GetCeilingHeight(origin, sector))
    {
        return false;
    }

    if((origin.z + floorOffset) < GetFloorHeight(origin, sector))
    {
        return false;
    }

    return PointWithinSectorEdges(origin, sector, extent);
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
// Creates a list of all sectors that were contacted
// by the actor's bounding box. Recursively checks
// sectors linked by a face portal as well
//

void kexCModel::RecursiveFindSectors(mapSector_t *sector)
{
    if(sector->validcount == validcount)
    {
        return;
    }

    sector->validcount = validcount;
    sectorList.Set(sector);

    for(int i = sector->faceStart; i < sector->faceEnd+3; ++i)
    {
        mapSector_t *s;
        mapFace_t *face;

        face = &faces[i];

        if(!(face->flags & FF_PORTAL) || face->sector <= -1)
        {
            // could be a solid wall or something
            continue;
        }

        s = &sectors[face->sector];

        // is this linked sector reachable?
        if(actorBounds.IntersectingBox(s->bounds))
        {
            RecursiveFindSectors(s);
        }
    }
}

//
// kexCModel::CheckSurroundingSectors
//
// Looks for linked sectors within the actor's radius
// and tries to clip the actor's z-axis with the ceiling
// or floor height. Actor is not linked to any sectors it
// interacts with
//

void kexCModel::CheckSurroundingSectors(void)
{
    mapSector_t *sec, *s, *best = NULL;
    float maxfloorz, maxceilingz;
    float floorz, ceilingz, diff;
    bool bChangeFloorHeight, bChangeCeilingHeight;
    
    sec = moveActor->Sector();
    maxfloorz = moveActor->FloorHeight();
    maxceilingz = end.z;
    
    bChangeFloorHeight = false;
    bChangeCeilingHeight = false;
    
    for(int i = sec->faceStart; i < sec->faceEnd+3; ++i)
    {
        mapFace_t *face = &faces[i];
        
        if(face->sector <= -1)
        {
            // doesn't link another sector
            continue;
        }
        
        s = &sectors[face->sector];
        
        if(s == sec)
        {
            // skip the sector that we're already in
            continue;
        }
        
        // is the actor crossing this portal face?
        if(PointOnFaceSide(end, face, actorRadius) < 0)
        {
            ceilingz = GetCeilingHeight(end, s);
            floorz = GetFloorHeight(end, s);
            
            diff = end.z - floorz;
            
            // determine the closest floor that can be stepped on
            if(diff <= 0 && diff >= -moveActor->StepHeight())
            {
                if(floorz > maxfloorz && moveActor->Velocity().z <= 0)
                {
                    best = s;
                    maxfloorz = floorz;
                    bChangeFloorHeight = true;
                }
            }
            
            // determine the ceiling closest to the actor
            if(ceilingz > floorz && ceilingz - end.z < actorHeight)
            {
                if(ceilingz > maxceilingz)
                {
                    best = s;
                    maxceilingz = ceilingz;
                    bChangeCeilingHeight = true;
                }
            }
        }
    }
    
    if(bChangeFloorHeight && best)
    {
        if(PointInsideSector(end, best, -actorRadius, moveActor->StepHeight()))
        {
            // step up into this sector
            end.z = maxfloorz;
            moveActor->FloorHeight() = maxfloorz;
        }
    }
    
    if(bChangeCeilingHeight)
    {
        // bump into the ceiling
        end.z = maxceilingz - actorHeight;
        moveActor->CeilingHeight() = maxceilingz;
    }
}

//
// kexCModel::CollideActorWithWorld
//
// Project the movement direction with all faces and sectors
// the actor makes contact with
//

void kexCModel::CollideActorWithWorld(void)
{
    mapSector_t *sector = moveActor->Sector();
    int moves;
    float r, h, d;
    kexVec3 cDir;
    kexVec3 contactNormals[CONTACT_COUNT];
    
    moves = 0;
    r = actorRadius*2;
    h = actorHeight*2;
    
    for(int i = 0; i < CONTACT_COUNT; ++i)
    {
        kexVec3::ToAxis(&forwardDir, NULL, NULL, moveDir.ToYaw(), 0, 0);
        
        fraction = 1;
        end = start + moveDir;
        interceptVector = end;
        
        contactNormal.Clear();
        contactFace = NULL;
        
        actorBounds.min.Set(-r, -r, -h);
        actorBounds.max.Set( r,  r,  h);
        
        actorBounds.min += end;
        actorBounds.max += end;
        actorBounds *= moveDir;
        
        sectorList.Reset();
        RecursiveFindSectors(sector);
        validcount++;
        
        // start tracing against all faces per contacted sector
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
}

//
// kexCModel::AdvanceActorToSector
//
// Determines the sector that the new origin point has landed
// on. Assumes that the actor's movement was already clipped
//

void kexCModel::AdvanceActorToSector(void)
{
    mapSector_t *sector = moveActor->Sector();
    
    for(unsigned int i = 0; i < sectorList.CurrentLength(); ++i)
    {
        if(PointInsideSector(end, sectorList[i], 0, moveActor->StepHeight()))
        {
            sector = sectorList[i];
        }
    }
    
    // clamp z-axis to floor and ceiling
    float floorz = GetFloorHeight(end, sector);
    float ceilingz = GetCeilingHeight(end, sector);
    
    if(end.z - floorz < 0)
    {
        end.z = floorz;
        moveDir.z = 0;
    }
    
    if(ceilingz - end.z < actorHeight)
    {
        end.z = ceilingz - actorHeight;
    }
    
    sector->flags |= SF_DEBUG;
    
    // update actor info
    moveActor->FloorHeight() = floorz;
    moveActor->CeilingHeight() = ceilingz;
    moveActor->SetSector(sector);
}

//
// kexCModel::MoveActor
//

bool kexCModel::MoveActor(kexActor *actor)
{
    mapSector_t *sector = actor->Sector();

    if(sector == NULL)
    {
        return false;
    }

    moveActor = actor;
    actorRadius = actor->Radius();
    actorHeight = actor->Height();
    moveDir = actor->Velocity();
    start = actor->Origin();

    // interact with world
    CollideActorWithWorld();
    
    // update sector position
    AdvanceActorToSector();
    
    // check surrounding sectors that can be
    // stepped on
    CheckSurroundingSectors();
    
    actor->Origin() = end;
    actor->Velocity() = moveDir;
    actor->LinkArea();
    return true;
}
