#pragma once

#ifdef _WIN32
#include "Path.h"
#else
#include <Path.h>
#endif

// Cross-platform (ImgMgmtCommon.cpp)
Path GetImagePath();
void CleanupImgFolder();
bool SaveWindowImage(const lstring &windowTitle, bool bThrottle = false);

// Platform-specific (ImgMgmt.cpp on Win, ImgMgmtMac.mm on Mac)
bool CaptureAndSaveWindowImage(const Path &filePath);

#ifdef __APPLE__
bool HasScreenRecordingPermission();
#endif
