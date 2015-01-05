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
//    Draw Lists
//

#include "renderMain.h"
#include "drawLists.h"
#include "world.h"

//
// merge_wall
//

static void merge_wall(vtxDrawList_t *list, vtxDrawList_t *aux, int left, int right, int rightEnd)
{
    int i, num, temp, leftEnd = right - 1;
    temp = left;
    num = rightEnd - left + 1;

    while(left <= leftEnd && right <= rightEnd)
    {
        if(list[left].texid > list[right].texid)
        {
            aux[temp++] = list[left++];
        }
        else
        {
            aux[temp++] = list[right++];
        }
    }
    while(left <= leftEnd)
    {
        aux[temp++] = list[left++];
    }
    while(right <= rightEnd)
    {
        aux[temp++] = list[right++];
    }
    for(i = 1; i <= num; i++, rightEnd--)
    {
        list[rightEnd] = aux[rightEnd];
    }
}

//
// msort_wall
//

static void msort_wall(vtxDrawList_t *list, vtxDrawList_t *temp, int left, int right)
{
    if(left < right)
    {
        int center = (left+right) / 2;

        msort_wall(list, temp, left, center);
        msort_wall(list, temp, center+1, right);
        merge_wall(list, temp, left, center+1, right);
    }
}

//
// kexDrawList::kexDrawList
//

kexDrawList::kexDrawList(void)
{
    this->index = 0;
    this->max = 0;
    this->list = NULL;
}

//
// kexDrawList::~kexDrawList
//

kexDrawList::~kexDrawList(void)
{
}

//
// kexDrawList::Init
//

void kexDrawList::Init(const drawlisttag_t tag)
{
    index   = 0;
    max     = 128;
    list    = (vtxDrawList_t*)Mem_Calloc(sizeof(vtxDrawList_t) * max, kexWorld::hb_world);
    drawTag = tag;
}

//
// kexDrawList::AddList
//

vtxDrawList_t *kexDrawList::AddList(void)
{
    vtxDrawList_t *alloclist;

    // exceeded max capacity?
    if(index == max)
    {
        vtxDrawList_t *old = list;
        vtxDrawList_t *newlist;

        // expand stack
        max += 128;

        // allocate new array
        newlist = (vtxDrawList_t*)Mem_Calloc(sizeof(vtxDrawList_t) * max, kexWorld::hb_world);
        memcpy(newlist, old, index * sizeof(vtxDrawList_t));

        list = newlist;
        Mem_Free(old);
    }

    alloclist = &list[index];

    alloclist->flags = 0;
    alloclist->texid = 0;
    alloclist->params = 0;
    alloclist->drawTag = drawTag;

    return &list[index++];
}

//
// kexDrawList::Clear
//

void kexDrawList::Clear(void)
{
    vtxDrawList_t *head;
    
    if(max > 0)
    {
        for(int i = 0; i < index; ++i)
        {
            head = &list[i];
            head->data = NULL;
        }
    }
}

//
// kexDrawList::Draw
//

void kexDrawList::Draw(void)
{
    int i;
    int drawcount;
    vtxDrawList_t *head;
    vtxDrawList_t *tail;

    drawcount = 0;

    if(max > 0)
    {
        vtxDrawList_t *temp = (vtxDrawList_t*)malloc(index * sizeof(vtxDrawList_t));
        msort_wall(list, temp, 0, index - 1);
        free(temp);
        
        tail = &list[index];

        for(i = 0; i < index; ++i)
        {
            vtxDrawList_t *rover;

            head = &list[i];

            // break if no data found in list
            if(!head->data)
            {
                break;
            }

            if(head->procfunc)
            {
                if(!head->procfunc(head, &drawcount))
                {
                    rover = &list[i+1];
                    continue;
                }
            }

            rover = &list[i+1];

            if(rover != tail)
            {
                if(head->texid == rover->texid)
                {
                    continue;
                }
            }

            if(head->preprocess)
            {
                head->preprocess(head);
            }

            //draw elements

            if(head->postprocess)
            {
                head->postprocess(head, &drawcount);
            }

            // reset elements
            drawcount = 0;
        }
    }
}
