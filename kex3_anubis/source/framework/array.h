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

#ifndef __KEXARRAY_H__
#define __KEXARRAY_H__

#include <assert.h>

template<class type>
class kexArray
{
public:
    kexArray(void);
    ~kexArray(void);

    typedef int         compare_t(const type*, const type*);

    void                Push(type o);
    void                Pop(void);
    type                *Grow(void);
    type                GetFirst(void);
    type                GetLast(void);
    void                Empty(void);
    void                Init(void);
    void                Resize(unsigned int size);
    type                IndexOf(unsigned int index) const;
    bool                Contains(const type check) const;
    void                Splice(const unsigned int start, unsigned int len);
    void                Sort(compare_t *function);
    void                Sort(compare_t *function, unsigned int count);

    const unsigned int  Length(void) const { return length; }
    type                GetData(const int index) { return data[index]; }

    type                &operator[](unsigned int index);
    const type          &operator[](unsigned int index) const;
    kexArray<type>      &operator=(const kexArray<type> &arr);

protected:
    type                *data;
    unsigned int        length;
    unsigned int        aidx;
};

//
// kexArray::kexArray
//
template<class type>
kexArray<type>::kexArray(void)
{
    Init();
}

//
// kexArray::~kexArray
//
template<class type>
kexArray<type>::~kexArray(void)
{
    Empty();
}

//
// kexArray::Init
//
template<class type>
void kexArray<type>::Init(void)
{
    data = NULL;
    length = 0;
    aidx = 0;
}

//
// kexArray::Resize
//
template<class type>
void kexArray<type>::Resize(unsigned int size)
{
    type *tmp;
    unsigned int cnt;

    if(size == length)
    {
        return;
    }

    if(size <= 0 && length != 0)
    {
        delete[] data;
        data = NULL;
        length = 0;
        return;
    }

    if(length == 0)
    {
        data = new type[size];
        length = size;
        return;
    }

    tmp = data;
    data = new type[size];

    if(length <= size)
    {
        cnt = length;
    }
    else
    {
        cnt = size;
    }

    for(unsigned int i = 0; i < cnt; i++)
    {
        data[i] = tmp[i];
    }

    length = size;
    delete[] tmp;
}

//
// kexArray::Push
//
template<class type>
void kexArray<type>::Push(type o)
{
    Resize(length+1);
    data[aidx++] = o;
}

//
// kexArray::Pop
//
template<class type>
void kexArray<type>::Pop(void)
{
    if(length == 0)
    {
        return;
    }

    Resize(length-1);
    aidx--;
}

//
// kexArray::Grow
//
template<class type>
type *kexArray<type>::Grow(void)
{
    Resize(length+1);
    return &data[length-1];
}

//
// kexArray::GetFirst
//
template<class type>
type kexArray<type>::GetFirst(void)
{
    assert(length > 0);
    return data[0];
}

//
// kexArray::GetLast
//
template<class type>
type kexArray<type>::GetLast(void)
{
    assert(length > 0);
    return data[length-1];
}

//
// kexArray::Empty
//
template<class type>
void kexArray<type>::Empty(void)
{
    if(data && length > 0)
    {
        delete[] data;
        data = NULL;
        length = 0;
        aidx = 0;
    }
}

//
// kexArray::IndexOf
//
template<class type>
type kexArray<type>::IndexOf(unsigned int index) const
{
    if(index >= length)
    {
        index = length-1;
    }

    return data[index];
}

//
// kexArray::Contains
//
template<class type>
bool kexArray<type>::Contains(const type check) const
{
    for(unsigned int i = 0; i < length; ++i)
    {
        if(data[i] == check)
        {
            return true;
        }
    }

    return false;
}

//
// kexArray::Splice
//
template<class type>
void kexArray<type>::Splice(const unsigned int start, unsigned int len)
{
    if(length == 0 || len == 0)
    {
        return;
    }

    if(len >= length)
    {
        len = length;
    }

    type *tmp = new type[len];

    for(unsigned int i = 0; i < len; i++)
    {
        tmp[i] = data[start+i];
    }

    delete[] data;
    data = tmp;
    length = length - len;
    aidx = length-1;
}

//
// kexArray::Sort
//
// Note that data will be shuffled around, so this could invalidate any
// pointers that relies on the array/data
//
template<class type>
void kexArray<type>::Sort(compare_t *function)
{
    if(data == NULL)
    {
        return;
    }

    typedef int compareCast(const void*, const void*);
    compareCast *cmpFunc = (compareCast*)function;

    qsort((void*)data, length, sizeof(type), cmpFunc);
}

//
// kexArray::Sort
//
// Note that data will be shuffled around, so this could invalidate any
// pointers that relies on the array/data
//
template<class type>
void kexArray<type>::Sort(compare_t *function, unsigned int count)
{
    if(data == NULL)
    {
        return;
    }

    typedef int compareCast(const void*, const void*);
    compareCast *cmpFunc = (compareCast*)function;

    qsort((void*)data, count, sizeof(type), cmpFunc);
}

//
// kexArray::operator[]
//
template <class type>
d_inline type &kexArray<type>::operator[](unsigned int index)
{
    assert(index < length);
    return data[index];
}

//
// kexArray::operator[]
//
template <class type>
d_inline const type &kexArray<type>::operator[](unsigned int index) const
{
    assert(index < length);
    return data[index];
}

//
// kexArray::operator=
//
template <class type>
kexArray<type> &kexArray<type>::operator=(const kexArray<type> &arr)
{
    if(data)
    {
        delete[] data;
    }

    data = NULL;
    length = arr.length;
    aidx = arr.aidx;

    if(arr.length > 0)
    {
        data = new type[arr.length];

        for(unsigned int i = 0; i < arr.length; i++)
        {
            data[i] = arr.data[i];
        }
    }

    return *this;
}

#endif
