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
//      OpenGL Loader/Interface
//

#include "SDL.h"
#include "kexlib.h"
#include "dgl.h"

#if defined(__GNUC__)
    __attribute__ ((visibility("default"))) unsigned long NvOptimusEnablement = 0x00000001;
#elif defined(_MSC_VER)
    _declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
#endif

GL_ARB_multitexture_Define();
GL_EXT_compiled_vertex_array_Define();
GL_ARB_texture_non_power_of_two_Define();
GL_ARB_texture_env_combine_Define();
GL_EXT_texture_env_combine_Define();
GL_EXT_texture_filter_anisotropic_Define();
GL_ARB_vertex_buffer_object_Define();
GL_ARB_shader_objects_Define();
GL_ARB_framebuffer_object_Define();

static kexGLContext glcontext;
kexGLContext *kex::cGLContext = &glcontext;

//
// kexGLContext::kexGLContext
//

kexGLContext::kexGLContext(void)
{
}

//
// kexGLContext::Init
//

void kexGLContext::Init(void)
{
    GL_ARB_multitexture_Init();
    GL_EXT_compiled_vertex_array_Init();
    GL_ARB_texture_non_power_of_two_Init();
    GL_ARB_texture_env_combine_Init();
    GL_EXT_texture_env_combine_Init();
    GL_EXT_texture_filter_anisotropic_Init();
    GL_ARB_vertex_buffer_object_Init();
    GL_ARB_shader_objects_Init();
    GL_ARB_framebuffer_object_Init();

    kex::cSystem->Printf("OpenGL Initialized\n");
}

//
// kexGLContext::Log
//

void kexGLContext::Log(const char *message, const char *file, int line)
{
    GLint err = glGetError();

    if(err != GL_NO_ERROR)
    {
        char str[64];

        switch(err)
        {
        case GL_INVALID_ENUM:
            strcpy(str, "INVALID_ENUM");
            break;
        case GL_INVALID_VALUE:
            strcpy(str, "INVALID_VALUE");
            break;
        case GL_INVALID_OPERATION:
            strcpy(str, "INVALID_OPERATION");
            break;
        case GL_STACK_OVERFLOW:
            strcpy(str, "STACK_OVERFLOW");
            break;
        case GL_STACK_UNDERFLOW:
            strcpy(str, "STACK_UNDERFLOW");
            break;
        case GL_OUT_OF_MEMORY:
            strcpy(str, "OUT_OF_MEMORY");
            break;
        default:
            sprintf(str, "0x%x", err);
            break;
        }

        kex::cSystem->Warning("\nGL ERROR (%s) on gl function: %s (file = %s, line = %i)\n\n",
            str, message, file, line);
    }
}

//
// kexGLContext::RegisterProc
//

void *kexGLContext::RegisterProc(const char *address)
{
    return kex::cSystem->GetProcAddress(address);
}

//
// kexGLContext::FindExtension
//

bool kexGLContext::FindExtension(const char *ext)
{
    const byte *extensions = NULL;
    const byte *start;
    byte *where, *terminator;

    // Extension names should not have spaces.
    where = (byte *) strrchr((char*)ext, ' ');
    if(where || *ext == '\0')
    {
        return 0;
    }

    extensions = dglGetString(GL_EXTENSIONS);

    start = extensions;
    for(;;)
    {
        where = (byte*)strstr((char*)start, ext);
        if(!where)
        {
            break;
        }

        terminator = where + strlen(ext);
        if(where == start || *(where - 1) == ' ')
        {
            if(*terminator == ' ' || *terminator == '\0')
            {
                return true;
            }

            start = terminator;
        }
    }

    return false;
}

//
// kexGLContext::CheckExtension
//

bool kexGLContext::CheckExtension(const char *ext)
{
    if(FindExtension(ext))
    {
        kex::cSystem->Warning("GL Extension: %s = true\n", ext);
        return true;
    }
    else
    {
        kex::cSystem->Warning("GL Extension: %s = false\n", ext);
    }

    return false;
}
