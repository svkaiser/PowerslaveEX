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
//      Main System
//

#include "SDL.h"
#include "kexlib.h"

class kexSystemLocal : public kexSystem
{
public:
    kexSystemLocal(void);
    ~kexSystemLocal(void);

    virtual void            Main(int argc, char **argv);
    virtual void            Init(void);
    virtual void            Sleep(unsigned long usecs);
    virtual void            Shutdown(void);
    virtual int             GetMS(void);
    virtual uint64_t        GetPerformanceCounter(void);
    virtual int             GetTicks(void);
    virtual void            SwapBuffers(void);
    virtual int             GetWindowFlags(void);
    virtual const char      *GetWindowTitle(void);
    virtual void            SetWindowTitle(const char *string);
    virtual void            SetWindowGrab(const bool bEnable);
    virtual void            WarpMouseToCenter(void);
    virtual void            *GetProcAddress(const char *proc);
    virtual int             CheckParam(const char *check);
    virtual const char      *GetBaseDirectory(void);
    virtual void            Log(const char *fmt, ...);
    virtual void            Printf(const char *string, ...);
    virtual void            CPrintf(rcolor color, const char *string, ...);
    virtual void            Warning(const char *string, ...);
    virtual void            DPrintf(const char *string, ...);
    virtual void            Error(const char *string, ...);

    virtual void            *Window(void) { return window; }

private:
    void                    InitVideo(void);

    SDL_Window              *window;
    SDL_GLContext           glContext;
};

kexCvar cvarFixedTime("fixedtime", CVF_INT|CVF_CONFIG, "0", "TODO");
kexCvar cvarVidWidth("v_width", CVF_INT|CVF_CONFIG, "640", "TODO");
kexCvar cvarVidHeight("v_height", CVF_INT|CVF_CONFIG, "480", "TODO");
kexCvar cvarVidWindowed("v_windowed", CVF_BOOL|CVF_CONFIG, "1", "TODO");
kexCvar cvarVidVSync("v_vsync", CVF_BOOL|CVF_CONFIG, "1", "TODO");
kexCvar cvarVidDepthSize("v_depthsize", CVF_INT|CVF_CONFIG, "24", "TODO");
kexCvar cvarVidStencilSize("v_stencilsize", CVF_INT|CVF_CONFIG, "8", "TODO");
kexCvar cvarVidBuffSize("v_buffersize", CVF_INT|CVF_CONFIG, "32", "TODO");

static kexSystemLocal systemLocal;
kexSystem *kex::cSystem = &systemLocal;

static char buffer[4096];

//
// quit
//

COMMAND(quit)
{
    kex::cSystem->Shutdown();
}

//
// kexSystemLocal::kexSystemLocal
//

kexSystemLocal::kexSystemLocal(void)
{
    bShuttingDown = false;
}

//
// kexSystemLocal::~kexSystemLocal
//

kexSystemLocal::~kexSystemLocal(void)
{
}

//
// kexSystemLocal::Sleep
//

void kexSystemLocal::Sleep(unsigned long usecs)
{
    SDL_Delay(usecs);
}

//
// kexSystemLocal::Shutdown
//

void kexSystemLocal::Shutdown(void)
{
    bShuttingDown = true;

    Mem_Purge(hb_static);
    Mem_Purge(hb_object);

    if(glContext)
    {
        SDL_GL_DeleteContext(glContext);
        glContext = NULL;
    }

    if(window)
    {
        SDL_DestroyWindow(window);
        window = NULL;
    }

    SDL_Quit();

    fclose(f_stdout);
    fclose(f_stderr);

    exit(0);
}

//
// kexSystemLocal::GetTicks
//

int kexSystemLocal::GetTicks(void)
{
    return SDL_GetTicks();
}

//
// kexSystemLocal::GetMS
//

int kexSystemLocal::GetMS(void)
{
    uint32_t ticks;
    static int basetime = 0;

    ticks = GetTicks();

    if(basetime == 0)
    {
        basetime = ticks;
    }

    return ticks - basetime;
}

//
// kexSystemLocal::GetPerformanceCounter
//

uint64_t kexSystemLocal::GetPerformanceCounter(void)
{
    return SDL_GetPerformanceCounter();
}

