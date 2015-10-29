#include "StdAfx.h"
#include "TimeOut.h"

CTimeOut::CTimeOut(DWORD dwTimeOutDuration)
: mCurrentTime(0), mTimeOutDuration(dwTimeOutDuration)
{
	Reset();
}

CTimeOut::~CTimeOut(void)
{
}
bool CTimeOut::IsTimeOut()
{
	DWORD dwCurrentTime(GetTickCount());
	bool bTimeOut = (dwCurrentTime - mCurrentTime > mTimeOutDuration);
	if (bTimeOut)
		Reset();
	return bTimeOut;
}
void CTimeOut::Reset()
{
	mCurrentTime = GetTickCount();
}