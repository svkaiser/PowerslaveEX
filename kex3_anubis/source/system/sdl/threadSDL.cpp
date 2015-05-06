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
//      Thread Class (SDL)
//

#include "SDL.h"
#include "kexlib.h"

class kexThreadSDL : public kexThread
{
public:
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

static kexThreadSDL thread;
kexThread *kex::cThread = &thread;

//
// kexThreadSDL::CreateThread
//

kexThread::kThread_t kexThreadSDL::CreateThread(const char *name, void *data, threadFunction_t function)
{
    return SDL_CreateThread(function, name, data);
}

//
// kexThreadSDL::GetThreadName
//

const char *kexThreadSDL::GetThreadName(kThread_t thread)
{
    return SDL_GetThreadName((SDL_Thread*)thread);
}

//
// kexThreadSDL::SetThreadPriority
//

int kexThreadSDL::SetThreadPriority(kThread_t thread, const threadPriority_t priority)
{
    SDL_ThreadPriority threadPriority;
    
    switch(priority)
    {
    case kexThread::TP_LOW:
        threadPriority = SDL_THREAD_PRIORITY_LOW;
        break;
        
    case kexThread::TP_MED:
        threadPriority = SDL_THREAD_PRIORITY_NORMAL;
        break;
        
    case kexThread::TP_HIGH:
        threadPriority = SDL_THREAD_PRIORITY_HIGH;
        break;
        
    default:
        return -1;
            
    }
    
    return SDL_SetThreadPriority(threadPriority);
}

//
// kexThreadSDL::WaitThread
//

void kexThreadSDL::WaitThread(kThread_t thread, int *status)
{
    SDL_WaitThread((SDL_Thread*)thread, status);
}

//
// kexThreadSDL::AllocMutex
//

kexThread::kMutex_t kexThreadSDL::AllocMutex(void)
{
    return SDL_CreateMutex();
}

//
// kexThreadSDL::LockMutex
//

int kexThreadSDL::LockMutex(kMutex_t mutex, const bool bTimeOut)
{
    if(bTimeOut)
    {
        return SDL_TryLockMutex((SDL_mutex*)mutex);
    }
    
    return SDL_LockMutex((SDL_mutex*)mutex);
}

//
// kexThreadSDL::UnlockMutex
//

int kexThreadSDL::UnlockMutex(kMutex_t mutex)
{
    return SDL_UnlockMutex((SDL_mutex*)mutex);
}

//
// kexThreadSDL::DestroyMutex
//

void kexThreadSDL::DestroyMutex(kMutex_t mutex)
{
    SDL_DestroyMutex((SDL_mutex*)mutex);
}

//
// kexThreadSDL::AllocCondition
//

kexThread::kCond_t kexThreadSDL::AllocCondition(void)
{
    return SDL_CreateCond();
}

//
// kexThreadSDL::ConditionDestroy
//

void kexThreadSDL::ConditionDestroy(kCond_t cond)
{
    SDL_DestroyCond((SDL_cond*)cond);
}

//
// kexThreadSDL::ConditionBroadcast
//

int kexThreadSDL::ConditionBroadcast(kCond_t cond)
{
    return SDL_CondBroadcast((SDL_cond*)cond);
}

//
// kexThreadSDL::ConditionWait
//

int kexThreadSDL::ConditionWait(kCond_t cond, kMutex_t mutex, uint32_t timeoutMS)
{
    if(timeoutMS == 0)
    {
        return SDL_CondWait((SDL_cond*)cond, (SDL_mutex*)mutex);
    }

    return SDL_CondWaitTimeout((SDL_cond*)cond, (SDL_mutex*)mutex, timeoutMS);
}
