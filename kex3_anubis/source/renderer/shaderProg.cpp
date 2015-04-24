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
//      Shader Program Object
//

#include "renderMain.h"

//
// kexShaderObj::kexShaderObj
//

kexShaderObj::kexShaderObj(void)
{
    Init();
}

//
// kexShaderObj::~kexShaderObj
//

kexShaderObj::~kexShaderObj(void)
{
}

//
// kexShaderObj::Init
//

void kexShaderObj::Init(void)
{
    this->programObj            = 0;
    this->vertexProgram         = 0;
    this->fragmentProgram       = 0;
    this->bHasErrors            = false;
    this->bLoaded               = false;
}

//
// kexShaderObj::Load
//

void kexShaderObj::Load(const char *vertexShader, const char *fragmentShader)
{
    Init();
    
    programObj = dglCreateProgramObjectARB();

    Compile(vertexShader, RST_VERTEX);
    Compile(fragmentShader, RST_FRAGMENT);

    Link();
}

//
// kexShaderObj::Bind
//

void kexShaderObj::Bind(void)
{
    if(programObj == kexRender::cBackend->glState.currentProgram)
    {
        return;
    }

    dglUseProgramObjectARB(programObj);
    kexRender::cBackend->glState.currentProgram = programObj;
}

//
// kexShaderObj::Delete
//

void kexShaderObj::Delete(void)
{
    if(bLoaded == false)
    {
        return;
    }

    dglDeleteObjectARB(fragmentProgram);
    dglDeleteObjectARB(vertexProgram);
    dglDeleteObjectARB(programObj);
    bLoaded = false;
}

//
// kexShaderObj::Compile
//

void kexShaderObj::Compile(const char *name, rShaderType_t type)
{
    rhandle *handle;
    byte *data;
    kexLexer *lexer;
    kexStr progBuffer;
    bool bSkipLines = false;

    if(!(lexer = kex::cParser->Open(name)))
    {
        kex::cSystem->Warning("kexShaderObj::Compile: %s not found\n", name);
        return;
    }

    while(lexer->CheckState())
    {
        char ch = lexer->GetChar();

        if(ch == '#')
        {
            lexer->Find();
            if(lexer->Matches("ifdef"))
            {
                lexer->Find();
                if(lexer->Matches("KEX_IPHONE"))
                {
#ifndef KEX_IPHONE
                    bSkipLines = true;
#endif
                }
                else if(lexer->Matches("KEX_MACOSX"))
                {
#ifndef KEX_MACOSX
                    bSkipLines = true;
#endif
                }
                else if(lexer->Matches("KEX_WIN32"))
                {
#ifndef KEX_WIN32
                    bSkipLines = true;
#endif
                }

                continue;
            }
            else if(lexer->Matches("endif"))
            {
                bSkipLines = false;
                continue;
            }
        }

        if(bSkipLines == false)
        {
            progBuffer += ch;
        }
    }

    if(progBuffer.Length() <= 0)
    {
        kex::cSystem->Warning("kexShaderObj::Compile: %s is empty\n", name);
        kex::cParser->Close();
        return;
    }

    if(type == RST_VERTEX)
    {
        vertexProgram = dglCreateShaderObjectARB(GL_VERTEX_SHADER);
        handle = &vertexProgram;
        vertFile = name;
    }
    else if(type == RST_FRAGMENT)
    {
        fragmentProgram = dglCreateShaderObjectARB(GL_FRAGMENT_SHADER);
        handle = &fragmentProgram;
        fragFile = name;
    }
    else
    {
        kex::cParser->Close();
        return;
    }

    data = (byte*)progBuffer.c_str();

    dglShaderSourceARB(*handle, 1, (const char**)&data, NULL);
    dglCompileShaderARB(*handle);
    dglAttachObjectARB(programObj, *handle);

    kex::cParser->Close();
}

//
// kexShaderObj::SetUniform
//

