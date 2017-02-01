// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once


#include <windows.h>
#include <shlwapi.h>
#include <stdio.h>
#include <tchar.h>
#include <vector>
#include <string>
#include <atlrx.h>

// TODO: reference additional headers your program requires here
#if defined(_UNICODE) || defined(UNICODE)
typedef std::wstring lstring;
#define lprintf wprintf
#define  lputchar putwchar
//#define lputs _putws
#define lstrncmp wcsncmp
#define lstrncmpi wcsnicmp
#define lfopen _wfopen
#else
typedef std::string lstring;
#define lprintf printf
#define  lputchar putchar
//#define lputs puts
#define lstrncmp strncmp
#define lstrncmpi strnicmp
#define lfopen fopen
#endif