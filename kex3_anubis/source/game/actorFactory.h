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

#ifndef __ACTOR_FACTORY_H__
#define __ACTOR_FACTORY_H__

class kexActorFactory
{
public:
    kexActorFactory(void);
    ~kexActorFactory(void);
    
    kexActor                    *Construct(const char *className, kexDict *def, const int type,
                                           const float x, const float y, const float z,
                                           const float yaw, const int sector = -1);
    kexActor                    *Spawn(const int type, const float x, const float y, const float z,
                                       const float yaw, const int sector = -1);
    kexActor                    *Spawn(const char *name, const float x, const float y, const float z,
                                       const float yaw, const int sector = -1);
    kexActor                    *Spawn(const kexStr &name, const float x, const float y, const float z,
                                       const float yaw, const int sector = -1);
    kexActor                    *SpawnFromActor(const char *name, const float x, const float y, const float z,
                                                kexActor *source, const float yaw);
    kexActor                    *SpawnFromActor(const kexStr &name, const float x, const float y, const float z,
                                                kexActor *source, const float yaw);
    kexMover*                   SpawnMover(const char *className, const int type, const int sector);
    kexFireballFactory          *SpawnFireballFactory(mapActor_t *mapActor);
    
private:
};

#endif
