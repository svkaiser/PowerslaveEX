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

#ifndef __RENDERMAIN_H__
#define __RENDERMAIN_H__

#include "renderBackend.h"
#include "renderScreen.h"
#include "renderUtils.h"
#include "shaderProg.h"
#include "renderPostProcess.h"

class kexRender
{
public:
    static kexRenderBackend     *cBackend;
    static kexTextureManager    *cTextures;
    static kexCpuVertList       *cVertList;
    static kexRenderScreen      *cScreen;
    static kexRenderUtils       *cUtils;
    static kexRenderPostProcess *cPostProcess;
};

#include "vertexBuffer.h"

#endif
