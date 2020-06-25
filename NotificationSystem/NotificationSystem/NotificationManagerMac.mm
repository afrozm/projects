//
//  NotificationManagerMac.cpp
//  NotificationSystem
//
//  Created by Afroz Muzammil on 28/6/16.
//  Copyright © 2016 Afroz Muzammil. All rights reserved.
//

#include "NotificationManagerMac.h"
#import <Cocoa/Cocoa.h>

#define NS_DS_MSG_NAME @"comAutodeskAdsNotMgrMsg"

@interface NotificationManagerMacNotificationHandler : NSObject

@end

@implementation NotificationManagerMacNotificationHandler
NotificationManagerMac *m_pNotificationManagerMac;

- (id)initWithNotificationManagerMac:(NotificationManagerMac*) pNotificationManagerMac {
    if (self = [super init]) {
        m_pNotificationManagerMac = pNotificationManagerMac;
        [[NSDistributedNotificationCenter defaultCenter] addObserver:self selector:@selector(handleNotificationMessage:) name: NS_DS_MSG_NAME object:nil];
    }
    return self;
}

- (void) handleNotificationMessage: (NSNotification*) note
{
    std::string notData;
    if (nil != note.object &&
        [note.object isKindOfClass:[NSString class]])
        notData = [(NSString *)note.object UTF8String];

    m_pNotificationManagerMac->AddNotificationToQueue(notData);
}

-(void) finalizeExit {
}

@end

static NotificationManagerMacNotificationHandler *sNotificationManagerMacNotificationHandler = nil;


int NotificationManagerMac::Initialize()
{
    if (!sNotificationManagerMacNotificationHandler)
        sNotificationManagerMacNotificationHandler = [[NotificationManagerMacNotificationHandler alloc] initWithNotificationManagerMac:this];
    return NotificationManager::Initialize();
}
int NotificationManagerMac::Finalize()
{
    if (sNotificationManagerMacNotificationHandler) {
        [sNotificationManagerMacNotificationHandler finalizeExit];
        sNotificationManagerMacNotificationHandler = nil;
    }
    return NotificationManager::Finalize();
}



int NotificationManagerMac::SendNotification(const std::string &notData)
{
    @autoreleasepool
    {
        [[NSDistributedNotificationCenter defaultCenter] postNotificationName:NS_DS_MSG_NAME
                                                                       object:[NSString stringWithUTF8String:notData.c_str()]
                                                                     userInfo:nil deliverImmediately:YES];
    }
    return 0;
}
