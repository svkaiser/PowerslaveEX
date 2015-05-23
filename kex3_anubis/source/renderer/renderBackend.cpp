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
//      Renderer backend
//

#include "kexlib.h"
#include "renderMain.h"

kexCvar kexRenderBackend::cvarRenderFinish("r_finish", CVF_BOOL|CVF_CONFIG, "1", "Force a GL command sync");

static kexRenderBackend renderBackend;
kexRenderBackend *kexRender::cBackend = &renderBackend;

//
// statglbackend
//

COMMAND(statglbackend)
{
    if(kex::cCommands->GetArgc() < 1)
    {
        return;
    }

    renderBackend.bPrintStats ^= 1;
}

//
// screenshot
//

COMMAND(screenshot)
{
    if(kex::cCommands->GetArgc() < 1)
    {
        return;
    }

    renderBackend.ScreenShot();
}

//
// kexRenderBackend::kexRenderBackend
//

kexRenderBackend::kexRenderBackend(void)
{
    this->maxTextureUnits           = 1;
    this->maxTextureSize            = 64;
    this->maxAnisotropic            = 0;
    this->maxColorAttachments       = 0;
    this->bWideScreen               = false;
    this->bFullScreen               = false;
    this->bIsInit                   = false;
    this->glState.glStateBits       = 0;
    this->glState.alphaFunction     = -1;
    this->glState.blendDest         = -1;
    this->glState.blendSrc          = -1;
    this->glState.cullType          = -1;
    this->glState.depthMask         = -1;
    this->glState.colormask         = -1;
    this->glState.currentUnit       = -1;
    this->glState.currentProgram    = 0;
    this->glState.currentFBO        = 0;
    this->validFrameNum             = 0;
    this->bPrintStats               = false;
}

//
// kexRenderBackend::~kexRenderBackend
//

kexRenderBackend::~kexRenderBackend(void)
{
}

//
// kexRenderBackend::GetOGLVersion
//

typedef enum
{
    OPENGL_VERSION_1_0,
    OPENGL_VERSION_1_1,
    OPENGL_VERSION_1_2,
    OPENGL_VERSION_1_3,
    OPENGL_VERSION_1_4,
    OPENGL_VERSION_1_5,
    OPENGL_VERSION_2_0,
    OPENGL_VERSION_2_1,
} glversion_t;

int kexRenderBackend::GetOGLVersion(const char* version)
{
    int MajorVersion;
    int MinorVersion;
    int versionvar;

    versionvar = OPENGL_VERSION_1_0;

    if(sscanf(version, "%d.%d", &MajorVersion, &MinorVersion) == 2)
    {
        if(MajorVersion > 1)
        {
            versionvar = OPENGL_VERSION_2_0;

            if(MinorVersion > 0)
            {
                versionvar = OPENGL_VERSION_2_1;
            }
        }
        else
        {
            versionvar = OPENGL_VERSION_1_0;

            if(MinorVersion > 0) { versionvar = OPENGL_VERSION_1_1; }
            if(MinorVersion > 1) { versionvar = OPENGL_VERSION_1_2; }
            if(MinorVersion > 2) { versionvar = OPENGL_VERSION_1_3; }
            if(MinorVersion > 3) { versionvar = OPENGL_VERSION_1_4; }
            if(MinorVersion > 4) { versionvar = OPENGL_VERSION_1_5; }
        }
    }

    return versionvar;
}

//
// kexRenderBackend::SetDefaultState
//