//
// kexSystemLocal::Init
//

void kexSystemLocal::Init(void)
{
    if(SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        kex::cSystem->Error("Failed to initialize SDL");
        return;
    }

    SDL_ShowCursor(0);
    kex::cSystem->Printf("SDL Initialized\n");
}

//
// kexSystemLocal::InitVideo
//

void kexSystemLocal::InitVideo(void)
{
    int newwidth;
    int newheight;
    int p;
    uint32_t flags = 0;
    
    bWindowed = cvarVidWindowed.GetBool();
    videoWidth = cvarVidWidth.GetInt();
    videoHeight = cvarVidHeight.GetInt();
    videoRatio = (float)videoWidth / (float)videoHeight;
    
    if(CheckParam("-window"))
    {
        bWindowed = true;
    }

    if(CheckParam("-fullscreen"))
    {
        bWindowed = false;
    }
    
    newwidth = newheight = 0;
    
    p = CheckParam("-width");
    if(p && p < argc - 1)
    {
        newwidth = atoi(argv[p+1]);
    }

    p = CheckParam("-height");
    if(p && p < argc - 1)
    {
        newheight = atoi(argv[p+1]);
    }
    
    if(newwidth && newheight)
    {
        videoWidth = newwidth;
        videoHeight = newheight;
    }

    if(cvarVidDepthSize.GetInt() != 8 &&
       cvarVidDepthSize.GetInt() != 16 &&
       cvarVidDepthSize.GetInt() != 24)
    {
        cvarVidDepthSize.Set(24);
    }

    if(cvarVidStencilSize.GetInt() != 8)
    {
        cvarVidStencilSize.Set(8);
    }
    
    if(cvarVidBuffSize.GetInt() != 8 &&
       cvarVidBuffSize.GetInt() != 16 &&
       cvarVidBuffSize.GetInt() != 24 &&
       cvarVidBuffSize.GetInt() != 32)
    {
        cvarVidBuffSize.Set(32);
    }

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_RED_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_GREEN_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_BLUE_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_ALPHA_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, cvarVidBuffSize.GetInt());
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, cvarVidDepthSize.GetInt());
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, cvarVidStencilSize.GetInt());
    SDL_GL_SetSwapInterval(cvarVidVSync.GetBool());
    
    flags |= SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_MOUSE_FOCUS;
    
    if(!bWindowed)
    {
        flags |= SDL_WINDOW_FULLSCREEN;
    }

    window = SDL_CreateWindow(kexStr::Format("Kex Engine (Anubis) - Version Date: %s", __DATE__),
                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              videoWidth, videoHeight, flags);
    
    if(window == NULL)
    {
        kex::cSystem->Error("kexSystem::InitVideo: Failed to create window");
    }

    if((glContext = SDL_GL_CreateContext(window)) == NULL)
    {
        kex::cSystem->Error("kexSystem::InitVideo: Failed to create opengl context");
    }

    SDL_GL_MakeCurrent(window, glContext);
    SDL_GL_SetSwapInterval(1);

    kex::cSystem->Printf("Video Initialized\n");
}

//
// kexSystemLocal::Log
//

void kexSystemLocal::Log(const char *fmt, ...)
{
#define MAX_LOGMESSAGE_LENGTH   3584
    va_list list;
    time_t copy;
    static char buffer[64];
    struct tm *local;
    char logMessage[MAX_LOGMESSAGE_LENGTH];

    SDL_memset(logMessage, 0, MAX_LOGMESSAGE_LENGTH);
    va_start(list, fmt);
    SDL_vsnprintf(logMessage, MAX_LOGMESSAGE_LENGTH - 1, fmt, list);
    va_end(list);

    SDL_memset(buffer, 0, sizeof(buffer));
    copy = time(0);
    local = localtime(&copy);
    strftime(buffer, sizeof(buffer), "%X", local);

    printf("%s: %s", buffer, logMessage);
}

//
// kexSystemLocal::SwapBuffers
//

void kexSystemLocal::SwapBuffers(void)
{
    SDL_GL_SwapWindow(window);
}

//
// kexSystemLocal::GetWindowFlags
//

int kexSystemLocal::GetWindowFlags(void)
{
    return SDL_GetWindowFlags(window);
}

