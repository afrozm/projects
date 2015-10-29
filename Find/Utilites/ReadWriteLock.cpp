#include "StdAfx.h"
#include "ReadWriteLock.h"

CReadWriteLock::CReadWriteLock()
	: mnReaders(0)
{

}
void CReadWriteLock::LockRead()
{
	CAutoLock autoLockCounter(mReaderCountLock);
	++mnReaders;
	if (mnReaders == 1)
		LockWrite();
}
void CReadWriteLock::UnlockRead()
{
	CAutoLock autoLockCounter(mReaderCountLock);
	--mnReaders;
	if (mnReaders == 0)
		UnlockWrite();
}
void CReadWriteLock::LockWrite()
{
	mWriteLock.Lock();
}
void CReadWriteLock::UnlockWrite()
{
	mWriteLock.Unlock();
}

CAutoReadLock::CAutoReadLock(CReadWriteLock &rwLock)
	: mReadLock(rwLock)
{
	mReadLock.LockRead();
}
CAutoReadLock::~CAutoReadLock()
{
	mReadLock.UnlockRead();
}