void kexRenderBackend::SetDefaultState(void)
{
    glState.glStateBits     = 0;
    glState.alphaFunction   = -1;
    glState.stencilFunction = -1;
    glState.stencilRef      = -1;
    glState.stencilOp[0]    = -1;
    glState.stencilOp[1]    = -1;
    glState.stencilOp[2]    = -1;
    glState.blendDest       = -1;
    glState.blendSrc        = -1;
    glState.cullType        = -1;
    glState.depthMask       = -1;
    glState.colormask       = -1;
    glState.currentUnit     = -1;
    glState.currentProgram  = 0;
    glState.currentFBO      = 0;
    glState.drawBuffer      = GL_NONE;
    glState.readBuffer      = GL_NONE;

    validFrameNum           = 0;

    dglClearDepth(1.0f);
    dglClearStencil(0);
    dglClearColor(0, 0, 0, 1);

    ResetViewPort();

    ClearBuffer(GLCB_ALL);
    SetState(GLSTATE_TEXTURE0, true);
    SetState(GLSTATE_CULL, true);
    SetState(GLSTATE_FOG, false);
    SetState(GLSTATE_SCISSOR, false);
    SetCull(GLCULL_BACK);
    SetDepth(GLFUNC_LEQUAL);
    SetAlphaFunc(GLFUNC_GEQUAL, 0.01f);
    SetBlend(GLSRC_SRC_ALPHA, GLDST_ONE_MINUS_SRC_ALPHA);
    SetDepthMask(1);
    SetColorMask(1);

    dglDisable(GL_NORMALIZE);
    dglShadeModel(GL_SMOOTH);
    dglHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    dglFogi(GL_FOG_MODE, GL_LINEAR);
    dglHint(GL_FOG_HINT, GL_NICEST);
    dglEnable(GL_DITHER);
    dglTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
    dglTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);

    dglEnableClientState(GL_VERTEX_ARRAY);
    dglEnableClientState(GL_TEXTURE_COORD_ARRAY);
    dglEnableClientState(GL_COLOR_ARRAY);
}

//
// kexRenderBackend::Init
//

void kexRenderBackend::Init(void)
{
    gl_vendor = (const char*)dglGetString(GL_VENDOR);
    gl_renderer = (const char*)dglGetString(GL_RENDERER);
    gl_version = (const char*)dglGetString(GL_VERSION);

    dglGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
    dglGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &maxTextureUnits);
    dglGetIntegerv(GL_MAX_COLOR_ATTACHMENTS_EXT, &maxColorAttachments);

    kex::cSystem->Printf("GL_VENDOR: %s\n", gl_vendor);
    kex::cSystem->Printf("GL_RENDERER: %s\n", gl_renderer);
    kex::cSystem->Printf("GL_VERSION: %s\n", gl_version);
    kex::cSystem->Printf("GL_MAX_TEXTURE_SIZE: %i\n", maxTextureSize);
    kex::cSystem->Printf("GL_MAX_TEXTURE_UNITS_ARB: %i\n", maxTextureUnits);
    kex::cSystem->Printf("GL_MAX_COLOR_ATTACHMENTS_EXT: %i\n", maxColorAttachments);

    SetDefaultState();
    
    if(has_GL_EXT_texture_filter_anisotropic)
    {
        dglGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropic);
    }

    bIsInit = true;
    
    kexRender::cTextures->Init();
    kexRender::cPostProcess->Init();

    kex::cSystem->Printf("Backend Renderer Initialized\n");
}

//
// kexRenderBackend::Shutdown
//

void kexRenderBackend::Shutdown(void)
{
    kex::cSystem->Printf("Shutting down render system\n");
    
    kexRender::cPostProcess->Shutdown();
    kexRender::cTextures->Shutdown();
}

//
// void kexRenderBackend::ResetViewPort
//

void kexRenderBackend::ResetViewPort(void)
{
    dglViewport(0, 0, kex::cSystem->VideoWidth(), kex::cSystem->VideoHeight());
}

//
// kexRenderBackend::SetOrtho
//

void kexRenderBackend::SetOrtho(const float x, const float y, const float w, const float h)
{
    kexMatrix mtx;

    dglMatrixMode(GL_PROJECTION);
    dglLoadIdentity();

    dglMatrixMode(GL_MODELVIEW);
    dglLoadIdentity();

    mtx.SetOrtho(x, w, h, y, -1, 1);
    dglLoadMatrixf(mtx.ToFloatPtr());
}

//
// kexRenderBackend::SetOrtho
//

void kexRenderBackend::SetOrtho(void)
{
    SetOrtho(0, 0, (float)kex::cSystem->VideoWidth(), (float)kex::cSystem->VideoHeight());
}

//
// kexRenderBackend::LoadProjectionMatrix
//

void kexRenderBackend::LoadProjectionMatrix(kexMatrix &matrix)
{
    dglMatrixMode(GL_PROJECTION);
    dglLoadMatrixf(matrix.ToFloatPtr());
}

//
// kexRenderBackend::LoadModelViewMatrix
//

void kexRenderBackend::LoadModelViewMatrix(kexMatrix &matrix)
{
    dglMatrixMode(GL_MODELVIEW);
    dglLoadMatrixf(matrix.ToFloatPtr());
}

//
// kexRenderBackend::SwapBuffers
//