//
// kexSystemLocal::GetWindowTitle
//

const char *kexSystemLocal::GetWindowTitle(void)
{
    return SDL_GetWindowTitle(window);
}

//
// kexSystemLocal::SetWindowTitle
//

void kexSystemLocal::SetWindowTitle(const char *string)
{
    SDL_SetWindowTitle(window, string);
}

//
// kexSystemLocal::SetWindowGrab
//

void kexSystemLocal::SetWindowGrab(const bool bEnable)
{
    SDL_SetWindowGrab(window, (SDL_bool)bEnable);
}

//
// kexSystemLocal::WarpMouseToCenter
//

void kexSystemLocal::WarpMouseToCenter(void)
{
    SDL_WarpMouseInWindow(window,
                          (unsigned short)(kex::cSystem->VideoWidth()/2),
                          (unsigned short)(kex::cSystem->VideoHeight()/2));
}

//
// kexSystemLocal::GetProcAddress
//

void *kexSystemLocal::GetProcAddress(const char *proc)
{
    void *func = SDL_GL_GetProcAddress(proc);

    if(!func)
    {
        Warning("GetProcAddress: %s not found\n", proc);
    }

    return func;
}

//
// kexSystemLocal::CheckParam
//

int kexSystemLocal::CheckParam(const char *check)
{
    for(int i = 1; i < argc; i++)
    {
        if(!kexStr::Compare(check, argv[i]))
        {
            return i;
        }
    }
    return 0;
}

//
// kexSystemLocal::Printf
//

void kexSystemLocal::Printf(const char *string, ...)
{
}

//
// kexSystemLocal::CPrintf
//

void kexSystemLocal::CPrintf(rcolor color, const char *string, ...)
{
}

//
// kexSystemLocal::Warning
//

void kexSystemLocal::Warning(const char *string, ...)
{
}

//
// kexSystemLocal::DPrintf
//

void kexSystemLocal::DPrintf(const char *string, ...)
{
}

//
// kexSystemLocal::Error
//

void kexSystemLocal::Error(const char* string, ...)
{
    va_list	va;

    va_start(va, string);
    vsprintf(buffer, string, va);
    va_end(va);

    fprintf(stderr, "Error - %s\n", buffer);
    fflush(stderr);

    Log(buffer);

    const SDL_MessageBoxButtonData buttons[1] =
    {
        {
            SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT,
            0,
            "OK"
        }
    };
    SDL_MessageBoxData data =
    {
        SDL_MESSAGEBOX_ERROR,
        NULL,
        "Error",
        buffer,
        1,
        buttons,
        NULL
    };

    int button = -1;
    SDL_ShowMessageBox(&data, &button);

    fclose(f_stdout);
    fclose(f_stderr);

    exit(0);    // just in case...
}

//
// kexSystemLocal::GetBaseDirectory
//

const char *kexSystemLocal::GetBaseDirectory(void)
{
    static const char dummyDirectory[] = {"."};
    // cache multiple requests
    if(!basePath)
    {
        size_t len = strlen(*argv);
        char *p = (basePath = (char*)Mem_Malloc(len + 1, hb_static)) + len - 1;

        strcpy(basePath, *argv);
        while (p > basePath && *p != DIR_SEPARATOR)
        {
            *p-- = 0;
        }

        if(*p == DIR_SEPARATOR)
        {
            *p-- = 0;
        }

        if(strlen(basePath) < 2)
        {
            Mem_Free(basePath);

            basePath = (char*)Mem_Malloc(1024, hb_static);
            if(!getcwd(basePath, 1024))
            {
                strcpy(basePath, dummyDirectory);
            }
        }
    }

    return basePath;
}

//
// kexSystemLocal::Main
//

void kexSystemLocal::Main(int argc, char **argv)
{
    this->argc = argc;
    this->argv = argv;

    f_stdout = freopen("stdout.txt", "wt", stdout);
    f_stderr = freopen("stderr.txt", "wt", stderr);

    kex::cSystem->Init();
    kexObject::Init();
    kex::cCvars->Init();
    kex::cActions->Init();
    kex::cPakFiles->Init();
    
    InitVideo();

    kex::cGLContext->Init();
    kex::render::cBackend->Init();
}
