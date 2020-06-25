//
//  NotificationManagerMac.hpp
//  NotificationSystem
//
//  Created by Afroz Muzammil on 28/6/16.
//  Copyright © 2016 Afroz Muzammil. All rights reserved.
//

#ifndef NotificationManagerMac_hpp
#define NotificationManagerMac_hpp

#include "NotificationManager.h"

class NotificationManagerMac : public NotificationManager {
public:
    virtual int Initialize();
    virtual int Finalize();

protected:
    int SendNotification(const std::string &notData);
};

#endif /* NotificationManagerMac_hpp */
