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

kexCvar kexScriptManager::cvarDumpMapScripts("g_dumpmapscripts", CVF_BOOL|CVF_CONFIG, "0", "Dumps compiled level scripts to disk");

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
    this->mapModule     = NULL;
    this->scriptNum     = 0;
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
// kexScriptManager::DelayScript
//

void kexScriptManager::DelayScript(const float time)
{
    asIScriptContext *context = asGetActiveContext();
    mapScriptInfo_t *mapScript;

    if(!context || context == scriptManagerLocal.Context())
    {
        return;
    }

    mapScript = (mapScriptInfo_t*)context->GetUserData(0);

    if(mapScript == NULL)
    {
        return;
    }

    context->Suspend();
    mapScript->delay = time;
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
    
    engine->RegisterGlobalFunction("void delay(const float)",
                                   asFUNCTION(kexScriptManager::DelayScript), asCALL_CDECL);

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
    
    ProcessScript("scripts/main.txt", module);
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
    
    if(ctx->GetState() == asEXECUTION_SUSPENDED)
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
                
                if(argNameTypes[0] != "kActor@" && argNameTypes[0] != "kAI@")
                {
                    kex::cSystem->Warning("%s must have kActor@ or kAI@ declared as the first argument\n",
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

void kexScriptManager::ProcessScript(const char *file, asIScriptModule *mod)
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
                    ProcessScript(nfile, mod);
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
        else if(ch == '$')
        {
            lexer->Find();

            if(lexer->Matches("script"))
            {
                scriptNum = lexer->GetNumber();
                const char *str = kexStr::Format("void mapscript_%i_root(kActor @instigator)", scriptNum);

                scriptBuffer += str;
                scrBuffer += str;
                continue;
            }
            else if(lexer->Matches("restart"))
            {
                lexer->ExpectNextToken(TK_SEMICOLON);

                kexStr str("Game.CallDelayedMapScript(");
                str += kexStr::Format("\"mapscript_%i_root\", instigator, 0);", scriptNum);

                scriptBuffer += str.c_str();
                scrBuffer += str.c_str();
                continue;
            }
        }

        scriptBuffer += ch;
        scrBuffer += ch;
    }

    mod->AddScriptSection(kexStr(file).StripExtension().StripPath(),
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

    asIScriptModule *tempModule;

    tempModule = engine->GetModule(kexStr(file).StripExtension().StripPath(),
                                   asGM_CREATE_IF_NOT_EXISTS);
    tempModule->AddScriptSection("externalSection", &data[0], size);
    tempModule->Build();

    Mem_Free(data);

    asIScriptFunction *func = tempModule->GetFunctionByDecl(function);

    if(func != 0)
    {
        ctx->Prepare(func);
        if(ctx->Execute() == asEXECUTION_FINISHED)
        {
            tempModule->Discard();
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
// kexScriptManager::LoadLevelScript
//

bool kexScriptManager::LoadLevelScript(const char *name)
{
    kexLexer *lexer;

    if(name[0] == 0)
    {
        return false;
    }

    if(!(lexer = kex::cParser->Open(name)))
    {
        return false;
    }

    scriptBuffer.Clear();
    scriptFiles.Empty();

    scriptNum = 0;

    mapModule = engine->GetModule("levelScript", asGM_ALWAYS_CREATE);

    ProcessScript(name, mapModule);
    scriptBuffer += "\0";

    if(cvarDumpMapScripts.GetBool())
    {
        kexStr fPath;

        fPath = kexStr::Format("%s\\scriptout.txt", kex::cvarBasePath.GetValue());
        fPath.NormalizeSlashes();

        scriptBuffer.WriteToFile(fPath);
    }

    mapModule->Build();
    return true;
}

//
// kexScriptManager::CallDelayedMapScript
//

void kexScriptManager::CallDelayedMapScript(const char *func, kexActor *instigator, const float delay)
{
    if(mapModule == NULL)
    {
        return;
    }

    mapScriptInfo_t *callScript = new mapScriptInfo_t;

    callScript->function = func;
    callScript->instigator = instigator;
    callScript->delay = delay;
    callScript->link.SetData(callScript);
    callScript->bDirty = false;
    callScript->context = engine->RequestContext();

    if(callScript->context)
    {
        callScript->context->SetUserData(callScript, 0);
    }

    callScript->link.Add(delayedMapScripts);
}

//
// kexScriptManager::CallDelayedMapScript
//

void kexScriptManager::CallDelayedMapScript(const int scriptNum, kexActor *instigator, const float delay)
{
    CallDelayedMapScript(kexStr::Format("mapscript_%i_root", scriptNum), instigator, delay);
}

//
// kexScriptManager::ExecuteMapScript
//

void kexScriptManager::ExecuteMapScript(mapScriptInfo_t *script)
{
    switch(script->context->Execute())
    {
    case asEXECUTION_EXCEPTION:
        kex::cSystem->Warning("%s", script->context->GetExceptionString());
        script->bDirty = true;
        break;

    case asEXECUTION_SUSPENDED:
        break;

    case asEXECUTION_FINISHED:
        script->bDirty = true;
        break;
    }
}

//
// kexScriptManager::RunMapScript
//

void kexScriptManager::RunMapScript(mapScriptInfo_t *script)
{
    kexStr function;
    asIScriptFunction *funcPtr;
    int state;

    if(script->context == NULL)
    {
        script->bDirty = true;
        return;
    }

    state = script->context->GetState();

    if(state == asEXECUTION_SUSPENDED)
    {
        // resume
        ExecuteMapScript(script);
        return;
    }

    if(state == asEXECUTION_ACTIVE)
    {
        script->context->PushState();
    }

    function = "void ";
    function += script->function;
    function += "(kActor@)";

    funcPtr = mapModule->GetFunctionByDecl(function);

    if(funcPtr == NULL)
    {
        if(state == asEXECUTION_ACTIVE)
        {
            script->context->PopState();
        }

        script->bDirty = true;
        return;
    }

    if(script->context->Prepare(funcPtr) != 0)
    {
        if(state == asEXECUTION_ACTIVE)
        {
            script->context->PopState();
        }

        script->bDirty = true;
        return;
    }

    script->context->SetArgObject(0, script->instigator);

    ExecuteMapScript(script);

    if(state == asEXECUTION_ACTIVE)
    {
        script->context->PopState();
    }
}

//
// kexScriptManager::DestroyMapScriptData
//

void kexScriptManager::DestroyMapScriptData(mapScriptInfo_t *script)
{
    if(script->context->GetState() == asEXECUTION_SUSPENDED)
    {
        script->context->Abort();
    }

    script->link.Remove();
    engine->ReturnContext(script->context);
}

//
// kexScriptManager::UpdateLevelScripts
//

void kexScriptManager::UpdateLevelScripts(void)
{
    mapScriptInfo_t *next = NULL;
    
    if(mapModule == NULL)
    {
        return;
    }
    
    for(mapScriptInfo_t *scrInfo = delayedMapScripts.Next(); scrInfo != NULL; scrInfo = next)
    {
        next = scrInfo->link.Next();

        if(scrInfo->bDirty == true)
        {
            DestroyMapScriptData(scrInfo);
            delete scrInfo;

            continue;
        }
        
        scrInfo->delay -= (1.0f / 60.0f);
        if(scrInfo->delay <= 0)
        {
            RunMapScript(scrInfo);
        }
    }
}

//
// kexScriptManager::HaltMapScript
//

void kexScriptManager::HaltMapScript(const int scriptNum)
{
    const char *search = kexStr::Format("mapscript_%i_", scriptNum);
    
    for(mapScriptInfo_t *scrInfo = delayedMapScripts.Next(); scrInfo != NULL; scrInfo = scrInfo->link.Next())
    {
        if(scrInfo->function.IndexOf(search) != -1)
        {
            scrInfo->bDirty = true;
        }
    }
}

//
// kexScriptManager::DestroyLevelScripts
//

void kexScriptManager::DestroyLevelScripts(void)
{
    if(mapModule)
    {
        mapScriptInfo_t *next = NULL;
        
        for(mapScriptInfo_t *scrInfo = delayedMapScripts.Next(); scrInfo != NULL; scrInfo = next)
        {
            next = scrInfo->link.Next();
            
            DestroyMapScriptData(scrInfo);
            delete scrInfo;
        }
        
        mapModule->Discard();
        mapModule = NULL;
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
