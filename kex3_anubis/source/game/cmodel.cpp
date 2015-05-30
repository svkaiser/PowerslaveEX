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

#define CONTACT_COUNT   5

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
// Returns true if the end point of the trace
// is away from the edge (assumes the edge is in
// clockwise order)
//

bool kexCModel::CheckEdgeSide(const kexVec3 &origin, mapEdge_t *edge, mapFace_t *face,
                              const float heightAdjust, const float extent)
{
    kexVec3 ldir = *edge->v2 - *edge->v1;
    kexVec3 pdir = (origin + kexVec3(0, 0, heightAdjust)) - *edge->v1;
    kexVec3 cp = pdir.Cross(ldir);

    if(cp.Dot(face->plane.Normal()) < 0)
    {
        float rSq;

        if(extent == 0)
        {
            return true;
        }

        rSq = extent * extent;

        if(cp.UnitSq() > (ldir.UnitSq() * rSq))
        {
            return true;
        }
    }

    return false;
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

bool kexCModel::TraceFacePlane(mapFace_t *face, const float extent1, const float extent2, const bool bTestOnly)
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
        if(!moveActor)
        {
            return false;
        }

        // check to see if there's enough headroom to go under this face
        if(CheckEdgeSide(end, face->BottomEdge(), face, actorHeight) ||
           CheckEdgeSide(end, face->TopEdge(), face, moveActor->StepHeight()))
        {
            // didn't contact the face
            return false;
        }

        if(CheckEdgeSide(end, face->LeftEdge(), face, 0, extent1) ||
           CheckEdgeSide(end, face->RightEdge(), face, 0, extent1))
        {
            return false;
        }
    }

    if(bTestOnly)
    {
        return true;
    }

    fraction = frac;
    interceptVector = hit;
    contactNormal = face->plane.Normal();
    contactFace = face;
    contactSector = &sectors[face->sectorOwner];
    return true;
}

//
// kexCModel::TestIntersectSector
//
// A much simplier version of TraceFacePlane
//

bool kexCModel::TestIntersectSector(mapFace_t *face, const float extent)
{
    float d1, d2;
    kexVec3 hit;

    d1 = PointOnFaceSide(start, face, extent);
    d2 = PointOnFaceSide(end, face, extent);

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

    // subtract by a small episilon unit; this is needed so that
    // the AI's check sight trace won't fall through small gaps
    if(!PointWithinSectorEdges(hit, &sectors[face->sectorOwner], extent - 4.096f))
    {
        return false;
    }

    fraction = frac;
    interceptVector = hit;
    contactNormal = face->plane.Normal();
    contactFace = face;
    contactSector = &sectors[face->sectorOwner];
    return true;
}

//
// kexCModel::TraceSphere
//
// Performs a intersection test on a 2D-circle
//

bool kexCModel::TraceSphere(const float radius, const kexVec2 &point,
                            const float heightMax, const float heightMin,
                            const bool bTestOnly)
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
    r       = radius + 1.024f;
    rd      = r * r - cDist.UnitSq();
    
    if(rd < 0)
    {
        return false;
    }
    
    float frac = (cp - kexMath::Sqrt(rd)) * (1.0f / len);
    
    if(frac <= 1 && frac < fraction)
    {
        if(bTestOnly)
        {
            return true;
        }
        
        kexVec3 hit;
        
        hit = start;
        hit.Lerp(end, frac);
        
        if(heightMax != 0)
        {
            if(hit.z > heightMax)
            {
                return false;
            }
        }
        if(heightMin != 0)
        {
            if(hit.z < heightMin)
            {
                return false;
            }
        }
        
        fraction = frac;
        interceptVector = hit;
        contactNormal.x = interceptVector.x - point.x;
        contactNormal.y = interceptVector.y - point.y;
        contactNormal.z = 0;
        contactNormal.Normalize();
        contactFace = NULL;
        return true;
    }
    
    return false;
}

//
// kexCModel::TraceSphere
//
// Performs a intersection test on a 3D-circle
//

