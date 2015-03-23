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

#ifndef __GAMEOBJECT_H__
#define __GAMEOBJECT_H__

//-----------------------------------------------------------------------------
//
// kexGameObject
//
//-----------------------------------------------------------------------------

BEGIN_EXTENDED_KEX_CLASS(kexGameObject, kexObject);
public:
    kexGameObject(void);
    virtual ~kexGameObject(void);

    virtual void                Tick(void) = 0;
    virtual void                OnRemove(void);
    virtual void                Remove(void);

    void                        Spawn(void);

    int                         AddRef(void);
    int                         RemoveRef(void);
    void                        SetTarget(kexGameObject *targ);
    const bool                  Removing(void) const;
    const bool                  GetSoundParameters(float &volume, float &pan);
    void                        PlaySound(const char *snd);
    void                        PlayLoopingSound(const char *snd);
    void                        PlaySound(const kexStr &snd) { PlaySound(snd.c_str()); }
    void                        PlayLoopingSound(const kexStr &snd) { PlayLoopingSound(snd.c_str()); }
    void                        StopSound(void);
    void                        StopLoopingSounds(void);

    kexLinklist<kexGameObject>  &Link(void) { return link; }
    kexVec3                     &Origin(void) { return origin; }
    kexAngle                    &Yaw(void) { return yaw; }
    kexAngle                    &Pitch(void) { return pitch; }
    kexAngle                    &Roll(void) { return roll; }
    kexGameObject               *Target(void) { return target; }
    int                         &TimeStamp(void) { return timeStamp; }

    static unsigned int         id;

    const unsigned int          ObjectID(void) const { return objID; }
    const int                   RefCount(void) const { return refCount; }
    const bool                  IsStale(void) const { return bStale; }

protected:
    kexLinklist<kexGameObject>  link;
    kexVec3                     origin;
    kexAngle                    yaw;
    kexAngle                    pitch;
    kexAngle                    roll;
    kexGameObject               *target;
    int                         timeStamp;

private:
    int                         refCount;
    unsigned int                objID;
    bool                        bStale;         // freed on next game tick
END_KEX_CLASS();

#endif
