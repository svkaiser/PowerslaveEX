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
//      Base System API
//

#include "kexlib.h"

kexCvar kexSystem::cvarFixedTime("fixedtime", CVF_INT|CVF_CONFIG, "0", "TODO");
kexCvar kexSystem::cvarVidWidth("v_width", CVF_INT|CVF_CONFIG, "640", "TODO");
kexCvar kexSystem::cvarVidHeight("v_height", CVF_INT|CVF_CONFIG, "480", "TODO");
kexCvar kexSystem::cvarVidWindowed("v_windowed", CVF_BOOL|CVF_CONFIG, "1", "TODO");
kexCvar kexSystem::cvarVidRefresh("v_refresh", CVF_INT|CVF_CONFIG, "60", "Video refresh rate (fullscreen only)");
kexCvar kexSystem::cvarVidVSync("v_vsync", CVF_BOOL|CVF_CONFIG, "1", "TODO");
kexCvar kexSystem::cvarVidDepthSize("v_depthsize", CVF_INT|CVF_CONFIG, "24", "TODO");
kexCvar kexSystem::cvarVidStencilSize("v_stencilsize", CVF_INT|CVF_CONFIG, "8", "TODO");
kexCvar kexSystem::cvarVidBuffSize("v_buffersize", CVF_INT|CVF_CONFIG, "32", "TODO");
kexCvar kexSystem::cvarVidDisplayRestart("v_displayrestart", CVF_INT, "-1", "TODO");

//
// quit
//

COMMAND(quit)
{
    kex::cSystem->Shutdown();
}

//
// kexSystem::kexSystem
//

kexSystem::kexSystem(void)
{
    this->bShuttingDown = false;
}

//
// kexSystem::GetWindowFlags
//

int kexSystem::GetWindowFlags(void)
{
    return 0;
}

//
// kexSystem::Log
//

void kexSystem::Log(const char *fmt, ...)
{
}

//
// kexSystem::GetWindowTitle
//

const char *kexSystem::GetWindowTitle(void)
{
    return NULL;
}

//
// kexSystem::SetWindowTitle
//

void kexSystem::SetWindowTitle(const char *string)
{
}

//
// kexSystem::SetWindowGrab
//

void kexSystem::SetWindowGrab(const bool bEnable)
{
}

//
// kexSystem::WarpMouseToCenter
//

void kexSystem::WarpMouseToCenter(void)
{
}

//
// kexSystem::GetProcAddress
//

void *kexSystem::GetProcAddress(const char *proc)
{
    return NULL;
}

//
// kexSystem::CheckParam
//

int kexSystem::CheckParam(const char *check)
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
// kexSystem::ReadConfigFile
//

bool kexSystem::ReadConfigFile(const char *file)
{
    char *buffer;
    int len;
    
    len = kex::cPakFiles->OpenExternalFile(file, (byte**)(&buffer));
    
    if(len == -1)
    {
        Warning("Warning: %s not found\n", file);
        return false;
    }
    
    kex::cCommands->Execute(buffer);
    Mem_Free(buffer);

    return true;
}

//
// kexSystem::WriteConfigFile
//

void kexSystem::WriteConfigFile(void)
{
    kexStr str(kexStr::Format("%s\\config.cfg", kex::cvarBasePath.GetValue()));
    str.NormalizeSlashes();
    
    FILE *f = fopen(str.c_str(), "w");
    
    if(f)
    {
        kex::cActions->WriteBindings(f);
        kex::cCvars->WriteToFile(f);
        fclose(f);
    }
}

//
// kexSystem::GetAvailableDisplayModes
//

void kexSystem::GetAvailableDisplayModes(kexArray<kexSystem::videoDisplayInfo_t> &list)
{
}

//
// kexSystem::GetClipboardText
//

const char *kexSystem::GetClipboardText(void)
{
    return NULL;
}

//
// kexSystem::Printf
//

void kexSystem::Printf(const char *string, ...)
{
}

//
// kexSystem::CPrintf
//

void kexSystem::CPrintf(rcolor color, const char *string, ...)
{
}

//
// kexSystem::Warning
//

void kexSystem::Warning(const char *string, ...)
{
}

//
// kexSystem::DPrintf
//

void kexSystem::DPrintf(const char *string, ...)
{
}

//
// kexSystem::Error
//

void kexSystem::Error(const char* string, ...)
{
}

//
// kexSystem::GetBaseDirectory
//

const char *kexSystem::GetBaseDirectory(void)
{
    return NULL;
}
