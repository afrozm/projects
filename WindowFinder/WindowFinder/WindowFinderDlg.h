
// WindowFinderDlg.h : header file
//

#pragma once
#include "BaseDlg.h"
#include "IAccessibleHelper.h"
#include "resource.h"
#include "EditFindCtrl.h"

// CWindowFinderDlg dialog
class CWindowFinderDlg : public CBaseDlg
{
// Construction
public:
	CWindowFinderDlg(CWnd* pParent = NULL);	// standard constructor
    CString getWindowText(HWND hWnd, bool &bOutIsHanged, int maxTextLength = 256) const;

// Dialog Data
	enum { IDD = IDD_WINDOWFINDER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
    virtual BOOL DestroyWindow();
    virtual BOOL PreTranslateMessage(MSG* pMsg);

// Implementation
protected:
	HICON m_hIcon;
	CPoint mCurPoint;
    HWND mhWndCurrent;
    CEditFindCtrl mEditWndTxt;
    enum 
    {
        CurrentWndHang,
        KeyUp,
        Tracking,
        ChildItemChanged,
        TrackingChanged,
    };
    unsigned m_uFlags;
    DWORD mAttachedThreaDID;

    IAccessibleHelper mAccessibleHelper;
    CRect mChildItemRect;
    DWORD mChildItemAccessibleUpdatedTime;

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

    DEFINE_FUNCTION_SET_GET_FLAGBIT(m_uFlags, CurrentWndHang)
    DEFINE_FUNCTION_SET_GET_FLAGBIT(m_uFlags, KeyUp)
    DEFINE_FUNCTION_SET_GET_FLAGBIT(m_uFlags, Tracking)
    DEFINE_FUNCTION_SET_GET_FLAGBIT(m_uFlags, ChildItemChanged)
    DEFINE_FUNCTION_SET_GET_FLAGBIT(m_uFlags, TrackingChanged)
    // Generated message map functions
	void ToggleTracking(bool bToggle = true);
    bool UpdateSelfText(WindowInfo& wi);
    bool UpdateChildItemText(WindowInfo& wi);
    bool UpdateProcessText(WindowInfo& wi);
    bool UpdateStyleText(WindowInfo& wi);
    bool UpdateParentText(WindowInfo& wi);
    bool UpdateChildrenText(WindowInfo& wi);
    bool UpdateForegroundText(WindowInfo& wi);
    bool UpdateFocusText(WindowInfo& wi);
    bool UpdateText();
    WindowInfo& GetWindowInfo(UpdateTextProc forProc = NULL);

    void UpdateChildItemLocation();

    HWND GetEditInfoWnd() const { return mEditWndTxt.GetSafeHwnd(); }
    void UpdateLinks();
    void SetCurrentWindow(HWND hWnd);
    void RefreshComboSearchWindows();
    void RefreshText();

    void OnCbnEditchangeComboSearchWindowImp(bool bFromEvent = false);

	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnSizing(UINT nSide, LPRECT lpRect);
    afx_msg void OnEnLinkEditInfo(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg LRESULT OnUpdateLinks(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnWindowIterOp(WPARAM wParam, LPARAM lParam);
    afx_msg void OnCbnEditchangeComboSearchWindow();
    afx_msg void OnCbnSelchangeComboSearchWindow();
	DECLARE_MESSAGE_MAP()
};
