// CreateProcessAsUser.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "ProcessUtil.h"
#include "Common.h"
#include <unistd.h>
#include <pthread.h>

int ProcessUtil::GetCurrentProcessId()
{
    return (int)::getpid();
}

int ProcessUtil::GetCurrentThreadId()
{
    uint64_t tid;
    pthread_threadid_np(NULL, &tid);
    return (int)tid;
}

void ProcessUtil::Sleep(unsigned milliSeconds)
{
    usleep(milliSeconds * 1000);
}
