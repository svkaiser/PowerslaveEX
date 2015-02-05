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
//      Script Handle Object
//      Based on the CScriptHandle class provided by Angelscript
//

#include "kexlib.h"
#include "game.h"
#include "scriptSystem.h"
#include "refObject.h"

//
// kexScriptObjHandle::Init
//

void kexScriptObjHandle::Init(void)
{
    asIScriptEngine *e = kexGame::cScriptManager->Engine();

    e->RegisterObjectType("ref", sizeof(kexScriptObjHandle), asOBJ_VALUE | asOBJ_ASHANDLE | asOBJ_APP_CLASS_CDAK);
    e->RegisterObjectBehaviour( "ref", asBEHAVE_CONSTRUCT, "void f()", asFUNCTIONPR(kexScriptObjHandle::ObjectConstruct, (kexScriptObjHandle*), void), asCALL_CDECL_OBJFIRST);
    e->RegisterObjectBehaviour("ref", asBEHAVE_CONSTRUCT, "void f(const ref &in)", asFUNCTIONPR(kexScriptObjHandle::ObjectConstruct, (kexScriptObjHandle*, const kexScriptObjHandle&), void), asCALL_CDECL_OBJFIRST);
    e->RegisterObjectBehaviour("ref", asBEHAVE_CONSTRUCT, "void f(const ?&in)", asFUNCTIONPR(kexScriptObjHandle::ObjectConstruct, (kexScriptObjHandle*, void*, int), void), asCALL_CDECL_OBJFIRST);
    e->RegisterObjectBehaviour("ref", asBEHAVE_DESTRUCT, "void f()", asFUNCTIONPR(kexScriptObjHandle::ObjectDeconstruct, (kexScriptObjHandle *), void), asCALL_CDECL_OBJFIRST);
    e->RegisterObjectBehaviour("ref", asBEHAVE_REF_CAST, "void f(?&out)", asMETHODPR(kexScriptObjHandle, Cast, (void**, int), void), asCALL_THISCALL);
    e->RegisterObjectMethod("ref", "ref &opAssign(const ref &in)", asMETHOD(kexScriptObjHandle, operator=), asCALL_THISCALL);
    e->RegisterObjectMethod("ref", "ref &opAssign(const ?&in)", asMETHOD(kexScriptObjHandle, Assign), asCALL_THISCALL);
    e->RegisterObjectMethod("ref", "bool opEquals(const ref &in) const", asMETHODPR(kexScriptObjHandle, operator==, (const kexScriptObjHandle &) const, bool), asCALL_THISCALL);
    e->RegisterObjectMethod("ref", "bool opEquals(const ?&in) const", asMETHODPR(kexScriptObjHandle, Equals, (void*, int) const, bool), asCALL_THISCALL);
}

//
// kexScriptObjHandle::kexScriptObjHandle
//

kexScriptObjHandle::kexScriptObjHandle()
{
    m_ref   = 0;
    m_type  = 0;
    owner   = NULL;
}

//
// kexScriptObjHandle::kexScriptObjHandle
//

kexScriptObjHandle::kexScriptObjHandle(const kexScriptObjHandle &other)
{
    m_ref   = other.m_ref;
    m_type  = other.m_type;
    owner   = other.owner;

    AddRefHandle();
}

//
// kexScriptObjHandle::kexScriptObjHandle
//

kexScriptObjHandle::kexScriptObjHandle(void *ref, asIObjectType *type)
{
    m_ref   = ref;
    m_type  = type;
    owner   = NULL;

    AddRefHandle();
}

//
// kexScriptObjHandle::kexScriptObjHandle
//
// This constructor shouldn't be called from the application
// directly as it requires an active script context
//
kexScriptObjHandle::kexScriptObjHandle(void *ref, int typeId)
{
    m_ref   = 0;
    m_type  = 0;
    owner   = NULL;

    Assign(ref, typeId);
}

//
// kexScriptObjHandle::~kexScriptObjHandle
//

kexScriptObjHandle::~kexScriptObjHandle()
{
    ReleaseHandle();
}

//
// kexScriptObjHandle::ReleaseHandle
//

void kexScriptObjHandle::ReleaseHandle(void)
{
    if(m_ref && m_type)
    {
        asIScriptEngine *engine = kexGame::cScriptManager->Engine();
        engine->ReleaseScriptObject(m_ref, m_type);

        m_ref   = 0;
        m_type  = 0;
        owner   = NULL;
    }
}

//
// kexScriptObjHandle::AddRefHandle
//

void kexScriptObjHandle::AddRefHandle(void)
{
    if(m_ref && m_type)
    {
        asIScriptEngine *engine = kexGame::cScriptManager->Engine();
        engine->AddRefScriptObject(m_ref, m_type);
    }
}

