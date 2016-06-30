//
//  main.c
//  Sample Console
//
//  Created by Afroz Muzammil on 4/9/15.
//  Copyright (c) 2015 Afroz Muzammil. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface MacMainThreadWrapper : NSObject
@property BOOL keepRunning;
@property int retVal;
@end


static int gargc;
static const char ** gargv;

@implementation MacMainThreadWrapper
-(id)init {
    if (self = [super init]) {
        [NSThread detachNewThreadSelector:@selector(threadMethod)
                                 toTarget:self
                               withObject:nil];
    }
    return self;
}

-(void)threadMethod{
    [NSThread currentThread].name = @"mac_main";
    extern int mac_main(int argc, const char * argv[]);
    self.retVal = mac_main(gargc, gargv);
    self.keepRunning = FALSE;
    [self performSelectorOnMainThread:@selector(wakeUpMainThreadRunloop:) withObject:nil waitUntilDone:NO];
    
}

- (void) wakeUpMainThreadRunloop:(id)arg
{
    // This method is executed on main thread!
    // It doesn't need to do anything actually, just having it run will
    // make sure the main thread stops running the runloop
}

@end

int main(int argc, const char * argv[])
{
    gargc = argc;
    gargv = argv;
    [NSApplication sharedApplication];
    NSRunLoop *theRL = [NSRunLoop currentRunLoop];
    MacMainThreadWrapper *tc = [[MacMainThreadWrapper alloc] init];
    tc.keepRunning = TRUE;
    while (tc.keepRunning && [theRL runMode:NSDefaultRunLoopMode beforeDate:[NSDate distantFuture]]);
    return tc.retVal;
}

