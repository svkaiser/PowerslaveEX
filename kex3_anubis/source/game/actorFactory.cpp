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
//      Simple class for managing and spawning actors
//

#include "kexlib.h"
#include "game.h"
#include "mover.h"
#include "actorFactory.h"

static kexActorFactory actorFactoryLocal;
kexActorFactory *kexGame::cActorFactory = &actorFactoryLocal;

//
// kexActorFactory::kexActorFactory
//

kexActorFactory::kexActorFactory(void)
{
}

//
// kexActorFactory::~kexActorFactory
//

kexActorFactory::~kexActorFactory(void)
{
}

//
// kexActorFactory::ConstructActor
//

kexActor *kexActorFactory::Construct(const char *className, kexDict *def, const int type,
                                     const float x, const float y, const float z,
                                     const float yaw, const int sector)
{
    kexActor *actor;
    
    if(!(actor = static_cast<kexActor*>(kexGame::cLocal->ConstructObject(className))))
    {
        return NULL;
    }
    
    actor->SetDefinition(def);
    
    actor->Origin().Set(x, y, z);
    actor->Yaw() = yaw;
    actor->Type() = type;
    
    if(sector <= -1)
    {
        actor->FindSector(actor->Origin());
    }
    else
    {
        actor->SetSector(&kexGame::cWorld->Sectors()[sector]);
    }
    
    actor->CallSpawn();
    return actor;
}

//
// kexActorFactory::SpawnActor
//

kexActor *kexActorFactory::Spawn(const int type, const float x, const float y, const float z,
                                 const float yaw, const int sector)
{
    kexStr className;
    kexDict *def;
    kexActor *actor;
    
    if((def = kexGame::cLocal->ActorDefs().GetEntry(type)))
    {
        if(!def->GetString("classname", className))
        {
            className = "kexActor";
        }

        if(kexGame::cLocal->NoMonstersEnabled() && className == "kexAI")
        {
            return NULL;
        }
    }
    else
    {
        switch(type)
        {
        case AT_PLAYER:
            className = "kexPuppet";
            break;
            
        case AT_FIREBALLSPAWNER:
        case AT_LASERSPAWNER:
            className = "kexFireballSpawner";
            break;
            
        default:
            className = "kexActor";
            break;
        }
    }
    
    actor = Construct(className, def, type, x, y, z, yaw, sector);
    return actor;
}

//
// kexActorFactory::SpawnActor
//

kexActor *kexActorFactory::Spawn(const char *name, const float x, const float y, const float z,
                                 const float yaw, const int sector)
{
    kexStr className = "kexActor";
    kexDict *def = NULL;
    kexActor *actor;
    int type = -1;
    kexHashList<kexDict>::hashKey_t *hashKey = kexGame::cLocal->ActorDefs().defs.GetHashKey(name);
    
    // when looking up the name from the hash, we have no way of knowing what
    // the type index is, so we need to pull the actual hash key and look
    // up the reference index
    if(hashKey)
    {
        if((def = &hashKey->data))
        {
            def->GetString("classname", className);
        }
    }
    
    if(hashKey)
    {
        type = hashKey->refIndex;
    }
    
    actor = Construct(className, def, type, x, y, z, yaw, sector);
    return actor;
}

//
// kexActorFactory::SpawnActor
//

kexActor *kexActorFactory::Spawn(const kexStr &name, const float x, const float y, const float z,
                                 const float yaw, const int sector)
{
    return Spawn(name.c_str(), x, y, z, yaw, sector);
}

//
// kexActorFactory::SpawnFromActor
//

kexActor *kexActorFactory::SpawnFromActor(const char *name, const float x, const float y, const float z,
                                          kexActor *source, const float yaw)
{
    kexActor *actor;
    kexVec3 start, end;
    
    if(!source)
    {
        return NULL;
    }
    
    actor = Spawn(name,
                  source->Origin().x,
                  source->Origin().y,
                  source->Origin().z,
                  yaw, source->SectorIndex());
    
    if(actor == NULL)
    {
        return NULL;
    }
    
    start = source->Origin();
    end = start;
    
    end.x += x;
    end.y += y;
    end.z += z;
    
    kexGame::cLocal->CModel()->Trace(source, source->Sector(), start, end);
    
    actor->Origin().Lerp(end, kexGame::cLocal->CModel()->Fraction());
    actor->SetSector(kexGame::cLocal->CModel()->ContactSector());

    actor->PrevOrigin() = actor->Origin();

    if(!kexGame::cLocal->CModel()->CheckActorPosition(actor, actor->Sector()))
    {
        actor->Origin().x = source->Origin().x;
        actor->Origin().y = source->Origin().y;
        actor->SetSector(source->Sector());
    }
    
    return actor;
}

//
// kexActorFactory::SpawnFromActor
//

kexActor *kexActorFactory::SpawnFromActor(const kexStr &name, const float x, const float y, const float z,
                                          kexActor *source, const float yaw)
{
    return SpawnFromActor(name.c_str(), x, y, z, source, yaw);
}

//
// kexActorFactory::SpawnMover
//

kexMover *kexActorFactory::SpawnMover(const char *className, const int type, const int sector)
{
    kexMover *mover;
    mapSector_t *sec;
    
    if(sector <= -1 || sector >= (int)kexGame::cWorld->NumSectors())
    {
        return NULL;
    }
    
    sec = &kexGame::cWorld->Sectors()[sector];
    
    if(sec->flags & SF_SPECIAL)
    {
        return NULL;
    }
    
    if(!(mover = static_cast<kexMover*>(kexGame::cLocal->ConstructObject(className))))
    {
        return NULL;
    }
    
    mover->Type() = type;
    mover->SetSector(sec);
    mover->CallSpawn();
    
    return mover;
}

//
// kexActorFactory::SpawnFireballFactory
//

kexFireballFactory *kexActorFactory::SpawnFireballFactory(mapActor_t *mapActor)
{
    kexFireballFactory *fbf;
    
    if(mapActor->sector <= -1 || mapActor->sector >= (int)kexGame::cWorld->NumSectors())
    {
        return NULL;
    }
    
    if(!(fbf = static_cast<kexFireballFactory*>(kexGame::cLocal->ConstructObject("kexFireballFactory"))))
    {
        return NULL;
    }
    
    fbf->FireballType() = mapActor->type;
    fbf->SetSector(&kexGame::cWorld->Sectors()[mapActor->sector]);
    fbf->CallSpawn();
    
    return fbf;
}
