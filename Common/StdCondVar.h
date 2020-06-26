
#pragma once

#include <condition_variable>
#include <mutex>

class StdConditionVariable
{
public:
    // bSingnalAllWaiting if true will send signal to all threads waiting for signal
    void Signal(bool bSingnalAllWaiting = false);
    // msTimeOut: timeout is milli seconds, negative value means to wait for infinite time
    // returns true if Signaled otherwise false if timeout occurred
    bool WaitForSignal(long long msTimeOut = -1);

private:
    std::condition_variable mConditionVariable; // Condition variable
    std::mutex mConditionVariableMutex;         // Used by wait functions
};

class StdMutex {
public:
    StdMutex();
    StdMutex(const StdMutex &) = delete; // no copy
    StdMutex& operator=(const StdMutex &other) = delete;  // no copy
    ~StdMutex();
    bool lock();
    bool try_lock_for(long long msTimeOut = -1);
    void unlock();
private:
#ifdef _WIN32
    HANDLE m_hMutex;
#else
    std::recursive_timed_mutex m_hMutex;
#endif // DEBUG
};

class StdAutoMutexLock {
public:
    StdAutoMutexLock(StdMutex &mutex, long long msTimeOut = -1);
    ~StdAutoMutexLock();
    bool IsLocked() const;
private:
    StdMutex &mMutex;
    long long muLockedThreadID;
    bool mbLocked;
};

