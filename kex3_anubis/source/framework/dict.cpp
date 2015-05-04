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
//      Hashkey lookups
//

#include "kexlib.h"

//
// kexHashKey::kexHashKey
//

kexHashKey::kexHashKey(const char *key, const char *value)
{
    this->key = key;
    this->value = value;
}

//
// kexHashKey::kexHashKey
//

kexHashKey::kexHashKey(void)
{
    this->key = "";
    this->value = "";
}

//
// kexHashKey::~kexHashKey
//

kexHashKey::~kexHashKey(void)
{
}

//
// kexHashKey::operator=
//

void kexHashKey::operator=(kexHashKey &hashKey)
{
    this->key = hashKey.key;
    this->value = hashKey.value;
}

//
// kexDict::kexDict
//

kexDict::kexDict(void)
{
    this->hashMask = MAX_HASH;
    this->hashSize = 0;
    this->hashlist = NULL;
}

//
// kexDict::~kexDict
//

kexDict::~kexDict(void)
{
    if(hashlist != NULL && hashSize != 0)
    {
        for(int i = 0; i < hashSize; i++)
        {
            hashlist[i].Empty();
        }

        delete[] hashlist;
    }
}

//
// kexDict::Add
//

void kexDict::Add(const char *key, const char *value)
{
    int hash = kexStr::Hash(key) & (hashMask-1);

    Resize(hash+1);
    hashlist[hash].Push(kexHashKey(key, value));
}

//
// kexDict::Empty
//

void kexDict::Empty(void)
{
    if(hashlist != NULL && hashSize != 0)
    {
        for(int i = 0; i < hashSize; i++)
        {
            hashlist[i].Empty();
        }
    }
}

//
// kexDict::Resize
//

void kexDict::Resize(int newSize)
{
    kexArray<kexHashKey> *oldList;
    int i;

    if(newSize == hashSize)
    {
        return;
    }

    if(hashlist == NULL && hashSize == 0)
    {
        hashlist = new kexArray<kexHashKey>[newSize];
        hashSize = newSize;
        return;
    }

    if(newSize < hashSize || hashSize <= 0)
    {
        return;
    }

    oldList = hashlist;
    hashlist = new kexArray<kexHashKey>[newSize];

    for(i = 0; i < hashSize; i++)
    {
        hashlist[i] = oldList[i];
    }

    oldList->Empty();
    delete[] oldList;

    hashSize = newSize;
}

//
// kexDict::Find
//

kexHashKey *kexDict::Find(const char *name)
{
    kexHashKey *k;
    kexArray<kexHashKey> *keyList;
    int hash = kexStr::Hash(name) & (hashMask-1);

    if(hash >= hashSize)
    {
        return NULL;
    }

    keyList = &hashlist[hash];

    for(unsigned int i = 0; i < keyList->Length(); i++)
    {
        k = &hashlist[hash][i];
        if(!strcmp(name, k->GetName()))
        {
            return k;
        }
    }
    return NULL;
}

//
// kexDict::GetFloat
//

bool kexDict::GetFloat(const char *key, float &out, const float defaultValue)
{
    kexHashKey *k;

    out = defaultValue;

    if(!(k = Find(key)))
    {
        return false;
    }

    out = (float)atof(k->GetString());
    return true;
}

//
// kexDict::GetFloat
//

bool kexDict::GetFloat(const kexStr &key, float &out, const float defaultValue)
{
    return GetFloat(key.c_str(), out, defaultValue);
}

//
// kexDict::GetInt
//

bool kexDict::GetInt(const char *key, int &out, const int defaultValue)
{
    kexHashKey *k;

    out = defaultValue;

    if(!(k = Find(key)))
    {
        return false;
    }

    out = atoi(k->GetString());
    return true;
}

//
// kexDict::GetInt
//

bool kexDict::GetInt(const kexStr &key, int &out, const int defaultValue)
{
    return GetInt(key.c_str(), out, defaultValue);
}

//
// kexDict::GetInt
//

bool kexDict::GetInt(const char *key, uint8_t &out, const int defaultValue)
{
    kexHashKey *k;

    out = defaultValue;

    if(!(k = Find(key)))
    {
        return false;
    }

    out = (uint8_t)atoi(k->GetString());
    return true;
}

//
// kexDict::GetInt
//

bool kexDict::GetInt(const char *key, int16_t &out, const int defaultValue)
{
    kexHashKey *k;

    out = defaultValue;

    if(!(k = Find(key)))
    {
        return false;
    }

    out = (int16_t)atoi(k->GetString());
    return true;
}

//
// kexDict::GetInt
//

bool kexDict::GetInt(const kexStr &key, int16_t &out, const int defaultValue)
{
    return GetInt(key.c_str(), out, defaultValue);
}

//
// kexDict::GetBool
//

bool kexDict::GetBool(const char *key, bool &out, const bool defaultValue)
{
    kexHashKey *k;

    out = defaultValue;

    if(!(k = Find(key)))
    {
        return false;
    }

    out = (atoi(k->GetString()) != 0);
    return true;
}

//
// kexDict::GetBool
//

bool kexDict::GetBool(const kexStr &key, bool &out, const bool defaultValue)
{
    return GetBool(key.c_str(), out, defaultValue);
}

//
// kexDict::GetBool
//

bool kexDict::GetBool(const char *key)
{
    kexHashKey *k;

    if(!(k = Find(key)))
    {
        return false;
    }

    return (atoi(k->GetString()) != 0);
}

//
// kexDict::GetString
//

bool kexDict::GetString(const char *key, kexStr &out)
{
    kexHashKey *k;

    if(!(k = Find(key)))
    {
        return false;
    }

    out = k->GetString();
    return true;
}

//
// kexDict::GetString
//

bool kexDict::GetString(const kexStr &key, kexStr &out)
{
    return GetString(key.c_str(), out);
}

//
// kexDict::GetVector
//

bool kexDict::GetVector(const char *key, kexVec3 &out)
{
    kexHashKey *k;

    out.Clear();

    if(!(k = Find(key)))
    {
        return false;
    }

    sscanf(k->GetString(), "%f %f %f", &out.x, &out.y, &out.z);
    return true;
}

//
// kexDict::GetVector
//

bool kexDict::GetVector(const kexStr &key, kexVec3 &out)
{
    return GetVector(key.c_str(), out);
}
