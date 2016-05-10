// NotificationSystem.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "NotificationManager.h"

int InitializeNotification()
{
    return NotificationManager::GetInstance().Initialize();
}

int FinalizeNotification()
{
    return NotificationManager::GetInstance().Finalize();
}

NotificationData CreateNotificationData()
{
    return NotificationManager::CreateNotificationData();
}

bool SetNotificationData(NotificationData notificationData, NSCharPtr key, NSCharPtr value, bool bOverwrite)
{
    return NotificationManager::SetNotificationData(notificationData, key, value, bOverwrite);
}

NSCharPtr GetNotificationData(NotificationData notificationData, NSCharPtr key)
{
    return NotificationManager::GetNotificationData(notificationData, key);
}

bool ReleaseNotificationData(NotificationData data)
{
    return NotificationManager::ReleaseNotificationData(data);
}

int SendNotification(NSCharPtr notificationName, NotificationData data)
{
    InitializeNotification();
    return NotificationManager::GetInstance().SendNotification(notificationName, data);
}

int RegisterNotification(NSCharPtr notificationName, NotificationHandler handler, void * pUserData)
{
    InitializeNotification();
    return NotificationManager::GetInstance().RegisterNotification(notificationName, handler, pUserData, true);
}

int UnRegisterNotification(NSCharPtr notificationName, NotificationHandler handler)
{
    return NotificationManager::GetInstance().RegisterNotification(notificationName, handler, NULL, false);
}
