
// WindowFinderDlg.h : header file
//

#pragma once
#include "BaseDlg.h"
#include "IAccessibleHelper.h"

// CWindowFinderDlg dialog
class CWindowFinderDlg : public CBaseDlg
{
// Construction
public:
	CWindowFinderDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_WINDOWFINDER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
	CPoint mCurPoint;
    HWND mhWndCurrent;
	bool mbKeyUp;
	bool mbTracking;
    DWORD mAttachedThreaDID;

    IAccessibleHelper mAccessibleHelper;
    CRect mChildItemRect;
    DWORD mChildItemAccessibleUpdatedTime;
    BOOL mbChildItemChanged;

    struct WindowInfo;
    typedef bool (CWindowFinderDlg::*UpdateTextProc)(WindowInfo&);
    struct WindowInfo {
        HWND hWnd;
        CString wndText;
        bool bUpdated;
        UpdateTextProc fnUpdateTextProc;
        WindowInfo(UpdateTextProc proc=NULL);
    };
    CArray<WindowInfo> mWindowsInfo;

	// Generated message map functions
	void ToggleTracking();
    bool UpdateSelfText(WindowInfo& wi);
    bool UpdateChildItemText(WindowInfo& wi);
    bool UpdateParentText(WindowInfo& wi);
    bool UpdateForegroundText(WindowInfo& wi);
    bool UpdateFocusText(WindowInfo& wi);
    bool UpdateText();

    void UpdateChildItemLocation();

	CString getWindowText(HWND hWnd) const;
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnSizing(UINT nSide, LPRECT lpRect);
	DECLARE_MESSAGE_MAP()
};
