// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>
#include <string>
#include <windows.h>
#include <shlwapi.h>
#include <shlobj.h>
#include "atlrx.h"
#include <map>
#include <set>
#include <vector>

// TODO: reference additional headers your program requires here
#if defined(UNICODE) || (_UNICODE)
typedef std::wstring lstring;
#else
typedef std::string lstring;
#endif

#if defined(_UNICODE) || defined(UNICODE)
#define lprintf wprintf
#define  lputchar putwchar
//#define lputs _putws
#define lstrncmp wcsncmp
#define lstrncmpi wcsnicmp
#define lfopen _wfopen
#else
#define lprintf printf
#define  lputchar putchar
//#define lputs puts
#define lstrncmp strncmp
#define lstrncmpi strnicmp
#define lfopen fopen
#endif