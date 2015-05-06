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

#ifndef __THREAD_H__
#define __THREAD_H__

class kexThread
{
public:
    
    typedef int (KDECL *threadFunction_t)(void *data);
    typedef void* kThread_t;
    typedef void* kMutex_t;
    typedef void* kCond_t;
    
    typedef enum
    {
        TP_LOW  = 0,
        TP_MED,
        TP_HIGH
    } threadPriority_t;
    
    virtual kThread_t   CreateThread(const char *name, void *data, threadFunction_t function);
    virtual const char  *GetThreadName(kThread_t thread);
    virtual int         SetThreadPriority(kThread_t thread, const threadPriority_t priority);
    virtual void        WaitThread(kThread_t thread, int *status);
    
    virtual kMutex_t    AllocMutex(void);
    virtual int         LockMutex(kMutex_t mutex, const bool bTimeOut = false);
    virtual int         UnlockMutex(kMutex_t mutex);
    virtual void        DestroyMutex(kMutex_t mutex);

    virtual kCond_t     AllocCondition(void);
    virtual void        ConditionDestroy(kCond_t cond);
    virtual int         ConditionBroadcast(kCond_t cond);
    virtual int         ConditionWait(kCond_t cond, kMutex_t mutex, uint32_t timeoutMS = 0);
};

#endif
