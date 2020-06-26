// CreateProcessAsUser.cpp : Defines the entry point for the console application.
//

#pragma once
#ifdef _WIN32
#include <windows.h>
#else
#endif // _WIN32
#include "Common.h"

namespace ProcessUtil {

    bool IsUserAdmin();

    // RunApplication flag
#define RAF_BLOCKING 1
#define RAF_ADMIN 2
#define RAF_NOWINDOW 4

#ifdef _WIN32
    bool RunApplication(LPCTSTR commandLine,
        unsigned uRAF = RAF_BLOCKING, unsigned long *outExitCode = NULL);
    bool RunApplication(int argc, LPCTSTR *argv,
        unsigned uRAF = RAF_BLOCKING, unsigned long *outExitCode = NULL);
#endif

    int GetCurrentProcessId();
    int GetCurrentThreadId();

    void Sleep(unsigned milliSeconds);
    unsigned long long GetTickCount();

    DWORD GetProcessExePath(DWORD pid, LPTSTR outPath, DWORD nSize);

    enum Time_SecondPrecision {
        Time_Seconds,
        Time_MilliSeconds,
        Time_MicroSeconds,
        Time_NanoSeconds
    };
    long long GetUTCTime(Time_SecondPrecision secondPrecision = Time_NanoSeconds);
    std::string GetLocalTimeString(Time_SecondPrecision secondPrecision = Time_NanoSeconds, long long inTime = 0);
    long long TimeToOtherTimePrecision(long long time, Time_SecondPrecision from = Time_NanoSeconds, Time_SecondPrecision to = Time_Seconds);


}
