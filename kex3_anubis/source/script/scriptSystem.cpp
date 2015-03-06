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
//      Script System
//

#include "kexlib.h"
#include "game.h"
#include "renderMain.h"
#include "scriptSystem.h"

#include "objects/refObject.h"
#include "objects/mathObject.h"
#include "objects/stringObject.h"
#include "objects/systemObject.h"
#include "objects/actorObject.h"
#include "objects/gameSysObject.h"

static kexScriptManager scriptManagerLocal;
kexScriptManager *kexGame::cScriptManager = &scriptManagerLocal;

static kexHeapBlock hb_script("script", false, NULL, NULL);

//
// call
//

COMMAND(call)
{
    if(kex::cCommands->GetArgc() < 2)
    {
        kex::cSystem->Printf("Usage: call <\"function name\">\n");
        return;
    }
    scriptManagerLocal.CallCommand(kexStr::Format("void %s(void)", kex::cCommands->GetArgv(1)));
}

//
// callfile
//

COMMAND(callfile)
{
    if(kex::cCommands->GetArgc() < 3)
    {
        kex::cSystem->Printf("Usage: callfile <\"file name\"> <\"function name\">\n");
        return;
    }
    scriptManagerLocal.CallExternalScript(kex::cCommands->GetArgv(1),
                                          kexStr::Format("void %s(void)", kex::cCommands->GetArgv(2)));
}

//
// scriptmem
//

COMMAND(scriptmem)
{
    kex::cSystem->CPrintf(RGBA(0, 255, 255, 255), "Script Memory Usage:\n");
    kex::cSystem->CPrintf(COLOR_YELLOW, "%ikb\n", kexHeap::Usage(hb_script) >> 10);
}

//
// statscripts
//

COMMAND(statscripts)
{
    scriptManagerLocal.bDrawGCStats ^= 1;
}

//
// kexScriptManager::kexScriptManager
//

kexScriptManager::kexScriptManager(void)
{
    this->engine        = NULL;
    this->ctx           = NULL;
    this->module        = NULL;
    this->bDrawGCStats  = false;
}

//
// kexScriptManager::~kexScriptManager
//

kexScriptManager::~kexScriptManager(void)
{
}

//
// kexScriptManager::MemAlloc
//

void *kexScriptManager::MemAlloc(size_t size)
{
    return Mem_Malloc(size, hb_script);
}

//
// kexScriptManager::MemFree
//

void kexScriptManager::MemFree(void *ptr)
{
    Mem_Free(ptr);
}

//
// kexScriptManager::MessageCallback
//

void kexScriptManager::MessageCallback(const asSMessageInfo *msg, void *param)
{
    switch(msg->type)
    {
    case asMSGTYPE_INFORMATION:
        kex::cSystem->Printf("%s (%d, %d) : %s\n",
                               msg->section,
                               msg->row,
                               msg->col,
                               msg->message);
        break;
    default:
        kex::cSystem->Error("%s (%d, %d) : %s\n",
                              msg->section,
                              msg->row,
                              msg->col,
                              msg->message);
        break;
    }
}

//
// kexScriptManager::Init
//

void kexScriptManager::Init(void)
{
    if(asSetGlobalMemoryFunctions(kexScriptManager::MemAlloc, kexScriptManager::MemFree) == -1)
    {
        kex::cSystem->Error("kexScriptManager::Init: Unable to register memory functions\n");
        return;
    }

    if(!(engine = asCreateScriptEngine(ANGELSCRIPT_VERSION)))
    {
        kex::cSystem->Error("kexScriptManager::Init: Unable to register script engine\n");
        return;
    }

    engine->SetEngineProperty(asEP_COMPILER_WARNINGS, 2);
    engine->SetMessageCallback(asFUNCTION(kexScriptManager::MessageCallback), 0, asCALL_CDECL);

    ctx = engine->CreateContext();

    kexScriptObjHandle::Init();
    kexScriptObjString::Init();
    kexScriptObjSystem::Init();
    kexScriptObjMath::Init();
    kexScriptObjVec3::Init();
    kexScriptObjQuat::Init();
    kexScriptObjAngle::Init();
    kexScriptObjActor::Init();
    kexScriptObjGame::Init();
    
    module = engine->GetModule("core", asGM_CREATE_IF_NOT_EXISTS);
    
    ProcessScript("scripts/main.txt");
    scriptBuffer += "\0";
    
    module->Build();

    if(PrepareFunction("void main(void)"))
    {
        Execute();
    }
    
    InitActions();
    kex::cSystem->Printf("Script System Initialized\n");
}

