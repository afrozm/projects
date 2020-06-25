#pragma once

class CTimeOut
{
	DWORD mCurrentTime;
	DWORD mTimeOutDuration;
public:
	CTimeOut(DWORD dwTimeOutDuration = 1000);
	~CTimeOut(void);
	bool IsTimeOut();
	DWORD GetTimeOutDuration() const {return mTimeOutDuration;}
	void SetTimeOutDuration(DWORD dwTimeOutDuration) {mTimeOutDuration=dwTimeOutDuration;}
	void Reset();
};
