#pragma once

#include "ReadWriteLock.h"

typedef int (*ThreadProcFn)(LPVOID pThreadData);

typedef CWinThread* NativeThreadPtr;

class TMThreadData;

class ThreadNotifier {
public:
	enum ThreadNotification {
		ThreadCreated,
		ThreadBeingRemove,
		ThreadStatus
	};
	virtual void Notify(ThreadNotification notifiation, DWORD dwThreadID, LPVOID notifyData = NULL) = 0;
};

typedef int (*ThreadIteratorFn)(DWORD threadID, LPVOID pUserData);

class ThreadManager
{
public:
	static ThreadManager& GetInstance();
	bool CreateThread(ThreadProcFn proc, LPVOID pUserData = NULL, int iThreadClass = 0, LPDWORD outThreadID = NULL, LPCTSTR threadName = NULL);
	bool RemoveThread(DWORD threadID);
	bool IsValidThread(DWORD threadID);
	void TerminateThreads(int iThreadClass = -1, DWORD dwMilliSeconds = 0);
	INT_PTR GetAllThreadCount();
	INT_PTR GetThreadCount(int iThreadClass = 0, bool bMatchingClass = true);
	INT_PTR GetThreadCount(const CArrayEx<int> &threadClass, bool bOnyInArray = true);
	DWORD GetThread(int iThreadClass = -1);
	DWORD GetCurrentThread() const {return GetCurrentThreadId();}
	bool IsThreadTerminated(DWORD threadID = -1);
	void SetNotifier(ThreadNotifier *pNotifier) {m_pNotifier=pNotifier;}
	void SetThreadStatusStr(LPCTSTR status, ...);
	int GetThreadClass(DWORD threadID = -1);
	DWORD WaitForThread(DWORD threadID, DWORD dwMilliSeconds = INFINITE);
	ULONGLONG GetThreadTime(DWORD threadID);
	int IterateThread(ThreadIteratorFn iterFn, LPVOID pUserData = NULL);
	bool TerminateThread(DWORD threadID);
    void SetThreadName(LPCTSTR threadName, DWORD threadID = -1);
    LPCTSTR GetThreadName(DWORD threadID = -1);
    const bool& GetIsTerminatedFlag();
private:
	CReadWriteLock& GetLock() { return mArrayLocker;}
	ThreadManager(void);
	~ThreadManager(void);
	void Notify(ThreadNotifier::ThreadNotification notifiation, DWORD threadID = -1, LPVOID notifyData = NULL);
	typedef CMap<DWORD, DWORD, TMThreadData*, TMThreadData*> CMapDWordToThreadData;
	CMapDWordToThreadData mThreads;
	CReadWriteLock mArrayLocker;
	ThreadNotifier *m_pNotifier;
};
