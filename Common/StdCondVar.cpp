

#include "StdCondVar.h"

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

