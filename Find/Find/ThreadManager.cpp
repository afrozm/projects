#include "StdAfx.h"
#include "ThreadManager.h"
#include "AutoLock.h"
#include "LoggerFactory.h"
#include "CountTimer.h"

class TMThreadData {
public:
	TMThreadData(ThreadProcFn threadFn, LPVOID pUserData = NULL, int iThreadClass = 0);
	LPVOID GetUserData() {return m_pUserData;}
	bool IsTerminated() {return mbIsTerminated;}
	void Terminate() {mbIsTerminated=true;}
	ThreadProcFn GetThreadProcFn() const {return mThreadProcFn;}
	int GetThreadClass() const {return miThreadClass;}
	void SetNativeThreadPtr(NativeThreadPtr nativeThreadPtr) { mThreadPtr=nativeThreadPtr;}
	NativeThreadPtr GetNativeThreadPtr() const {return mThreadPtr;}
	ULONGLONG GetThreadTime() const;
	CString GetStr() const;
	DWORD GetThreadId() const {return GetNativeThreadPtr()->m_nThreadID;}
private:
	ThreadProcFn mThreadProcFn;
	LPVOID m_pUserData;
	bool mbIsTerminated;
	int miThreadClass;
	NativeThreadPtr mThreadPtr;
	ULONGLONG mStartTime;
};

TMThreadData::TMThreadData(ThreadProcFn threadFn, LPVOID pUserData, int iThreadClass)
: mThreadProcFn(threadFn), m_pUserData(pUserData), mbIsTerminated(false),
miThreadClass(iThreadClass), mThreadPtr(NULL), mStartTime(GetTickCount64())
{
}
ULONGLONG TMThreadData::GetThreadTime() const
{
	return GetTickCount64() - mStartTime;
}
CString TMThreadData::GetStr() const
{
	CString retval;
	if (this != NULL) {
		CountTimer timeTaken;
		timeTaken.SetTimeDuration(GetThreadTime());
		retval.Format(_T("ThreadClass: %d Time: %s"), miThreadClass, timeTaken.GetString());
	}
	else retval = _T("{null}");
	return retval;
}

ThreadManager::ThreadManager(void)
{
}

ThreadManager::~ThreadManager(void)
{
}
ThreadManager& ThreadManager::GetInstance()
{
	static ThreadManager tm;
	return tm;
}
static void LogThreadData(TMThreadData *pThread)
{
	LoggerFacory::GetInstance().GetLogger(_T("")).Log(Logger::kLogLevelError, _T("Thread crashed!!!\nThread Data=%s"), pThread->GetStr());
}
static DWORD WINAPI ThreadManagerThreadProc(LPVOID lpParameter)
{
	TMThreadData *pThread = (TMThreadData *)lpParameter;
	__try {
		pThread->GetThreadProcFn()(pThread->GetUserData());
	}
	__except(EXCEPTION_EXECUTE_HANDLER) {
		__try {
			LogThreadData(pThread);
		}
		__except(EXCEPTION_EXECUTE_HANDLER) {}
	}
	ThreadManager::GetInstance().RemoveThread(pThread->GetThreadId());
	return 0;
}
bool ThreadManager::CreateThread(ThreadProcFn threadProcFn, LPVOID pUserData /* = NULL */, int iThreadClass /* = 0 */, LPDWORD outThreadID /* = NULL */)
{
	if (iThreadClass < 0)
		iThreadClass = 0;
	if (outThreadID != NULL)
		*outThreadID = 0;
	TMThreadData *pThread = new TMThreadData(threadProcFn, pUserData, iThreadClass);
	NativeThreadPtr nativeThreadPtr = AfxBeginThread((AFX_THREADPROC)ThreadManagerThreadProc, pThread, 0, 0, CREATE_SUSPENDED);
	bool bSuccess(false);
	if (nativeThreadPtr != NULL) {
		pThread->SetNativeThreadPtr(nativeThreadPtr);
		{
			//CAutoLock autoLock(mArrayLocker.GetWriteLock());
			mArrayLocker.LockWrite();
			mThreads[nativeThreadPtr->m_nThreadID]= pThread;
			mArrayLocker.UnlockWrite();
		}
		Notify(ThreadNotifier::ThreadCreated, nativeThreadPtr->m_nThreadID);
		nativeThreadPtr->ResumeThread();
		bSuccess = true;
	}
	else {
		delete pThread;
		pThread = NULL;
	}
	if (outThreadID != NULL && pThread)
		*outThreadID = pThread->GetThreadId();
	return bSuccess;
}
bool ThreadManager::RemoveThread(DWORD threadID)
{
	bool bRemoved(IsValidThread(threadID));
	if (bRemoved)
		Notify(ThreadNotifier::ThreadBeingRemove, threadID);
	if (bRemoved)
	{
		//CAutoLock autoLock(mArrayLocker.GetWriteLock());
		mArrayLocker.LockWrite();
		TMThreadData *pThread(NULL);
		bRemoved = mThreads.Lookup(threadID, pThread) == TRUE;
		if (bRemoved)
			mThreads.RemoveKey(threadID);
		mArrayLocker.UnlockWrite();
		if (pThread != NULL)
			delete pThread;
	}
	return bRemoved;
}