void kexRenderBackend::SwapBuffers(void)
{
    PrintStats();
    kexRender::cUtils->ClearDebugLine();
    
    if(cvarRenderFinish.GetBool())
    {
        dglFinish();
    }

    kex::cSystem->SwapBuffers();
    validFrameNum++;

    glState.numStateChanges = 0;
    glState.numTextureBinds = 0;
}

//
// kexRenderBackend::ClearBuffer
//

void kexRenderBackend::ClearBuffer(const int bit)
{
    int clearBit = 0;

    if(bit == GLCB_ALL)
    {
        clearBit = (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }
    else
    {
        if(bit & GLCB_COLOR)
        {
            clearBit |= GL_COLOR_BUFFER_BIT;
        }
        if(bit & GLCB_DEPTH)
        {
            clearBit |= GL_DEPTH_BUFFER_BIT;
        }
        if(bit & GLCB_STENCIL)
        {
            clearBit |= GL_STENCIL_BUFFER_BIT;
        }
    }

    dglClear(clearBit);
}

//
// kexRenderBackend::ClearBindedTexture
//

void kexRenderBackend::ClearBindedTexture(void)
{
    int unit = glState.currentUnit;
    glState.textureUnits[unit].currentTexture = 0;

    dglBindTexture(GL_TEXTURE_2D, 0);
}

//
// kexRenderBackend::SetScissorRect
//

void kexRenderBackend::SetScissorRect(const int x, const int y, const int w, const int h)
{
    int vh;
    int sx, sy, sw, sh;
    
    if(!(glState.glStateBits & (1 << GLSTATE_SCISSOR)))
    {
        return;
    }
    
    vh = kex::cSystem->VideoHeight();

    sx = x;
    sy = vh-h;
    sw = w-x;
    sh = vh-(y+(vh-h));

    if(sx < 0) sx = 0;
    if(sy < 0) sy = 0;
    if(sw < 0) sw = 0;
    if(sh < 0) sh = 0;

    dglScissor(sx, sy, sw, sh);
}

//
// kexRenderBackend::SetClearStencil
//

void kexRenderBackend::SetClearStencil(const int value)
{
    dglClearStencil(value);
}

//
// kexRenderBackend::SetStencil
//

void kexRenderBackend::SetStencil(const int func, const int ref,
                                  const int opFail, const int opZFail, const int opZPass)
{
    int pFunc = glState.stencilFunction ^ func;

    if(pFunc != 0 || glState.stencilRef != ref)
    {
        int glFunc = GL_ALWAYS;

        switch(func)
        {
        case GLFUNC_EQUAL:
            glFunc = GL_EQUAL;
            break;

        case GLFUNC_ALWAYS:
            glFunc = GL_ALWAYS;
            break;

        case GLFUNC_LEQUAL:
            glFunc = GL_LEQUAL;
            break;

        case GLFUNC_GEQUAL:
            glFunc = GL_GEQUAL;
            break;

        case GLFUNC_NOTEQUAL:
            glFunc = GL_NOTEQUAL;
            break;

        case GLFUNC_GREATER:
            glFunc = GL_GREATER;
            break;

        case GLFUNC_LESS:
            glFunc = GL_LESS;
            break;

        case GLFUNC_NEVER:
            glFunc = GL_NEVER;
            break;
        }

        dglStencilFunc(glFunc, ref, 0xff);

        glState.stencilFunction = func;
        glState.stencilRef = ref;
        glState.numStateChanges++;
    }

    if( glState.stencilOp[0] != opFail ||
        glState.stencilOp[1] != opZFail ||
        glState.stencilOp[2] != opZPass)
    {
        int glOp1 = GL_REPLACE, glOp2 = GL_REPLACE, glOp3 = GL_REPLACE;

        switch(opFail)
        {
        case GLSO_REPLACE:
            glOp1 = GL_REPLACE;
            break;

        case GLSO_KEEP:
            glOp1 = GL_KEEP;
            break;

        case GLSO_INCR:
            glOp1 = GL_INCR;
            break;

        case GLSO_DECR:
            glOp1 = GL_DECR;
            break;
        }

        switch(opZFail)
        {
        case GLSO_REPLACE:
            glOp2 = GL_REPLACE;
            break;

        case GLSO_KEEP:
            glOp2 = GL_KEEP;
            break;

        case GLSO_INCR:
            glOp2 = GL_INCR;
            break;

        case GLSO_DECR:
            glOp2 = GL_DECR;
            break;
        }

        switch(opZPass)
        {
        case GLSO_REPLACE:
            glOp3 = GL_REPLACE;
            break;

        case GLSO_KEEP:
            glOp3 = GL_KEEP;
            break;

        case GLSO_INCR:
            glOp3 = GL_INCR;
            break;

        case GLSO_DECR:
            glOp3 = GL_DECR;
            break;
        }

        glState.stencilOp[0] = opFail;
        glState.stencilOp[1] = opZFail;
        glState.stencilOp[2] = opZPass;

        dglStencilOp(glOp1, glOp2, glOp3);
        glState.numStateChanges++;
    }
}

//
// kexRenderBackend::SetState
//

void kexRenderBackend::SetState(const int bits, bool bEnable)
{
    int stateFlag = 0;

    switch(bits)
    {
    case GLSTATE_BLEND:
        stateFlag = GL_BLEND;
        break;

    case GLSTATE_CULL:
        stateFlag = GL_CULL_FACE;
        break;

    case GLSTATE_TEXTURE0:
        SetTextureUnit(0);
        stateFlag = GL_TEXTURE_2D;
        break;

    case GLSTATE_TEXTURE1:
        SetTextureUnit(1);
        stateFlag = GL_TEXTURE_2D;
        break;

    case GLSTATE_TEXTURE2:
        SetTextureUnit(2);
        stateFlag = GL_TEXTURE_2D;
        break;

    case GLSTATE_TEXTURE3:
        SetTextureUnit(3);
        stateFlag = GL_TEXTURE_2D;
        break;

    case GLSTATE_ALPHATEST:
        stateFlag = GL_ALPHA_TEST;
        break;

    case GLSTATE_TEXGEN_S:
        stateFlag = GL_TEXTURE_GEN_S;
        break;

    case GLSTATE_TEXGEN_T:
        stateFlag = GL_TEXTURE_GEN_T;
        break;

    case GLSTATE_DEPTHTEST:
        stateFlag = GL_DEPTH_TEST;
        break;

    case GLSTATE_FOG:
        stateFlag = GL_FOG;
        break;

    case GLSTATE_STENCILTEST:
        stateFlag = GL_STENCIL_TEST;
        break;

    case GLSTATE_SCISSOR:
        stateFlag = GL_SCISSOR_TEST;
        break;

    default:
        kex::cSystem->Warning("kexRenderBackend::SetState: unknown bit flag: %i\n", bits);
        return;
    }

    if(bEnable && !(glState.glStateBits & (1 << bits)))
    {
        dglEnable(stateFlag);
        glState.glStateBits |= (1 << bits);
        glState.numStateChanges++;
    }
    else if(!bEnable && (glState.glStateBits & (1 << bits)))
    {
        dglDisable(stateFlag);
        glState.glStateBits &= ~(1 << bits);
        glState.numStateChanges++;
    }
}

//
// kexRenderBackend::SetState
//

void kexRenderBackend::SetState(unsigned int flags)
{
    for(int i = 0; i < NUMGLSTATES; i++)
    {
        if(!(flags & BIT(i)))
        {
            SetState(i, false);
            continue;
        }

        SetState(i, true);
    }
}

//
// kexRenderBackend::SetFunc
//

void kexRenderBackend::SetAlphaFunc(int func, float val)
{
    int pFunc = (glState.alphaFunction ^ func) |
                (glState.alphaFuncThreshold != val);

    if(pFunc == 0)
    {
        return;
    }

    int glFunc = 0;

    switch(func)
    {
    case GLFUNC_EQUAL:
        glFunc = GL_EQUAL;
        break;

    case GLFUNC_ALWAYS:
        glFunc = GL_ALWAYS;
        break;

    case GLFUNC_LEQUAL:
        glFunc = GL_LEQUAL;
        break;

    case GLFUNC_GEQUAL:
        glFunc = GL_GEQUAL;
        break;

    case GLFUNC_NEVER:
        glFunc = GL_NEVER;
        break;
    }

    dglAlphaFunc(glFunc, val);

    glState.alphaFunction = func;
    glState.alphaFuncThreshold = val;
    glState.numStateChanges++;
}

//
// kexRenderBackend::SetDepth
//

void kexRenderBackend::SetDepth(int func)
{
    int pFunc = glState.depthFunction ^ func;

    if(pFunc == 0)
    {
        return;
    }

    int glFunc = 0;

    switch(func)
    {
    case GLFUNC_EQUAL:
        glFunc = GL_EQUAL;
        break;

    case GLFUNC_ALWAYS:
        glFunc = GL_ALWAYS;
        break;

    case GLFUNC_LEQUAL:
        glFunc = GL_LEQUAL;
        break;

    case GLFUNC_GEQUAL:
        glFunc = GL_GEQUAL;
        break;

    case GLFUNC_NOTEQUAL:
        glFunc = GL_NOTEQUAL;
        break;

    case GLFUNC_GREATER:
        glFunc = GL_GREATER;
        break;

    case GLFUNC_LESS:
        glFunc = GL_LESS;
        break;

    case GLFUNC_NEVER:
        glFunc = GL_NEVER;
        break;
    }

    dglDepthFunc(glFunc);
    glState.depthFunction = func;
    glState.numStateChanges++;
}

//
// kexRenderBackend::SetBlend
//

void kexRenderBackend::SetBlend(int src, int dest)
{
    int pBlend = (glState.blendSrc ^ src) | (glState.blendDest ^ dest);

    if(pBlend == 0)
    {
        return;
    }

    int glSrc = GL_ONE;
    int glDst = GL_ONE;

    switch(src)
    {
    case GLSRC_ZERO:
        glSrc = GL_ZERO;
        break;

    case GLSRC_ONE:
        glSrc = GL_ONE;
        break;

    case GLSRC_DST_COLOR:
        glSrc = GL_DST_COLOR;
        break;

    case GLSRC_ONE_MINUS_DST_COLOR:
        glSrc = GL_ONE_MINUS_DST_COLOR;
        break;

    case GLSRC_SRC_ALPHA:
        glSrc = GL_SRC_ALPHA;
        break;

    case GLSRC_ONE_MINUS_SRC_ALPHA:
        glSrc = GL_ONE_MINUS_SRC_ALPHA;
        break;

    case GLSRC_DST_ALPHA:
        glSrc = GL_DST_ALPHA;
        break;

    case GLSRC_ONE_MINUS_DST_ALPHA:
        glSrc = GL_ONE_MINUS_DST_ALPHA;
        break;

    case GLSRC_ALPHA_SATURATE:
        glSrc = GL_SRC_ALPHA_SATURATE;
        break;
    }

    switch(dest)
    {
    case GLDST_ZERO:
        glDst = GL_ZERO;
        break;

    case GLDST_ONE:
        glDst = GL_ONE;
        break;

    case GLDST_SRC_COLOR:
        glDst = GL_SRC_COLOR;
        break;

    case GLDST_ONE_MINUS_SRC_COLOR:
        glDst = GL_ONE_MINUS_SRC_COLOR;
        break;

    case GLDST_SRC_ALPHA:
        glDst = GL_SRC_ALPHA;
        break;

    case GLDST_ONE_MINUS_SRC_ALPHA:
        glDst = GL_ONE_MINUS_SRC_ALPHA;
        break;

    case GLDST_DST_ALPHA:
        glDst = GL_DST_ALPHA;
        break;

    case GLDST_ONE_MINUS_DST_ALPHA:
        glDst = GL_ONE_MINUS_DST_ALPHA;
        break;
    }

    dglBlendFunc(glSrc, glDst);

    glState.blendSrc = src;
    glState.blendDest = dest;
    glState.numStateChanges++;
}

//
// kexRenderBackend::SetCull
//

void kexRenderBackend::SetCull(int type)
{
    int pCullType = glState.cullType ^ type;

    if(pCullType == 0)
    {
        return;
    }

    int cullType = 0;

    switch(type)
    {
    case GLCULL_FRONT:
        cullType = GL_FRONT;
        break;
    case GLCULL_BACK:
        cullType = GL_BACK;
        break;
    default:
        return;
    }

    dglCullFace(cullType);

    glState.cullType = type;
    glState.numStateChanges++;
}

//
// kexRenderBackend::SetPolyMode
//

void kexRenderBackend::SetPolyMode(int type)
{
    int pPolyMode = glState.polyMode ^ type;

    if(pPolyMode == 0)
    {
        return;
    }

    int polyMode = 0;

    switch(type)
    {
    case GLPOLY_FILL:
        polyMode = GL_FILL;
        break;

    case GLPOLY_LINE:
        polyMode = GL_LINE;
        break;

    default:
        return;
    }

    dglPolygonMode(GL_FRONT_AND_BACK, polyMode);

    glState.polyMode = type;
    glState.numStateChanges++;
}

//
// kexRenderBackend::SetDepthMask
//

void kexRenderBackend::SetDepthMask(int enable)
{
    int pEnable = glState.depthMask ^ enable;

    if(pEnable == 0)
    {
        return;
    }

    int flag = 0;

    switch(enable)
    {
    case 1:
        flag = GL_TRUE;
        break;

    case 0:
        flag = GL_FALSE;
        break;
    default:
        return;
    }

    dglDepthMask(flag);

    glState.depthMask = enable;
    glState.numStateChanges++;
}

//
// kexRenderBackend::SetColorMask
//

void kexRenderBackend::SetColorMask(int enable)
{
    int pEnable = glState.colormask ^ enable;

    if(pEnable == 0)
    {
        return;
    }

    int flag = 0;

    switch(enable)
    {
    case 1:
        flag = GL_TRUE;
        break;

    case 0:
        flag = GL_FALSE;
        break;
    default:
        return;
    }

    dglColorMask(flag, flag, flag, flag);

    glState.colormask = enable;
    glState.numStateChanges++;
}

//
// kexRenderBackend::SetTextureUnit
//

void kexRenderBackend::SetTextureUnit(int unit)
{
    if(unit > MAX_TEXTURE_UNITS || unit < 0)
    {
        return;
    }

    if(unit == glState.currentUnit)
    {
        return;
    }

    dglActiveTextureARB(GL_TEXTURE0_ARB + unit);
    dglClientActiveTextureARB(GL_TEXTURE0_ARB + unit);
    glState.currentUnit = unit;
}

//
// kexRenderBackend::DisableShaders
//

void kexRenderBackend::DisableShaders(void)
{
    if(glState.currentProgram != 0)
    {
        dglUseProgramObjectARB(0);
        glState.currentProgram = 0;
    }
}

//
// kexRenderBackend::GetDepthSizeComponent
//

const int kexRenderBackend::GetDepthSizeComponent(void)
{
    int depthSize = kexSystem::cvarVidDepthSize.GetInt();

    switch(depthSize)
    {
    case 16:
        return GL_DEPTH_COMPONENT16_ARB;
    case 24:
        return GL_DEPTH_COMPONENT24_ARB;
    case 32:
        return GL_DEPTH_COMPONENT32_ARB;
    default:
        kex::cSystem->Warning("GetDepthSizeComponent: unknown depth size (%i)", depthSize);
        break;
    }

    return GL_DEPTH_COMPONENT;
}

//
// kexRenderBackend::RestoreFrameBuffer
//

void kexRenderBackend::RestoreFrameBuffer(void)
{
    dglBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
    dglDrawBuffer(GL_BACK);
    dglReadBuffer(GL_BACK);

    glState.currentFBO = 0;
}

//
// kexRenderBackend::SetDrawBuffer
//

void kexRenderBackend::SetDrawBuffer(const GLenum state)
{
    if(glState.drawBuffer == state)
    {
        return; // already set
    }
    
    dglDrawBuffer(state);
    glState.drawBuffer = state;
}

//
// kexRenderBackend::SetReadBuffer
//

void kexRenderBackend::SetReadBuffer(const GLenum state)
{
    if(glState.readBuffer == state)
    {
        return; // already set
    }
    
    dglReadBuffer(state);
    glState.readBuffer = state;
}

//
// kexRenderBackend::ScreenShot
//

void kexRenderBackend::ScreenShot(void)
{
    int shotnum = 0;
    kexBinFile file;
    kexStr filePath;
    kexImage image;

    while(shotnum < 1000)
    {
        filePath = kexStr(kexStr::Format("%s\\shot%03d.png", kex::cvarBasePath.GetValue(), shotnum));
        filePath.NormalizeSlashes();

        if(!kexBinFile::Exists(filePath.c_str()))
        {
            file.Create(filePath.c_str());
            break;
        }

        shotnum++;
    }

    if(!file.IsOpened())
    {
        return;
    }

    image.LoadFromScreenBuffer();
    image.WritePNG(file);

    file.Close();

    kex::cSystem->Printf("Saved Screenshot %s\n", filePath.c_str());
}

//
// kexRenderBackend::PrintStats
//

void kexRenderBackend::PrintStats(void)
{
    if(!bPrintStats)
    {
        return;
    }
    
    kexRenderUtils::PrintStatsText("state changes", ": %i", glState.numStateChanges);
    kexRenderUtils::PrintStatsText("texture binds", ": %i", glState.numTextureBinds);
    kexRenderUtils::AddDebugLineSpacing();
}
