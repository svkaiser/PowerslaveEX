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

#ifndef __SCRIPT_REF_OBJ_H__
#define __SCRIPT_REF_OBJ_H__

class kexScriptObjHandle
{
public:
    kexScriptObjHandle();
    kexScriptObjHandle(const kexScriptObjHandle &other);
    kexScriptObjHandle(void *ref, asIObjectType *type);
    ~kexScriptObjHandle();

    kexScriptObjHandle      &operator=(const kexScriptObjHandle &other);
    void                    Set(void *ref, asIObjectType *type);

    bool                    operator==(const kexScriptObjHandle &o) const;
    bool                    operator!=(const kexScriptObjHandle &o) const;
    bool                    Equals(void *ref, int typeId) const;

    void                    Cast(void **outRef, int typeId);
    asIObjectType           *GetType(void);
    void                    Clear(void) { m_ref = m_type = NULL; }

    void                    *owner;

    static void             Init(void);
    static void             ObjectConstruct(kexScriptObjHandle *self) { new(self)kexScriptObjHandle(); }
    static void             ObjectConstruct(kexScriptObjHandle *self, const kexScriptObjHandle &other)
    {
        new(self)kexScriptObjHandle(other);
    }
    static void             ObjectConstruct(kexScriptObjHandle *self, void *ref, int typeID)
    {
        new(self)kexScriptObjHandle(ref, typeID);
    }
    static void             ObjectDeconstruct(kexScriptObjHandle *self) { self->~kexScriptObjHandle(); }

protected:
    void                    ReleaseHandle(void);
    void                    AddRefHandle(void);

    // These shouldn't be called directly by the
    // application as they requires an active context
    kexScriptObjHandle(void *ref, int typeId);
    kexScriptObjHandle &Assign(void *ref, int typeId);

    void                    *m_ref;
    asIObjectType           *m_type;
};

#endif
