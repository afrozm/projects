// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifdef _WIN32

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

#else

#ifndef strcpy_s 
#define strcpy_s(d,l,s) strlcpy(d,s,l)
#endif

#if DEBUG
#ifndef _DEBUG
#define _DEBUG 1
#endif
#endif

#ifndef UNREFERENCED_PARAMETER
#define UNREFERENCED_PARAMETER(x) (void)x
#endif

#endif



