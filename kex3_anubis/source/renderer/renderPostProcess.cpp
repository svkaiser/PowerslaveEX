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
//      Post Process Manager
//

#include "renderMain.h"
#include "renderPostProcess.h"

static kexRenderPostProcess postProcessLocal;
kexRenderPostProcess *kexRender::cPostProcess = &postProcessLocal;

kexCvar kexRenderPostProcess::cvarRenderFXAA("r_fxaa", CVF_BOOL|CVF_CONFIG, "0", "Enables fxaa");
kexCvar kexRenderPostProcess::cvarRenderBloom("r_bloom", CVF_BOOL|CVF_CONFIG, "0", "Enables bloom");
kexCvar kexRenderPostProcess::cvarBloomThreshold("r_bloomthreshold", CVF_FLOAT|CVF_CONFIG, "0.7",
                                                 0.625f, 1, "Sets bloom intensity");

//
// kexRenderPostProcess::kexRenderPostProcess
//

kexRenderPostProcess::kexRenderPostProcess(void)
{
}

//
// kexRenderPostProcess::~kexRenderPostProcess
//

kexRenderPostProcess::~kexRenderPostProcess(void)
{
}

//
// kexRenderPostProcess::Init
//

void kexRenderPostProcess::Init(void)
{
    fxaaShader.Load("progs/fxaa.vert", "progs/fxaa.frag");
    blurShader.Load("progs/blur.vert", "progs/blur.frag");
    bloomShader.Load("progs/bloom.vert", "progs/bloom.frag");
    
    ReloadFBOs();
}

//
// kexRenderPostProcess::Shutdown
//

void kexRenderPostProcess::Shutdown(void)
{
    fxaaFBO.Delete();
    bloomFBO.Delete();
    blurFBO[0].Delete();
    blurFBO[1].Delete();

    fxaaShader.Delete();
    blurShader.Delete();
    bloomShader.Delete();
}

//
// kexRenderPostProcess::ReloadFBOs
//

void kexRenderPostProcess::ReloadFBOs(void)
{
    int width = kexMath::RoundPowerOfTwo(kex::cSystem->VideoWidth());
    int height = kexMath::RoundPowerOfTwo(kex::cSystem->VideoHeight());
    
    fxaaFBO.InitColorAttachment(0);
    bloomFBO.InitColorAttachment(0);
    blurFBO[0].InitColorAttachment(0, width >> 1, height >> 1);
    blurFBO[1].InitColorAttachment(0, width >> 3, height >> 3);
}

//
// kexRenderPostProcess::RenderFXAA
//

void kexRenderPostProcess::RenderFXAA(void)
{
    int w = kex::cSystem->VideoWidth();
    int h = kex::cSystem->VideoHeight();
    
    if(!has_GL_ARB_shader_objects       ||
       !has_GL_ARB_framebuffer_object   ||
       !cvarRenderFXAA.GetBool())
    {
        return;
    }
    
    if(!fxaaShader.IsLoaded())
    {
        return;
    }
    
    kexRender::cBackend->SetOrtho();
    kexRender::cBackend->SetBlend(GLSRC_SRC_ALPHA, GLDST_ONE_MINUS_SRC_ALPHA);
    kexRender::cBackend->SetState(GLSTATE_SCISSOR, false);

    fxaaFBO.CopyBackBuffer();
    fxaaFBO.BindImage();
    
    fxaaShader.Bind();
    fxaaShader.SetUniform("uDiffuse", 0);
    fxaaShader.SetUniform("uViewWidth", (float)w);
    fxaaShader.SetUniform("uViewHeight", (float)h);
    fxaaShader.SetUniform("uMaxSpan", 8.0f);
    fxaaShader.SetUniform("uReduceMax", 8.0f);
    fxaaShader.SetUniform("uReduceMin", 128.0f);

    kexRender::cVertList->BindDrawPointers();
    kexRender::cVertList->AddQuad(0, (float)(h - fxaaFBO.Height()), 0,
                                  (float)fxaaFBO.Width(),
                                  (float)fxaaFBO.Height(),
                                  0, 1,
                                  1, 0,
                                  255, 255, 255, 255);
    
    kexRender::cVertList->DrawElements();
    kexRender::cBackend->DisableShaders();
    kexRender::cBackend->SetState(GLSTATE_SCISSOR, true);
    
    fxaaFBO.UnBindImage();
}

