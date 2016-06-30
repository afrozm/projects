// NotificationSystem.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "NotificationSystem.h"
#include "LibraryMessageSender.h"
#include "STLUtils.h"

LibraryMessageSender sLibraryMessageSender;

using namespace STLUtils;

int InitializeNotification()
{
    int retVal(0);
    if (!sLibraryMessageSender.IsLoaded()) {
        sLibraryMessageSender.SetFunctionNameSuffix("_NotificationSystem");
        sLibraryMessageSender.SetLibraryName(DEFAULT_LIB_PREFIX "NotificationSystem" DEFAULT_LIB_EXTN);
        std::string result;
        sLibraryMessageSender.ProcessMessageWithArguments("InitializeNotification", result);
        ChangeType(result, retVal);
    }
    return retVal;
}

int FinalizeNotification()
{
    int retVal(0);
    if (sLibraryMessageSender.IsLoaded()) {
        std::string result;
        sLibraryMessageSender.ProcessMessageWithArguments("FinalizeNotification", result);
        ChangeType(result, retVal);
    }
    return retVal;
}

int SendNotification(NSCharPtr notificationName, NotificationData data)
{
    if (notificationName == NULL || *notificationName == 0)
        return 1;
    InitializeNotification();
    int retVal(0);
    Paramters params;
    if (data == NULL)
        data = &params;
    std::string result;
    data->SetParamValue(NS_NOTF_NAME, notificationName);
    sLibraryMessageSender.ProcessMessage("SendNotification", data->ToString() ,result);
    ChangeType(result, retVal);
    return retVal;
}

int RegisterNotification(NSCharPtr notificationName, NotificationHandler handler, void * pUserData)
{
    InitializeNotification();
    int retVal(0);
    Paramters params;
    std::string result;
    params.SetParamValue(NS_NOTF_NAME, notificationName);
    ChangeType((void*)handler, result);
    params.SetParamValue("Handler", result);
    ChangeType(pUserData, result);
    params.SetParamValue("UserData", result);
    sLibraryMessageSender.ProcessMessage("RegisterNotification", params.ToString() ,result);
    ChangeType(result, retVal);
    return retVal;
}

int UnRegisterNotification(NSCharPtr notificationName, NotificationHandler handler)
{
    Paramters params;
    std::string result;
    params.SetParamValue(NS_NOTF_NAME, notificationName);
    ChangeType((void*)handler, result);
    params.SetParamValue("Handler", result);
    sLibraryMessageSender.ProcessMessage("UnRegisterNotification", params.ToString() ,result);
    int retVal(0);
    ChangeType(result, retVal);
    return retVal;
}
