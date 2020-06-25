#include "StdAfx.h"
#include "AutoLock.h"

CAutoLock::CAutoLock(CSyncObject &syncObj)
: mSyncObj(syncObj)
{
	mSyncObj.Lock();
}

CAutoLock::~CAutoLock(void)
{
	mSyncObj.Unlock();
}
bool CAutoLock::IsLocked(CSyncObject &syncObj)
{
	bool bLocked(true);
	if (syncObj.Lock(1)) {
		bLocked = false;
		syncObj.Unlock();
	}
	return bLocked;
}
bool CAutoLock::IsLocked()
{
	return IsLocked(mSyncObj);
}
