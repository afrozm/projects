//
//  MacUtil.cpp
//  key2
//
//  Created by Afroz Muzammil on 25/6/20.
//  Copyright © 2020 Autodesk. All rights reserved.
//

#include "MacUtil.h"
#import <Cocoa/Cocoa.h>
#import <AppKit/AppKit.h>
#include <sstream>
#include <codecvt>

namespace MacUtil {
std::string WstringToString(const std::wstring &inSource)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.to_bytes(inSource);
}
bool HasPermissionForAccess()
{
    @autoreleasepool {
    // See if we have accessibility permissions, and if not, prompt the user to
    // visit System Preferences.
        NSDictionary *options = @{(__bridge id)kAXTrustedCheckOptionPrompt: @YES};
    Boolean appHasPermission = AXIsProcessTrustedWithOptions(
                                 (__bridge CFDictionaryRef)options);
    return appHasPermission;
    }
}
class MacUtilManager {
public:
    static MacUtilManager& Get() {
        static MacUtilManager sFrontWindowManager;
        return sFrontWindowManager;
    }
    bool GetFrontWindowTitle(std::string &outTitle) {
        bool bTitleChanged = false;
        @autoreleasepool {
        // Get the process ID of the frontmost application.
        NSRunningApplication* app = [[NSWorkspace sharedWorkspace]
                                      frontmostApplication];
        pid_t pid = [app processIdentifier];
        if (!HasPermissionForAccess())
           return bTitleChanged; // we don't have accessibility permissions

        // Get the accessibility element corresponding to the frontmost application.
        AXUIElementRef appElem = AXUIElementCreateApplication(pid);
        if (!appElem) {
          return bTitleChanged;
        }

        // Get the accessibility element corresponding to the frontmost window
        // of the frontmost application.
        AXUIElementRef window = NULL;
        if (AXUIElementCopyAttributeValue(appElem,
              kAXFocusedWindowAttribute, (CFTypeRef*)&window) != kAXErrorSuccess) {
          CFRelease(appElem);
          return bTitleChanged;
        }

        // Finally, get the title of the frontmost window.
        CFStringRef title = NULL;
        AXError result = AXUIElementCopyAttributeValue(window, kAXTitleAttribute,
                           (CFTypeRef*)&title);

        // At this point, we don't need window and appElem anymore.
        CFRelease(window);
        CFRelease(appElem);

        if (result != kAXErrorSuccess) {
          // Failed to get the window title.
          return bTitleChanged;
        }
        // Success! Now, do something with the title, e.g. copy it somewhere.
            NSString *aNSString = (__bridge NSString *)title;
            outTitle = [[app localizedName] UTF8String];
            outTitle += " - ";
            outTitle += [aNSString UTF8String];
        // Once we're done with the title, release it.
        CFRelease(title);
            bTitleChanged = oldStringTitle != outTitle;
            oldStringTitle = outTitle;
        }
        return bTitleChanged;
    }
    bool GetClipboardString(std::string &outString) {
        @autoreleasepool {
            NSPasteboard *pasteboard = [NSPasteboard generalPasteboard];
            if (changeCount == pasteboard.changeCount)
                return false;
            changeCount = pasteboard.changeCount;
            NSString *string = [pasteboard stringForType:NSPasteboardTypeString];
            if (string == Nil)
                return false;
            outString = [string UTF8String];
            
            return true;
        }
    }
private:
    MacUtilManager() {
        
    }
    std::string oldStringTitle;
    //std::string oldStringClipboard;
    NSInteger changeCount = 0;
};

bool GetFrontWindowTitle(std::string &outTitle)
{
    return MacUtilManager::Get().GetFrontWindowTitle(outTitle);
}
bool GetClipboardString(std::string &outString)
{
   return MacUtilManager::Get().GetClipboardString(outString);
}
}
