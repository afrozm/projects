#pragma once

#include "CountTimer.h"

typedef void (*PercentUpdateCallback)(double curPercentage, void *pUserData);

#define PF_UPDATE_AT_TIME_OUT FLAGBIT(0)

class CPercentage
{
private:
	unsigned __int64 m_uTotal;
	double m_uCurrentPercent;
	PercentUpdateCallback m_pPercentUpdateCallbackFn;
	void *m_pUserData;
	CountTimer mUpdateTimer;
	unsigned muFlags;
public:
	CPercentage(unsigned __int64 total = 0, PercentUpdateCallback pucb = NULL, void *pUserData = NULL);
	bool Update(unsigned __int64 currentDone);
	void SetCallback(PercentUpdateCallback pucb = NULL, void *pUserData = NULL)
	{
		m_pPercentUpdateCallbackFn = pucb;
		m_pUserData = pUserData;
	}
	void SetTototal(unsigned __int64 total) {m_uTotal=total;}
	CountTimer& GetConterTimer() {return mUpdateTimer;}
	void SetFlag(unsigned uFlag, bool bSet = true) {SET_UNSET_FLAG(muFlags,uFlag,bSet);}
	unsigned GetFlags() const {return muFlags;}
	~CPercentage(void);
};
