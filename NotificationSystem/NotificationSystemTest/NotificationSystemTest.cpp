// NotificationSystemTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "NotificationSystem.h"

static void MainNotificationHandler(NSCharPtr notificationName, NotificationData data, void *pUserData)
{
    printf("Received notification: %s\n", notificationName);
}

#ifdef _WIN32
int main(int argc, const char * argv[])
#else
int mac_main(int argc, const char * argv[])
#endif
{
    InitializeNotification();
    bool bContinue(true);
    while (bContinue)
    {
        printf("1. Register\n2. Send\n3. Exit\n\t");
        char ch(getchar());
        switch (ch)
        {
        case '1':
        case '2':
            printf("\tName: ");
            {
                char name[256];
                scanf_s("%255s", name, (unsigned)(sizeof(name)/sizeof(name[0])));

                int retVal = 0;
                if (ch == '1')
                    retVal = RegisterNotification(name, MainNotificationHandler, NULL);
                else
                    retVal = SendNotification(name, NULL);
                printf("status: %d\n", retVal);
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

