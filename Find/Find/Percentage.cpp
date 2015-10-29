#include "StdAfx.h"
#include "Percentage.h"

CPercentage::CPercentage(unsigned __int64 total, PercentUpdateCallback pucb, void *pUserData)
: m_uTotal(total), m_pPercentUpdateCallbackFn(pucb), m_pUserData(pUserData), m_uCurrentPercent(0), muFlags(0)
{
}

CPercentage::~CPercentage(void)
{
}

bool CPercentage::Update(unsigned __int64 currentDone)
{
	if (m_uTotal == 0)
		return false;
	double curpercentate = (((double)currentDone / double(m_uTotal)) * 100.0);
	if (curpercentate > 100)
		curpercentate = 100;
	bool bUpdate(mUpdateTimer.UpdateTimeDuration());
	if (!bUpdate && !IS_FLAG_SET(GetFlags(), PF_UPDATE_AT_TIME_OUT))
		bUpdate = curpercentate - m_uCurrentPercent >= 0.01;
	if (bUpdate) {
		m_uCurrentPercent = curpercentate;
		if (m_pPercentUpdateCallbackFn)
			m_pPercentUpdateCallbackFn(m_uCurrentPercent, m_pUserData);
	}
	return bUpdate;
}