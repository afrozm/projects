
#include "stdafx.h"
#include "CountTimer.h"
#include "ProcessUtil.h"
#include "stlutils.h"
using namespace STLUtils;

#define TIMER_SIZE 7
static const char *sktimeNames[] = {
    "year",
    "month",
    "day",
    "hour",
    "minute",
    "second",
    "millisecond",
};
static const long long skTimeDuration[] = {
    1000LL * 60LL * 60LL * 24LL * 365LL,
    1000 * 60 * 60 * 24 * 30UL,
    1000 * 60 * 60 * 24,
    1000 * 60 * 60,
    1000 * 60,
    1000,
    1
};
struct STime {
    long long times[TIMER_SIZE]; // index 0 - timeNames[0], 1 timeNames[1]
    STime(long long milliSecs)
    {
        for (int i = 0; i < TIMER_SIZE; i++) {
            times[i] = milliSecs / skTimeDuration[i];
            milliSecs %= skTimeDuration[i];
        }
    }
};
CountTimer::CountTimer(bool bCountDownWards /* = false */, long long timerUpdateDuration /* = 1000 */)
{
    m_bCountDownWards = bCountDownWards;
    mCurTime = mLastTime = ProcessUtil::GetTickCount();
    mTimeDuration = 0;
    mTimerUpdateDuration = timerUpdateDuration;
}
void CountTimer::SetTimeDuration(long long timeDuration)
{
    m_bCountDownWards = true; // its a count down timer
    mTimeDuration = timeDuration;
}
long long CountTimer::GetTimeDuration(bool bAfterForceUpdate /* = false */)
{
    UpdateTimeDuration(bAfterForceUpdate);
    return mTimeDuration;
}

void CountTimer::GetString(TCHAR *str, int size)
{
    std::string outTime(GetString());
    if (str && size > 0) {
        int i = 0;
        for (i = 0; i < size-1&&i < outTime.length(); ++i)
            str[i] = outTime[i];
        str[i] = 0;
    }
}

std::string CountTimer::GetString(unsigned timePrecision /* = 2 */)
{
    STime sTime(GetTimeDuration(true));
    std::string outStr;
    const int kiTimeStringCount(timePrecision ? timePrecision : 2);
    for (int i = 0, pc=0; i < TIMER_SIZE && pc < kiTimeStringCount; i++) {
        if (sTime.times[i] || pc==0 && i==TIMER_SIZE-1) {
            std::string timeStr;
            ChangeType(sTime.times[i], timeStr);
            outStr += " " + timeStr + " " + sktimeNames[i] + (sTime.times[i] > 1 ? "s" : "");
            ++pc;
        }
    }
    return outStr;
}

bool CountTimer::UpdateTimeDuration(bool bForce /* = false */)
{
    long long curTime = ProcessUtil::GetTickCount();
    if (bForce || curTime - mCurTime > mTimerUpdateDuration) {
        long long interval = curTime - mLastTime;
        if (m_bCountDownWards) {
            mTimeDuration -= interval;
            if (mTimeDuration < 0)
                mTimeDuration = 0;
            mLastTime = curTime;
        }
        else {
            mTimeDuration = interval;
        }
        mCurTime = curTime;
        return true;
    }
    return false;
}
