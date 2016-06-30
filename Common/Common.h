#pragma once

#include <string>

#ifdef _WIN32
#include <tchar.h>
#else

#if defined(_UNICODE) || defined(UNICODE)
#define lstrcmpi wcscasecmp
#else
#define lstrcmpi strcasecmp
#endif

#endif

#if defined(_UNICODE) || defined(UNICODE)
#define lprintf wprintf
#define  lputchar putwchar
//#define lputs _putws
#define lstrncmp wcsncmp
#define lstrncmpi wcsnicmp
typedef std::wstring lstring;
#define lfopen _wfopen

#ifndef _WIN32
#if !defined(__T)
#define __T(x) L ##x
#endif
#define _T(x) __T(x)
#endif

typedef wchar_t TCHAR;

#else
#define lprintf printf
#define  lputchar putchar
//#define lputs puts
#define lstrncmp strncmp
#define lstrncmpi strnicmp
#define lfopen fopen
typedef std::string lstring;

#define _T(x) x
typedef char TCHAR;
#endif
