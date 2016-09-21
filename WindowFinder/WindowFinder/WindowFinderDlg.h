
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
    virtual BOOL DestroyWindow();

// Implementation
protected:
	HICON m_hIcon;
	CPoint mCurPoint;
    HWND mhWndCurrent, mhWndEdit;
    bool mbCurrentWndHang;
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
    bool UpdateProcessText(WindowInfo& wi);
    bool UpdateStyleText(WindowInfo& wi);
    bool UpdateParentText(WindowInfo& wi);
    bool UpdateForegroundText(WindowInfo& wi);
    bool UpdateFocusText(WindowInfo& wi);
    bool UpdateText();
    const WindowInfo& GetWindowInfo(INT_PTR i = 0) const;

    void UpdateChildItemLocation();

	CString getWindowText(HWND hWnd, bool &bOutIsHanged) const;
    HWND GetEditInfoWnd() const { return mhWndEdit; }
    bool IsCurrentWindowHung() const { return mbCurrentWndHang; }
    void UpdateLinks();

	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnSizing(UINT nSide, LPRECT lpRect);
    afx_msg void OnEnLinkEditInfo(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg LRESULT OnUpdateLinks(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};
