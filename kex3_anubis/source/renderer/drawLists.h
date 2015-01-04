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

#ifndef __DRAWLISTS_H__
#define __DRAWLISTS_H__

typedef enum
{
    DLT_FACE,
    NUMDRAWLISTS
} drawlisttag_t;

typedef struct vtxDrawList_s
{
    void                *data;
    bool                (*procfunc)(struct vtxDrawList_s*, int*);
    bool                (*preprocess)(struct vtxDrawList_s*);
    void                (*postprocess)(struct vtxDrawList_s*, int*);
    dtexture            texid;
    int                 flags;
    int                 params;
    float               fparams;
    drawlisttag_t       drawTag;
} vtxDrawList_t;

class kexDrawList
{
public:
    kexDrawList(void);
    ~kexDrawList(void);

    void                Init(const drawlisttag_t tag);
    void                Clear(void);
    vtxDrawList_t       *AddList(void);
    void                Draw(void);

    void                Reset(void) { index = 0; }
    const int           GetSize(void) const { return max; }

private:
    vtxDrawList_t       *list;

    int                 index;
    int                 max;
    drawlisttag_t       drawTag;
};

#endif
