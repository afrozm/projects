#pragma once
#include "Paramters.h"

typedef const char* NSCharPtr;
typedef Paramters* NotificationData;
#define NS_NOTF_NAME "_name"

// InitializeNotification
//  Initializes notification system
//  Should be called once per program but it will not have any effect if called many times
int InitializeNotification();
// FinalizeNotification
//  Closes the notification system
//  Should be called strictly one time when process is going to shutdown
//  All the pending notifications are removed
int FinalizeNotification();


int SendNotification(const char * notificationName, NotificationData data);

typedef void(*NotificationHandler)(const char * notificationName, NotificationData data, void *pUserData);

int RegisterNotification(const char * notificationName, NotificationHandler handler, void *pUserData);
int UnRegisterNotification(const char * notificationName, NotificationHandler handler);

