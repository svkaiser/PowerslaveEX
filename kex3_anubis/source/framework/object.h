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

#ifndef __OBJECT_H__
#define __OBJECT_H__

#define BEGIN_KEX_CLASS(classname)                      \
class classname {                                       \
public:                                                 \
    static kexRTTI          info;                       \
    static kexObject        *Create(void);              \
    virtual kexRTTI         *GetInfo(void) const

#define BEGIN_EXTENDED_KEX_CLASS(classname, supername)  \
class classname : public supername {                    \
public:                                                 \
    static kexRTTI          info;                       \
    static kexObject        *Create(void);              \
    virtual kexRTTI         *GetInfo(void) const

#define END_KEX_CLASS() }

#define DEFINE_KEX_CLASS(classname, supername)                  \
    kexRTTI classname::info(#classname, #supername,             \
        classname::Create,                                      \
        (void(kexObject::*)(void))&classname::Spawn);           \
    kexRTTI *classname::GetInfo(void) const {                   \
        return &(classname::info);                              \
    }

#define DECLARE_KEX_CLASS(classname, supername)             \
    DEFINE_KEX_CLASS(classname, supername)                  \
    kexObject *classname::Create(void) {                    \
        return new classname;                               \
    }

#define DECLARE_ABSTRACT_KEX_CLASS(classname, supername)    \
    DEFINE_KEX_CLASS(classname, supername)                  \
    kexObject *classname::Create(void) {                    \
        return NULL;                                        \
    }

class kexBinFile;
class kexObject;
class kexRTTI;

typedef void(kexObject::*spawnObjFunc_t)(void);

class kexRTTI
{
public:
    kexRTTI(const char *classname, const char *supername,
            kexObject *(*Create)(void),
            void(kexObject::*Spawn)(void));
    ~kexRTTI(void);

    void                    Init(void);
    void                    Destroy(void);
    bool                    InstanceOf(const kexRTTI *objInfo) const;
    kexObject               *(*Create)(void);
    void                    (kexObject::*Spawn)(void);

    int                     type_id;
    const char              *classname;
    const char              *supername;
    kexRTTI                 *next;
    kexRTTI                 *super;
};

BEGIN_KEX_CLASS(kexObject);
    ~kexObject(void);

    const char              *ClassName(void) const;
    const kexStr            ClassString(void) const;
    const char              *SuperName(void) const;
    const kexStr            SuperString(void) const;
    bool                    InstanceOf(const kexRTTI *objInfo) const;
    bool                    InstanceOf(const kexStr &className) const;
    void                    CallSpawn(void);
    void                    Spawn(void);
    spawnObjFunc_t          ExecSpawnFunction(kexRTTI *objInfo);

    void                    *operator new(size_t s);
    void                    operator delete(void *ptr);

    static void             Init(void);
    static void             Shutdown(void);
    static kexRTTI          *Get(const char *classname);
    static kexObject        *Create(const char *name);
    static void             ListClasses(void);

    static int              roverID;
    static kexRTTI          *root;

    private:
    static bool             bInitialized;
    static int              numObjects;
END_KEX_CLASS();

#endif
