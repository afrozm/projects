#include "NotificationSystemMessageHandler.h"
#include <stdio.h>
#include "NotificationSystem.h"
#include "STLUtils.h"

using namespace STLUtils;

ILibraryExports* ILibraryExports::GetLibraryMessageHandler()
{
    return &NotificationSystemMessageHandler::GetInstance();
}

NotificationSystemMessageHandler& NotificationSystemMessageHandler::GetInstance()
{
    static NotificationSystemMessageHandler sExampleMessageHandler;
    return sExampleMessageHandler;
}

NotificationSystemMessageHandler::NotificationSystemMessageHandler()
{
    ADD_MESSAGE_HANDLER(NotificationSystemMessageHandler, InitializeNotification);
    ADD_MESSAGE_HANDLER(NotificationSystemMessageHandler, FinalizeNotification);
    ADD_MESSAGE_HANDLER(NotificationSystemMessageHandler, SendNotification);
    ADD_MESSAGE_HANDLER(NotificationSystemMessageHandler, RegisterNotification);
    ADD_MESSAGE_HANDLER(NotificationSystemMessageHandler, UnRegisterNotification);
}


NotificationSystemMessageHandler::~NotificationSystemMessageHandler()
{
}

void NotificationSystemMessageHandler::InitializeNotification(const StdString & msg, Paramters & params, StdString & outResult)
{
    int retVal(::InitializeNotification());
    ChangeType(retVal, outResult);
}
void NotificationSystemMessageHandler::FinalizeNotification(const StdString & msg, Paramters & params, StdString & outResult)
{
    int retVal(::FinalizeNotification());
    ChangeType(retVal, outResult);
}
void NotificationSystemMessageHandler::SendNotification(const StdString & msg, Paramters & params, StdString & outResult)
{
    int retVal(::SendNotification(params.GetParamValue(NS_NOTF_NAME).c_str(), &params));
    ChangeType(retVal, outResult);
}
void NotificationSystemMessageHandler::RegisterNotification(const StdString & msg, Paramters & params, StdString & outResult)
{
    void *pUserData(NULL);
    ChangeType(params.GetParamValue("Handler"), pUserData);
    NotificationHandler handler = (NotificationHandler)pUserData;
    ChangeType(params.GetParamValue("UserData"), pUserData);
    int retVal = ::RegisterNotification(params.GetParamValue(NS_NOTF_NAME).c_str(), handler, pUserData);
    ChangeType(retVal, outResult);
}
void NotificationSystemMessageHandler::UnRegisterNotification(const StdString & msg, Paramters & params, StdString & outResult)
{
    void *pUserData(NULL);
    ChangeType(params.GetParamValue("Handler"), pUserData);
    NotificationHandler handler = (NotificationHandler)pUserData;
    int retVal = ::UnRegisterNotification(params.GetParamValue(NS_NOTF_NAME).c_str(), handler);
    ChangeType(retVal, outResult);
}
