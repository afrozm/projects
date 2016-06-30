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

#include "ILibraryExports.h"
#include <stdio.h>

#ifdef _WIN32
// windows symbols are exported from .def file
#define DLT_EXPORT
#else
// symbols exported from visibility default and from exp file
#define DLT_EXPORT __attribute__((visibility("default")))
#endif

extern "C" {
    DLT_EXPORT int Initialize_NotificationSystem()
    {
        ILibraryExports *pHandler(ILibraryExports::GetLibraryMessageHandler());
        if (pHandler)
            return pHandler->Initialize();
        return -1;
    }
    DLT_EXPORT int Finalize_NotificationSystem()
    {
        ILibraryExports *pHandler(ILibraryExports::GetLibraryMessageHandler());
        if (pHandler)
            return pHandler->Finalize();
        return -1;
    }
    DLT_EXPORT const char* ProcessMessage_NotificationSystem(const char *msg, const char *msgData)
    {
        if (msg == NULL)
            return NULL;
        ILibraryExports *pHandler(ILibraryExports::GetLibraryMessageHandler());
        if (pHandler) {
            if (NULL == msgData)
                msgData = "";
            ILibraryExports::StdString outResult;
            bool bHandled((pHandler->ProcessMessage(msg, msgData, outResult)));
#if _DEBUG
            if (!bHandled)
                printf("No message handler for %s\n", msg);
#endif
            UNREFERENCED_PARAMETER(bHandled);
            if (outResult.length() == 0)
                return NULL;
            const size_t dataLen(outResult.length() + 1);
            char *outData = new char[dataLen];
            strcpy_s(outData, dataLen, outResult.c_str());
            return outData;
        }
        return NULL;
    }
    DLT_EXPORT void FreeMessage_NotificationSystem(const char *msgResult)
    {
        if (msgResult != NULL)
            delete[] msgResult;
    }
}