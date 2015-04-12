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

#ifndef __KEXMSORT_H__
#define __KEXMSORT_H__

#include <assert.h>

class kexMSort
{
public:
    template<class type>
    static void Sort(type *t, const unsigned int count, int (*function)(void*, void*))
    {
        assert(count > 0);

        type *temp = new type[count];
        Process(t, temp, 0, count - 1, function);
        delete[] temp;
    }

private:
    template<class type>
    static void Merge(type *t, type *aux, int left, int right, int rightEnd, int (*function)(void*, void*))
    {
        int i, num, temp, leftEnd = right - 1;
        temp = left;
        num = rightEnd - left + 1;

        while(left <= leftEnd && right <= rightEnd)
        {
            if(function(&t[left], &t[right]) > 0)
            {
                aux[temp++] = t[left++];
            }
            else
            {
                aux[temp++] = t[right++];
            }
        }
        while(left <= leftEnd)
        {
            aux[temp++] = t[left++];
        }
        while(right <= rightEnd)
        {
            aux[temp++] = t[right++];
        }
        for(i = 1; i <= num; i++, rightEnd--)
        {
            t[rightEnd] = aux[rightEnd];
        }
    }

    template<class type>
    static void Process(type *t, type *temp, int left, int right, int (*function)(void*, void*))
    {
        if(left < right)
        {
            int center = (left+right) / 2;
            
            Process(t, temp, left, center, function);
            Process(t, temp, center+1, right, function);
            Merge(t, temp, left, center+1, right, function);
        }
    }
};

#endif
