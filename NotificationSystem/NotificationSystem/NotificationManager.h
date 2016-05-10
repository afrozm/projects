#pragma once
#include "NotificationSystem.h"
#include "Common.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <list>
#include <map>

class NotificationManager
{
public:
    static NotificationManager& GetInstance();
    virtual int Initialize();
    virtual int Finalize();

    static NotificationData CreateNotificationData();
    static bool SetNotificationData(NotificationData notificationData, NSCharPtr key, NSCharPtr value, bool bOverwrite);
    static NSCharPtr GetNotificationData(NotificationData notificationData, NSCharPtr key);
    static bool ReleaseNotificationData(NotificationData data);

    int RegisterNotification(NSCharPtr notificationName, NotificationHandler handler, void * pUserData, bool bRegister = true);
    int SendNotification(NSCharPtr notificationName, NotificationData data);
protected:
    void WorkerThreadProc();
    virtual int SendNotification(const lstring &notData) = 0;
    NotificationManager();
    virtual ~NotificationManager();
    bool IsFinished();

    bool mbFinished;
    // Worker thread and queue
    std::mutex mMutextQueue;
    std::condition_variable mcvQueue;
    std::list<NotificationData> mQueuedData;
    std::thread *m_pWorkerThread;

    // Notification registration
    struct NotficationHandlerData
    {
        NotficationHandlerData(NotificationHandler inhandler, void *inPUserData) : handler(inhandler), pUserData(inPUserData) {}
        NotificationHandler handler;
        void *pUserData;
    };
    typedef std::map<lstring, std::list<NotficationHandlerData>> MapNotificationHandler;
    MapNotificationHandler mMapNotificationHandler;
    std::mutex mMutextHandler;
};

