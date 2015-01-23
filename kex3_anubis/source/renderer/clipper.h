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

#ifndef __CLIPPER_H__
#define __CLIPPER_H__

class kexRenderView;

class kexClipper
{
public:
    kexClipper(void);

    void                    Clear(void);
    void                    AddRangeSpan(const float left, const float right);
    bool                    CheckRange(const float left, const float right);

    void                    SetRenderView(kexRenderView *v) { view = v; }

    typedef struct clipNode_s
    {
        float               left;
        float               right;
        struct clipNode_s   *prev;
        struct clipNode_s   *next;
    } clipNode_t;

private:
    clipNode_t              *GetNew(void);
    clipNode_t              *AddNewRange(const float left, const float right);
    void                    AddToClipList(const float left, const float right);
    void                    Free(clipNode_t *clipNode);
    void                    RemoveRange(clipNode_t *clipNode);
    bool                    RangeVisible(const float left, const float right);

    static const float      maxClipSpan;

    clipNode_t              *freeClipList;
    clipNode_t              *clipList;
    kexRenderView           *view;
};

#endif