//
// kexScriptManager::Shutdown
//

void kexScriptManager::Shutdown(void)
{
    kex::cSystem->Printf("Shutting down scripting system\n");

    ctx->Release();
    engine->Release();

    Mem_Purge(hb_script);
}

//
// kexScriptManager::PushState
//

void kexScriptManager::PushState(void)
{
    state = ctx->GetState();

    if(state == asEXECUTION_ACTIVE)
    {
        ctx->PushState();
    }
}

//
// kexScriptManager::PopState
//

void kexScriptManager::PopState(void)
{
    if(state == asEXECUTION_ACTIVE)
    {
        ctx->PopState();
    }
}

//
// kexScriptManager::PrepareFunction
//

bool kexScriptManager::PrepareFunction(asIScriptFunction *function)
{
    if(function == NULL)
    {
        return false;
    }

    PushState();

    if(ctx->Prepare(function) != 0)
    {
        PopState();
        return false;
    }

    return true;
}

//
// kexScriptManager::PrepareFunction
//

bool kexScriptManager::PrepareFunction(const char *function)
{
    return PrepareFunction(module->GetFunctionByDecl(function));
}

//
// kexScriptManager::ExecuteFunction
//

bool kexScriptManager::Execute(void)
{
    if(ctx->Execute() == asEXECUTION_EXCEPTION)
    {
        PopState();
        kex::cSystem->Error("%s", ctx->GetExceptionString());
        return false;
    }

    PopState();
    return true;
}

//
// kexScriptManager::GetArgTypesFromFunction
//

void kexScriptManager::GetArgTypesFromFunction(kexStrList &list, asIScriptFunction *function)
{
    kexStr strTemp;
    
    for(unsigned int i = 0; i < function->GetParamCount(); ++i)
    {
        int idx;

        strTemp = function->GetVarDecl(i);
        idx = strTemp.IndexOf("const");

        if(idx != -1)
        {
            strTemp.Remove(idx, strTemp.IndexOf(" ")+1);
        }
        
        strTemp.Remove(strTemp.IndexOf(" "), strTemp.Length());
        list.Push(kexStr(strTemp));
    }
}

//
// kexScriptManager::InitActions
//

void kexScriptManager::InitActions(void)
{
    kexLexer *lexer;
    kexStr actionName;
    asIScriptFunction *func;
    
    if(!(lexer = kex::cParser->Open("scripts/actions.txt")))
    {
        return;
    }
    
    while(lexer->CheckState())
    {
        lexer->Find();
        if(lexer->TokenType() == TK_IDENIFIER)
        {
            actionName = lexer->Token();
            
            lexer->GetString();
            if((func = module->GetFunctionByDecl(lexer->StringToken())) != 0)
            {
                kexStrList argNameTypes;
                
                if(func->GetParamCount() == 0)
                {
                    kex::cSystem->Warning("%s must have at least kActor@ declared as the first argument\n",
                                          actionName.c_str());
                    continue;
                }
                
                GetArgTypesFromFunction(argNameTypes, func);
                
                if(argNameTypes[0] != "kActor@")
                {
                    kex::cSystem->Warning("%s must have kActor@ declared as the first argument\n",
                                          actionName.c_str());
                    continue;
                }
                
                asIScriptFunction **f = actionList.Add(actionName);
                *f = func;

                kexGame::cActionDefManager->RegisterScriptAction(actionName, argNameTypes, func->GetParamCount());
            }
            else
            {
                kex::cSystem->Warning("%s is not declared in the script\n", lexer->StringToken());
            }
        }
    }
    
    kex::cParser->Close();
}

//
// kexScriptManager::HasScriptFile
//

