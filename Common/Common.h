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
typedef std::string otherstring;
typedef char otherchar;
#define OTHER_T(x) x
#define lfopen(f,p,m) _wfopen_s(&f,p,m)

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
#define lfopen(f,p,m) f=fopen(p,m)
#define sprintf_s snprintf
#define strcat_s(d,n,s) strlcat(d,s,n)
typedef std::string lstring;
typedef std::wstring otherstring;
typedef wchar_t otherchar;
#ifndef _WIN32
#define _T(x) x
#endif
#define OTHER__T(x) L ##x
#define OTHER_T(x) OTHER__T(x)

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
#define lstrcpy strcpy
#define lstrlen strlen
#define lstrcat strcat
#define _tprintf printf
#define _vsctprintf(m,a) vsnprintf(NULL, 0, m, a)
#define _vstprintf_s vsnprintf
#define _tcstok_s strtok_r
#define sprintf_s snprintf
#define lstrcmp strcmp
#endif

#define memcpy_s(d,dn,s,sn) memcpy(d,s,dn<sn?dn:sn)
#define _fseeki64 fseeko
#define fread_s(d,dn,ds,n,f) fread(d,dn<n?dn:n,ds,f)
#define _stprintf_s sprintf

#define _TCHAR TCHAR
#define _tmain main
#define UNREFERENCED_PARAMETER(x)

typedef const TCHAR* LPCTSTR;
typedef TCHAR* LPTSTR;

typedef long long INT64;

#ifndef HMODULE
typedef void* HMODULE;
#endif

typedef unsigned int DWORD;
typedef unsigned long long ULONGLONG;


#endif // !_WIN32
