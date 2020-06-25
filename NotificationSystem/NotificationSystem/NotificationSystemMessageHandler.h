#pragma once
#include "LibraryMessageHandler.h"

// Rename this NotificationSystemMessageHandler class and start handling messages here
class NotificationSystemMessageHandler :
    public LibraryMessageHandler
{
public:
    static NotificationSystemMessageHandler& GetInstance();

private:
    NotificationSystemMessageHandler();
    ~NotificationSystemMessageHandler();

    void InitializeNotification(const StdString &msg, Paramters &params, StdString &outResult);
    void FinalizeNotification(const StdString &msg, Paramters &params, StdString &outResult);
    void SendNotification(const StdString &msg, Paramters &params, StdString &outResult);
    void RegisterNotification(const StdString &msg, Paramters &params, StdString &outResult);
    void UnRegisterNotification(const StdString &msg, Paramters &params, StdString &outResult);
};

