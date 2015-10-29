#include "StdAfx.h"
#include "CountTimer.h"

static const TCHAR *stimeNames[] = {
	_T("year"),
	_T("month"),
	_T("day"),
	_T("hour"),
	_T("minute"),
	_T("second"),
	_T("millisecond"),
};
static const __int64 stimeDuration[] = {
	(INT64)1000UL*60UL*60UL*24UL*365UL,
	(INT64)1000*60*60*24*30UL,
	1000*60*60*24,
	1000*60*60,
	1000*60,
	1000,
	1
};
STime::STime(__int64 milliSecs)
{
	for (int i = 0; i < TIMER_SIZE; i++) {
		times[i] = milliSecs / stimeDuration[i];
		milliSecs %= stimeDuration[i];
	}
}
CountTimer::CountTimer(ULONGLONG timerUpdateDuration, bool bCountDownWards)
{
	Reset(timerUpdateDuration, bCountDownWards);
}
void CountTimer::Reset(ULONGLONG timerUpdateDuration /* = 1000 */, bool bCountDownWards /* = false */)
{
	m_bCountDownWards = bCountDownWards;
	mCurTime = GetTickCount64();
	mTimeDuration = 0;
	mTimerUpdateDuration = timerUpdateDuration;
}
void CountTimer::SetTimeDuration(__int64 timeDuration)
{
	mTimeDuration = timeDuration;
}
__int64 CountTimer::GetTimeDuration(void)
{
	UpdateTimeDuration();
	return mTimeDuration;
}

void CountTimer::GetString(TCHAR *str, int size)
{
	UpdateTimeDuration(true);
	GetString(mTimeDuration, str, size);
}
CString CountTimer::GetString()
{
	TCHAR str[4096];
	GetString(str, ARRAYSIZE(str));
	return str;
}

CString CountTimer::GetString( ULONGLONG timeDuration )
{
	TCHAR str[4096];
	GetString(timeDuration, str, ARRAYSIZE(str));
	return str;
}

void CountTimer::GetString( ULONGLONG timeDuration, TCHAR *str, int size )
{
	STime sTime(timeDuration);
	int i;
	for (i = 0; i < TIMER_SIZE -2; i++) {
		if (sTime.times[i])
			break;
	}
	int len = _sntprintf_s(str, size, size, _T("%d %s%s"), (int)sTime.times[i], stimeNames[i],
		(int)sTime.times[i] > 1 ? _T("s") : _T(""));
	i++;
	if (i < TIMER_SIZE -1 && sTime.times[i] > 0) {
		len = _sntprintf_s(str+len, size-len, size-len, _T(" %d %s%s"), (int)sTime.times[i], stimeNames[i],
			(int)sTime.times[i] > 1 ? _T("s") : _T(""));
	}
}

bool CountTimer::UpdateTimeDuration(bool bForce)
{
	ULONGLONG curTime = GetTickCount64();
	if (bForce || ((curTime - mCurTime) >= mTimerUpdateDuration)) {
		ULONGLONG interval = curTime - mCurTime;
		if (m_bCountDownWards) {
			mTimeDuration -= interval;
			if (mTimeDuration < 0)
				mTimeDuration = 0;
		}
		else {
			mTimeDuration += interval;
		}
		mCurTime = curTime;
		return true;
	}
	return false;
}