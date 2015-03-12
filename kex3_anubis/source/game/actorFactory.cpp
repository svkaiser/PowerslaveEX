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
//      Actor Creation
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
        actor->SetSector(&kexGame::cLocal->World()->Sectors()[sector]);
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
// kexActorFactory::SpawnMover
//

kexMover *kexActorFactory::SpawnMover(const char *className, const int type, const int sector)
{
    kexMover *mover;
    
    if(sector <= -1 || sector >= (int)kexGame::cLocal->World()->NumSectors())
    {
        return NULL;
    }
    
    if(!(mover = static_cast<kexMover*>(kexGame::cLocal->ConstructObject(className))))
    {
        return NULL;
    }
    
    mover->Type() = type;
    mover->SetSector(&kexGame::cLocal->World()->Sectors()[sector]);
    mover->CallSpawn();
    
    return mover;
}

//
// kexActorFactory::SpawnFireballFactory
//

kexFireballFactory *kexActorFactory::SpawnFireballFactory(mapActor_t *mapActor)
{
    kexFireballFactory *fbf;
    
    if(mapActor->sector <= -1 || mapActor->sector >= (int)kexGame::cLocal->World()->NumSectors())
    {
        return NULL;
    }
    
    if(!(fbf = static_cast<kexFireballFactory*>(kexGame::cLocal->ConstructObject("kexFireballFactory"))))
    {
        return NULL;
    }
    
    fbf->FireballType() = mapActor->type;
    fbf->SetSector(&kexGame::cLocal->World()->Sectors()[mapActor->sector]);
    fbf->CallSpawn();
    
    return fbf;
}
