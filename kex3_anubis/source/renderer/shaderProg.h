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

#ifndef __SHADERPROG_H__
#define __SHADERPROG_H__

typedef enum
{
    RST_VERTEX          = 0,
    RST_FRAGMENT,
    RST_TOTAL
} rShaderType_t;

class kexShaderObj
{
public:
    kexShaderObj(void);
    ~kexShaderObj(void);

    void                        Init(void);
    void                        Load(const char *vertexShader, const char *fragmentShader);
    void                        Compile(const char *name, rShaderType_t type);
    bool                        Link(void);
    void                        Bind(void);
    void                        Delete(void);
    void                        SetUniform(const char *name, const int val);
    void                        SetUniform(const char *name, const int *val, const int size);
    void                        SetUniform(const char *name, const float val);
    void                        SetUniform(const char *name, kexVec2 &val);
    void                        SetUniform(const char *name, kexVec3 &val);
    void                        SetUniform(const char *name, kexVec3 *val, const int size);
    void                        SetUniform(const char *name, kexVec4 &val);
    void                        SetUniform(const char *name, kexMatrix &val, bool bTranspose = false);

    rhandle                     &Program(void) { return programObj; }
    rhandle                     &VertexProgram(void) { return vertexProgram; }
    rhandle                     &FragmentProgram(void) { return fragmentProgram; }
    const bool                  HasErrors(void) const { return bHasErrors; }
    const bool                  IsLoaded(void) const { return bLoaded; }

private:
    void                        DumpErrorLog(const rShaderType_t type, const rhandle handle);

    rhandle                     programObj;
    rhandle                     vertexProgram;
    rhandle                     fragmentProgram;
    bool                        bHasErrors;
    bool                        bLoaded;
    kexStr                      fragFile;
    kexStr                      vertFile;
};

#endif
