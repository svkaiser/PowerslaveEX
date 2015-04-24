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
    fxaaShader.Delete();
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
    float pw;
    float ph;
    
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
    
    kexRender::cBackend->SetBlend(GLSRC_SRC_ALPHA, GLDST_ONE_MINUS_SRC_ALPHA);
    fxaaFBO.CopyBackBuffer();
    fxaaFBO.BindImage();
    
    fxaaShader.Bind();
    fxaaShader.SetUniform("uDiffuse", 0);
    fxaaShader.SetUniform("uViewWidth", (float)w);
    fxaaShader.SetUniform("uViewHeight", (float)h);
    fxaaShader.SetUniform("uMaxSpan", 8.0f);
    fxaaShader.SetUniform("uReduceMax", 8.0f);
    fxaaShader.SetUniform("uReduceMin", 128.0f);
    
    kexRender::cBackend->SetOrtho();
    
    pw = (float)w / (float)fxaaFBO.Width();
    ph = (float)h / (float)fxaaFBO.Height();
    
    kexRender::cVertList->BindDrawPointers();
    kexRender::cVertList->AddQuad(0, 0, 0,
                                  (float)fxaaFBO.Width(),
                                  (float)fxaaFBO.Height() * ph,
                                  0, ph,
                                  1, 0,
                                  255, 255, 255, 255);
    
    kexRender::cVertList->DrawElements();
    kexRender::cBackend->DisableShaders();
    
    fxaaFBO.UnBindImage();
}

//
// kexRenderPostProcess::RenderBloom
//

void kexRenderPostProcess::RenderBloom(void)
{
    //int w = kex::cSystem->VideoWidth();
    //int h = kex::cSystem->VideoHeight();
    //float pw;
    //float ph;
    
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
    
#if 0
    kexRender::cBackend->SetBlend(GLSRC_SRC_ALPHA, GLDST_ONE_MINUS_SRC_ALPHA);
    kexRender::cBackend->SetState(GLSTATE_BLEND, true);
    kexRender::cBackend->SetState(GLSTATE_ALPHATEST, false);
    kexRender::cBackend->SetState(GLSTATE_DEPTHTEST, false);
    
    bloomFBO.CopyBackBuffer();
    bloomFBO.BindImage();
    
    bloomShader.Bind();
    bloomShader.SetUniform("uDiffuse", 0);
    bloomShader.SetUniform("uBloomThreshold", 0.75f);
    
    kexRender::cBackend->SetOrtho();
    
    pw = (float)w / (float)bloomFBO.Width();
    ph = (float)h / (float)bloomFBO.Height();
    
    kexRender::cVertList->BindDrawPointers();
    kexRender::cVertList->AddQuad(0, 0, 0,
                                  (float)bloomFBO.Width(),
                                  (float)bloomFBO.Height() * ph,
                                  0, ph,
                                  1, 0,
                                  255, 255, 255, 255);
    
    kexRender::cVertList->DrawElements();
    
    bloomFBO.UnBindImage();
    
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
        
        pw = (float)w / (float)blurFBO[i].Width();
        ph = (float)h / (float)blurFBO[i].Height();
        
        kexRender::cVertList->BindDrawPointers();
        kexRender::cVertList->AddQuad(0, 0, 0,
                                      (float)blurFBO[i].Width(),
                                      (float)blurFBO[i].Height() * ph,
                                      0, ph,
                                      1, 0,
                                      255, 255, 255, 255);
        
        kexRender::cVertList->DrawElements();
        bloomFBO.UnBind();
        
        // vertical
        blurFBO[i].CopyFrameBuffer(bloomFBO);
        bloomFBO.Bind();
        blurFBO[i].BindImage();
        
        blurShader.SetUniform("uSize", (float)blurFBO[i].Height());
        blurShader.SetUniform("uDirection", 0);
        
        pw = (float)w / (float)blurFBO[i].Width();
        ph = (float)h / (float)blurFBO[i].Height();
        
        kexRender::cVertList->BindDrawPointers();
        kexRender::cVertList->AddQuad(0, 0, 0,
                                      (float)blurFBO[i].Width(),
                                      (float)blurFBO[i].Height() * ph,
                                      0, ph,
                                      1, 0,
                                      255, 255, 255, 255);
        
        kexRender::cVertList->DrawElements();
        bloomFBO.UnBind();
    }
    
    kexRender::cBackend->DisableShaders();
    
    bloomFBO.BindImage();
    kexRender::cBackend->SetBlend(GLSRC_ONE_MINUS_DST_COLOR, GLDST_ONE);
    pw = (float)w / (float)bloomFBO.Width();
    ph = (float)h / (float)bloomFBO.Height();
    
    kexRender::cVertList->BindDrawPointers();
    kexRender::cVertList->AddQuad(0, 0, 0,
                                  (float)bloomFBO.Width(),
                                  (float)bloomFBO.Height() * ph,
                                  0, ph,
                                  1, 0,
                                  255, 255, 255, 255);
    
    kexRender::cVertList->DrawElements();
    
    bloomFBO.UnBindImage();
#endif
}
