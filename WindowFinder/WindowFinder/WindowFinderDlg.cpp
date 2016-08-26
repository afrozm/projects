
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
	: CBaseDlg(CWindowFinderDlg::IDD, pParent), mCurWindow(NULL), mbKeyUp(true), mbTracking(true)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
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

	SetTimer(TIMER_ID_REFRESH, 100, NULL);
	GetControlResizer().AddControl(IDC_EDIT_INFO);
	GetControlResizer().AddControl(IDC_STATIC_COORD_SELF, RSZF_RIGHT_FIXED | RSZF_BOTTOM_FIXED | RSZF_SIZE_FIXED);
	GetControlResizer().AddControl(IDC_STATIC_TITLE_SELF, RSZF_RIGHT_FIXED | RSZF_BOTTOM_FIXED | RSZF_SIZE_FIXED);
	int cids[] = {IDC_STATIC_TITLE_SCREEN, IDC_STATIC_COORD, IDC_STATIC_TITLE_WND, IDC_STATIC_COORD_WND};
	for (int i = 0; i < sizeof(cids)/sizeof(cids[0]); ++i)
		GetControlResizer().AddControl(cids[i], RSZF_BOTTOM_FIXED | RSZF_SIZE_FIXED);
	CString title;
	title.Format(_T("WindowFinder - %s"), mbTracking ? _T("ON") : _T("OFF"));
	SetWindowText(title);
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
		TCHAR *text = new TCHAR[textLen];
		::SendMessage(hWnd, WM_GETTEXT, textLen, (LPARAM)text);
		outStr = text;
		delete []text;
		return outStr;
	}
	return outStr;
}

void CWindowFinderDlg::OnTimer( UINT_PTR nIDEvent )
{
	KillTimer(TIMER_ID_REFRESH);
	while (mbTracking) {
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
		CWnd *pParent(pCurWind ? pCurWind->GetParent() : NULL);
		if (curPoint != mCurPoint) {
			mCurPoint = curPoint;
			CString coord;
			coord.Format(_T("%d,%d"), mCurPoint.x, mCurPoint.y);
			SetDlgItemText(IDC_STATIC_COORD, coord);
			coord = _T("");
			if (pCurWind) {
				pCurWind->ScreenToClient(&curPoint);
				coord.Format(_T("%d,%d"), curPoint.x, curPoint.y);
			}
			SetDlgItemText(IDC_STATIC_COORD_SELF, coord);
			coord = _T("");
			curPoint = mCurPoint;
			if (pParent) {
				pParent->ScreenToClient(&curPoint);
				coord.Format(_T("%d,%d"), curPoint.x, curPoint.y);
			}
			SetDlgItemText(IDC_STATIC_COORD_WND, coord);
		}
		HWND curHWND = pCurWind->GetSafeHwnd();
		if (curHWND == mCurWindow)
			break;
		if (pCurWind->GetSafeHwnd() == NULL)
			break;
		mCurWindow = curHWND;
		CString wndInfo, text;
		text.Format(_T("Handle: 0x%x (%d)"), mCurWindow, mCurWindow);
		wndInfo += text + _T("\r\n");
        text.Empty();
        //if (mCurWindow != GetDlgItem(IDC_EDIT_INFO)->GetSafeHwnd()) {
        //    mAccessibleHelper.InitFromPoint(mCurPoint.x, mCurPoint.y);
        //    text = mAccessibleHelper.GetValue(_T("Name")).c_str();
        //    if (text.IsEmpty())
        //        text = mAccessibleHelper.GetValue(_T("Value")).c_str();
        //}
        if (text.IsEmpty())
		    text = getWindowText(mCurWindow);
		wndInfo += _T("Title: ") + text + _T("\r\n");
		{
			TCHAR className[1024] = {0};
			GetClassName(curHWND, className, sizeof(className)/sizeof(className[0]));
			text = className;
			wndInfo += _T("Class: ") + text + _T("\r\n");
		}
		if (pParent) {
			text.Format(_T("Parent Handle: 0x%x (%d)"), pParent->GetSafeHwnd(), pParent->GetSafeHwnd());
			wndInfo += text + _T("\r\n");
			text = getWindowText(pParent->GetSafeHwnd());
			wndInfo += _T("Parent Title: ") + text + _T("\r\n");
		}
		CRect cr;
		pCurWind->GetWindowRect(cr);
		text.Format(_T("Screen Window Rect: left: %d, top: %d, right: %d, bottom: %d, width: %d, height: %d"), cr.left, cr.top, cr.right, cr.bottom, cr.Width(), cr.Height());
		wndInfo += text + _T("\r\n");
		if (pParent) {
			pParent->ScreenToClient(cr);
			text.Format(_T("Parent Window Rect: left: %d, top: %d, right: %d, bottom: %d, width: %d, height: %d"), cr.left, cr.top, cr.right, cr.bottom, cr.Width(), cr.Height());
			wndInfo += text + _T("\r\n");
		}
		pCurWind->GetClientRect(cr);
		text.Format(_T("Client Rect: left: %d, top: %d, right: %d, bottom: %d, width: %d, height: %d"), cr.left, cr.top, cr.right, cr.bottom, cr.Width(), cr.Height());
		wndInfo += text + _T("\r\n");
		pParent = GetForegroundWindow();
		BOOL bAttached(FALSE);
		DWORD attachedThreaDID(-1);
		if (pParent) {
			attachedThreaDID = GetWindowThreadProcessId(pParent->GetSafeHwnd(), NULL);
			bAttached = AttachThreadInput(GetCurrentThreadId(), attachedThreaDID, TRUE);
			DWORD le =  GetLastError();
			text.Format(_T("Foregorund Handle: 0x%x (%d)"), pParent->GetSafeHwnd(), pParent->GetSafeHwnd());
			wndInfo += text + _T("\r\n");
			text = _T("");
			text = getWindowText(pParent->GetSafeHwnd());
			wndInfo += _T("Foregorund Title: ") + text + _T("\r\n");
		}
		pParent = GetFocus();
		if (pParent) {
			text.Format(_T("Focus Handle: 0x%x (%d)"), pParent->GetSafeHwnd(), pParent->GetSafeHwnd());
			wndInfo += text + _T("\r\n");
			text = _T("");
			text = getWindowText(pParent->GetSafeHwnd());
			wndInfo += _T("Focus Title: ") + text + _T("\r\n");
			TCHAR className[1024] = {0};
			GetClassName(pParent->GetSafeHwnd(), className, sizeof(className)/sizeof(className[0]));
			text = className;
			wndInfo += _T("Focus Class: ") + text + _T("\r\n");
		}
		if (bAttached)
			AttachThreadInput(GetCurrentThreadId(), attachedThreaDID, FALSE);
		SetDlgItemText(IDC_EDIT_INFO, wndInfo);
		break;
	}
	if (GetAsyncKeyState(VK_CONTROL)) {
		if (mbKeyUp) {
			mbKeyUp = false;
			ToggleTracking();
		}
	}
	else
		mbKeyUp = true;
	SetTimer(TIMER_ID_REFRESH, 100, NULL);
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
}


