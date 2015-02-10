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

#ifndef __SCRIPT_SYS_H__
#define __SCRIPT_SYS_H__

#include "angelscript.h"

class kexScriptManager
{
public:
    kexScriptManager(void);
    ~kexScriptManager(void);

    void                                Init(void);
    void                                Shutdown(void);
    void                                CallExternalScript(const char *file, const char *function);
    void                                CallCommand(const char *decl);
    void                                DrawGCStats(void);
    void                                RegisterMethod(const char *name, const char *decl,
                                                       const asSFuncPtr &funcPointer);

    static void                         *MemAlloc(size_t size);
    static void                         MemFree(void *ptr);

    template<class derived, class base>
    static derived                      *RefCast(base *c) { return static_cast<derived*>(c); }
    
    kexHashList<asIScriptFunction**>    &ActionList(void) { return actionList; }

    asIScriptEngine                     *Engine(void) { return engine; }
    asIScriptContext                    *Context(void) { return ctx; }
    asIScriptModule                     *Module(void) { return module; }

    bool                                bDrawGCStats;

private:
    void                                InitActions(void);
    void                                GetArgTypesFromFunction(kexStrList &list, asIScriptFunction *function);
    void                                ProcessScript(const char *file);
    bool                                HasScriptFile(const char *file);

    static void                         MessageCallback(const asSMessageInfo *msg, void *param);

    kexStrList                          scriptFiles;
    kexStr                              scriptBuffer;
    
    kexHashList<asIScriptFunction**>    actionList;

    asIScriptEngine                     *engine;
    asIScriptContext                    *ctx;
    asIScriptModule                     *module;
};

#endif
