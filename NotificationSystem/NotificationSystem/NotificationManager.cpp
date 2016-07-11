#include "stdafx.h"
#include "NotificationManager.h"
#ifdef _WIN32
#include "NotificationManagerWin.h"
#elif defined(__APPLE__)
#include "NotificationManagerMac.h"
#endif
#include "ProcessUtil.h"
#include "stlutils.h"
#include <ctime>
#include "StdCondVar.h"

void NotificationManager::WorkerThreadProc()
{
    while (!IsFinished()) {
        NotificationData data(NULL);
        {
            std::unique_lock<std::mutex> lk(mMutextQueue);
            mcvQueue.wait(lk, [this] {return !mQueuedData.empty() || IsFinished();});
            if (!mQueuedData.empty()) {
                data = mQueuedData.front();
                mQueuedData.pop_front();
            }
            lk.unlock();
        }
        if (NULL != data) {
            // process data
            const std::string &notificationName(data->GetParamValue(NS_NOTF_NAME));
            std::list<NotficationHandlerData> handlers;
            if (!notificationName.empty())
            {
                std::lock_guard<std::mutex> guard(mMutextHandler);
                for (auto &hander : mMapNotificationHandler) {
                    if (!_stricmp(hander.first.c_str(), notificationName.c_str()))
                        handlers.insert(handlers.end(), hander.second.begin(), hander.second.end());
                }
            }
            for (auto &handler : handlers)
                handler.handler(notificationName.c_str(), data, handler.pUserData);
            // check if reply requested
            if (handlers.size() > 0)
            {
                std::string reply(data->GetParamValue(NS_NOTF_REPLY));
                if (_stricmp(reply.c_str(), notificationName.c_str())) {
                    data->RemoveParam(NS_NOTF_SYNC);
                    SendNotification(reply.c_str(), data);
                }
            }
            delete data;
        }
    }
}

NotificationManager::NotificationManager()
    : mbFinished(false), m_pWorkerThread(NULL)
{
}


NotificationManager::~NotificationManager()
{
    Finalize();
}


bool NotificationManager::IsFinished()
{
    return mbFinished;
}

NotificationManager& NotificationManager::GetInstance()
{
    static
#ifdef _WIN32
        NotificationManagerWin
#else
    NotificationManagerMac
#endif // _WIN32
        sNotificationManager;
    return sNotificationManager;
}

int NotificationManager::Initialize()
{
    int retVal(0);
    if (m_pWorkerThread == NULL) {
        mbFinished = false;
        m_pWorkerThread = new std::thread(&NotificationManager::WorkerThreadProc, this);
    }
    return retVal;
}

int NotificationManager::Finalize()
{
    int retVal(0);
    if (!mbFinished && m_pWorkerThread != NULL) {
        mbFinished = true;
        {
            std::lock_guard<std::mutex> lk(mMutextQueue);
            while (!mQueuedData.empty())
            {
                delete mQueuedData.front();
                mQueuedData.pop_front();
            }
        }
        mcvQueue.notify_all();
        m_pWorkerThread->join();
        delete m_pWorkerThread;
        m_pWorkerThread = NULL;
    }
    return retVal;
}

int NotificationManager::RegisterNotification(NSCharPtr notificationName, NotificationHandler handler, void * pUserData, bool bRegister /* = true */)
{
    if (notificationName == NULL || *notificationName == 0)
        return -1;
    std::lock_guard<std::mutex> guard(mMutextHandler);
    int retVal(0);
    if (handler == NULL && !bRegister)
        mMapNotificationHandler.erase(notificationName);
    else {
        auto cit(mMapNotificationHandler.find(notificationName));
        bool bExists(cit != mMapNotificationHandler.end());
        if (bExists) {
            for (auto dit(cit->second.begin()); dit != cit->second.end(); ++dit) {
                if (dit->handler == handler) {
                    if (!bRegister)
                        cit->second.erase(dit);
                    else
                        dit->pUserData = pUserData;
                    bRegister = false;
                    break;
                }
            }
        }
        if (bRegister) {
            if (bExists)
                cit->second.push_back(NotficationHandlerData(handler, pUserData));
            else
                mMapNotificationHandler[notificationName].push_back(NotficationHandlerData(handler, pUserData));
        }
        else if (bExists && cit->second.empty())
            mMapNotificationHandler.erase(notificationName);
    }
    return retVal;
}

struct NM_SyncCallData
{
    StdConditionVariable cv;
    Paramters recveiedData;
    NM_SyncCallData(NotificationManager *pMgr, NotificationData data);
    ~NM_SyncCallData();
private:
    NotificationManager *pNotMgr;
    NotificationData data;
    bool bIsSync;
    std::string replyNot, notName;
};

static void NotificationHandler_SyncCall(const char * notificationName, NotificationData data, void *pUserData)
{
    data->SetParamValue(NS_NOTF_REPLIED, "1");
    ((NM_SyncCallData *)pUserData)->recveiedData = *data;
    ((NM_SyncCallData *)pUserData)->cv.Signal();
}
NM_SyncCallData::NM_SyncCallData(NotificationManager *pMgr, NotificationData indata)
    : pNotMgr(pMgr), data(indata)
{
    bIsSync = data->GetParamValue(NS_NOTF_SYNC) == "1";
    if (bIsSync) {
        notName = data->GetParamValue(NS_NOTF_NAME);
        replyNot = data->GetParamValue(NS_NOTF_NAME);
        // make reply as - message.pid.tid.time
        std::string strId;
        using namespace STLUtils;
        // pid
        ChangeType(ProcessUtil::GetCurrentProcessId(), strId);
        replyNot += ".reply." + strId + ".";
        // tid - thread id
        ChangeType(ProcessUtil::GetCurrentThreadId(), strId);
        replyNot += strId + ".";
        // time stamp
        ChangeType(std::time(nullptr), strId);
        replyNot += strId;
        data->SetParamValue(NS_NOTF_REPLY, replyNot);
        // registry reply
        pNotMgr->RegisterNotification(replyNot.c_str(), NotificationHandler_SyncCall, this);
    }
}
NM_SyncCallData::~NM_SyncCallData()
{
    if (bIsSync) {
        unsigned sleepMilliSec(0);
        STLUtils::ChangeType(data->GetParamValue("_timeout"), sleepMilliSec);
        if (sleepMilliSec == 0 || sleepMilliSec > 60 * 1000)
            sleepMilliSec = 300;
        // wait for .3 sec to receive the reply
        cv.WaitForSignal(sleepMilliSec);
        // un-register reply
        pNotMgr->RegisterNotification(replyNot.c_str(), NULL, NULL, false);
        *data += recveiedData;
        if (data->GetParamValue(NS_NOTF_REPLIED).empty()) {
            data->SetParamValue("_error", "timeout");
            ProcessUtil::Sleep(50); // Sleep extra 50ms in case pending notification
        }
        data->RemoveParam(NS_NOTF_REPLY);
        data->SetParamValue(NS_NOTF_NAME, notName);
    }
}
int NotificationManager::SendNotification(NSCharPtr notificationName, NotificationData data)
{
    std::string strData;
    data->SetParamValue(NS_NOTF_NAME, notificationName);
    NM_SyncCallData synCallData(this, data);
    strData = data->ToString();
    int retVal = SendNotification(strData);
    return retVal;
}

void NotificationManager::AddNotificationToQueue(const std::string &notData)
{
    Paramters *notDataParams = new Paramters(notData);
    {
        std::lock_guard<std::mutex> lk(mMutextQueue);
        mQueuedData.push_back(notDataParams);
    }
    mcvQueue.notify_one();
}
