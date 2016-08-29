
// WindowFinderDlg.cpp : implementation file
//

#include "stdafx.h"
#include "WindowFinder.h"
#include "WindowFinderDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define TIMER_ID_REFRESH 11
#define TIMER_REFRESH_INTERVAL 100 // 100 ms


// CAboutDlg dialog used for App About

class CAboutDlg : public CBaseDlg
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CBaseDlg(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CBaseDlg::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CBaseDlg)
END_MESSAGE_MAP()


// CWindowFinderDlg dialog




CWindowFinderDlg::CWindowFinderDlg(CWnd* pParent /*=NULL*/)
	: CBaseDlg(CWindowFinderDlg::IDD, pParent), mbKeyUp(true), mbTracking(true),
    mhWndCurrent(NULL), mChildItemAccessibleUpdatedTime(0), mbChildItemChanged(FALSE), mAttachedThreaDID(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    mWindowsInfo.Add(WindowInfo(&CWindowFinderDlg::UpdateSelfText));
    mWindowsInfo.Add(WindowInfo(&CWindowFinderDlg::UpdateChildItemText));
    mWindowsInfo.Add(WindowInfo(&CWindowFinderDlg::UpdateParentText));
    mWindowsInfo.Add(WindowInfo(&CWindowFinderDlg::UpdateForegroundText));
    mWindowsInfo.Add(WindowInfo(&CWindowFinderDlg::UpdateFocusText));
}

void CWindowFinderDlg::DoDataExchange(CDataExchange* pDX)
{
	CBaseDlg::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CWindowFinderDlg, CBaseDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_WM_SIZING()
END_MESSAGE_MAP()


// CWindowFinderDlg message handlers

BOOL CWindowFinderDlg::OnInitDialog()
{
	CBaseDlg::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX+1, _T("Always On Top"));
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX+2, _T("Tracking"));
			if (mbTracking)
				pSysMenu->CheckMenuItem(IDM_ABOUTBOX+2, MF_CHECKED);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	SetTimer(TIMER_ID_REFRESH, TIMER_REFRESH_INTERVAL, NULL);
	GetControlResizer().AddControl(IDC_EDIT_INFO);
	GetControlResizer().AddControl(IDC_STATIC_COORD_SELF, RSZF_RIGHT_FIXED | RSZF_BOTTOM_FIXED | RSZF_SIZE_FIXED);
	GetControlResizer().AddControl(IDC_STATIC_TITLE_SELF, RSZF_RIGHT_FIXED | RSZF_BOTTOM_FIXED | RSZF_SIZE_FIXED);
	int cids[] = {IDC_STATIC_TITLE_SCREEN, IDC_STATIC_COORD, IDC_STATIC_TITLE_WND, IDC_STATIC_COORD_WND};
	for (int i = 0; i < sizeof(cids)/sizeof(cids[0]); ++i)
		GetControlResizer().AddControl(cids[i], RSZF_BOTTOM_FIXED | RSZF_SIZE_FIXED);
	CString title;
	title.Format(_T("WindowFinder - %s"), mbTracking ? _T("ON") : _T("OFF"));
	SetWindowText(title);
    GetDlgItem(IDC_COMBO_SEARCH_WINDOW)->EnableWindow(!mbTracking);
	return TRUE;  // return TRUE  unless you set the focus to a control
}
static void SetTopMost( HWND hWnd, const BOOL TopMost )
{
	ASSERT( ::IsWindow( hWnd ));
	HWND hWndInsertAfter = ( TopMost ? HWND_TOPMOST : HWND_NOTOPMOST );
	::SetWindowPos( hWnd, hWndInsertAfter, 0, 0 , 0 , 0, SWP_NOMOVE | SWP_NOSIZE );
}
void CWindowFinderDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	switch (nID & 0xFFFF) {
		case IDM_ABOUTBOX:
		{
			CAboutDlg dlgAbout;
			dlgAbout.DoModal();
		}
		break;
		case IDM_ABOUTBOX+1: // always on top
			{
				CMenu* pSysMenu = GetSystemMenu(FALSE);
				bool topLevel((pSysMenu->GetMenuState(IDM_ABOUTBOX+1, MF_BYCOMMAND) & MF_CHECKED) != 0);
				topLevel = !topLevel;
				pSysMenu->CheckMenuItem(IDM_ABOUTBOX+1, topLevel ? MF_CHECKED : MF_UNCHECKED);
				SetTopMost(GetSafeHwnd(), topLevel);
			}
			break;
		case IDM_ABOUTBOX+2: // Tracking on/off
			{
				ToggleTracking();
			}
			break;
	}
	CBaseDlg::OnSysCommand(nID, lParam);
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CWindowFinderDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CBaseDlg::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CWindowFinderDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

