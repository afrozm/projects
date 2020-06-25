// NotificationSystemTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "NotificationSystem.h"
#include "stlutils.h"
#include "CountTimer.h"

static void MainNotificationHandler(NSCharPtr notificationName, NotificationData data, void *pUserData)
{
    const char *chakri = "|/-|\\";
    static CountTimer ct(false, 30);
    static int iChakri(0);
    const unsigned repeatCount(STLUtils::ChangeType<std::string, unsigned>(data->GetParamValue("__repeat", "0")));
    if (repeatCount <= 1)
        printf("Received notification: %s\nnotification data:%s\n", notificationName, data->ToString().c_str());
    else if (ct.UpdateTimeDuration()) {
        printf("\b%c", chakri[iChakri]);
        iChakri = (iChakri + 1) % 5;
    }
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
        printf("1. Register\n2. Send\n3. Unregister\n4. Exit\n\t");
        char ch(getchar());
        switch (ch)
        {
        case '1':
        case '2':
        case '3':
            printf("\tName: ");
            {
                char name[256];
                scanf_s("%255s", name, (unsigned)(sizeof(name)/sizeof(name[0])));
                std::string notName(name);

                int retVal = 0;
                if (ch == '1' || ch == '3') {
                    retVal = ch == '1' ?
                        RegisterNotification(name, MainNotificationHandler, NULL)
                        : UnRegisterNotification(name, MainNotificationHandler);
                }
                else {
                    printf("\nEnter notification data as key=value and EOF to end\n");
                    std::string param;
                    bool bHasData(false);
                    while (1) {
                        scanf_s("%255s", name, (unsigned)(sizeof(name) / sizeof(name[0])));
                        if (!_stricmp(name, "EOF"))
                            break;
                        if (name[0]) {
                            param += name;
                            param += "\r\n";
                            bHasData = true;
                        }
                        else
                        {
                            break;
                        }
                    }
                    Paramters prm(param);
                    NotificationData data(bHasData ? &prm : NULL);
                    const unsigned repeatCount(STLUtils::ChangeType<std::string, unsigned>(prm.GetParamValue("__repeat", "1")));
                    CountTimer ct(false, 1);
                    for (unsigned i=0; i<repeatCount; ++i)
                        retVal = SendNotification(notName.c_str(), data);
                    printf("data:\n%s\n", prm.ToString().c_str());
                    if (repeatCount > 1) {
                        printf("num of requests:%d\n", repeatCount);
                        printf("Time taken:%s\n", ct.GetString().c_str());
                    }
                }
                printf("status: %d\n", retVal);
            }
            break;
        case '4':
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

