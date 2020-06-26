// alterkeys.c
// http://osxbook.com
//
// Complile using the following command line:
//     gcc -Wall -o alterkeys alterkeys.c -framework ApplicationServices
//
// You need superuser privileges to create the event tap, unless accessibility
// is enabled. To do so, select the "Enable access for assistive devices"
// checkbox in the Universal Access system preference pane.

#include <ApplicationServices/ApplicationServices.h>
#include <CoreGraphics/CGEvent.h>
#include "MacUtil.h"
#include "Logger.h"

CFMachPortRef      eventTap;

static std::string GetModifiers(CGEventFlags flagsP) {
    std::string modifiers;
    CGEventFlags flags[] = {
        kCGEventFlagMaskCommand,
        kCGEventFlagMaskAlternate,
        kCGEventFlagMaskControl,
        kCGEventFlagMaskShift,
        kCGEventFlagMaskSecondaryFn
    };
    const char *strFlags[] = {
        "⌘",
        "⌥",
        "⌃",
        "⇧",
        "Fn"
    };
    for (int i=0; i<sizeof(flags)/sizeof(flags[0]); ++i) {
        if ((flagsP & flags[i]) == flags[i]) {
            modifiers += strFlags[i];
        }
    }
    if (!modifiers.empty())
        modifiers += " ";
    return modifiers;
}

// This callback will be invoked every time there is a keystroke.
//
CGEventRef
myCGEventCallback(CGEventTapProxy proxy, CGEventType type,
                  CGEventRef event, void *refcon)
{
    if (type == kCGEventTapDisabledByTimeout) {
        CGEventTapEnable(eventTap, true);
        return event;
    }
    // Paranoid sanity check.
    if ((type != kCGEventKeyDown))
        return event;
    
    // The incoming keycode.
    CGKeyCode keycode = (CGKeyCode)CGEventGetIntegerValueField(
                                                               event, kCGKeyboardEventKeycode);
    UniCharCount actualStringLength;
    UniChar chars[3] = {0};
    wchar_t wchars[3] = {0};
    CGEventKeyboardGetUnicodeString(event, 3, &actualStringLength, chars);
    wchars[0] = chars[0];
    wchars[1] = chars[1];
    wchars[2] = chars[2];
    std::string str = MacUtil::WstringToString(wchars);
    std::string modifiers = GetModifiers(CGEventGetFlags(event));

    if (chars[0] < 32 && chars[1] == 0) {
        LOGGER_LOG("%skeycode=%d, char=%s", modifiers.c_str(), keycode, str.c_str());
    }
    else {
        Logger::GetInstance()->DisableTempNewLine();
        LOGGER_LOG("%s", str.c_str());
    }
    
    
    
    
    if (keycode == 53)
        CFRunLoopStop(CFRunLoopGetCurrent());
    
    //    // Swap 'a' (keycode=0) and 'z' (keycode=6).
    //    if (keycode == (CGKeyCode)0)
    //        keycode = (CGKeyCode)6;
    //    else if (keycode == (CGKeyCode)6)
    //        keycode = (CGKeyCode)0;
    //
    //    // Set the modified keycode field in the event.
    //    CGEventSetIntegerValueField(
    //                                event, kCGKeyboardEventKeycode, (int64_t)keycode);
    
    // We must return the event for it to be useful.
    return event;
}
void ActiveSenseTimerCallback(CFRunLoopTimerRef timer, void *info)
{
    std::string str;
    if (MacUtil::GetFrontWindowTitle(str))
        LOGGER_LOG("Front App: %s", str.c_str());
    if (MacUtil::GetClipboardString(str))
        LOGGER_LOG("Clipboard text: %s", str.c_str());
    
}
static void InitTimer()
{
    CFTimeInterval TIMER_INTERVAL = 0.3;
    CFAbsoluteTime FireTime = CFAbsoluteTimeGetCurrent() + TIMER_INTERVAL;
    CFRunLoopTimerRef mTimer = CFRunLoopTimerCreate(kCFAllocatorDefault,
                                                    FireTime,
                                                    TIMER_INTERVAL, 0, 0,
                                                    ActiveSenseTimerCallback,
                                                    NULL);
    CFRunLoopAddTimer(CFRunLoopGetCurrent(), mTimer, kCFRunLoopCommonModes);
}
//CGEventRef  MyCGEventTapCallBack(CGEventTapProxy  proxy,
//CGEventType type, CGEventRef  event, void * __nullable userInfo)
//{
//    return event;
//}
class ConsoleLogger : public LogTarget
{
public:
    ConsoleLogger() : LogTarget("console") {
        Logger *pLogger = Logger::GetInstance();
        pLogger->DisableLogSummary();
        pLogger->DisableWriteLogLevel();
        pLogger->DisableWritePidAndTid();
    }
    virtual void Log(const char *logMessage) override
    {
        printf("%s", logMessage);
        fflush(stdout);
    }
    ~ConsoleLogger()
    {
        SM_CALL_INSTANCE_METHOD(Logger, LogSummary, false);
    }
private:
};
int
main(void)
{
    ConsoleLogger cl;
    CGEventMask        eventMask;
    CFRunLoopSourceRef runLoopSource;
    if (!MacUtil::HasPermissionForAccess()) {
        LOGGER_LOG_ERROR("No permission");
        return 2;
    }
    // Create an event tap. We are interested in key presses.
    eventMask = ((1 << kCGEventKeyDown) | (1 << kCGEventKeyUp)); // kCGEventMaskForAllEvents;
    //eventTap = CGEventTapCreate(kCGSessionEventTap, kCGHeadInsertEventTap, kCGEventTapOptionDefault,
    //                 kCGEventMaskForAllEvents, MyCGEventTapCallBack, nullptr);
    eventTap = CGEventTapCreate(kCGSessionEventTap, kCGTailAppendEventTap, kCGEventTapOptionListenOnly,
                                eventMask, myCGEventCallback, NULL);
    if (!eventTap) {
        LOGGER_LOG_ERROR("failed to create event tap");
        exit(1);
    }
    
    // Create a run loop source.
    runLoopSource = CFMachPortCreateRunLoopSource(
                                                  kCFAllocatorDefault, eventTap, 0);
    
    // Add to the current run loop.
    CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource,
                       kCFRunLoopCommonModes);
    
    // Enable the event tap.
    CGEventTapEnable(eventTap, true);
    InitTimer();
    // Set it all running.
    CFRunLoopRun();
    
    // In a real program, one would have arranged for cleaning up.
    CGEventTapEnable(eventTap, false);
    
    exit(0);
}
