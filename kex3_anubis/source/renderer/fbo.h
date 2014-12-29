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

#ifndef __FBO_H__
#define __FBO_H__

class kexImage;

class kexFBO
{
public:
    kexFBO(void);
    ~kexFBO(void);

    void                    CheckStatus(void);
    void                    InitColorAttachment(const int attachment, const int width, const int height);
    void                    InitColorAttachment(const int attachment);
    void                    InitDepthAttachment(const int width, const int height);
    void                    CopyBackBuffer(void);
    void                    CopyFrameBuffer(const kexFBO &fbo);
    kexImage                ToImage(void);
    void                    Delete(void);
    void                    BindImage(void);
    void                    Bind(void);
    void                    UnBind(void);
    void                    UnBindImage(void);

    const int               Width(void) const { return fboWidth; }
    const int               Height(void) const { return fboHeight; }
    const unsigned int      Attachment(void) const { return fboAttachment; }
    const dtexture          FBOID(void) const { return fboId; }
    const dtexture          RBOID(void) const { return rboId; }
    const dtexture          TexID(void) const { return fboTexId; }

private:
    dtexture                fboId;
    dtexture                rboId;
    dtexture                fboTexId;
    bool                    bLoaded;
    unsigned int            fboAttachment;
    int                     fboWidth;
    int                     fboHeight;
};

#endif
