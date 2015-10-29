#pragma once

class CAutoLock
{
public:
	CAutoLock(CSyncObject &syncObj);
	~CAutoLock(void);
	static bool IsLocked(CSyncObject &syncObj);
	bool IsLocked();
private:
	CSyncObject &mSyncObj;
};
