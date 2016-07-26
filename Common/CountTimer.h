#pragma once

#include "Common.h"

class CountTimer {
    long long mLastTime;
    long long mCurTime;
    long long mTimeDuration;
    long long mTimerUpdateDuration;
    bool m_bCountDownWards;
public:
    CountTimer(bool bCountDownWards = false, long long timerUpdateDuration = 1000);
    void SetTimeDuration(long long timeDuration);
    long long GetTimeDuration(bool bAfterForceUpdate = false);

    void GetString(TCHAR *str, int size);
    std::string GetString(unsigned timePrecision = 2);
    bool UpdateTimeDuration(bool bForce = false);
};

