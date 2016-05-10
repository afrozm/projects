// NotificationSystemTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "NotificationSystem.h"

static void MainNotificationHandler(NSCharPtr notificationName, NotificationData data, void *pUserData)
{
    _tprintf(_T("Received notification: %s\n"), notificationName);
}

int main()
{
    InitializeNotification();
    bool bContinue(true);
    while (bContinue)
    {
        _tprintf(_T("1. Register\n2. Send\n3. Exit\n\t"));
        TCHAR ch(_getche());
        switch (ch)
        {
        case '1':
        case '2':
            _tprintf(_T("\tName: "));
            {
                TCHAR name[256];
                _tscanf_s(_T("%256s"), name, (unsigned int)_countof(name));

                int retVal = 0;
                if (ch == '1')
                    retVal = RegisterNotification(name, MainNotificationHandler, NULL);
                else
                    retVal = SendNotification(name, NULL);
                _tprintf(_T("status: %d\n"), retVal);
            }
            break;
        case '3':
        case 27:
            bContinue = false;
            break;
        default:
            break;
        }
    }
    FinalizeNotification();
    return 0;
}

