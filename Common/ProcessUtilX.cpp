// CreateProcessAsUser.cpp : Defines the entry point for the console application.
//

#include "ProcessUtil.h"
#include <cmath>
#ifndef _WIN32 // osx or linux
#ifdef __APPLE__
#include <mach-o/dyld.h>
#include <libproc.h>
#include <mach/mach_time.h>
#include <sys/time.h>
#include <mach/clock.h>
#include <mach/mach.h>
static int mac_clock_gettime(struct timespec &tmSpec)
{
    mach_timespec_t mts = {0};
    clock_serv_t cclock = 0;
    host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
    int retVal = clock_get_time(cclock, &mts);
    mach_port_deallocate(mach_task_self(), cclock);
    tmSpec.tv_sec = mts.tv_sec;
    tmSpec.tv_nsec = mts.tv_nsec;
    return retVal;
}
#else // linux
#include <dirent.h>
#include <sys/wait.h>
#include <fstream>
#endif // apple
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <libgen.h>
#include <sys/sysctl.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <thread>
#include <chrono>
#else
#endif

#ifdef _WIN32
// On windows do not use crt libraries for mutex, thread, etc
// Use only windows native APIs
#define localtime_xplat(t,o) localtime_s(o,t)
#else
#include <semaphore.h>
#define localtime_xplat(t,o) localtime_r(t,o)
#endif // _WIN32


long long ProcessUtil::GetUTCTime(Time_SecondPrecision secondPrecision /* = Time_NanoSeconds */)
{
    struct timespec tmSpec;
    tmSpec.tv_sec = 0;
    tmSpec.tv_nsec = 0;
#ifdef _WIN32
    timespec_get(&tmSpec, TIME_UTC);
#elif defined(__APPLE__)
    mac_clock_gettime(tmSpec);
#else
    clock_gettime(CLOCK_REALTIME, &tmSpec);
#endif
    long long outTime = tmSpec.tv_sec;
    if (secondPrecision > 0) {
        long long nanoSec(tmSpec.tv_nsec);
        outTime = TimeToOtherTimePrecision(outTime, Time_Seconds, secondPrecision) + TimeToOtherTimePrecision(nanoSec, Time_NanoSeconds, secondPrecision);
    }
    return outTime;
}

std::string ProcessUtil::GetLocalTimeString(Time_SecondPrecision secondPrecision /* = Time_NanoSeconds */, long long inTime /* = 0 */)
{
    if (inTime <= 0)
        inTime = GetUTCTime(secondPrecision);
    struct tm tmLocal;
    time_t timep(TimeToOtherTimePrecision(inTime, secondPrecision, Time_Seconds));
    localtime_xplat(&timep, &tmLocal);
    char szTimeInSec[25] = { 0 };
    strftime(szTimeInSec, 24, "%Y-%m-%dT%H:%M:%S", &tmLocal);
    char szPrecisionNsTime[32] = { 0 };
    if (secondPrecision > 0) {
        long long subSec = inTime % TimeToOtherTimePrecision(1, Time_Seconds, secondPrecision);
        snprintf(szPrecisionNsTime, 31, ".%lld", subSec);
    }

    char szTimeZone[11] = { 0 };
    strftime(szTimeZone, 10, "%z", &tmLocal);

    std::string strTime;
    strTime += szTimeInSec;
    strTime += szPrecisionNsTime;
    strTime += szTimeZone;
    return strTime;
}

long long ProcessUtil::TimeToOtherTimePrecision(long long time, Time_SecondPrecision from /* = Time_NanoSeconds */, Time_SecondPrecision to /* = Time_Seconds */)
{
    int diff(from - to);
    if (diff == 0)
        return time;
    long long converseionFactor((long long)std::pow(1000, std::abs(diff)));
    if (diff > 0)
        return time / converseionFactor;
    return time * converseionFactor;
}