bool kexCModel::TraceSphere(const float radius, const kexVec3 &point)
{
    kexVec3 org;
    kexVec3 dir;
    kexVec3 cDist;
    float cp;
    float rd;
    float r;
    
    org = (point - start);
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
    r       = radius + 1.024f;
    rd      = r * r - cDist.UnitSq();
    
    if(rd < 0)
    {
        return false;
    }
    
    float frac = (cp - kexMath::Sqrt(rd)) * (1.0f / len);
    
    if(frac >= -1 && frac <= 1 && frac < fraction)
    {
        kexVec3 hit;
        
        hit = start;
        hit.Lerp(end, frac);
        
        fraction = frac;
        interceptVector = hit;
        contactNormal = interceptVector - point;
        contactNormal.Normalize();
        contactFace = NULL;
        return true;
    }
    
    return false;
}

//
// kexCModel::PushFromRadialBounds
//

void kexCModel::PushFromRadialBounds(const kexVec2 &point, const float radius)
{
    kexVec2 org;
    float dist;
    float r;
    
    org = end;
    dist = org.DistanceSq(point);
    r = radius + 1.024f;

    if(dist <= (r * r))
    {
        kexVec2 dir = (org - point).Normalize();
        dist = (r - kexMath::Sqrt(dist)) + 0.1f;

        moveDir.x += dir.x * dist;
        moveDir.y += dir.y * dist;
    }
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
    if(CheckEdgeSide(end, face->TopEdge(), face, moveActor->StepHeight()))
    {
        // walk over this face
        return false;
    }
    
    // check to see if there's enough headroom to go under this face
    if(face->BottomEdge()->flags & EGF_TOPSTEP)
    {
        if(CheckEdgeSide(end, face->BottomEdge(), face, actorHeight))
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
            
            if(TraceSphere(actorRadius, P))
            {
                contactFace = face;
            }
            return false;
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
            
            if(TraceSphere(actorRadius, P))
            {
                contactFace = face;
            }
            return false;
        }
    }
    
    // collide with face plane
    return TraceFacePlane(face, actorRadius);
}

//
// kexCModel::TraceActorsInSector
//

void kexCModel::TraceActorsInSector(mapSector_t *sector)
{
    if(moveActor && !(moveActor->Flags() & AF_SOLID) &&
        !moveActor->InstanceOf(&kexProjectile::info))
    {
        // this moveActor is not a valid object to test collision with
        return;
    }
    
    // test all linked actors in this sector
    for(kexActor *actor = sector->actorList.Next();
        actor != NULL;
        actor = actor->SectorLink().Next())
    {
        float r;
        
        if(actor == sourceActor)
        {
            // don't check self
            continue;
        }

        if(!(actor->Flags() & AF_SOLID) &&
           !(actor->Flags() & AF_TOUCHABLE) &&
           !(actor->Flags() & AF_SHOOTABLE))
        {
            // ignore this actor
            continue;
        }
        
        // if we're doing movement collision tests then perform a 2D-intersection
        // test with the actor's radial bounds, otherwise do 3D-intersection when
        // performing ray-trace tests.
        if(moveActor)
        {
            float underLip, z, height;
            bool bTestOnly = false;
            bool bTestProjectile = false;
            bool bIsAProjectile = moveActor->InstanceOf(&kexProjectile::info);

            if(bIsAProjectile && actor == moveActor->Target())
            {
                // don't let projectiles collide with its source/owner
                continue;
            }

            underLip = actor->Radius() - actor->StepHeight();

            if(underLip < 0 || bIsAProjectile)
            {
                underLip = 0;
            }

            z = actor->Origin().z;
            height = actor->Height() + underLip;

            if(end.z > z + height || actorHeight + end.z < (z - underLip))
            {
                // either over or under actor
                continue;
            }

            r = (actor->Radius() * 0.5f) + moveActor->Radius();
            bTestProjectile = (bIsAProjectile && actor->Flags() & AF_SHOOTABLE);

            // do actual collision if the following conditions are met:
            // 1: the actor is solid
            // 2: the moveactor is a projectile AND the actor we're testing is shootable
            // 3: the actor is touchable

            if(!bTestProjectile && (actor->Flags() & AF_SOLID) == 0)
            {
                bTestOnly = true;
            }
            
            if(TraceSphere(r, actor->Origin().ToVec2(), z + height, 0, bTestOnly))
            {   
                if(actor->Flags() & AF_TOUCHABLE)
                {
                    actor->OnTouch(moveActor);
                }
                
                if(actor->Flags() & AF_SOLID || bTestProjectile)
                {
                    contactActor = actor;
                    contactSector = actor->Sector();
                }
            }
        }
        else if(actor->Flags() & (AF_SOLID|AF_SHOOTABLE))
        {
            float z;
            float d;
            float minz = 0;
            kexVec3 vOrg = actor->Origin();

            if(actor->InstanceOf(&kexAI::info) && static_cast<kexAI*>(actor)->AIFlags() & AIF_FLYING)
            {
                minz = -actor->StepHeight();
            }
            
            // adjust z-height of the testing sphere. this will closely mimic
            // doing a trace against a capsule
            d = kexMath::Sqrt(vOrg.DistanceSq(start) / end.DistanceSq(start));
            z = ((end.z - start.z) * d + start.z) - vOrg.z;
            
            kexMath::Clamp(z, minz, actor->Height());
            r = actor->Radius();
            
            if(TraceSphere(r, vOrg + kexVec3(0, 0, z)))
            {
                contactActor = actor;
                contactSector = actor->Sector();
            }
        }
    }
}

