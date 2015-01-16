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
    float                   GetFloorHeight(const kexVec3 &origin, mapSector_t *sector);
    float                   GetCeilingHeight(const kexVec3 &origin, mapSector_t *sector);
    bool                    PointWithinSectorEdges(const kexVec3 &origin, mapSector_t *sector,
                                                   const float extent = 0);
    bool                    SectorLinksToSector(const kexVec3 &origin, mapSector_t *source, mapSector_t *dest,
                                                const float extent = 0);
    bool                    PointInsideSector(const kexVec3 &origin, mapSector_t *sector,
                                              const float extent = 0);
    bool                    PointInsideFace(const kexVec3 &origin, mapFace_t *face,
                                            const float extent = 0);

    bool                    MoveActor(kexActor *actor);
    bool                    CheckActorPosition(kexActor *actor);
    void                    SlideAgainstFaces(mapSector_t *sector);
    void                    Reset(void);

    const int               ValidCount(void) const { return validcount; }

private:
    bool                    CheckEdgeSide(mapEdge_t *edge, const kexVec3 &dir, const float heightAdjust);
    void                    GetSurroundingSectors(void);
    bool                    TraceFacePlane(mapFace_t *face, const float extent1 = 0, const float extent2 = 0);
    bool                    CollideFace(mapFace_t *face);
    bool                    TraceFaceVertex(mapFace_t *face, const kexVec2 &point);
    void                    RecursiveFindSectors(mapSector_t *sector);

    mapVertex_t             *vertices;
    mapSector_t             *sectors;
    mapFace_t               *faces;
    mapPoly_t               *polys;

    static int              validcount;

    kexStack<mapSector_t*>  sectorList;
    kexActor                *moveActor;
    mapFace_t               *contactFace;
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