void kexShaderObj::SetUniform(const char *name, const int val)
{
    int loc = dglGetUniformLocationARB(programObj, name);

    if(loc != -1)
    {
        dglUniform1iARB(loc, val);
    }
}

//
// kexShaderObj::SetUniform
//

void kexShaderObj::SetUniform(const char *name, const int *val, const int size)
{
    int loc = dglGetUniformLocationARB(programObj, name);

    if(loc != -1)
    {
        dglUniform1ivARB(loc, size, val);
    }
}

//
// kexShaderObj::SetUniform
//

void kexShaderObj::SetUniform(const char *name, const float val)
{
    int loc = dglGetUniformLocationARB(programObj, name);

    if(loc != -1)
    {
        dglUniform1fARB(loc, val);
    }
}

//
// kexShaderObj::SetUniform
//

void kexShaderObj::SetUniform(const char *name, kexVec2 &val)
{
    int loc = dglGetUniformLocationARB(programObj, name);

    if(loc != -1)
    {
        dglUniform2fvARB(loc, 1, val.ToFloatPtr());
    }
}

//
// kexShaderObj::SetUniform
//

void kexShaderObj::SetUniform(const char *name, kexVec3 &val)
{
    int loc = dglGetUniformLocationARB(programObj, name);

    if(loc != -1)
    {
        dglUniform3fvARB(loc, 1, val.ToFloatPtr());
    }
}

//
// kexShaderObj::SetUniform
//

void kexShaderObj::SetUniform(const char *name, kexVec3 *val, const int size)
{
    int loc = dglGetUniformLocationARB(programObj, name);

    if(loc != -1)
    {
        dglUniform3fvARB(loc, size, reinterpret_cast<float*>(&val[0].x));
    }
}

//
// kexShaderObj::SetUniform
//

void kexShaderObj::SetUniform(const char *name, kexVec4 &val)
{
    int loc = dglGetUniformLocationARB(programObj, name);

    if(loc != -1)
    {
        dglUniform4fvARB(loc, 1, val.ToFloatPtr());
    }
}

//
// kexShaderObj::SetUniform
//

void kexShaderObj::SetUniform(const char *name, kexMatrix &val, bool bTranspose)
{
    int loc = dglGetUniformLocationARB(programObj, name);

    if(loc != -1)
    {
        dglUniformMatrix4fvARB(loc, 1, bTranspose, val.ToFloatPtr());
    }
}

//
// kexShaderObj::DumpErrorLog
//

void kexShaderObj::DumpErrorLog(const rShaderType_t type, const rhandle handle)
{
    int logLength;

    dglGetObjectParameterivARB(handle, GL_INFO_LOG_LENGTH, &logLength);

    if(logLength > 0)
    {
        char *log;
        const char *file;

        log = (char*)Mem_Alloca(logLength);

        dglGetInfoLogARB(handle, logLength, &logLength, log);

        if(type == RST_VERTEX)
        {
            file = vertFile.c_str();
        }
        else if(type == RST_FRAGMENT)
        {
            file = fragFile.c_str();
        }
        else
        {
            file = "(invalid)";
        }

        kex::cSystem->Warning("\n");
        kex::cSystem->Warning("----------------------------\n");
        kex::cSystem->Warning("%s\n", file);
        kex::cSystem->Warning("%s", log);
        kex::cSystem->Warning("----------------------------\n\n");
    }
}

//
// kexShaderObj::Link
//

bool kexShaderObj::Link(void)
{
    int linked;

    bHasErrors = false;

    dglLinkProgramARB(programObj);
    dglGetObjectParameterivARB(programObj, GL_LINK_STATUS, &linked);

    if(!linked)
    {
        bHasErrors = true;
        DumpErrorLog(RST_VERTEX, vertexProgram);
        DumpErrorLog(RST_FRAGMENT, fragmentProgram);

        kex::cSystem->Error("kexShaderObj::Link: see log for shader compiler error");
    }
    else
    {
        dglUseProgramObjectARB(programObj);
    }

    dglUseProgramObjectARB(0);
    bLoaded = true;
    return (linked > 0);
}