//
// kexCModel::SlideAgainstFaces
//
// Collides with solid wall faces and painstakingly checks
// for every possible case for the actor squeezing through
// a portal. If the actor can't pass through, then it will
// collide with the portal as if it's a solid wall.
//

void kexCModel::SlideAgainstFaces(mapSector_t *sector)
{
    for(int i = sector->faceStart; i < sector->faceEnd+3; ++i)
    {
        mapFace_t *face = &faces[i];

        if(face->flags & FF_WATER)
        {
            if(moveActor->Flags() & AF_NOENTERWATER)
            {
                CollideFace(face);
            }
            continue;
        }

        if(moveActor->InstanceOf(&kexAI::info) && face->flags & FF_BLOCKAIGROUND &&
            !(static_cast<kexAI*>(moveActor)->AIFlags() & AIF_FLYING))
        {
            CollideFace(face);
            continue;
        }

        if(i <= sector->faceEnd && face->flags & FF_PORTAL && face->sector >= 0)
        {
            mapSector_t *s = &sectors[face->sector];
            float ceilingz = GetCeilingHeight(end, s);
            float floorz = GetFloorHeight(end, s);

            // prevent flagged actors from entering a sector with water.
            // though at least still let flying AI objects enter the sector
            if(moveActor->Flags() & AF_NOENTERWATER &&
                !(moveActor->InstanceOf(&kexAI::info) &&
                static_cast<kexAI*>(moveActor)->AIFlags() & AIF_FLYING))
            {
                if(s->floorFace->flags & FF_WATER)
                {
                    // avoid entering into water
                    CollideFace(face);
                    continue;
                }
            }

            if(moveActor->Flags() & AF_NOEXITWATER &&
                !(s->flags & SF_WATER) && !(s->floorFace->flags & FF_WATER))
            {
                // avoid exiting water
                CollideFace(face);
                continue;
            }

            if(moveActor->Flags() & AF_NOEXITLAVA && !(s->floorFace->flags & FF_LAVA))
            {
                // avoid exiting lava
                CollideFace(face);
                continue;
            }

            // so here we have a portal that borders between two sectors that the object
            // can fit in, height-wise, but we might have a case where the portal itself is
            // too short to walk through. need to check for that
            if((sector->ceilingFace->flags & FF_SOLID && s->floorFace->flags & FF_SOLID) &&
                moveActor->CeilingHeight() - floorz < actorHeight)
            {
                // not enough room to cross through this portal
                CollideFace(face);
                continue;
            }

            if(moveActor->Flags() & AF_NODROPOFF)
            {
                if(face->flags & FF_BLOCKAIFALLERS)
                {
                    CollideFace(face);
                    continue;
                }

                if(moveActor->InstanceOf(&kexAI::info) && s->floorFace->flags & FF_LAVA &&
                    !(static_cast<kexAI*>(moveActor)->AIFlags() & AIF_NOLAVADAMAGE))
                {
                    // avoid entering into lava
                    CollideFace(face);
                    continue;
                }

                if(GetFloorHeight(start, moveActor->Sector()) - floorz > moveActor->FallHeight())
                {
                    CollideFace(face);
                }
            }
            else
            {
                float fc = kexMath::Fabs(s->floorFace->plane.c);
                float cc = kexMath::Fabs(s->ceilingFace->plane.c);

                if((s->ceilingFace->flags & FF_SOLID && s->floorFace->flags & FF_SOLID) &&
                   ceilingz - floorz < actorHeight && (fc > 0.5f && cc > 0.5f))
                {
                    kexPlane *fp = &s->floorFace->plane;
                    kexPlane *cp = &s->ceilingFace->plane;
                    
                    if(fp->Distance(start) < 0 || cp->Distance(start) > actorHeight)
                    {
                        // couldn't squeeze through this portal
                        CollideFace(face);
                    }
                }
            }
        }

        if(!(face->flags & FF_SOLID) || face->flags & FF_WATER)
        {
            continue;
        }

        if(i <= sector->faceEnd)
        {
            CollideFace(face);
        }
    }
}

