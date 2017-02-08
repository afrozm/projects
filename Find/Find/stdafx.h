// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once
#include <stdlib.h>
#include <crtdbg.h>

#ifndef _SECURE_ATL
#define _SECURE_ATL 1
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#endif

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER				// Allow use of features specific to Windows XP or later.
#define WINVER 0x0600		// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0600	// Change this to the appropriate value to target other versions of Windows.
#endif						

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600	// Change this to the appropriate value to target other versions of IE.
#endif

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit

// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <atlrx.h>
#include <afxole.h>

#include <afxdisp.h>        // MFC Automation classes
#include <shlwapi.h>
#include <Dlgs.h>

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT
#include <afxdlgs.h>
#include <afxmt.h>			// MFC support for synchronizing threads
#include <afxdhtml.h>		// HTML Dialogs

#include <vector>
#include <map>
#include "STLUtils.h"
#include <afxdisp.h>
#include "resource.h"		// main symbols

#define WM_SET_STATUS_MESSAGE WM_USER+0x500
#define WM_IS_SERVER WM_USER+0x501
#define WM_TOGGLE_PREVIEW WM_USER+0x502
#define WM_CREATE_PREVIEW WM_USER+0x503
#define WM_LOAD_PREVIEW WM_USER+0x504
#define WM_RESIZE_PREVIEW WM_USER+0x505
#define WM_SERVER_ADDTSPANEL WM_USER+0x506
#define WM_FINDTREE_EXPAND_PATH WM_USER+0x507

typedef CStringArray CArrayCString;
typedef CSortedArray<CString> CSortedArrayCString;

#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif

#include <string>
#if defined(UNICODE) || defined(_UNICODE)
typedef std::wstring lstring;
#else
typedef std::string lstring;
#endif

#include "SystemUtils.h"
#include "StringUtils.h"
#include <Dbt.h>
#include <afxcontrolbars.h>

#include <Strsafe.h>

#define lstrcpyS(d,s) StringCchCopy(d, _countof(d), s)

#ifdef _DEBUG
//#include <vld.h>
#endif