bool ThreadManager::IsValidThread(DWORD threadID)
{
	CAutoReadLock autoLock(mArrayLocker);
	return mThreads.PLookup(threadID) != NULL;
}
void ThreadManager::TerminateThreads(int iThreadClass)
{
	CAutoReadLock autoLock(mArrayLocker);
	for (POSITION pos(mThreads.GetStartPosition()); pos != NULL; ) {
		TMThreadData *pThread(NULL);
		DWORD threadID((DWORD)-1);
		mThreads.GetNextAssoc(pos, threadID, pThread);
		if (iThreadClass < 0 || pThread->GetThreadClass() == iThreadClass)
			pThread->Terminate();
	}
}
INT_PTR ThreadManager::GetAllThreadCount()
{
	CAutoReadLock autoLock(mArrayLocker);
	return mThreads.GetCount();
}
INT_PTR ThreadManager::GetThreadCount(int iThreadClass, bool bMatchingClass)
{
	if (iThreadClass < 0)
		return GetAllThreadCount();
	CAutoReadLock autoLock(mArrayLocker);
	INT_PTR count = 0;
	for (POSITION pos(mThreads.GetStartPosition()); pos != NULL; ) {
		TMThreadData *pThread(NULL);
		DWORD threadID((DWORD)-1);
		mThreads.GetNextAssoc(pos, threadID, pThread);
		bool bMatch(pThread->GetThreadClass() == iThreadClass);
		if (bMatch == bMatchingClass)
			++count;
	}
	return count;
}
INT_PTR ThreadManager::GetThreadCount(const CArrayEx<int> &threadClass, bool bOnyInArray)
{
	CAutoReadLock autoLock(mArrayLocker);
	INT_PTR count = 0;
	for (POSITION pos(mThreads.GetStartPosition()); pos != NULL; ) {
		TMThreadData *pThread(NULL);
		DWORD threadID((DWORD)-1);
		mThreads.GetNextAssoc(pos, threadID, pThread);
		bool bFound(threadClass.Find(pThread->GetThreadClass()) >= 0);
		if (bFound == bOnyInArray)
			++count;
	}
	return count;
}
DWORD ThreadManager::GetThread(int iThreadClass)
{
	CAutoReadLock autoLock(mArrayLocker);
	for (POSITION pos(mThreads.GetStartPosition()); pos != NULL; ) {
		TMThreadData *pThread(NULL);
		DWORD threadID((DWORD)-1);
		mThreads.GetNextAssoc(pos, threadID, pThread);
		ASSERT(threadID == pThread->GetThreadId());
		if (iThreadClass < 0 || pThread->GetThreadClass() == iThreadClass)
			return threadID;
	}
	return 0;
}
void ThreadManager::Notify(ThreadNotifier::ThreadNotification notifiation, DWORD threadID /* = -1 */, LPVOID notifyData /* = NULL */)
{
	__try {
		if (m_pNotifier != NULL) {
			if ((int)threadID < 0)
				threadID = GetCurrentThreadId();
			m_pNotifier->Notify(notifiation, threadID, notifyData);
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER) {
	}
}
void ThreadManager::SetThreadStatusStr(LPCTSTR msg, ...)
{
	if (msg != NULL) {
		va_list arg;
		va_start(arg, msg);
		int len = _vsctprintf(msg, arg) + 4*sizeof(TCHAR); // _vscprintf doesn't count + 1; terminating '\0'
		TCHAR *buf = new TCHAR[len];
		_vstprintf_s(buf, len, msg, arg);
		Notify(ThreadNotifier::ThreadStatus, (DWORD)-1, buf);
		delete buf;
	}
}

bool ThreadManager::IsThreadTerminated(DWORD threadID /* = -1 */)
{
	if ((int)threadID < 0)
		threadID = GetCurrentThreadId();
	CAutoReadLock autoLock(mArrayLocker);
	TMThreadData *pThread(NULL);
	mThreads.Lookup(threadID, pThread);
    if (pThread == NULL && GetCurrentThreadId() == threadID)
        return false;
	return pThread == NULL || pThread->IsTerminated();
}

int ThreadManager::GetThreadClass(DWORD threadID /* = -1 */)
{
	if ((int)threadID < 0)
		threadID = GetCurrentThreadId();
	CAutoReadLock autoLock(mArrayLocker);
	TMThreadData *pThread(NULL);
	mThreads.Lookup(threadID, pThread);
	return pThread ? pThread->GetThreadClass() : -1;
}

DWORD ThreadManager::WaitForThread(DWORD threadID, DWORD dwMilliSeconds /* = -1 */)
{
	HANDLE hThreadHandle(NULL);
	{
		TMThreadData *pThread(NULL);
		CAutoReadLock autoLock(mArrayLocker);
		mThreads.Lookup(threadID, pThread);
		if (pThread != NULL)
			hThreadHandle = pThread->GetNativeThreadPtr()->m_hThread;
	}
	DWORD waitResult(0);
	if (hThreadHandle)
		waitResult = WaitForSingleObject(hThreadHandle, dwMilliSeconds);
	return waitResult;
}

int ThreadManager::IterateThread( ThreadIteratorFn iterFn, LPVOID pUserData /*= NULL*/ )
{
	int retVal(0);
	CAutoReadLock autoLock(mArrayLocker);
	for (POSITION pos(mThreads.GetStartPosition()); pos != NULL; ) {
		TMThreadData *pThread(NULL);
		DWORD threadID((DWORD)-1);
		mThreads.GetNextAssoc(pos, threadID, pThread);
		retVal = iterFn(threadID, pUserData);
		if (retVal)
			break;
	}
	return retVal;
}

ULONGLONG ThreadManager::GetThreadTime( DWORD threadID )
{
	TMThreadData *pThread(NULL);
	CAutoReadLock autoLock(mArrayLocker);
	mThreads.Lookup(threadID, pThread);
	return pThread ? pThread->GetThreadTime() : 0;
}

bool ThreadManager::TerminateThread( DWORD threadID )
{
	TMThreadData *pThread(NULL);
	CAutoReadLock autoLock(mArrayLocker);
	mThreads.Lookup(threadID, pThread);
	if (pThread)
		pThread->Terminate();
	return pThread != NULL;
}
