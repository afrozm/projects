

#include "StdCondVar.h"
#include "ProcessUtil.h"

void StdConditionVariable::Signal(bool bSingnalAllWaiting /* = false */)
{
    if (bSingnalAllWaiting)
        mConditionVariable.notify_all();
    else
        mConditionVariable.notify_one();
}
bool StdConditionVariable::WaitForSignal(long long msTimeOut /* = -1 */)
{
    bool bSignalled(true);
    std::unique_lock<std::mutex> lk(mConditionVariableMutex);
    
    if (msTimeOut < 0)
        mConditionVariable.wait(lk);
    else
        bSignalled = (mConditionVariable.wait_for(lk, std::chrono::milliseconds(msTimeOut)) == std::cv_status::no_timeout);
    return bSignalled;
}



// Mutex

StdMutex::StdMutex()
{
#ifdef _WIN32
    m_hMutex = CreateMutex(NULL, FALSE, NULL);
#endif // _WIN32
}

StdMutex::~StdMutex()
{
#ifdef _WIN32
    if (m_hMutex != NULL)
        CloseHandle(m_hMutex);
    m_hMutex = NULL;
#endif // _WIN32
}

bool StdMutex::lock()
{
    return try_lock_for();
}

bool StdMutex::try_lock_for(long long msTimeOut /*= -1*/)
{
    bool bLocked(false);
#ifdef _WIN32
    bLocked = WaitForSingleObject(m_hMutex, (DWORD)msTimeOut) == WAIT_OBJECT_0;
#else
    if (msTimeOut < 0) {
        try {
            m_hMutex.lock();
            bLocked = true;
        }
        catch (...) {
            bLocked = false;
        }
    }
    else if (msTimeOut == 0)
        bLocked = m_hMutex.try_lock();
    else
        bLocked = m_hMutex.try_lock_for(std::chrono::milliseconds(msTimeOut));
#endif // _WIN32
    return bLocked;
}

void StdMutex::unlock()
{
#ifdef _WIN32
    ReleaseMutex(m_hMutex);
#else
    m_hMutex.unlock();
#endif // _WIN32
}


StdAutoMutexLock::StdAutoMutexLock(StdMutex &mutex, long long msTimeOut /*= -1*/)
    : mMutex(mutex), muLockedThreadID(0)
{
    if (msTimeOut < 0) {
        mMutex.lock();
        muLockedThreadID = ProcessUtil::GetCurrentThreadId();
    }
    else if (mMutex.try_lock_for(msTimeOut))
        muLockedThreadID = ProcessUtil::GetCurrentThreadId();
    mbLocked = muLockedThreadID != 0;
}


StdAutoMutexLock::~StdAutoMutexLock()
{
    if (mbLocked)
        mMutex.unlock();
    mbLocked = false;
    muLockedThreadID = 0;
}

bool StdAutoMutexLock::IsLocked() const
{
    return muLockedThreadID == ProcessUtil::GetCurrentThreadId();
}
