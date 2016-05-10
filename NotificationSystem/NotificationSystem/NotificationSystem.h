#pragma once

#ifdef _WIN32
typedef const wchar_t* NSCharPtr;
#else
typedef const char* NSCharPtr;
#endif // _WIN32

typedef void* NotificationData;

extern "C" {

    // InitializeNotification
    //  Initializes notification system
    //  Should be called once per program but it will not have any effect if called many times
    int InitializeNotification();
    // FinalizeNotification
    //  Closes the notification system
    //  Should be called strictly one time when process is going to shutdown
    //  All the pending notifications are removed
    int FinalizeNotification();

    NotificationData CreateNotificationData();
    bool SetNotificationData(NotificationData notificationData, NSCharPtr key, NSCharPtr value, bool bOverwrite);
    NSCharPtr GetNotificationData(NotificationData notificationData, NSCharPtr key);
    bool ReleaseNotificationData(NotificationData data);

    int SendNotification(NSCharPtr notificationName, NotificationData data);

    typedef void(*NotificationHandler)(NSCharPtr notificationName, NotificationData data, void *pUserData);

    int RegisterNotification(NSCharPtr notificationName, NotificationHandler handler, void *pUserData);
    int UnRegisterNotification(NSCharPtr notificationName, NotificationHandler handler);

}