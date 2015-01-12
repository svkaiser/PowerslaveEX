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

    float                   PointOnFaceSide(const kexVec3 &origin, mapFace_t *face, const float extent = 0);
    float                   GetFloorHeight(const kexVec3 &origin, mapSector_t *sector);
    float                   GetCeilingHeight(const kexVec3 &origin, mapSector_t *sector);
    bool                    PointInsideSector(const kexVec3 &origin, mapSector_t *sector, const float extent = 0);
    bool                    PointInsideFace(const kexVec3 &origin, mapFace_t *face, const float extent = 0);

    bool                    TryMove(kexActor *actor, kexVec3 &position);
    bool                    CheckActorPosition(kexActor *actor);
    void                    SlideAgainstFaces(mapSector_t *sector);
    void                    Reset(void);

private:
    bool                    CollideFace(mapFace_t *face);
    bool                    IntersectFaceEdge(mapFace_t *face);
    bool                    CollideVertex(const kexVec2 &point);

    mapSector_t             *GetNextSector(kexActor *actor, mapSector_t *sector, kexVec3 &position);

    mapVertex_t             *vertices;
    mapSector_t             *sectors;
    mapFace_t               *faces;
    mapPoly_t               *polys;

    kexActor                *moveActor;
    kexVec3                 interceptVector;
    kexVec3                 start;
    kexVec3                 end;
    kexVec3                 moveDir;
    kexBBox                 actorBounds;
    float                   actorRadius;
    float                   actorHeight;
    float                   fraction;
};

#endif