//
// kexRenderPostProcess::RenderBloom
//

void kexRenderPostProcess::RenderBloom(void)
{
    int w = kex::cSystem->VideoWidth();
    int h = kex::cSystem->VideoHeight();
    kexTexture fBuffer;
    
    if(!has_GL_ARB_shader_objects       ||
       !has_GL_ARB_framebuffer_object   ||
       !cvarRenderBloom.GetBool())
    {
        return;
    }
    
    if(!bloomShader.IsLoaded() || !blurShader.IsLoaded())
    {
        return;
    }
    
    kexRender::cBackend->SetBlend(GLSRC_SRC_ALPHA, GLDST_ONE_MINUS_SRC_ALPHA);
    kexRender::cBackend->SetState(GLSTATE_BLEND, true);
    kexRender::cBackend->SetState(GLSTATE_ALPHATEST, false);
    kexRender::cBackend->SetState(GLSTATE_DEPTHTEST, false);
    kexRender::cBackend->SetState(GLSTATE_SCISSOR, false);

    kexRender::cBackend->SetOrtho();
    
    fBuffer.BindFrameBuffer();
    bloomFBO.Bind();
    bloomShader.Bind();
    bloomShader.SetUniform("uDiffuse", 0);
    bloomShader.SetUniform("uBloomThreshold", cvarBloomThreshold.GetFloat());
    
    kexRender::cVertList->BindDrawPointers();
    kexRender::cVertList->AddQuad(0, 0, 0, (float)w, (float)h, 0, 1, 1, 0, 255, 255, 255, 255);
    kexRender::cVertList->DrawElements(false);
    
    bloomFBO.UnBind();
    
    blurShader.Bind();
    blurShader.SetUniform("uDiffuse", 0);
    blurShader.SetUniform("uBlurRadius", 1.0f);
    
    for(int i = 0; i < 2; ++i)
    {
        // horizonal
        blurFBO[i].CopyFrameBuffer(bloomFBO);
        bloomFBO.Bind();
        blurFBO[i].BindImage();
        blurShader.SetUniform("uSize", (float)blurFBO[i].Width());
        blurShader.SetUniform("uDirection", 1);
        kexRender::cVertList->DrawElements(false);
        bloomFBO.UnBind();
        
        // vertical
        blurFBO[i].CopyFrameBuffer(bloomFBO);
        bloomFBO.Bind();
        blurFBO[i].BindImage();
        blurShader.SetUniform("uSize", (float)blurFBO[i].Height());
        blurShader.SetUniform("uDirection", 0);
        kexRender::cVertList->DrawElements(false);
        bloomFBO.UnBind();
    }
    
    kexRender::cBackend->DisableShaders();
    
    bloomFBO.BindImage();
    kexRender::cBackend->SetBlend(GLSRC_ONE_MINUS_DST_COLOR, GLDST_ONE);
    
    kexRender::cVertList->BindDrawPointers();
    kexRender::cVertList->AddQuad(0, (float)(h - bloomFBO.Height()), 0,
                                  (float)bloomFBO.Width(),
                                  (float)bloomFBO.Height(),
                                  0, 1,
                                  1, 0,
                                  255, 255, 255, 255);
    
    kexRender::cVertList->DrawElements();
    bloomFBO.UnBindImage();

    kexRender::cBackend->SetState(GLSTATE_SCISSOR, true);
    fBuffer.Delete();
}
