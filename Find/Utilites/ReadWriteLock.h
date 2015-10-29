#pragma once

#include "AutoLock.h"

class  CMutexEx : public CMutex {
public:
	CMutexEx() : lockedThread(0) {}
	BOOL Lock(DWORD dwTimeout = INFINITE)
	{
		BOOL bRet(__super::Lock(dwTimeout));
		lockedThread = GetCurrentThreadId();
		return bRet;
	}
	BOOL Unlock()
	{
		BOOL bRet(__super::Unlock());
		DWORD curThread = GetCurrentThreadId();
        UNREFERENCED_PARAMETER(curThread);
		DWORD dwErr(GetLastError());
        UNREFERENCED_PARAMETER(dwErr);
		lockedThread = 0;
		return bRet;
	}
private:
	DWORD lockedThread;
};
class CSemaphoreEx : public CSemaphore {
public:
	CSemaphoreEx() : lockedThread(0) {}
	BOOL Lock(DWORD dwTimeout = INFINITE)
	{
		BOOL bRet(__super::Lock(dwTimeout));
		lockedThread = GetCurrentThreadId();
		return bRet;
	}
	BOOL Unlock()
	{
		DWORD curThread = GetCurrentThreadId();
        UNREFERENCED_PARAMETER(curThread);
		lockedThread = 0;
		BOOL bRet(__super::Unlock());
		DWORD dwErr(GetLastError());
        UNREFERENCED_PARAMETER(dwErr);
		return bRet;
	}
private:
	DWORD lockedThread;
};
class CReadWriteLock
{
public:
	CReadWriteLock();
	void LockRead();
	void UnlockRead();
	void LockWrite();
	void UnlockWrite();
	CSemaphore& GetWriteLock() {return mWriteLock;}
private:
	volatile unsigned mnReaders;
	CMutex mReaderCountLock;
	CSemaphore mWriteLock;
};

class CAutoReadLock {
public:
	CAutoReadLock(CReadWriteLock &);
	~CAutoReadLock();
private:
	CReadWriteLock &mReadLock;
};