//
// kexScriptObjHandle::operator=
//

kexScriptObjHandle &kexScriptObjHandle::operator=(const kexScriptObjHandle &other)
{
    // Don't do anything if it is the same reference
    if(m_ref == other.m_ref)
    {
        return *this;
    }

    ReleaseHandle();

    m_ref   = other.m_ref;
    m_type  = other.m_type;
    owner   = other.owner;

    AddRefHandle();
    return *this;
}

//
// kexScriptObjHandle::Set
//

void kexScriptObjHandle::Set(void *ref, asIObjectType *type)
{
    if(m_ref == ref)
    {
        return;
    }

    ReleaseHandle();

    m_ref  = ref;
    m_type = type;

    AddRefHandle();
}

//
// kexScriptObjHandle::GetType
//

asIObjectType *kexScriptObjHandle::GetType(void)
{
    return m_type;
}

//
// kexScriptObjHandle::Assign
//
// This method shouldn't be called from the application
// directly as it requires an active script context
//
kexScriptObjHandle &kexScriptObjHandle::Assign(void *ref, int typeId)
{
    // When receiving a null handle we just clear our memory
    if(typeId == 0)
    {
        Set(0, 0);
        return *this;
    }

    // Dereference received handles to get the object
    if(typeId & asTYPEID_OBJHANDLE)
    {
        // Store the actual reference
        ref = *(void**)ref;
        typeId &= ~asTYPEID_OBJHANDLE;
    }

    // Get the object type
    asIScriptEngine  *engine = kexGame::cScriptManager->Engine();
    asIObjectType    *type   = engine->GetObjectTypeById(typeId);

    Set(ref, type);
    return *this;
}

//
// kexScriptObjHandle::operator==
//

bool kexScriptObjHandle::operator==(const kexScriptObjHandle &o) const
{
    if(m_ref  == o.m_ref &&
            m_type == o.m_type &&
            owner == o.owner)
    {
        return true;
    }

    //
    // TODO: If type is not the same, we should attempt to do a dynamic cast,
    //       which may change the pointer for application registered classes
    //
    return false;
}

//
// kexScriptObjHandle::operator!=
//

bool kexScriptObjHandle::operator!=(const kexScriptObjHandle &o) const
{
    return !(*this == o);
}

//
// kexScriptObjHandle::Equals
//

bool kexScriptObjHandle::Equals(void *ref, int typeId) const
{
    // Null handles are received as reference to a null handle
    if(typeId == 0)
    {
        ref = 0;
    }

    // Dereference handles to get the object
    if(typeId & asTYPEID_OBJHANDLE)
    {
        // Compare the actual reference
        ref = *(void**)ref;
        typeId &= ~asTYPEID_OBJHANDLE;
    }

    //
    // TODO: If typeId is not the same, we should attempt to do a dynamic cast,
    //       which may change the pointer for application registered classes
    //
    if(ref == m_ref)
    {
        return true;
    }

    return false;
}

//
// kexScriptObjHandle::Cast
//
// AngelScript: used as '@obj = cast<obj>(ref);'
//

void kexScriptObjHandle::Cast(void **outRef, int typeId)
{
    // If we hold a null handle, then just return null
    if(m_type == 0)
    {
        *outRef = 0;
        return;
    }

    // It is expected that the outRef is always a handle
    assert(typeId & asTYPEID_OBJHANDLE);

    // Compare the type id of the actual object
    typeId &= ~asTYPEID_OBJHANDLE;
    asIScriptEngine  *engine = kexGame::cScriptManager->Engine();
    asIObjectType    *type   = engine->GetObjectTypeById(typeId);

    *outRef = 0;

    if(type == m_type)
    {
        // If the requested type is a script function it is
        // necessary to check if the functions are compatible too
        if(m_type->GetFlags() & asOBJ_SCRIPT_FUNCTION)
        {
            asIScriptFunction *func = reinterpret_cast<asIScriptFunction*>(m_ref);
            if( !func->IsCompatibleWithTypeId(typeId) )
            {
                return;
            }
        }

        // Must increase the ref count as we're returning a new reference to the object
        AddRefHandle();
        *outRef = m_ref;
    }
    else if(m_type->GetFlags() & asOBJ_SCRIPT_OBJECT)
    {
        // Attempt a dynamic cast of the stored handle to the requested handle type
        if(engine->IsHandleCompatibleWithObject(m_ref, m_type->GetTypeId(), typeId))
        {
            // The script type is compatible so we can simply return the same pointer
            AddRefHandle();
            *outRef = m_ref;
        }
    }
    else
    {
        // TODO: Check for the existance of a reference cast behaviour.
        //       Both implicit and explicit casts may be used
        //       Calling the reference cast behaviour may change the actual
        //       pointer so the AddRef must be called on the new pointer
    }
}
