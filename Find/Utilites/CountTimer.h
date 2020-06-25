#pragma once

#define TIMER_SIZE 7
struct STime {
	__int64 times[TIMER_SIZE]; // index 0 - timeNames[0], 1 timeNames[1]
	STime(__int64 milliSecs);
};
class CountTimer {
	ULONGLONG mCurTime;
	__int64 mTimeDuration;
	ULONGLONG mTimerUpdateDuration;
	bool m_bCountDownWards;
public:
	CountTimer(ULONGLONG timerUpdateDuration = 1000, bool bCountDownWards = false);
	void Reset(ULONGLONG timerUpdateDuration = 1000, bool bCountDownWards = false);
	void SetTimeDuration(__int64 timeDuration = 0);
	void SetTimeUpdateDuration(ULONGLONG timeUpdateDuration) {mTimerUpdateDuration=timeUpdateDuration;}
	ULONGLONG GetTimeUpdateDuration() const  {return mTimerUpdateDuration;}
	__int64 GetTimeDuration(void);
	void GetString(TCHAR *str, int size);
	CString GetString();
	bool UpdateTimeDuration(bool bForce = false);
	static void GetString(ULONGLONG timeDuration, TCHAR *str, int size);
	static CString GetString(ULONGLONG timeDuration);
};
