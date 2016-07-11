
#pragma once

#include <condition_variable>

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

