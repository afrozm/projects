
// WindowFinder.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CWindowFinderApp:
// See WindowFinder.cpp for the implementation of this class
//

#define WM_WINDOW_ITER_OP WM_USER+0x125


class CWindowFinderApp : public CWinApp
{
public:
	CWindowFinderApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CWindowFinderApp theApp;