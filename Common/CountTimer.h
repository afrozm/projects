#pragma once

#include "Common.h"
#include <windows.h>
#include "StdConsole.h"

class CountTimer {
    DWORD mLastTime;
    DWORD mCurTime;
    __int64 mTimeDuration;
    DWORD mTimerUpdateDuration;
    bool m_bCountDownWards;
    ConsolePrinter m_CP;
public:
    CountTimer(bool bCountDownWards = false, DWORD timerUpdateDuration = 1000);
    void SetTimeDuration(__int64 timeDuration);
    __int64 GetTimeDuration(void);

    void GetString(TCHAR *str, int size);
    bool UpdateTimeDuration();
    void PrintTimeDuration();
};

