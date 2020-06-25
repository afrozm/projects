// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

// Windows Header Files:
#include <windows.h>
#include <Dbt.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include <shlobj.h>
#include <shlwapi.h>
#include <Psapi.h>
#include <time.h>

#include <gdiplus.h>
#include <string>
#if defined(_UNICODE) || defined(UNICODE)
#define lprintf wprintf
#define  lputchar putwchar
//#define lputs _putws
#define lstrncmp wcsncmp
#define lstrncmpi wcsnicmp
typedef std::wstring lstring;
#define lfopen _wfopen
#else
#define lprintf printf
#define  lputchar putchar
//#define lputs puts
#define lstrncmp strncmp
#define lstrncmpi strnicmp
#define lfopen fopen
typedef std::string lstring;
#endif

#include "atlrx.h"