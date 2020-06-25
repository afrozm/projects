#pragma once

class Progress
{
public:
	Progress();
	void SetTask(unsigned long long total, unsigned int percentParticipation = 100);
	bool UpdateProgress(unsigned long long currentDone);
    unsigned long long GetCurrentDone() const { return mCurrentDone; }
	double GetCurrentPercentageDone() {return mSartPercentage+mCurrentPercentDone;}
	void Reset();
protected:
	unsigned long long mTotal;
	unsigned long long mCurrentDone;
	double mCurrentPercentDone;
    double mPrevPercentDone;
	double mSartPercentage;
	double mPercentageParticipation;
	double mPercentagePerticipated;
};
