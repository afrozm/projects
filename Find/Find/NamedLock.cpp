#include "StdAfx.h"
#include "NamedLock.h"

CNamedLock::CNamedLock(LPCTSTR lockName)
: mhMutexHandle(NULL)
{
	HANDLE hMutex = ::CreateMutex(NULL, TRUE, lockName);
	if (hMutex != NULL)
	{
		if (ERROR_ALREADY_EXISTS != GetLastError())
		{
			mhMutexHandle = hMutex;
		}
		else
		{
			::ReleaseMutex(hMutex);
			::CloseHandle(hMutex);
		}
	}
}

CNamedLock::~CNamedLock(void)
{
	if (NULL != mhMutexHandle)
	{
		::ReleaseMutex(mhMutexHandle);
		::CloseHandle(mhMutexHandle);
		mhMutexHandle = NULL;
	}
}
