#include "stdafx.h"
#include "NotificationManager.h"
#ifdef _WIN32
#include "NotificationManagerWin.h"
#elif defined(__APPLE__)
#include "NotificationManagerMac.h"
#endif

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
                    if (!lstrcmpi(hander.first.c_str(), notificationName.c_str()))
                        handlers.insert(handlers.end(), hander.second.begin(), hander.second.end());
                }
            }
            for (auto &handler : handlers)
                handler.handler(notificationName.c_str(), data, handler.pUserData);
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
                    if (!bRegister) {
                        cit->second.erase(dit);
                        bExists = false;
                        break;
                    }
                    else
                        dit->pUserData = pUserData;
                    bRegister = false;
                }
            }
        }
        if (bRegister) {
            if (bExists)
                cit->second.push_back(NotficationHandlerData(handler, pUserData));
            else
                mMapNotificationHandler[notificationName].push_back(NotficationHandlerData(handler, pUserData));
        }
    }
    return retVal;
}

int NotificationManager::SendNotification(NSCharPtr notificationName, NotificationData data)
{
    std::string strData;
    data->SetParamValue(NS_NOTF_NAME, notificationName);
    strData = data->ToString();
    return SendNotification(strData);
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
