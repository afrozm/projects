
#include "stdafx.h"
#include "CountTimer.h"
#define TIMER_SIZE 7
static const TCHAR *sktimeNames[] = {
    _T("year"),
    _T("month"),
    _T("day"),
    _T("hour"),
    _T("minute"),
    _T("second"),
    _T("millisecond"),
};
static const __int64 skTimeDuration[] = {
    1000LL * 60LL * 60LL * 24LL * 365LL,
    1000 * 60 * 60 * 24 * 30UL,
    1000 * 60 * 60 * 24,
    1000 * 60 * 60,
    1000 * 60,
    1000,
    1
};
struct STime {
    __int64 times[TIMER_SIZE]; // index 0 - timeNames[0], 1 timeNames[1]
    STime(__int64 milliSecs)
    {
        for (int i = 0; i < TIMER_SIZE; i++) {
            times[i] = milliSecs / skTimeDuration[i];
            milliSecs %= skTimeDuration[i];
        }
    }
};
CountTimer::CountTimer(bool bCountDownWards /* = false */, DWORD timerUpdateDuration /* = 1000 */)
{
    m_bCountDownWards = bCountDownWards;
    mCurTime = mLastTime = GetTickCount();
    mTimeDuration = 0;
    mTimerUpdateDuration = timerUpdateDuration;
}
void CountTimer::SetTimeDuration(__int64 timeDuration)
{
    m_bCountDownWards = true; // its a count down timer
    mTimeDuration = timeDuration;
}
__int64 CountTimer::GetTimeDuration(void)
{
    UpdateTimeDuration();
    return mTimeDuration;
}

void CountTimer::GetString(TCHAR *str, int size)
{
    STime sTime(mTimeDuration);
    int i;
    for (i = 0; i < TIMER_SIZE - 2; i++) {
        if (sTime.times[i])
            break;
    }
    int len = _sntprintf_s(str, size, size, _T("%d %s%s"), (int)sTime.times[i], sktimeNames[i],
        (int)sTime.times[i] > 1 ? _T("s") : _T(""));
    i++;
    if (i < TIMER_SIZE - 1 && sTime.times[i] > 0) {
        len = _sntprintf_s(str + len, size - len, size - len, _T(" %d %s%s"), (int)sTime.times[i], sktimeNames[i],
            (int)sTime.times[i] > 1 ? _T("s") : _T(""));
    }
}
bool CountTimer::UpdateTimeDuration()
{
    DWORD curTime = GetTickCount();
    if (curTime - mCurTime > mTimerUpdateDuration) {
        DWORD interval = curTime - mLastTime;
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
void CountTimer::PrintTimeDuration()
{
    if (UpdateTimeDuration()) {
        TCHAR str[32];
        GetString(str, 32);
        m_CP.Print(str);
    }
}


