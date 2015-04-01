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

#ifndef __HASHLIST_H__
#define __HASHLIST_H__

template<class type>
class kexHashList
{
public:
    kexHashList(void);

    type                *Add(const char *tname, kexHeapBlock &hb = hb_static);
    type                *Add(const char *tname, const int index, kexHeapBlock &hb = hb_static);
    type                *Find(const char *tname) const;
    type                *Find(const int index) const;
    type                *GetData(const int index);
    type                *Next(void);
    char                *GetName(const int index);

    typedef struct hashKey_s
    {
        type            data;
        filepath_t      name;
        int             refIndex;
        hashKey_s       *next;
    } hashKey_t;

    hashKey_t           *GetHashKey(const char *tname);
    hashKey_t           *GetHashKey(const unsigned int index);
    hashKey_t           *AddAndReturnHashKey(const char *tname, kexHeapBlock &hb = hb_static);

    hashKey_t           *hashlist[MAX_HASH];
    hashKey_t           *rover;
};

//
// kexHashList::kexHashList
//
template<class type>
kexHashList<type>::kexHashList(void)
{
    memset(hashlist, 0, sizeof(hashKey_t*) * MAX_HASH);
}

//
// kexHashList::Add
//
template<class type>
type *kexHashList<type>::Add(const char *tname, kexHeapBlock &hb)
{
    unsigned int hash;

    hashKey_t *o = (hashKey_t*)Mem_Calloc(sizeof(hashKey_t), hb);
    strncpy(o->name, tname, MAX_FILEPATH);
    o->refIndex = -1;

    // add to hash for future reference
    hash = kexStr::Hash(o->name);
    o->next = hashlist[hash];
    hashlist[hash] = o;

    return &o->data;
}

//
// kexHashList::Add
//
template<class type>
type *kexHashList<type>::Add(const char *tname, const int index, kexHeapBlock &hb)
{
    unsigned int hash;
    
    hashKey_t *o = (hashKey_t*)Mem_Calloc(sizeof(hashKey_t), hb);
    strncpy(o->name, tname, MAX_FILEPATH);
    o->refIndex = index;
    
    // add to hash for future reference
    hash = index & (MAX_HASH-1);
    o->next = hashlist[hash];
    hashlist[hash] = o;

    return &o->data;
}

//
// kexHashList::AddAndReturnHashKey
//
template<class type>
typename kexHashList<type>::hashKey_t *
kexHashList<type>::AddAndReturnHashKey(const char *tname, kexHeapBlock &hb)
{
    unsigned int hash;

    hashKey_t *o = (hashKey_t*)Mem_Calloc(sizeof(hashKey_t), hb);
    strncpy(o->name, tname, MAX_FILEPATH);
    o->refIndex = -1;

    // add to hash for future reference
    hash = kexStr::Hash(o->name);
    o->next = hashlist[hash];
    hashlist[hash] = o;

    return o;
}

//
// kexHashList::Find
//
template<class type>
type *kexHashList<type>::Find(const char *tname) const
{
    hashKey_t *t;
    unsigned int hash;

    hash = kexStr::Hash(tname);

    for(t = hashlist[hash]; t; t = t->next)
    {
        if(!strcmp(tname, t->name))
        {
            return &t->data;
        }
    }

    return NULL;
}

//
// kexHashList::GetHashKey
//
template<class type>
typename kexHashList<type>::hashKey_t *kexHashList<type>::GetHashKey(const char *tname)
{
    hashKey_t *t;
    unsigned int hash;

    hash = kexStr::Hash(tname);

    for(t = hashlist[hash]; t; t = t->next)
    {
        if(!strcmp(tname, t->name))
        {
            return t;
        }
    }

    return NULL;
}

//
// kexHashList::GetHashKey
//
template<class type>
typename kexHashList<type>::hashKey_t *kexHashList<type>::GetHashKey(const unsigned int index)
{
    return hashlist[index];
}

//
// kexHashList::Find
//
template<class type>
type *kexHashList<type>::Find(const int index) const
{
    unsigned int hash;
    hashKey_t *t;
    
    hash = index & (MAX_HASH-1);

    for(t = hashlist[hash]; t; t = t->next)
    {
        if(t->refIndex == -1)
        {
            continue;
        }

        if(t->refIndex == index)
        {
            return &t->data;
        }
    }

    return NULL;
}

//
// kexHashList::GetData
//
template<class type>
type *kexHashList<type>::GetData(const int index)
{
    rover = hashlist[index];
    return &rover->data;
}

//
// kexHashList::Next
//
template<class type>
type *kexHashList<type>::Next(void)
{
    if(rover->next)
    {
        rover = rover->next;
        return &rover->data;
    }

    return NULL;
}

//
// kexHashList::GetName
//
template<class type>
char *kexHashList<type>::GetName(const int index)
{
    return rover->name;
}

#endif
