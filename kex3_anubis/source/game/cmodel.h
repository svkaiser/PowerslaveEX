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

#ifndef __CMODEL_H__
#define __CMODEL_H__

class kexWorld;
class kexActor;

class kexCModel
{
public:
    kexCModel(void);
    ~kexCModel(void);

    void                    Setup(kexWorld *world);

    float                   PointOnFaceSide(const kexVec3 &origin, mapFace_t *face,
                                            const float extent = 0);
    float                   GetFloorHeight(const kexVec3 &origin, mapSector_t *sector, bool bTestWater = false);
    float                   GetCeilingHeight(const kexVec3 &origin, mapSector_t *sector, bool bTestWater = false);
    bool                    PointWithinSectorEdges(const kexVec3 &origin, mapSector_t *sector,
                                                   const float extent = 0);
    bool                    PointInsideSector(const kexVec3 &origin, mapSector_t *sector,
                                              const float extent = 0, const float floorOffset = 0);
    bool                    PointInsideFace(const kexVec3 &origin, mapFace_t *face,
                                            const float extent = 0);
    bool                    CheckEdgeSide(const kexVec3 &origin, mapEdge_t *edge, mapFace_t *face,
                                          const float heightAdjust, const float extent = 0);

    bool                    MoveActor(kexActor *actor);
    bool                    Trace(kexActor *actor, mapSector_t *sector,
                                  const kexVec3 &start_pos, const kexVec3 &end_pos,
                                  const float radius = 0, bool bTestActors = true);
    bool                    CheckActorPosition(kexActor *actor, mapSector_t *initialSector);
    bool                    ActorTouchingFace(kexActor *actor, mapFace_t *face);
    void                    Reset(void);

    const int               ValidCount(void) const { return validcount; }
    mapFace_t               *ContactFace(void) { return contactFace; }
    kexActor                *ContactActor(void) { return contactActor; }
    mapSector_t             *ContactSector(void) { return contactSector; }
    kexVec3                 &InterceptVector(void) { return interceptVector; }
    kexVec3                 &ContactNormal(void) { return contactNormal; }
    const float             &Fraction(void) { return fraction; }

private:
    void                    TraceActorsInSector(mapSector_t *sector);
    void                    CollideActorWithWorld(void);
    void                    AdvanceActorToSector(void);
    void                    SlideAgainstFaces(mapSector_t *sector);
    void                    CheckSurroundingSectors(void);
    bool                    TraceFacePlane(mapFace_t *face, const float extent1 = 0, const float extent2 = 0,
                                           const bool bTestOnly = false);
    bool                    TestIntersectSector(mapFace_t *face, const float extent);
    bool                    CollideFace(mapFace_t *face);
    bool                    TraceSphere(const float radius, const kexVec2 &point,
                                        const float heightMax = 0, const float heightMin = 0,
                                        const bool bTestOnly = false);
    bool                    TraceSphere(const float radius, const kexVec3 &point);
    void                    PushFromRadialBounds(const kexVec2 &point, const float radius = 0);
    void                    GetContactSectors(mapSector_t *initial);

    mapVertex_t             *vertices;
    mapSector_t             *sectors;
    mapFace_t               *faces;
    mapPoly_t               *polys;

    static int              validcount;

    sectorList_t            sectorList;
    kexActor                *moveActor;
    kexActor                *sourceActor;
    mapSector_t             *contactSector;
    mapFace_t               *contactFace;
    kexActor                *contactActor;
    kexVec3                 interceptVector;
    kexVec3                 start;
    kexVec3                 end;
    kexVec3                 moveDir;
    kexVec3                 forwardDir;
    kexVec3                 contactNormal;
    kexBBox                 actorBounds;
    float                   actorRadius;
    float                   actorHeight;
    float                   fraction;
};

#endif