CString CWindowFinderDlg::getWindowText(HWND hWnd) const
{
	CString outStr;
	if (hWnd != GetDlgItem(IDC_EDIT_INFO)->GetSafeHwnd()) {

		int textLen = (int)::SendMessage(hWnd, WM_GETTEXTLENGTH, 0, 0) + 1;
        if (textLen > 256)
            textLen = 256;
		TCHAR *text = new TCHAR[textLen];
		::SendMessage(hWnd, WM_GETTEXT, textLen, (LPARAM)text);
		outStr = text;
		delete []text;
		return outStr;
	}
	return outStr;
}

static CString getRectText(const CRect &inRect, const CString &inRectName)
{
    CString outText;

    outText.Format(_T("%s: %d,%d   %d,%d - (%d,%d)\r\n"), inRectName,
        inRect.left, inRect.top, inRect.right, inRect.bottom,
        inRect.Width(), inRect.Height());

    return outText;
}

void CWindowFinderDlg::OnTimer( UINT_PTR nIDEvent )
{
	KillTimer(TIMER_ID_REFRESH);
	if (mbTracking) {
		CPoint curPoint;
		GetCursorPos(&curPoint);
		CWnd *pCurWind(WindowFromPoint(curPoint));
		if (pCurWind) {
			CPoint clP(curPoint);
			pCurWind->ScreenToClient(&clP);
			CWnd *pChildWnd(pCurWind->ChildWindowFromPoint(clP, CWP_SKIPINVISIBLE));
			if (pChildWnd)
				pCurWind = pChildWnd;
		}
        mhWndCurrent = pCurWind->GetSafeHwnd();
		CWnd *pParent(pCurWind ? pCurWind->GetParent() : NULL);
		if (curPoint != mCurPoint) {
			mCurPoint = curPoint;
            UpdateChildItemLocation();
            CString coord;
			coord.Format(_T("%d,%d"), mCurPoint.x, mCurPoint.y);
			SetDlgItemText(IDC_STATIC_COORD, coord);
			coord = _T("");
			if (pCurWind) {
                if (!mChildItemRect.IsRectEmpty()) {
                    curPoint.x = mCurPoint.x - mChildItemRect.left;
                    curPoint.y = mCurPoint.y - mChildItemRect.top;
                }
                else {
                    pCurWind->ScreenToClient(&curPoint);
                }
				coord.Format(_T("%d,%d"), curPoint.x, curPoint.y);
			}
			SetDlgItemText(IDC_STATIC_COORD_SELF, coord);
			coord = _T("");
			curPoint = mCurPoint;
            if (pParent)
				pParent->ScreenToClient(&curPoint);
            coord.Format(_T("%d,%d"), curPoint.x, curPoint.y);
            SetDlgItemText(IDC_STATIC_COORD_WND, coord);
		}
        UpdateText();
	}
	if (GetAsyncKeyState(VK_CONTROL)) {
		if (mbKeyUp) {
			mbKeyUp = false;
			ToggleTracking();
		}
	}
	else
		mbKeyUp = true;
    SetTimer(TIMER_ID_REFRESH, TIMER_REFRESH_INTERVAL, NULL);
}

