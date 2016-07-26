#pragma once

#include <string>

#ifdef _WIN32
#include <tchar.h>
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


#else
#define lprintf printf
#define  lputchar putchar
//#define lputs puts
#define lstrncmp strncmp
#define lstrncmpi strnicmp
#define lfopen fopen
typedef std::string lstring;

#ifndef _WIN32
#define _T(x) x
#endif

#endif

#ifndef _WIN32
#define _tcscpy_s(d,n,s) strlcpy(d,s,n)
#if defined(_UNICODE) || defined(UNICODE)
#define lstrcmpi wcscasecmp
typedef wchar_t TCHAR;
#else
typedef char TCHAR;
#define lstrcmpi strcasecmp
#define _stricmp strcasecmp
#endif
#endif // !_WIN32
