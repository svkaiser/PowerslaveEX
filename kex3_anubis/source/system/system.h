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

#ifndef __SYSTEMBASE_H__
#define __SYSTEMBASE_H__

class kexSystem
{
public:
    kexSystem(void);

    typedef struct
    {
        int width;
        int height;
        int refresh;
        float aspectRatio;
    } videoDisplayInfo_t;

    virtual void                            Main(int argc, char **argv) = 0;
    virtual void                            Init(void) = 0;
    virtual void                            Shutdown(void) = 0;
    virtual void                            SwapBuffers(void) = 0;
    virtual int                             GetWindowFlags(void);
    virtual const char                      *GetWindowTitle(void);
    virtual void                            SetWindowTitle(const char *string);
    virtual void                            SetWindowGrab(const bool bEnable);
    virtual void                            WarpMouseToCenter(void);
    virtual void                            *GetProcAddress(const char *proc);
    virtual int                             CheckParam(const char *check);
    virtual const char                      *GetBaseDirectory(void);
    virtual void                            Log(const char *fmt, ...);
    virtual void                            Printf(const char *string, ...);
    virtual void                            CPrintf(rcolor color, const char *string, ...);
    virtual void                            Warning(const char *string, ...);
    virtual void                            DPrintf(const char *string, ...);
    virtual void                            Error(const char *string, ...);
    virtual bool                            ReadConfigFile(const char *file);
    virtual void                            WriteConfigFile(void);
    virtual const char                      *GetClipboardText(void);
    virtual void                            GetAvailableDisplayModes(kexArray<videoDisplayInfo_t> &list);

    static kexCvar                          cvarFixedTime;
    static kexCvar                          cvarVidWidth;
    static kexCvar                          cvarVidHeight;
    static kexCvar                          cvarVidWindowed;
    static kexCvar                          cvarVidRefresh;
    static kexCvar                          cvarVidVSync;
    static kexCvar                          cvarVidDepthSize;
    static kexCvar                          cvarVidStencilSize;
    static kexCvar                          cvarVidBuffSize;
    static kexCvar                          cvarVidDisplayRestart;

    int                                     VideoWidth(void) { return videoWidth; }
    int                                     VideoHeight(void) { return videoHeight; }
    float                                   VideoRatio(void) { return videoRatio; }
    bool                                    IsWindowed(void) { return bWindowed; }
    virtual void                            *Window(void) { return NULL; }
    bool                                    IsShuttingDown(void) { return bShuttingDown; }
    const int                               Argc(void) const { return argc; }
    const char                              **Argv(void) { return (const char**)argv; }

protected:
    int                                     videoWidth;
    int                                     videoHeight;
    float                                   videoRatio;
    bool                                    bWindowed;
    bool                                    bShuttingDown;
    FILE                                    *f_stdout;
    FILE                                    *f_stderr;
    int                                     argc;
    char                                    **argv;
    char                                    *basePath;
};

#endif
