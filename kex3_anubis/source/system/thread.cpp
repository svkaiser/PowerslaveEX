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
//      Thread Class
//

#include "kexlib.h"

//
// kexThread::CreateThread
//

kexThread::kThread_t kexThread::CreateThread(const char *name, void *data, threadFunction_t function)
{
    return NULL;
}

//
// kexThread::GetThreadName
//

const char *kexThread::GetThreadName(kThread_t thread)
{
    return NULL;
}

//
// kexThread::SetThreadPriority
//

int kexThread::SetThreadPriority(kThread_t thread, const threadPriority_t priority)
{
    return -1;
}

//
// kexThread::WaitThread
//

void kexThread::WaitThread(kThread_t thread, int *status)
{
}

//
// kexThread::AllocMutex
//

kexThread::kMutex_t kexThread::AllocMutex(void)
{
    return NULL;
}

//
// kexThread::LockMutex
//

int kexThread::LockMutex(kMutex_t mutex, const bool bTimeOut)
{
    return -1;
}

//
// kexThread::UnlockMutex
//

int kexThread::UnlockMutex(kMutex_t mutex)
{
    return -1;
}

//
// kexThread::DestroyMutex
//

void kexThread::DestroyMutex(kMutex_t mutex)
{
}

//
// kexThread::AllocCondition
//

kexThread::kCond_t kexThread::AllocCondition(void)
{
    return NULL;
}

//
// kexThread::ConditionDestroy
//

void kexThread::ConditionDestroy(kCond_t cond)
{
}

//
// kexThread::ConditionBroadcast
//

int kexThread::ConditionBroadcast(kCond_t cond)
{
    return -1;
}

//
// kexThread::ConditionWait
//

int kexThread::ConditionWait(kCond_t cond, kMutex_t mutex, uint32_t timeoutMS)
{
    return -1;
}
