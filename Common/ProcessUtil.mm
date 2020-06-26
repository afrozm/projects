// CreateProcessAsUser.cpp : Defines the entry point for the console application.
//

#include "ProcessUtil.h"
#include "Common.h"
#include <unistd.h>
#include <pthread.h>
#include <mach/mach.h>
#include <mach/mach_time.h>

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
unsigned long long ProcessUtil::GetTickCount()
{
    unsigned long long tickCount(mach_absolute_time());
    mach_timebase_info_data_t timeBaseInfo = {0};
    mach_timebase_info(&timeBaseInfo);
    tickCount = (tickCount * timeBaseInfo.numer) / timeBaseInfo.denom;
    tickCount /= 1000000;
    return tickCount;
}
