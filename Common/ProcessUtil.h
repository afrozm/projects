// CreateProcessAsUser.cpp : Defines the entry point for the console application.
//

#pragma once
#ifdef _WIN32
#include <windows.h>
#else

#endif // _WIN32

namespace ProcessUtil {

    BOOL IsUserAdmin(VOID);

    // RunApplication flag
#define RAF_BLOCKING 1
#define RAF_ADMIN 2
#define RAF_NOWINDOW 4

    bool RunApplication(LPCTSTR commandLine,
        unsigned uRAF = RAF_BLOCKING, unsigned long *outExitCode = NULL);
    bool RunApplication(int argc, LPCTSTR *argv,
        unsigned uRAF = RAF_BLOCKING, unsigned long *outExitCode = NULL);


    int GetCurrentProcessId();
    int GetCurrentThreadId();

    void Sleep(unsigned milliSeconds);
}