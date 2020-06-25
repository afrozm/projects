#pragma once

class CNamedLock
{
public:
	CNamedLock(LPCTSTR lockName);
	~CNamedLock(void);
	bool IsLocked() {return mhMutexHandle!=NULL;}
private:
	HANDLE mhMutexHandle;
};
