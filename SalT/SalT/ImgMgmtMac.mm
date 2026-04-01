#include "ImgMgmt.h"
#include "Logger.h"
#import <Cocoa/Cocoa.h>
#import <CoreGraphics/CoreGraphics.h>

bool HasScreenRecordingPermission()
{
	if (!CGPreflightScreenCaptureAccess()) {
		CGRequestScreenCaptureAccess();
		return false;
	}
	return true;
}

bool CaptureAndSaveWindowImage(const Path &filePath)
{
	@autoreleasepool {
		NSRunningApplication *frontApp = [[NSWorkspace sharedWorkspace] frontmostApplication];
		if (!frontApp) {
			LOGGER_LOG("CaptureWindow: no front app");
			return false;
		}

		pid_t pid = [frontApp processIdentifier];

		CFArrayRef windowList = CGWindowListCopyWindowInfo(
			kCGWindowListOptionOnScreenOnly | kCGWindowListExcludeDesktopElements,
			kCGNullWindowID);
		if (!windowList) {
			LOGGER_LOG("CaptureWindow: no window list");
			return false;
		}

		CGWindowID windowID = 0;
		for (CFIndex i = 0; i < CFArrayGetCount(windowList); ++i) {
			CFDictionaryRef info = (CFDictionaryRef)CFArrayGetValueAtIndex(windowList, i);
			CFNumberRef pidRef = (CFNumberRef)CFDictionaryGetValue(info, kCGWindowOwnerPID);
			int windowPID = 0;
			CFNumberGetValue(pidRef, kCFNumberIntType, &windowPID);
			if (windowPID == pid) {
				CFNumberRef windowIDRef = (CFNumberRef)CFDictionaryGetValue(info, kCGWindowNumber);
				int wid = 0;
				CFNumberGetValue(windowIDRef, kCFNumberIntType, &wid);
				windowID = (CGWindowID)wid;
				break;
			}
		}
		CFRelease(windowList);

		if (windowID == 0) {
			LOGGER_LOG("CaptureWindow: no window found for pid %d", pid);
			return false;
		}

		CGImageRef image = CGWindowListCreateImage(
			CGRectNull,
			kCGWindowListOptionIncludingWindow,
			windowID,
			kCGWindowImageBoundsIgnoreFraming);
		if (!image) {
			LOGGER_LOG("CaptureWindow: CGWindowListCreateImage returned NULL (screen recording permission needed?)");
			return false;
		}

		NSBitmapImageRep *bitmapRep = [[NSBitmapImageRep alloc] initWithCGImage:image];
		CGImageRelease(image);
		if (!bitmapRep) {
			LOGGER_LOG("CaptureWindow: failed to create bitmap rep");
			return false;
		}

		NSData *jpegData = [bitmapRep representationUsingType:NSBitmapImageFileTypeJPEG
		                                          properties:@{NSImageCompressionFactor: @0.8}];
		if (!jpegData) {
			LOGGER_LOG("CaptureWindow: JPEG encoding failed");
			return false;
		}

		NSString *path = [NSString stringWithUTF8String:filePath.c_str()];
		BOOL written = [jpegData writeToFile:path atomically:YES];
		if (!written) {
			LOGGER_LOG("CaptureWindow: failed to write %s", filePath.c_str());
		}
		return written;
	}
}
