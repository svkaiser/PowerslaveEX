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

#ifndef __RENDER_POSTPROCESS_H__
#define __RENDER_POSTPROCESS_H__

class kexRenderPostProcess
{
public:
    kexRenderPostProcess(void);
    ~kexRenderPostProcess(void);
    
    void                        Init(void);
    void                        Shutdown(void);
    
    void                        RenderFXAA(void);
    void                        RenderBloom(void);
    void                        ReloadFBOs(void);

    static kexCvar              cvarRenderFXAA;
    static kexCvar              cvarRenderBloom;
    static kexCvar              cvarBloomThreshold;

private:

    kexShaderObj                fxaaShader;
    kexShaderObj                blurShader;
    kexShaderObj                bloomShader;
    kexFBO                      fxaaFBO;
    kexFBO                      blurFBO[2];
    kexFBO                      bloomFBO;
};

#endif