void CWindowFinderDlg::OnSizing( UINT nSide, LPRECT lpRect )
{
	if (lpRect->right - lpRect->left < 450) {
		switch (nSide) {
		case WMSZ_BOTTOMRIGHT:
		case WMSZ_RIGHT:
		case WMSZ_TOPRIGHT:
			lpRect->right = lpRect->left + 450;
			break;
		case WMSZ_BOTTOMLEFT:
		default:
			lpRect->left = lpRect->right - 450;
		}
	}
	if (lpRect->bottom - lpRect->top < 300) {
		switch (nSide) {
		case WMSZ_BOTTOM:
		case WMSZ_BOTTOMLEFT:
		case WMSZ_BOTTOMRIGHT:
			lpRect->bottom = lpRect->top + 300;
			break;
		default:
			lpRect->top = lpRect->bottom - 300;
		}
	}
	__super::OnSizing(nSide, lpRect);
}

void CWindowFinderDlg::ToggleTracking()
{
	mbTracking = !mbTracking;
	CString title;
	title.Format(_T("WindowFinder - %s"), mbTracking ? _T("ON") : _T("OFF"));
	SetWindowText(title);
	CMenu* pSysMenu = GetSystemMenu(FALSE);
	pSysMenu->CheckMenuItem(IDM_ABOUTBOX+2, mbTracking ? MF_CHECKED : MF_UNCHECKED);
    GetDlgItem(IDC_COMBO_SEARCH_WINDOW)->EnableWindow(!mbTracking);
}

bool CWindowFinderDlg::UpdateChildItemText(WindowInfo& wi)
{
    bool bChildItemUpdated(false);
    if (mbTracking) {
        if (mhWndCurrent != GetDlgItem(IDC_EDIT_INFO)->GetSafeHwnd()) {
            UpdateChildItemLocation();
            CString &text(wi.wndText);
            TCHAR *feildsName[] = {
                _T("Name"),
                _T("Value"),
                _T("Description")
            };
            for (auto name : feildsName) {
                text = mAccessibleHelper.GetValue(_T("Name")).c_str();
                if (!text.IsEmpty())
                    break;
            }
            text = _T("Child Item: ") + text + _T("   ");
            text += getRectText(mChildItemRect, _T("Screen Rect"));
            bChildItemUpdated = mbChildItemChanged != FALSE;
        }
    }
    return bChildItemUpdated;
}

bool CWindowFinderDlg::UpdateSelfText(WindowInfo& wi)
{
    bool bUpdated(wi.hWnd != mhWndCurrent);
    if (bUpdated) {
        wi.hWnd = mhWndCurrent;
        CString &text(wi.wndText);
        text.Format(_T("Handle: 0x%x (%d)\r\n"), mhWndCurrent, mhWndCurrent);
        text += _T("Title: ") + getWindowText(mhWndCurrent) + _T("\r\n");
        {
            TCHAR className[1024] = { 0 };
            GetClassName(mhWndCurrent, className, sizeof(className) / sizeof(className[0]));
            text += _T("Class: ") + CString(className) + _T("\r\n");
        }
        CWnd *pCurWind(FromHandle(mhWndCurrent));
        CRect cr;
        pCurWind->GetWindowRect(cr);
        text += getRectText(cr, _T("Screen Rect"));
        CWnd *pParent(pCurWind->GetParent());
        if (pParent) {
            pParent->ScreenToClient(cr);
            text += getRectText(cr, _T("Rect wrt Parent"));
        }
        pCurWind->GetClientRect(cr);
        text += getRectText(cr, _T("Client Rect"));
    }
    return bUpdated;
}

bool CWindowFinderDlg::UpdateParentText(WindowInfo& wi)
{
    CWnd *pParent(mhWndCurrent ? FromHandle(mhWndCurrent)->GetParent() : NULL);
    bool bUpdated(wi.hWnd != pParent->GetSafeHwnd());
    if (bUpdated) {
        wi.hWnd = pParent->GetSafeHwnd();
        CString &text(wi.wndText);
        if (pParent) {
            text.Format(_T("Parent Handle: 0x%x (%d)\r\n"), pParent->GetSafeHwnd(), pParent->GetSafeHwnd());
            text += _T("Parent Title: ") + getWindowText(pParent->GetSafeHwnd()) + _T("\r\n");
        }
        else
            text.Empty();
    }
    return bUpdated;
}

