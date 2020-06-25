// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "Common.h"
#include "STLUtils.h"

#ifdef _WIN32
#include <windows.h>
#include <shlwapi.h>
#include <atlrx.h>
#include <tchar.h>
#include <conio.h>
#else
#include <curses.h>
#endif
#include <stdio.h>
#include <vector>
#include <string>

// TODO: reference additional headers your program requires here
#if defined(_UNICODE) || defined(UNICODE)
typedef std::wstring lstring;
#define lprintf wprintf
#define  lputchar putwchar
//#define lputs _putws
#define lstrncmp wcsncmp
#define lstrncmpi wcsnicmp

#else
typedef std::string lstring;
#define lprintf printf
#define  lputchar putchar
//#define lputs puts
#define lstrncmp strncmp
#define lstrncmpi strnicmp

#endif