//
// kexCModel::ActorTouchingFace
//
// Returns true if the actor is touching any of the four edges of the face
//

bool kexCModel::ActorTouchingFace(kexActor *actor, mapFace_t *face)
{
    return (!CheckEdgeSide(actor->Origin(), face->BottomEdge(), face, actor->Height()*0.95f) &&
            !CheckEdgeSide(actor->Origin(), face->TopEdge(), face, actor->StepHeight()) &&
            !CheckEdgeSide(actor->Origin(), face->LeftEdge(), face, 0) &&
            !CheckEdgeSide(actor->Origin(), face->RightEdge(), face, 0));
}

//
// kexCModel::CheckActorPosition
//

bool kexCModel::CheckActorPosition(kexActor *actor, mapSector_t *initialSector)
{
    if(initialSector && !PointWithinSectorEdges(actor->Origin(), actor->Sector()))
    {
        actor->Origin() = actor->PrevOrigin();
        actor->SetSector(initialSector);
        return false;
    }
    else
    {
        for(int i = actor->Sector()->faceStart; i <= actor->Sector()->faceEnd; ++i)
        {
            mapFace_t *face = &faces[i];
            
            if(!(face->flags & FF_SOLID) || face->sector >= 0)
            {
                continue;
            }
            
            float d = PointOnFaceSide(actor->Origin(), face, actor->Radius());
            
            // inside wall?
            if(d < 0 && d > -actor->Radius())
            {
                if(ActorTouchingFace(actor, face))
                {
                    // eject out
                    actor->Origin() -= (face->plane.Normal() * d);
                    return false;
                }
            }
        }
    }

    return true;
}

//
// kexCModel::PointOnFaceSide
//
// Returns the distance from the face's plane
//

