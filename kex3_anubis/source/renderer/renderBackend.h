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

#ifndef __RENDERBACKEND_H__
#define __RENDERBACKEND_H__

#include "dgl.h"

typedef unsigned int dtexture;

#if defined(KEX_MACOSX)
typedef void* rhandle;
#else
typedef unsigned int rhandle;
#endif

typedef enum
{
    GLSTATE_BLEND   = 0,
    GLSTATE_CULL,
    GLSTATE_TEXTURE0,
    GLSTATE_TEXTURE1,
    GLSTATE_TEXTURE2,
    GLSTATE_TEXTURE3,
    GLSTATE_DEPTHTEST,
    GLSTATE_STENCILTEST,
    GLSTATE_SCISSOR,
    GLSTATE_ALPHATEST,
    GLSTATE_TEXGEN_S,
    GLSTATE_TEXGEN_T,
    GLSTATE_FOG,
    NUMGLSTATES
} glState_t;

typedef enum
{
    GLFUNC_LEQUAL   = 0,
    GLFUNC_GEQUAL,
    GLFUNC_EQUAL,
    GLFUNC_NOTEQUAL,
    GLFUNC_GREATER,
    GLFUNC_LESS,
    GLFUNC_ALWAYS,
    GLFUNC_NEVER,
} glFunctions_t;

typedef enum {
    GLCULL_FRONT    = 0,
    GLCULL_BACK
} glCullType_t;

typedef enum
{
    GLPOLY_FILL     = 0,
    GLPOLY_LINE
} glPolyMode_t;

typedef enum
{
    GLSRC_ZERO      = 0,
    GLSRC_ONE,
    GLSRC_DST_COLOR,
    GLSRC_ONE_MINUS_DST_COLOR,
    GLSRC_SRC_ALPHA,
    GLSRC_ONE_MINUS_SRC_ALPHA,
    GLSRC_DST_ALPHA,
    GLSRC_ONE_MINUS_DST_ALPHA,
    GLSRC_ALPHA_SATURATE,
} glSrcBlend_t;

typedef enum
{
    GLDST_ZERO      = 0,
    GLDST_ONE,
    GLDST_SRC_COLOR,
    GLDST_ONE_MINUS_SRC_COLOR,
    GLDST_SRC_ALPHA,
    GLDST_ONE_MINUS_SRC_ALPHA,
    GLDST_DST_ALPHA,
    GLDST_ONE_MINUS_DST_ALPHA,
} glDstBlend_t;

typedef enum
{
    GLCB_COLOR      = BIT(0),
    GLCB_DEPTH      = BIT(1),
    GLCB_STENCIL    = BIT(2),
    GLCB_ALL        = (GLCB_COLOR|GLCB_DEPTH|GLCB_STENCIL)
} glClearBit_t;

typedef enum
{
    GLSO_REPLACE    = 0,
    GLSO_KEEP,
    GLSO_INCR,
    GLSO_DECR
} glStencilOp_t;

typedef struct
{
    float   x;
    float   y;
    float   z;
    float   tu;
    float   tv;
    byte    r;
    byte    g;
    byte    b;
    byte    a;
} vtx_t;

#include "dgl.h"
#include "image.h"
#include "textureObject.h"
#include "textureManager.h"
#include "fbo.h"
#include "cpuVertexList.h"

class kexRenderBackend
{
public:
    kexRenderBackend(void);
    ~kexRenderBackend(void);

    void                            Init(void);
    void                            Shutdown(void);
    void                            SetDefaultState(void);
    void                            SetOrtho(void);
    void                            SetOrtho(const float x, const float y, const float w, const float h);
    void                            SwapBuffers(void);
    void                            ClearBuffer(const int bit = GLCB_ALL);
    void                            ClearBindedTexture(void);
    void                            SetState(const int bits, bool bEnable);
    void                            SetState(unsigned int flags);
    void                            SetAlphaFunc(int func, float val);
    void                            SetDepth(int func);
    void                            SetBlend(int src, int dest);
    void                            SetCull(int type);
    void                            SetPolyMode(int type);
    void                            SetDepthMask(int enable);
    void                            SetColorMask(int enable);
    void                            SetTextureUnit(int unit);
    void                            SetScissorRect(const int x, const int y, const int w, const int h);
    void                            SetClearStencil(const int value);
    void                            SetStencil(const int func, const int ref,
                                               const int opFail, const int opZFail, const int opZPass);
    void                            DisableShaders(void);
    const int                       GetDepthSizeComponent(void);
    void                            RestoreFrameBuffer(void);
    void                            ScreenShot(void);
    void                            ResetViewPort(void);
    void                            SetDrawBuffer(const GLenum state);
    void                            SetReadBuffer(const GLenum state);
    void                            LoadProjectionMatrix(kexMatrix &matrix);
    void                            LoadModelViewMatrix(kexMatrix &matrix);
    void                            PrintStats(void);

    const int                       MaxTextureUnits(void) const { return maxTextureUnits; }
    const int                       MaxTextureSize(void) const { return maxTextureSize; }
    const int                       MaxColorAttachments(void) const { return maxColorAttachments; }
    const float                     MaxAnisotropic(void) const { return maxAnisotropic; }
    const bool                      IsWideScreen(void) const { return bWideScreen; }
    const bool                      IsFullScreen(void) const { return bFullScreen; }
    const bool                      IsInitialized(void) { return bIsInit; }
    const int                       ValidFrameNum(void) const { return validFrameNum; }

    static const int                MAX_TEXTURE_UNITS   = 4;
    static kexCvar                  cvarRenderFinish;

    bool                            bPrintStats;

    typedef struct
    {
        dtexture                    currentTexture;
        int                         environment;
    } texUnit_t;

    typedef struct
    {
        int                         glStateBits;
        int                         depthFunction;
        int                         blendSrc;
        int                         blendDest;
        int                         cullType;
        int                         polyMode;
        int                         depthMask;
        int                         colormask;
        int                         alphaFunction;
        float                       alphaFuncThreshold;
        int                         stencilFunction;
        int                         stencilRef;
        int                         stencilOp[3];
        int                         currentUnit;
        rhandle                     currentProgram;
        dtexture                    currentFBO;
        texUnit_t                   textureUnits[MAX_TEXTURE_UNITS];
        int                         numStateChanges;
        int                         numTextureBinds;
        GLenum                      drawBuffer;
        GLenum                      readBuffer;
    } glState_t;

    glState_t                       glState;

private:
    int                             GetOGLVersion(const char* version);

    int                             maxTextureUnits;
    int                             maxTextureSize;
    int                             maxColorAttachments;
    float                           maxAnisotropic;
    bool                            bWideScreen;
    bool                            bFullScreen;
    bool                            bIsInit;

    const char                      *gl_vendor;
    const char                      *gl_renderer;
    const char                      *gl_version;

    int                             validFrameNum;
};

#endif
