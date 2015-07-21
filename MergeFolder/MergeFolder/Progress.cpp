#include "stdafx.h"
#include "Progress.h"
#include <math.h>

Progress::Progress()
:mTotal(0), mCurrentPercentDone(0), mPercentageParticipation(0),
mPercentagePerticipated(0), mSartPercentage(0), mCurrentDone(0)
{
}

void Progress::SetTask(UINT64 total, unsigned int percentParticipation)
{
	if (total == 0)
		return;
	mTotal = total;
	if (percentParticipation > 100)
		percentParticipation = 100;
	mPercentageParticipation = percentParticipation;
	mPercentageParticipation *= (100-mPercentagePerticipated)/100;
	mSartPercentage = mPercentagePerticipated;
	mPercentagePerticipated += mPercentageParticipation;
	mPercentageParticipation /= 100;
	mCurrentDone = 0;
	mCurrentPercentDone = 0;
}
bool Progress::UpdateProgress(UINT64 currentDone)
{
	bool bUpdated = mCurrentDone != currentDone;
	if (bUpdated) {
		double curPercentage = (currentDone * 100.0) / mTotal;
		curPercentage *= mPercentageParticipation;
		mCurrentPercentDone = curPercentage;
		mCurrentDone = currentDone;
	}
	return bUpdated;
}

void Progress::Reset()
{
	mTotal = 0;
	mCurrentPercentDone = 0;
	mPercentageParticipation = 0;
	mPercentagePerticipated = 0;
	mSartPercentage = 0;
	mCurrentDone = 0;
}