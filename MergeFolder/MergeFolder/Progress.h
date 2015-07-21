#pragma once

#ifndef UINT64
#ifdef _WIN32
typedef unsigned __int64 UINT64;
#else
#endif
typedef unsigned long long UINT64;
#endif

class Progress
{
public:
	Progress();
	void SetTask(UINT64 total, unsigned int percentParticipation = 100);
	bool UpdateProgress(UINT64 currentDone);
	double GetCurrentPercentageDone() {return mSartPercentage+mCurrentPercentDone;}
	void Reset();
protected:
	UINT64 mTotal;
	UINT64 mCurrentDone;
	double mCurrentPercentDone;
	double mSartPercentage;
	double mPercentageParticipation;
	double mPercentagePerticipated;
};