bool CWindowFinderDlg::UpdateForegroundText(WindowInfo& wi)
{
    CWnd *pParent = GetForegroundWindow();
    bool bUpdated(wi.hWnd != pParent->GetSafeHwnd());
    if (bUpdated) {
        wi.hWnd = pParent->GetSafeHwnd();
        DWORD attachedThreaDID(-1);
        CString &text(wi.wndText);
        if (pParent) {
            if (mAttachedThreaDID)
                AttachThreadInput(GetCurrentThreadId(), mAttachedThreaDID, FALSE);
            mAttachedThreaDID = GetWindowThreadProcessId(pParent->GetSafeHwnd(), NULL);
            BOOL bAttached = AttachThreadInput(GetCurrentThreadId(), mAttachedThreaDID, TRUE);
            if (!bAttached)
                mAttachedThreaDID = 0;
            DWORD le = GetLastError();
            text.Format(_T("Foregorund Handle: 0x%x (%d)\r\n"), pParent->GetSafeHwnd(), pParent->GetSafeHwnd());
            text += _T("Foregorund Title: ") + getWindowText(pParent->GetSafeHwnd()) + _T("\r\n");
        }
        else
            text.Empty();
    }
    return bUpdated;
}

bool CWindowFinderDlg::UpdateFocusText(WindowInfo& wi)
{
    CWnd *pParent = GetFocus();
    bool bUpdated(wi.hWnd != pParent->GetSafeHwnd());
    if (bUpdated) {
        wi.hWnd = pParent->GetSafeHwnd();
        CString &text(wi.wndText);
        if (pParent) {
            text.Format(_T("Focus Handle: 0x%x (%d)\r\n"), pParent->GetSafeHwnd(), pParent->GetSafeHwnd());
            text += _T("Focus Title: ") + getWindowText(pParent->GetSafeHwnd()) + _T("\r\n");
            TCHAR className[1024] = { 0 };
            GetClassName(pParent->GetSafeHwnd(), className, sizeof(className) / sizeof(className[0]));
            text += _T("Focus Class: ") + CString(className) + _T("\r\n");
        }
        else
            text.Empty();
    }
    return bUpdated;
}

bool CWindowFinderDlg::UpdateText()
{
    bool bUpdated(false);
    for (INT_PTR i = 0; i < mWindowsInfo.GetCount(); ++i) {
        WindowInfo &wi(mWindowsInfo.ElementAt(i));
        if ((this->*wi.fnUpdateTextProc)(wi))
            bUpdated = true;
    }
    if (bUpdated) {
        CString text;
        for (INT_PTR i = 0; i < mWindowsInfo.GetCount(); ++i)
            text += mWindowsInfo.ElementAt(i).wndText;

        SetDlgItemText(IDC_EDIT_INFO, text);
    }

    return bUpdated;
}

void CWindowFinderDlg::UpdateChildItemLocation()
{
    if ((GetTickCount() - mChildItemAccessibleUpdatedTime) > (TIMER_REFRESH_INTERVAL-10)) {
        mAccessibleHelper.InitFromPoint(mCurPoint.x, mCurPoint.y);
        mAccessibleHelper.Location();
        CRect oldRecr(mChildItemRect);
        mAccessibleHelper.GetRect(mChildItemRect);
        mbChildItemChanged = oldRecr != mChildItemRect;
        mChildItemAccessibleUpdatedTime = GetTickCount();
    }
}

CWindowFinderDlg::WindowInfo::WindowInfo(UpdateTextProc proc)
    : bUpdated(false), hWnd(NULL), fnUpdateTextProc(proc)
{

}
