#pragma once


// CBaseDlg dialog
#include "afxdialogex.h"
#include "MFCBaseDlg.h"
#include "ControlResizer.h"

#define  WM_GET_CLIENT_RECT2 WM_USER+4272

class CBaseDlg : public CMFCBaseDlg
{
	DECLARE_DYNAMIC(CBaseDlg)

public:
	CBaseDlg(UINT nIDTemplate, CWnd* pParent = NULL);   // standard constructor
	virtual BOOL CreateView(CWnd* pParent = NULL);
	virtual ~CBaseDlg();
	void AdjustWidthHeightWithParent(BOOL bAdjustWidht = TRUE, BOOL bAdjustHeight = TRUE);
	virtual void GetClientRect2(LPRECT outClientRect) const; // Returns entire client rect - for scroll view it should return total scroll area
	CControlResizer& GetControlResizer() {return mControlResizer;}
	void GetControlRect(int controlID, LPRECT outControlRect);
	void GetControlRect(CWnd *pChildControl, LPRECT outControlRect);
	bool SetWindowSize(int width, int height);
	bool SetWindowPosition(int x, int y);
	bool SetWindowRect(LPRECT newRect);
	static void GetClientRect2(HWND hWnd, LPRECT outClientRect);
// Dialog Data

protected:
	CControlResizer mControlResizer;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnGetClientRect2(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};