float kexCModel::PointOnFaceSide(const kexVec3 &origin, mapFace_t *face, const float extent)
{
    return face->plane.Dot(origin) - (face->plane.d + extent);
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

float kexCModel::GetFloorHeight(const kexVec3 &origin, mapSector_t *sector, bool bTestWater)
{
    mapVertex_t *v;
    mapFace_t *face;
    float dist;

    face = &faces[sector->faceEnd+2];

    if(bTestWater && !(face->flags & FF_SOLID) && face->flags & FF_PORTAL)
    {
        while(!(face->flags & FF_SOLID) && face->flags & FF_PORTAL)
        {
            mapSector_t *s = &sectors[face->sector];
            face = &faces[s->faceEnd+2];
        }
    }

    v = &vertices[face->vertexStart];

    dist = kexVec3::Dot(kexVec3(v->origin.x - origin.x,
                                v->origin.y - origin.y,
                                v->origin.z), face->plane.Normal()) / face->plane.c;

    return dist;
}

//
// kexCModel::GetCeilingHeight
//

float kexCModel::GetCeilingHeight(const kexVec3 &origin, mapSector_t *sector, bool bTestWater)
{
    mapVertex_t *v;
    mapFace_t *face;
    float dist;

    face = &faces[sector->faceEnd+1];

    if(bTestWater && !(face->flags & FF_SOLID) && face->flags & FF_PORTAL)
    {
        while(!(face->flags & FF_SOLID) && face->flags & FF_PORTAL)
        {
            mapSector_t *s = &sectors[face->sector];
            face = &faces[s->faceEnd+1];
        }
    }

    v = &vertices[face->vertexStart];

    dist = kexVec3::Dot(kexVec3(v->origin.x - origin.x,
                                v->origin.y - origin.y,
                                v->origin.z), face->plane.Normal()) / face->plane.c;

    return dist;
}

//
// kexCModel::GetContactSectors
//
// Creates a list of all sectors that were contacted
// by the actor's bounding box. Also test collision
// against all actors for every sector that's tested
//

void kexCModel::GetContactSectors(mapSector_t *initial)
{
    unsigned int sectorCount = 0;
    
    sectorList.Reset();
    sectorList.Set(initial);
    initial->validcount = validcount;
    
    do
    {
        mapSector_t *sec = sectorList[sectorCount++];
        
        TraceActorsInSector(sec);
        
        for(int i = sec->faceStart; i < sec->faceEnd+3; ++i)
        {
            mapFace_t *face = &faces[i];
            mapSector_t *s;
            
            if(!(face->flags & FF_PORTAL) || face->sector <= -1)
            {
                // could be a solid wall or something
                continue;
            }
            
            s = &sectors[face->sector];
            
            if(s->validcount == validcount)
            {
                // we already checked this sector
                continue;
            }
            
            // is this linked sector reachable?
            if(actorBounds.IntersectingBox(s->bounds))
            {
                sectorList.Set(s);
                s->validcount = validcount;
            }
        }
        
    } while(sectorCount < sectorList.CurrentLength());
    
    validcount++;
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
    mapSector_t *sec, *best = NULL;
    float maxfloorz, maxceilingz;
    float floorz, ceilingz, diff;
    bool bChangeFloorHeight, bChangeCeilingHeight;
    unsigned int sectorCount = 0;
    
    maxfloorz = moveActor->FloorHeight();
    maxceilingz = end.z;
    
    bChangeFloorHeight = false;
    bChangeCeilingHeight = false;
    
    sectorList.Reset();
    sectorList.Set(moveActor->Sector());
    moveActor->Sector()->validcount = validcount;
    
    do
    {
        sec = sectorList[sectorCount++];
        
        for(int i = sec->faceStart; i < sec->faceEnd+1; ++i)
        {
            mapFace_t *face = &faces[i];
            mapSector_t *s;
            
            if(!(face->flags & FF_PORTAL) || face->sector <= -1)
            {
                // could be a solid wall or something
                continue;
            }
            
            s = &sectors[face->sector];
            
            if(!PointWithinSectorEdges(end, s, -actorRadius))
            {
                continue;
            }
            
            if(s->validcount == validcount)
            {
                // we already checked this sector
                continue;
            }
            
            sectorList.Set(s);
            s->validcount = validcount;
            
            if(!CheckEdgeSide(end, face->BottomEdge(), face, actorRadius) &&
               !CheckEdgeSide(end, face->TopEdge(), face, actorRadius))
            {
                // is the actor crossing this portal face?
                if(PointOnFaceSide(end, face, actorRadius) < 0)
                {
                    ceilingz = GetCeilingHeight(end, s, true);
                    floorz = GetFloorHeight(end, s, !(moveActor->Flags() & AF_NOENTERWATER));
                    
                    if(kexMath::Fabs(ceilingz - floorz) < actorHeight)
                    {
                        // no headroom
                        continue;
                    }

                    diff = end.z - floorz;
                    
                    // determine the closest floor that can be stepped on
                    if(diff <= 0 && diff >= -moveActor->StepHeight())
                    {
                        if(kexMath::Fabs(s->floorFace->plane.c) > 0.5f &&
                            floorz > maxfloorz && moveActor->Movement().z <= 0)
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
                            // check non-steep ceiling slopes only
                            if(kexMath::Fabs(s->ceilingFace->plane.c) > 0.5f)
                            {
                                best = s;
                                maxceilingz = ceilingz;
                                bChangeCeilingHeight = true;
                            }
                        }
                    }
                }
            }
        }
        
    } while(sectorCount < sectorList.CurrentLength());
    
    if(bChangeFloorHeight && best && best->floorFace->flags & FF_SOLID)
    {
        if(PointInsideSector(end, best, -actorRadius, moveActor->StepHeight()))
        {
            if(moveActor->InstanceOf(&kexPuppet::info))
            {
                float step = (end.z - maxfloorz);
                kexPuppet *puppet = static_cast<kexPuppet*>(moveActor);

                if(step < 0 && puppet->Owner()->StepViewZ() >= 0)
                {
                    puppet->Owner()->StepViewZ() = step;
                }
            }

            // step up into this sector
            end.z = maxfloorz;
            moveActor->FloorHeight() = maxfloorz;
        }
    }
    
    if(bChangeCeilingHeight)
    {
        // bump into the ceiling
        if(maxceilingz > maxfloorz && (maxceilingz - maxfloorz) > actorHeight)
        {
            end.z = maxceilingz - actorHeight;
            moveActor->CeilingHeight() = maxceilingz;
        }
    }
    
    validcount++;
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
        float projAmt;

        kexVec3::ToAxis(&forwardDir, NULL, NULL, moveDir.ToYaw(), 0, 0);
        
        fraction = 1;
        end = start + moveDir;
        interceptVector = end;
        
        contactNormal.Clear();
        contactFace = NULL;
        contactActor = NULL;
        contactSector = NULL;
        
        actorBounds.min.Set(-r, -r, -h);
        actorBounds.max.Set( r,  r,  h);
        
        actorBounds.min += end;
        actorBounds.max += end;
        actorBounds *= moveDir;
        
        GetContactSectors(sector);
        
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

        if(contactActor)
        {
            PushFromRadialBounds(contactActor->Origin().ToVec2(), contactActor->Radius());
        }

        if(contactFace && !(moveActor->Flags() & AF_COLLIDEDWALL))
        {
            moveActor->Flags() |= AF_COLLIDEDWALL;
            moveActor->CollidedWallAngle() = contactFace->angle;
            moveActor->CollidedWallNormal() = contactFace->plane.Normal();
        }

        if(!moveActor->OnCollide(this))
        {
            return;
        }
        
        contactNormals[moves++] = contactNormal;
        projAmt = 1.0f - fraction + 0.1f;
        
        // try all interacted normals
        for(int j = 0; j < moves; ++j)
        {
            if(moveDir.Dot(contactNormals[j]) >= 0)
            {
                continue;
            }
            
            moveDir.Project(contactNormals[j], projAmt);
            
            // try bumping against another plane
            for(int k = 0; k < moves; ++k)
            {
                if(k == j || moveDir.Dot(contactNormals[k]) >= 0)
                {
                    continue;
                }
                
                // bump into second plane
                moveDir.Project(contactNormals[k], projAmt);
                
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
    bool bWater1 = (sector->flags & SF_WATER) != 0;
    bool bWater2;
    
    for(unsigned int i = 0; i < sectorList.CurrentLength(); ++i)
    {
        float offset = 0;

        if(!(sectorList[i]->floorFace->flags & FF_WATER))
        {
            offset = moveActor->StepHeight();
        }

        if(PointInsideSector(end, sectorList[i], 0, offset))
        {
            if(sectorList[i]->ceilingFace->flags & FF_WATER)
            {
                if(moveActor->Flags() & AF_NOENTERWATER &&
                    (moveActor->InstanceOf(&kexAI::info) &&
                    static_cast<kexAI*>(moveActor)->AIFlags() & AIF_FLYING))
                {
                    if(&sectors[sectorList[i]->ceilingFace->sector] == sector)
                    {
                        continue;
                    }
                }
            }

            // handle special cases for flat/thin, see-through floors
            if(sectorList[i]->ceilingFace->flags & FF_SOLID &&
               sectorList[i]->ceilingFace->flags & FF_PORTAL)
            {
                // if we're standing above that sector then don't enter it
                if(&sectors[sectorList[i]->ceilingFace->sector] == sector)
                {
                    continue;
                }
            }

            sector = sectorList[i];
        }
    }
    
    // clamp z-axis to floor and ceiling
    float floorz = GetFloorHeight(end, sector, !(moveActor->Flags() & AF_NOENTERWATER));
    float ceilingz = GetCeilingHeight(end, sector, !(moveActor->Flags() & AF_NOEXITWATER));
    
    bWater2 = (sector->flags & SF_WATER) != 0;

    // don't clip z axis if we're entering water
    if(!(bWater1 ^ bWater2) && sector->floorFace->flags & FF_SOLID)
    {
        if(end.z - floorz < 0)
        {
            end.z = floorz;
            moveDir.z = 0;
        }
    }

    if((!(bWater1 ^ bWater2) && sector->ceilingFace->flags & FF_SOLID) ||
       moveActor->Flags() & AF_NOEXITWATER)
    {
        if(ceilingz - end.z < actorHeight)
        {
            end.z = ceilingz - actorHeight;
        }
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

    actor->Flags() &= ~AF_COLLIDEDWALL;

    moveActor = actor;
    sourceActor = actor;
    actorRadius = actor->Radius();
    actorHeight = actor->Height();
    moveDir = actor->Movement();
    start = actor->Origin();
    actor->PrevOrigin() = start;

    // interact with world
    CollideActorWithWorld();
    
    // update sector position
    AdvanceActorToSector();
    
    // check surrounding sectors that can be
    // stepped on
    CheckSurroundingSectors();
    
    actor->Origin() = end;
    actor->Movement() = moveDir;

    CheckActorPosition(actor, sector);
    actor->LinkArea();
    return true;
}

//
// kexCModel::Trace
//

bool kexCModel::Trace(kexActor *actor, mapSector_t *sector,
                      const kexVec3 &start_pos, const kexVec3 &end_pos,
                      const float radius, bool bTestActors)
{
    unsigned int sectorCount;

    if(sector == NULL)
    {
        return false;
    }

    sectorList.Reset();

    moveActor = NULL;
    sourceActor = actor;
    actorRadius = 0;
    actorHeight = 0;
    start = start_pos;
    end = end_pos;
    moveDir = end - start;
    fraction = 1;
    interceptVector = end;
    contactNormal.Clear();
    contactSector = sector;
    contactFace = NULL;
    contactActor = NULL;

    sectorList.Set(sector);
    sector->validcount = validcount;
    sectorCount = 0;

    do
    {
        mapSector_t *s = sectorList[sectorCount++];

        if(bTestActors)
        {
            // check for actors in this sector
            TraceActorsInSector(s);
        }

        for(int i = s->faceStart; i < s->faceEnd+3; ++i)
        {
            mapFace_t *face = &faces[i];

            if(face->plane.Dot(moveDir) > 0)
            {
                // ray isn't facing the plane
                continue;
            }

            if(face->flags & FF_PORTAL && face->sector >= 0)
            {
                if(sectors[face->sector].validcount == validcount)
                {
                    // we already checked this sector
                    continue;
                }

                // test if the trace intersected the portal. immediately enter
                // next sector if its a water surface
                if(face->flags & FF_WATER || TraceFacePlane(face, 0, radius, true))
                {
                    mapSector_t *next = &sectors[face->sector];

                    if(face->flags & FF_WATER)
                    {
                        if( face->plane.Distance(start) >= 0 &&
                            face->plane.Distance(end) >= 0)
                        {
                            // start and end points are both in front
                            // of the water surface
                            continue;
                        }
                    }

                    // add to list if the ray passes through the portal
                    sectorList.Set(next);
                    next->validcount = validcount;
                    contactSector = next;
                }
            }
            else if(face->flags & FF_SOLID)
            {
                if(i <= s->faceEnd)
                {
                    // test solid wall
                    TraceFacePlane(face, 0, radius);
                }
                else
                {
                    // test ceiling/floor
                    TestIntersectSector(face, radius);
                }
            }
        }

    } while(sectorCount < sectorList.CurrentLength());

    validcount++;
    return (fraction != 1);
}
