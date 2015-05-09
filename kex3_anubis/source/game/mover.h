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

#ifndef __MOVER_H__
#define __MOVER_H__

#include "gameObject.h"

//-----------------------------------------------------------------------------
//
// kexMover
//
//-----------------------------------------------------------------------------

BEGIN_EXTENDED_KEX_CLASS(kexMover, kexGameObject);
public:
    kexMover(void);
    ~kexMover(void);

    virtual void            Tick(void);
    virtual void            Remove();

    bool                    CheckActorHeight(const float height);

    int                     &Type(void) { return type; }
    mapSector_t             *Sector(void) { return sector; }
    void                    SetSector(mapSector_t *s);
    void                    UpdateFloorOrigin(void);
    void                    UpdateCeilingOrigin(void);

    bool                    PlayerInside(void);

protected:
    int                     type;
    mapSector_t             *sector;
END_KEX_CLASS();

//-----------------------------------------------------------------------------
//
// kexDoor
//
//-----------------------------------------------------------------------------

BEGIN_EXTENDED_KEX_CLASS(kexDoor, kexMover);
public:
    kexDoor(void);
    ~kexDoor(void);

    virtual void            Tick(void);
    void                    Spawn(void);

private:
    typedef enum
    {
        DS_IDLE     = 0,
        DS_UP,
        DS_DOWN,
        DS_WAIT
    } doorState_t;

    float                   waitDelay;
    float                   moveSpeed;
    float                   lip;
    bool                    bDirection;
    float                   baseHeight;
    float                   destHeight;
    float                   raiseHeight;
    float                   currentHeight;
    float                   currentTime;
    doorState_t             state;
END_KEX_CLASS();

//-----------------------------------------------------------------------------
//
// kexFloor
//
//-----------------------------------------------------------------------------

BEGIN_EXTENDED_KEX_CLASS(kexFloor, kexMover);
public:
    kexFloor(void);
    ~kexFloor(void);

    virtual void            Tick(void);
    void                    Spawn(void);
    void                    Trigger(void);

private:
    typedef enum
    {
        FS_IDLE     = 0,
        FS_DOWN,
        FS_UP,
        FS_LOWERED
    } floorState_t;

    float                   moveSpeed;
    float                   lip;
    float                   baseHeight;
    float                   destHeight;
    float                   currentHeight;
    floorState_t            state;
END_KEX_CLASS();

//-----------------------------------------------------------------------------
//
// kexLift
//
//-----------------------------------------------------------------------------

BEGIN_EXTENDED_KEX_CLASS(kexLift, kexMover);
public:
    kexLift(void);
    ~kexLift(void);

    virtual void            Tick(void);
    void                    Spawn(void);

private:
    typedef enum
    {
        LS_IDLE     = 0,
        LS_UP,
        LS_DOWN,
        LS_WAIT
    } liftState_t;

    float                   waitDelay;
    float                   triggerDelay;
    float                   moveSpeed;
    float                   lip;
    bool                    bDirection;
    float                   baseHeight;
    float                   destHeight;
    float                   currentHeight;
    float                   currentTime;
    float                   currentDelay;
    liftState_t             state;
END_KEX_CLASS();

BEGIN_EXTENDED_KEX_CLASS(kexLiftImmediate, kexLift);
public:
    kexLiftImmediate(void);
    ~kexLiftImmediate(void);
END_KEX_CLASS();

//-----------------------------------------------------------------------------
//
// kexDropPad
//
//-----------------------------------------------------------------------------

BEGIN_EXTENDED_KEX_CLASS(kexDropPad, kexMover);
public:
    kexDropPad(void);
    ~kexDropPad(void);

    virtual void            Tick(void);
    void                    Spawn(void);
    void                    Start(void);
    void                    Reset(void);

private:
    typedef enum
    {
        DPS_IDLE    = 0,
        DPS_FALLING,
        DPS_DOWN,
        DPS_RAISE,
        DPS_COUNTDOWN
    } dropPadState_t;

    float                   triggerDelay;
    float                   moveSpeed;
    float                   baseHeight;
    float                   destHeight;
    float                   currentHeight;
    float                   currentDelay;
    float                   speedAccel;
    dropPadState_t          state;
    mapSector_t             *linkedSector;
END_KEX_CLASS();

//-----------------------------------------------------------------------------
//
// kexFloatingPlatform
//
//-----------------------------------------------------------------------------

BEGIN_EXTENDED_KEX_CLASS(kexFloatingPlatform, kexMover);
public:
    kexFloatingPlatform(void);
    ~kexFloatingPlatform(void);

    virtual void            Tick(void);
    void                    Spawn(void);

private:
    float                   moveSpeed;
    kexAngle                angOffset;
    float                   moveHeight;
    float                   baseHeight;
    float                   currentHeight;
    mapSector_t             *linkedSector;
    int                     time;
END_KEX_CLASS();

//-----------------------------------------------------------------------------
//
// kexScriptedMover
//
//-----------------------------------------------------------------------------

BEGIN_EXTENDED_KEX_CLASS(kexScriptedMover, kexMover);
public:
    kexScriptedMover(void);
    ~kexScriptedMover(void);

    virtual void            Tick(void);
    void                    Start(const float height, const float speed,
                                  const mapEvent_t *ev, bool bCeiling);

private:
    float                   moveSpeed;
    float                   moveHeight;
    float                   currentHeight;
    bool                    bCeiling;
    mapSector_t             *linkedSector;
END_KEX_CLASS();

#endif
