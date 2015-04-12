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

#ifndef __KEXQUEUE_H__
#define __KEXQUEUE_H__

#include <assert.h>

template<class type>
class kexQueue
{
public:
    kexQueue(void);
    ~kexQueue(void);
    
    void                Init(const unsigned int size);
    void                Push(type *t);
    type                *Pop(void);
    
    const unsigned int  Head(void) { return head; }
    const unsigned int  Tail(void) { return tail; }
    const unsigned int  Length(void) { return length; }
    
    type                &operator[](unsigned int index);
    const type          &operator[](unsigned int index) const;

protected:
    unsigned int        length;
    unsigned int        head;
    unsigned int        tail;
    type                *data;
    type                defaultDataSize[64];
};

//
// kexQueue::kexQueue
//
template<class type>
kexQueue<type>::kexQueue(void)
{
    this->length = 64;
    this->head = 0;
    this->tail = 0;
    this->data = this->defaultDataSize;
}

//
// kexQueue::~kexQueue
//
template<class type>
kexQueue<type>::~kexQueue(void)
{
    if(data != defaultDataSize)
    {
        delete[] data;
        data = NULL;
    }
}

//
// kexQueue::Init
//
template<class type>
void kexQueue<type>::Init(const unsigned int size)
{
    assert(size > 0);

    this->length = size;
    this->head = 0;
    this->tail = 0;
    
    if(size > 64)
    {
        this->data = new type[this->length];
    }
    else
    {
        this->data = this->defaultDataSize;
    }
}

//
// kexQueue::Push
//
template<class type>
void kexQueue<type>::Push(type *t)
{
    data[head] = *t;
    head = (head + 1) % length;
}

//
// kexQueue::Pop
//
template<class type>
type *kexQueue<type>::Pop(void)
{
    type *ret;
    
    if(tail == head)
    {
        return NULL;
    }
    
    ret = &data[tail];
    tail = (tail + 1) % length;
    
    return ret;
}

//
// kexQueue::operator[]
//
template <class type>
type &kexQueue<type>::operator[](unsigned int index)
{
    assert(index < length);
    return data[index];
}

//
// kexQueue::operator[]
//
template <class type>
const type &kexQueue<type>::operator[](unsigned int index) const
{
    assert(index < length);
    return data[index];
}

#endif
