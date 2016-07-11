//
//  UrlTestRequestURLData.m
//  UrlTest
//
//  Created by Afroz Muzammil on 1/7/16.
//  Copyright © 2016 Afroz Muzammil. All rights reserved.
//

#include <string>

#import <Foundation/Foundation.h>

std::string GetURLData(const std::string &inRequestURL)
{
    
    std::string responseString;
    __block NSString *reponseStr = nil;
    

    @autoreleasepool {
        NSURLSession *delegateFreeSession = [NSURLSession sessionWithConfiguration:nil];
        
        [[delegateFreeSession dataTaskWithURL: [NSURL URLWithString: [NSString stringWithUTF8String: inRequestURL.c_str()]]
                            completionHandler:^(NSData *data, NSURLResponse *response,
                                                NSError *error) {
                                NSMutableString *repStr = [NSMutableString stringWithCapacity:4096];
                                [repStr appendFormat:@"Response: \n%@\n with error: \n%@.\n", response, error ];
                                [repStr appendFormat:@"\nSTART DATA:\n%@\nEND DATA\n",
                                                        [[NSString alloc] initWithData: data
                                                        encoding: NSUTF8StringEncoding]];
                                reponseStr = [[NSString alloc] initWithString: repStr];
                            }] resume];
        while (reponseStr == nil)
            [NSThread sleepForTimeInterval: 0.1f];
        [NSThread sleepForTimeInterval: 0.1f];
        const char *utfStr = [reponseStr UTF8String];
        responseString.assign(utfStr);
    }
    return responseString;
}