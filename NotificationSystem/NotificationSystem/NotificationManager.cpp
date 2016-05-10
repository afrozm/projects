#include "stdafx.h"
#include "NotificationManager.h"
#include "Property.h"
#include "StringUtils.h"
#ifdef _WIN32
#include "NotificationManagerWin.h"
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
            lstring notificationName(GetNotificationData(data, _T("_name")));
            std::list<NotficationHandlerData> handlers;
            if (!notificationName.empty())
            {
                std::lock_guard<std::mutex> guard(mMutextHandler);
                for (auto &hander : mMapNotificationHandler) {
                    if (StringUtils::WildCardMatch(hander.first, notificationName))
                        handlers.insert(handlers.end(), hander.second.begin(), hander.second.end());
                }
            }
            for (auto &handler : handlers)
                handler.handler(notificationName.c_str(), data, handler.pUserData);
            // delete data
            ReleaseNotificationData(data);
        }
    }
}

NotificationManager::NotificationManager()
    : mbFinished(false), m_pWorkerThread(NULL)
{
}


NotificationManager::~NotificationManager()
{
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
                ReleaseNotificationData(mQueuedData.front());
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

NotificationData NotificationManager::CreateNotificationData()
{
    return new Property;
}

bool NotificationManager::SetNotificationData(NotificationData notificationData, NSCharPtr key, NSCharPtr value, bool bOverwrite)
{
    bool bSet(NULL != notificationData);
    if (bSet) {
        Property *pProperty((Property *)notificationData);
        if (NULL == key)
            pProperty->RemoveAll();
        else if (NULL == value)
            pProperty->RemoveKey(key);
        else
            pProperty->SetValue(key, value, bOverwrite);
    }
    return bSet;
}

NSCharPtr NotificationManager::GetNotificationData(NotificationData notificationData, NSCharPtr key)
{
    if (NULL != notificationData && NULL != key)
        return ((Property*)notificationData)->GetValue(key).c_str();
    return NULL;
}

bool NotificationManager::ReleaseNotificationData(NotificationData data)
{
    bool bRelease(data != NULL);
    if (bRelease)
        delete (Property*)data;
    return bRelease;
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
    int retVal(2);
    lstring strData;
    {
        PropertySet ps;
        Property &prop(ps.GetMapProperty()[_T("data")]);
        if (data)
            prop = *(Property*)data;
        prop.SetValue(_T("_name"), notificationName);
        PropertySetStreamer pss;
        pss.SetPropertySetStream(ps);
        pss.WrtieToString(strData);
        strData += _T("#end");
    }
    return SendNotification(strData);
}
