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
//      Wall Clipper
//

#include "kexlib.h"
#include "renderMain.h"
#include "renderView.h"
#include "clipper.h"

const float kexClipper::maxClipSpan = (kexMath::pi * 2);

//
// kexClipper::kexClipper
//

kexClipper::kexClipper(void)
{
    this->freeClipList = NULL;
    this->clipList = NULL;
    this->view = NULL;
}

//
// kexClipper::Clear
//

void kexClipper::Clear(void)
{
    kexClipper::clipNode_t *node = clipList;
    kexClipper::clipNode_t *temp;

    while(node != NULL)
    {
        temp = node;
        node = node->next;
        Free(temp);
    }

    clipList = NULL;
}

//
// kexClipper::GetNew
//

kexClipper::clipNode_t *kexClipper::GetNew(void)
{
    if(freeClipList)
    {
        kexClipper::clipNode_t *p = freeClipList;
        freeClipList = p->next;
        return p;
    }

    return new clipNode_t;
}

//
// kexClipper::AddNewRange
//

kexClipper::clipNode_t *kexClipper::AddNewRange(const float left, const float right)
{
    kexClipper::clipNode_t *clip = GetNew();

    clip->left = left;
    clip->right = right;
    clip->next = clip->prev = NULL;

    return clip;
}

//
// kexClipper::AddRangeSpan
//

void kexClipper::AddRangeSpan(const float left, const float right)
{
    float an1 = left;
    float an2 = right;

    if(an1 < 0) an1 += maxClipSpan;
    if(an2 < 0) an2 += maxClipSpan;

    if(an1 > an2)
    {
        AddToClipList(an1, maxClipSpan);
        AddToClipList(0, an2);
        return;
    }

    AddToClipList(an1, an2);
}

//
// kexClipper::AddToClipList
//

void kexClipper::AddToClipList(const float left, const float right)
{
    kexClipper::clipNode_t *node, *temp, *prevNode;

    if(clipList)
    {
        node = clipList;

        while(node != NULL && node->left < right)
        {
            if(node->left >= left && node->right <= right)
            {
                temp = node;
                node = node->next;
                RemoveRange(temp);
            }
            else
            {
                if(node->left <= left && node->right >= right)
                {
                    return;
                }
                else
                {
                    node = node->next;
                }
            }
        }

        node = clipList;

        while(node != NULL)
        {
            if(node->left >= left && node->left <= right)
            {
                node->left = left;
                return;
            }
            if(node->right >= left && node->right <= right)
            {
                // check for possible merger
                if(node->next && node->next->left <= right)
                {
                    node->right = node->next->right;
                    RemoveRange(node->next);
                }
                else
                {
                    node->right = right;
                }

                return;
            }

            node = node->next;
        }

        //just add range
        node = clipList;
        prevNode = NULL;

        temp = AddNewRange(left, right);

        while(node != NULL && node->left < right)
        {
            prevNode = node;
            node = node->next;
        }

        temp->next = node;

        if(node == NULL)
        {
            temp->prev = prevNode;

            if(prevNode)
            {
                prevNode->next = temp;
            }

            if(!clipList)
            {
                clipList = temp;
            }
        }
        else
        {
            if(node == clipList)
            {
                clipList->prev = temp;
                clipList = temp;
            }
            else
            {
                temp->prev = prevNode;
                prevNode->next = temp;
                node->prev = temp;
            }
        }
    }
    else
    {
        clipList = AddNewRange(left, right);
    }
}

//
// kexClipper::CheckRange
//

bool kexClipper::CheckRange(const float left, const float right)
{
    float an1 = left;
    float an2 = right;

    if(an1 < 0) an1 += maxClipSpan;
    if(an2 < 0) an2 += maxClipSpan;

    if(an1 > an2)
    {
        return (RangeVisible(an1, maxClipSpan) || RangeVisible(0, an2));
    }

    return RangeVisible(an1, an2);
}

//
// kexClipper::RangeVisible
//

bool kexClipper::RangeVisible(const float left, const float right)
{
    kexClipper::clipNode_t *clip;
    clip = clipList;

    if(right == 0 && clip && clip->left == 0)
    {
        return false;
    }

    while(clip != NULL && clip->left < right)
    {
        if(left >= clip->left && right <= clip->right)
        {
            return false;
        }

        clip = clip->next;
    }

    return true;
}

//
// kexClipper::Free
//

void kexClipper::Free(kexClipper::clipNode_t *clipNode)
{
    clipNode->next = freeClipList;
    freeClipList = clipNode;
}

//
// kexClipper::RemoveRange
//

void kexClipper::RemoveRange(kexClipper::clipNode_t *clipNode)
{
    if(clipNode == clipList)
    {
        clipList = clipList->next;
    }
    else
    {
        if(clipNode->prev)
        {
            clipNode->prev->next = clipNode->next;
        }

        if(clipNode->next)
        {
            clipNode->next->prev = clipNode->prev;
        }
    }

    Free(clipNode);
}
