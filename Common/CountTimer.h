#pragma once

#include "Common.h"
#ifndef DISABLE_CONSOLE_PRINTER
#include "StdConsole.h"
#endif // DISABLE_CONSOLE_PRINTER


class CountTimer {
    long long mLastTime;
    long long mCurTime;
    long long mTimeDuration;
    long long mTimerUpdateDuration;
    bool m_bCountDownWards;
#ifndef DISABLE_CONSOLE_PRINTER
    ConsolePrinter mCP;
#endif
public:
    CountTimer(bool bCountDownWards = false, long long timerUpdateDuration = 1000);
    void SetTimeDuration(long long timeDuration);
    long long GetTimeDuration(bool bAfterForceUpdate = false);

    void GetString(TCHAR *str, int size, bool bForceUpdate = false);
    std::string GetString(unsigned timePrecision = 2, bool bForceUpdate = false);
    bool UpdateTimeDuration(bool bForce = false);
    void PrintTimeDuration();
};