bool kexScriptManager::HasScriptFile(const char *file)
{
    kexStr fileName(file);

    fileName.StripExtension().StripPath();

    for(unsigned int i = 0; i < scriptFiles.Length(); i++)
    {
        if(scriptFiles[i] == fileName)
        {
            return true;
        }
    }

    return false;
}

//
// kexScriptManager::ProcessScript
//

void kexScriptManager::ProcessScript(const char *file)
{
    kexLexer *lexer;
    kexStr scrBuffer;

    if(!(lexer = kex::cParser->Open(file)))
    {
        return;
    }

    while(lexer->CheckState())
    {
        char ch = lexer->GetChar();

        if(ch == '#')
        {
            lexer->Find();
            if(lexer->Matches("include"))
            {
                lexer->GetString();
                char *nfile = lexer->StringToken();

                if(!HasScriptFile(nfile))
                {
                    ProcessScript(nfile);
                    scriptFiles.Push(kexStr(nfile).StripExtension().StripPath());
                }
                continue;
            }
            else
            {
                kex::cParser->Error("kexScriptManager::ProcessScript: unknown token: %s\n",
                                      lexer->Token());
            }
        }

        scriptBuffer += ch;
        scrBuffer += ch;
    }

    module->AddScriptSection(kexStr(file).StripExtension().StripPath(),
                             scrBuffer.c_str(), scrBuffer.Length());

    kex::cParser->Close();
}

//
// kexScriptManager::RegisterMethod
//

void kexScriptManager::RegisterMethod(const char *name, const char *decl,
                                      const asSFuncPtr &funcPointer)
{
    engine->RegisterObjectMethod(name, decl, funcPointer, asCALL_THISCALL);
}

//
// kexScriptManager::CallExternalScript
//

void kexScriptManager::CallExternalScript(const char *file, const char *function)
{
    unsigned int size;
    char *data = NULL;

    if((size = kex::cPakFiles->OpenExternalFile(file, (byte**)&data)) == -1)
    {
        kex::cSystem->Warning("No file named %s\n", file);
        return;
    }

    asIScriptModule *mod;

    mod = engine->GetModule(kexStr(file).StripExtension().StripPath(),
                            asGM_CREATE_IF_NOT_EXISTS);
    mod->AddScriptSection("externalSection", &data[0], size);
    mod->Build();

    Mem_Free(data);

    asIScriptFunction *func = mod->GetFunctionByDecl(function);

    if(func != 0)
    {
        ctx->Prepare(func);
        if(ctx->Execute() == asEXECUTION_FINISHED)
        {
            mod->Discard();
            return;
        }

        kex::cSystem->Warning("Execution of %s did not finish\n", function);
        return;
    }
    kex::cSystem->Warning("No function declared as %s\n", function);
}

//
// kexScriptManager::CallCommand
//

void kexScriptManager::CallCommand(const char *decl)
{
    asIScriptFunction *func = module->GetFunctionByDecl(decl);

    if(func != 0)
    {
        int state = ctx->GetState();

        if(state == asEXECUTION_ACTIVE)
        {
            ctx->PushState();
        }

        ctx->Prepare(func);
        if(ctx->Execute() == asEXECUTION_EXCEPTION)
        {
            kex::cSystem->Error("%s", ctx->GetExceptionString());
        }

        if(state == asEXECUTION_ACTIVE)
        {
            ctx->PopState();
        }
    }
}

//
// kexScriptManager::DrawGCStats
//

void kexScriptManager::DrawGCStats(void)
{
    unsigned int data[5];

    if(bDrawGCStats == false)
    {
        return;
    }

    engine->GetGCStatistics(
        &data[0],
        &data[1],
        &data[2],
        &data[3],
        &data[4]);

    kexRender::cUtils->PrintStatsText("CurrentSize:", ": %i", data[0]);
    kexRender::cUtils->PrintStatsText("Total Destroyed:", ": %i", data[1]);
    kexRender::cUtils->PrintStatsText("Total Detected:", ": %i", data[2]);
    kexRender::cUtils->PrintStatsText("New Objects:", ": %i", data[3]);
    kexRender::cUtils->PrintStatsText("Total New Destroyed:", ": %i", data[4]);
    kexRender::cUtils->AddDebugLineSpacing();
}
