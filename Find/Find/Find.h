// Find.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols

#ifdef _DEBUG
#include <vld.h>
#endif // _DEBUG

// CFindApp:
// See Find.cpp for the implementation of this class
//

class CFindApp : public CWinApp
{
public:
	CFindApp();
	static bool IsServer();
// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
private:
	bool mbIsServer;
};

extern CFindApp theApp;

CString WildCardExpToRegExp(LPCTSTR wildCardExp);
