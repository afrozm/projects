
// WindowFinderDlg.cpp : implementation file
//

#include "stdafx.h"
#include "WindowFinder.h"
#include "WindowFinderDlg.h"
#include "afxdialogex.h"
#include "ProcessUtil.h"
#include <stlutils.h>
#include "RefreshWindowThread.h"
#include "StringUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define TIMER_ID_REFRESH 11
#define TIMER_ID_KEY 12
#define TIMER_REFRESH_INTERVAL 300 // 300 ms


#define LINK_TEXT_HANDLE _T("Handle: ")
#define LINK_TEXT_CHILD_ELEM _T("Child Elements:")

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

#define WF_GET_FN_ADDR(f) &CWindowFinderDlg::f
#define GetWindowInfoEx(f) GetWindowInfo(WF_GET_FN_ADDR(f))


CWindowFinderDlg::CWindowFinderDlg(CWnd* pParent /*=NULL*/)
	: CBaseDlg(CWindowFinderDlg::IDD, pParent), m_uFlags(0),
    mhWndCurrent(NULL), mChildItemAccessibleUpdatedTime(0), mAttachedThreaDID(0)
{
    SetKeyUp();
    SetTracking();
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    mWindowsInfo.Add(WindowInfo(&CWindowFinderDlg::UpdateSelfText));
    mWindowsInfo.Add(WindowInfo(&CWindowFinderDlg::UpdateChildItemText));
    mWindowsInfo.Add(WindowInfo(&CWindowFinderDlg::UpdateProcessText));
    mWindowsInfo.Add(WindowInfo(&CWindowFinderDlg::UpdateStyleText));
    mWindowsInfo.Add(WindowInfo(&CWindowFinderDlg::UpdateParentText));
    mWindowsInfo.Add(WindowInfo(&CWindowFinderDlg::UpdateChildrenText));
    mWindowsInfo.Add(WindowInfo(&CWindowFinderDlg::UpdateForegroundText));
    mWindowsInfo.Add(WindowInfo(&CWindowFinderDlg::UpdateFocusText));
}

void CWindowFinderDlg::DoDataExchange(CDataExchange* pDX)
{
	CBaseDlg::DoDataExchange(pDX);
}

BOOL CWindowFinderDlg::DestroyWindow()
{
    mAccessibleHelper.InitFromPoint();
    return __super::DestroyWindow();
}

BOOL CWindowFinderDlg::PreTranslateMessage(MSG* pMsg)
{
    switch (pMsg->message)
    {
    case WM_KEYUP:
        if (VK_F5 == pMsg->wParam) {
            RefreshComboSearchWindows();
            return TRUE;
        }
        break;
    default:
        break;
    }
    return __super::PreTranslateMessage(pMsg);
}

#define WM_WF_UPDATE_LINKS WM_USER+0x593

BEGIN_MESSAGE_MAP(CWindowFinderDlg, CBaseDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_WM_SIZING()
    ON_NOTIFY(EN_LINK, IDC_EDIT_INFO, &CWindowFinderDlg::OnEnLinkEditInfo)
    ON_MESSAGE(WM_WF_UPDATE_LINKS, OnUpdateLinks)
    ON_MESSAGE(WM_WINDOW_ITER_OP, OnWindowIterOp)
    ON_CBN_EDITCHANGE(IDC_COMBO_SEARCH_WINDOW, &CWindowFinderDlg::OnCbnEditchangeComboSearchWindow)
    ON_CBN_SELCHANGE(IDC_COMBO_SEARCH_WINDOW, &CWindowFinderDlg::OnCbnSelchangeComboSearchWindow)
END_MESSAGE_MAP()


// CWindowFinderDlg message handlers

#define IS_HEX(c) (((c)>='0'&&(c)<='9')||(c)>='A'&&(c)<='F'||(c)>='a'&&(c)<='f')

static int GetHexCharsEnd(LPCTSTR hexStr)
{
    int hexend(-1);

    if (hexStr) {
        if (hexStr[0] == '0') {
            if (hexStr[1] == 'x' || hexStr[1] == 'X') {
                hexStr += 2;
                while (*hexStr)
                {
                    TCHAR ch(*hexStr);
                    if (!IS_HEX(ch))
                        break;
                    ++hexend;
                    ++hexStr;
                }
                if (hexend > -1)
                    hexend += 3; // 0x plus the last char matched
            }
        }
    }
    return hexend;
}

void CWindowFinderDlg::UpdateLinks()
{
    CString text;
    CRichEditCtrl *pCtrl((CRichEditCtrl *)GetDlgItem(IDC_EDIT_INFO));
    pCtrl->GetWindowText(text);
    pCtrl->SetWindowText(text);
    if (!IsTracking()) {
        LPCTSTR links[] = { LINK_TEXT_HANDLE, LINK_TEXT_CHILD_ELEM };
        for (auto link : links) {
            long pos = 0;
            const long linkLen = lstrlen(link);
            FINDTEXTEX ft = { { pos,-1 }, link };
            while (pos >= 0)
            {
                ft.chrg.cpMin = pos;
                pos = pCtrl->FindText(FR_DOWN, &ft);
                if (pos < 0)
                    continue;
                int startPos(pos), endPos(-1);
                CString wndText;
                pos += linkLen;
                const int wndTextLen(pCtrl->GetTextRange(pos, pos + 32, wndText));
                if (LINK_TEXT_HANDLE == link) {
                    startPos += linkLen;
                    endPos = GetHexCharsEnd(wndText);
                }
                else {
                    TCHAR ch(wndTextLen > 0 ? wndText.GetAt(0) : 0);
                    if (STR_CHAR_IS_LINE(ch))
                        endPos = linkLen;
                }
                if (endPos < 0)
                    continue;
                {
                    pCtrl->SetSel(startPos, startPos + endPos);
                    CHARFORMAT2 cf2;
                    ZeroMemory(&cf2, sizeof(cf2));
                    cf2.cbSize = sizeof(cf2);
                    cf2.dwMask = CFM_LINK;
                    cf2.dwEffects = CFE_LINK;
                    pCtrl->SetSelectionCharFormat(cf2);
                }
            }
        }
    }
}

void CWindowFinderDlg::SetCurrentWindow(HWND hWnd)
{
    if (hWnd) {
        //const WindowInfo &wi(GetWindowInfo());
        GetWindowInfo().hWnd = NULL;
        //((WindowInfo*)&wi)->hWnd = NULL; // Set it null so that current window text is refreshed
        mhWndCurrent = hWnd;
        RefreshText();
    }
}

void CWindowFinderDlg::RefreshComboSearchWindows()
{
    if (!IsTracking())
        CRefreshWindowThread::GetInstance()->RefreshWindows();
}

void CWindowFinderDlg::RefreshText()
{
    UpdateText();
    PostMessage(WM_WF_UPDATE_LINKS);
}

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
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX+2, _T("Tracking\tPause/Ctl+Alt+P"));
			if (IsTracking())
				pSysMenu->CheckMenuItem(IDM_ABOUTBOX+2, MF_CHECKED);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
    mEditWndTxt.SubclassDlgItem(IDC_EDIT_INFO, this);
	SetTimer(TIMER_ID_REFRESH, TIMER_REFRESH_INTERVAL, NULL);
    SetTimer(TIMER_ID_KEY, 100, NULL);
	GetControlResizer().AddControl(IDC_EDIT_INFO);
	GetControlResizer().AddControl(IDC_STATIC_COORD_SELF, RSZF_RIGHT_FIXED | RSZF_BOTTOM_FIXED | RSZF_SIZE_FIXED);
	GetControlResizer().AddControl(IDC_STATIC_TITLE_SELF, RSZF_RIGHT_FIXED | RSZF_BOTTOM_FIXED | RSZF_SIZE_FIXED);
    GetControlResizer().AddControl(IDC_COMBO_SEARCH_WINDOW, RSZF_RIGHT_FIXED | RSZF_BOTTOM_FIXED | RSZF_SIZEY_FIXED);
	int cids[] = {IDC_STATIC_TITLE_SCREEN, IDC_STATIC_COORD, IDC_STATIC_TITLE_WND, IDC_STATIC_COORD_WND};
	for (int i = 0; i < sizeof(cids)/sizeof(cids[0]); ++i)
		GetControlResizer().AddControl(cids[i], RSZF_BOTTOM_FIXED | RSZF_SIZE_FIXED);
    ToggleTracking(false);
    CRichEditCtrl *pCtrl = (CRichEditCtrl*)GetDlgItem(IDC_EDIT_INFO);
    pCtrl->SetEventMask(ENM_LINK);
    pCtrl->SendMessage(EM_AUTOURLDETECT, TRUE);
    if (pCtrl->GetStyle() & ES_READONLY)
        pCtrl->SetBackgroundColor(FALSE, GetSysColor(CTLCOLOR_DLG));

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

CString CWindowFinderDlg::getWindowText(HWND hWnd, bool &bOutIsHanged, int maxTextLength /* = 256 */) const
{
	CString outStr;
    bool bCheckOnlyHang(bOutIsHanged);
    bOutIsHanged = false;
	while (hWnd && hWnd != GetEditInfoWnd()) {
        DWORD_PTR result(0);
		bOutIsHanged = ::SendMessageTimeout(hWnd, WM_GETTEXTLENGTH, 0, 0, 0, 50, &result) == 0;
        if (bOutIsHanged)
            bOutIsHanged = GetLastError() == ERROR_TIMEOUT;
        if (bOutIsHanged)
            break;
        if (bCheckOnlyHang)
            break;
        if (result == 0)
            break;
        unsigned textLen((unsigned)result + 1);
        if (maxTextLength > 0 && textLen > (unsigned)maxTextLength)
            textLen = maxTextLength;
		TCHAR *text = new TCHAR[textLen];
        text[0] = 0;
		::SendMessage(hWnd, WM_GETTEXT, textLen, (LPARAM)text);
		outStr = text;
		delete []text;
		break;
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
    if (nIDEvent == TIMER_ID_REFRESH) {
        KillTimer(TIMER_ID_REFRESH);
        if (IsTracking()) {
            CPoint curPoint;
            GetCursorPos(&curPoint);
            HWND hCurWind(::WindowFromPoint(curPoint));
            if (hCurWind) {
                CPoint clP(curPoint);
                ::ScreenToClient(hCurWind, &clP);
                HWND hChildWnd(::ChildWindowFromPointEx(hCurWind, clP, CWP_SKIPINVISIBLE));
                if (hChildWnd)
                    hCurWind = hChildWnd;
            }
            if (mhWndCurrent != hCurWind) {
                mhWndCurrent = hCurWind;
                bool bCurrentWndHang = true;
                getWindowText(mhWndCurrent, bCurrentWndHang);
                SetCurrentWndHang(bCurrentWndHang);
            }
            if (curPoint != mCurPoint) {
                mCurPoint = curPoint;
                UpdateChildItemLocation();
                CString coord;
                coord.Format(_T("%d,%d"), mCurPoint.x, mCurPoint.y);
                SetDlgItemText(IDC_STATIC_COORD, coord);
                coord = _T("");
                if (hCurWind) {
                    if (!mChildItemRect.IsRectEmpty()) {
                        curPoint.x = mCurPoint.x - mChildItemRect.left;
                        curPoint.y = mCurPoint.y - mChildItemRect.top;
                    }
                    else {
                        ::ScreenToClient(hCurWind, &curPoint);
                    }
                    coord.Format(_T("%d,%d"), curPoint.x, curPoint.y);
                }
                SetDlgItemText(IDC_STATIC_COORD_SELF, coord);
                coord = _T("");
                curPoint = mCurPoint;
                HWND hWndParent(hCurWind ? ::GetParent(hCurWind) : NULL);
                if (hWndParent)
                    ::ScreenToClient(hWndParent, &curPoint);
                coord.Format(_T("%d,%d"), curPoint.x, curPoint.y);
                SetDlgItemText(IDC_STATIC_COORD_WND, coord);
            }
            UpdateText();
        }
        SetTimer(TIMER_ID_REFRESH, TIMER_REFRESH_INTERVAL, NULL);
    }
    else { // TIMER_ID_KEY
        int shorcutKeysp[] = { VK_CONTROL, VK_MENU, 'P' };
        bool bShorcurPressed(true);
        for (auto sk : shorcutKeysp) {
            bShorcurPressed = bShorcurPressed && GetAsyncKeyState(sk);
            if (!bShorcurPressed)
                break;
        }
        if (GetAsyncKeyState(VK_PAUSE)||bShorcurPressed) {
            if (IsKeyUp()) {
                SetKeyUp(false);
                ToggleTracking();
            }
        }
        else if (!IsKeyUp())
            SetKeyUp(true);
    }
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
static HWND HWNDFromString(LPCTSTR lpstrHwnd)
{
    CString str(lpstrHwnd ? lpstrHwnd : _T("0x0"));
    str.Trim();
    HWND hWnd(NULL);
    std::string strHwnd;
    STLUtils::ChangeType(std::wstring(str), strHwnd);
    void *pWnd(NULL);
    STLUtils::ChangeType(strHwnd, pWnd);
    hWnd = (HWND)pWnd;
    return hWnd;
}
void CWindowFinderDlg::OnEnLinkEditInfo(NMHDR *pNMHDR, LRESULT *pResult)
{
    if (IsTracking())
        return;
    ENLINK *pEnLink = reinterpret_cast<ENLINK *>(pNMHDR);
    if (pEnLink && pEnLink->msg == WM_LBUTTONUP) {
        CRichEditCtrl *pCtrl = (CRichEditCtrl*)GetDlgItem(IDC_EDIT_INFO);
        CString text;
        pCtrl->GetTextRange(pEnLink->chrg.cpMin, pEnLink->chrg.cpMax, text);
        if (text.Find(_T("0x")) == 0) { // window handle
            HWND hWnd(HWNDFromString(text));
            SetCurrentWindow(hWnd);
        }
        else if (text == LINK_TEXT_CHILD_ELEM) {
            CRefreshWindowThread::GetInstance()->RefreshWindows(mhWndCurrent);
        }
        *pResult = 0;
    }
}

LRESULT CWindowFinderDlg::OnUpdateLinks(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
    UpdateLinks();
    return TRUE;
}

LRESULT CWindowFinderDlg::OnWindowIterOp(WPARAM wParam, LPARAM lParam)
{
    if (wParam == 0) {// iter finished
        if (lParam) { //child windows of a window
            HWND hWndParent((HWND)lParam);
            if (hWndParent && hWndParent == mhWndCurrent) {
                GetWindowInfoEx(UpdateChildrenText).hWnd = hWndParent;
                RefreshText(); // Got list of child windows - refresh the text
            }
        }
        else { //top level windows
            OnCbnEditchangeComboSearchWindowImp();
        }
    }
    return TRUE;
}

void CWindowFinderDlg::OnCbnEditchangeComboSearchWindow()
{
    OnCbnEditchangeComboSearchWindowImp(true);
}

void CWindowFinderDlg::OnCbnSelchangeComboSearchWindow()
{
    CComboBox *pComboBox((CComboBox*)GetDlgItem(IDC_COMBO_SEARCH_WINDOW));
    int curSel = pComboBox->GetCurSel();
    HWND hWnd(curSel >= 0 ? (HWND)pComboBox->GetItemData(curSel) : NULL);
    SetCurrentWindow(hWnd);
}

static INT_PTR SplitString(const CString& inString, CStringArray &outStrings, LPCTSTR sep)
{
    outStrings.RemoveAll();
    CString str(inString);
    while (1) {
        int pos = str.FindOneOf(sep);
        if (pos >= 0) {
            CString addStr(str.Left(pos).Trim());
            if (!addStr.IsEmpty())
                outStrings.Add(addStr);
            str.Delete(0, pos + 1);
        }
        else break;
    }
    if (!str.IsEmpty()) {
        CString addStr(str.Trim());
            if (!addStr.IsEmpty())
                outStrings.Add(addStr);
    }
    return outStrings.GetCount();
}

void CWindowFinderDlg::OnCbnEditchangeComboSearchWindowImp(bool bFromEvent /* = false */)
{
    CComboBox *pComboBox((CComboBox*)GetDlgItem(IDC_COMBO_SEARCH_WINDOW));
    CString text;
    pComboBox->GetWindowText(text);
    text.MakeLower();
    CStringArray texts;
    const INT_PTR textsCount(SplitString(text, texts, _T(" \t\r\n")));
    COMBOBOXINFO cbi = { sizeof(COMBOBOXINFO) };
    pComboBox->GetComboBoxInfo(&cbi);
    ::SendMessage(cbi.hwndList, LB_RESETCONTENT, 0, 0);
    if (bFromEvent)
        pComboBox->ShowDropDown();
    else if (::GetForegroundWindow() == GetSafeHwnd()
        && ::GetFocus() != cbi.hwndItem)
        pComboBox->SetFocus();
    const CArray<CWindowEntry>& windowList(CRefreshWindowThread::GetInstance()->GetWindowList(true));
    for (INT_PTR i = 0; i < windowList.GetCount(); ++i) {
        bool bAdd(textsCount == 0);
        const CWindowEntry &we(windowList[i]);
        if (!bAdd) {
            bAdd = true;
            const CString &windowText(we.GetDesc(CWindowEntry::Long));
            for (INT_PTR k = 0; k < textsCount && bAdd; ++k)
                bAdd = windowText.Find(texts[k]) >= 0;
        }
        if (bAdd) {
            int row = pComboBox->AddString(we.GetDesc());
            pComboBox->SetItemData(row, (DWORD_PTR)(HWND)we);
        }
    }
    if (bFromEvent && pComboBox->GetCount() == 0) { // Not found - but a valid window handle - add it
        HWND hwnd(HWNDFromString(text));
        if (hwnd && IsWindow(hwnd)) {
            CWindowEntry we(hwnd);
            we.UpdateDesc();
            int row = pComboBox->AddString(we.GetDesc());
            pComboBox->SetItemData(row, (DWORD_PTR)(HWND)we);
        }
    }
}

void CWindowFinderDlg::ToggleTracking(bool bToggle /* = true */)
{
    if (bToggle) {
        SetTracking(!IsTracking());
        SetTrackingChanged();
    }
	CString title;
	title.Format(_T("WindowFinder - %s"), IsTracking() ? _T("ON") : _T("OFF"));
	SetWindowText(title);
	CMenu* pSysMenu = GetSystemMenu(FALSE);
	pSysMenu->CheckMenuItem(IDM_ABOUTBOX+2, IsTracking() ? MF_CHECKED : MF_UNCHECKED);
    GetDlgItem(IDC_COMBO_SEARCH_WINDOW)->EnableWindow(!IsTracking());
    SetDlgItemText(IDC_COMBO_SEARCH_WINDOW, IsTracking() ? _T("Press Pause or Ctl+Alt+P keys to toggle tracking.") : _T(""));
    if (bToggle)
        RefreshText();
    RefreshComboSearchWindows();
}

bool CWindowFinderDlg::UpdateChildItemText(WindowInfo& wi)
{
    bool &bChildItemUpdated(wi.bUpdated);
    if (mhWndCurrent != GetEditInfoWnd()) {
        CString &text(wi.wndText);
        if (!IsTrackingChanged())
            text.Empty();
        if (IsTracking()) {
            text.Empty();
            UpdateChildItemLocation();
            bChildItemUpdated = IsChildItemChanged() || IsTrackingChanged();
            if (!IsCurrentWndHang() && mAccessibleHelper && bChildItemUpdated) {
                TCHAR *feildsName[] = {
                    _T("Name"),
                    _T("Value"),
                    _T("Description"),
                };
                for (auto name : feildsName) {
                    CString localText = mAccessibleHelper.GetValue(name).c_str();
                    if (!localText.IsEmpty()) {
                        text += CString(_T("Child Item ")) + name + _T(": ") + localText + _T("\r\n");
                    }
                }
                CRect rect;
                mAccessibleHelper.GetRect(rect, IAccessibleHelper::GRF_WRTSelf);
                text += getRectText(rect, _T("Child Item Rect"));
                text += getRectText(mChildItemRect, _T("Child Item Screen Rect"));
                mAccessibleHelper.GetRect(rect, IAccessibleHelper::GRF_WRTParent);
                text += getRectText(rect, _T("Child Item Rect wrt parent"));
            }
        }
    }
    return bChildItemUpdated;
}

bool CWindowFinderDlg::UpdateProcessText(WindowInfo& wi)
{
    wi.bUpdated = GetWindowInfo().bUpdated;
    if (wi.bUpdated) {
        CString &text(wi.wndText);
        text = _T("Process Id: ");
        DWORD pid(0);
        GetWindowThreadProcessId(GetWindowInfo().hWnd, &pid);
        CString str;
        str.Format(_T("%d\r\n"), pid);
        text += str;
        str.Empty();
        {
            TCHAR processImageName[1024] = { 0 };
            ProcessUtil::GetProcessExePath(pid, processImageName, _countof(processImageName));
            str = processImageName;
        }
        if (!str.IsEmpty())
            text += _T("Process path: ") + str + _T("\r\n");
    }
    return wi.bUpdated;
}
struct StyleText
{
    DWORD style;
    LPCTSTR text;
};
#define ADD_STYLE_TEXT(a, s) a.Add({s, _T(#s)})
static CString getStyleText(const CArray<StyleText> &sWndStyle, DWORD style)
{
    CString styleTxt;
    styleTxt.Format(_T("0x%x "), style);
    for (int i = 0; i < sWndStyle.GetCount(); ++i) {
        const StyleText &sT(sWndStyle[i]);
        if ((sT.style&style) == sT.style) {
            styleTxt += sT.text;
            styleTxt += _T("|");
            style &= ~sT.style;
        }
    }
    if (styleTxt.GetAt(styleTxt.GetLength() - 1) == '|')
        styleTxt.Delete(styleTxt.GetLength() - 1);
    styleTxt += _T("\r\n");
    return styleTxt;
}
static CString getStyleText(DWORD style)
{
    static CArray<StyleText> sWndStyle;
    if (sWndStyle.IsEmpty()) {
        ADD_STYLE_TEXT(sWndStyle, WS_OVERLAPPEDWINDOW);
        ADD_STYLE_TEXT(sWndStyle, WS_POPUPWINDOW);
        ADD_STYLE_TEXT(sWndStyle, WS_CAPTION);
        ADD_STYLE_TEXT(sWndStyle, WS_POPUP);
        ADD_STYLE_TEXT(sWndStyle, WS_CHILD);
        ADD_STYLE_TEXT(sWndStyle, WS_MINIMIZE);
        ADD_STYLE_TEXT(sWndStyle, WS_VISIBLE);
        ADD_STYLE_TEXT(sWndStyle, WS_DISABLED);
        ADD_STYLE_TEXT(sWndStyle, WS_CLIPSIBLINGS);
        ADD_STYLE_TEXT(sWndStyle, WS_CLIPCHILDREN);
        ADD_STYLE_TEXT(sWndStyle, WS_MAXIMIZE);
        ADD_STYLE_TEXT(sWndStyle, WS_BORDER);
        ADD_STYLE_TEXT(sWndStyle, WS_DLGFRAME);
        ADD_STYLE_TEXT(sWndStyle, WS_VSCROLL);
        ADD_STYLE_TEXT(sWndStyle, WS_HSCROLL);
        ADD_STYLE_TEXT(sWndStyle, WS_SYSMENU);
        ADD_STYLE_TEXT(sWndStyle, WS_THICKFRAME);
        ADD_STYLE_TEXT(sWndStyle, WS_GROUP);
        ADD_STYLE_TEXT(sWndStyle, WS_TABSTOP);
        ADD_STYLE_TEXT(sWndStyle, WS_MINIMIZEBOX);
        ADD_STYLE_TEXT(sWndStyle, WS_MAXIMIZEBOX);
        ADD_STYLE_TEXT(sWndStyle, WS_OVERLAPPED);
    }
    return getStyleText(sWndStyle, style);
}
static CString getExStyleText(DWORD style)
{

    static CArray<StyleText> sWndStyle;
    if (sWndStyle.IsEmpty()) {
        ADD_STYLE_TEXT(sWndStyle, WS_EX_PALETTEWINDOW);
        ADD_STYLE_TEXT(sWndStyle, WS_EX_OVERLAPPEDWINDOW);
        ADD_STYLE_TEXT(sWndStyle, WS_EX_DLGMODALFRAME);
        ADD_STYLE_TEXT(sWndStyle, WS_EX_NOPARENTNOTIFY);
        ADD_STYLE_TEXT(sWndStyle, WS_EX_TOPMOST);
        ADD_STYLE_TEXT(sWndStyle, WS_EX_ACCEPTFILES);
        ADD_STYLE_TEXT(sWndStyle, WS_EX_TRANSPARENT);
        ADD_STYLE_TEXT(sWndStyle, WS_EX_MDICHILD);
        ADD_STYLE_TEXT(sWndStyle, WS_EX_TOOLWINDOW);
        ADD_STYLE_TEXT(sWndStyle, WS_EX_WINDOWEDGE);
        ADD_STYLE_TEXT(sWndStyle, WS_EX_CLIENTEDGE);
        ADD_STYLE_TEXT(sWndStyle, WS_EX_CONTEXTHELP);
        ADD_STYLE_TEXT(sWndStyle, WS_EX_RIGHT);
        ADD_STYLE_TEXT(sWndStyle, WS_EX_LEFT);
        ADD_STYLE_TEXT(sWndStyle, WS_EX_RTLREADING);
        ADD_STYLE_TEXT(sWndStyle, WS_EX_LTRREADING);
        ADD_STYLE_TEXT(sWndStyle, WS_EX_LEFTSCROLLBAR);
        ADD_STYLE_TEXT(sWndStyle, WS_EX_RIGHTSCROLLBAR);
        ADD_STYLE_TEXT(sWndStyle, WS_EX_CONTROLPARENT);
        ADD_STYLE_TEXT(sWndStyle, WS_EX_STATICEDGE);
        ADD_STYLE_TEXT(sWndStyle, WS_EX_APPWINDOW);
        ADD_STYLE_TEXT(sWndStyle, WS_EX_LAYERED);
        ADD_STYLE_TEXT(sWndStyle, WS_EX_NOINHERITLAYOUT);
        ADD_STYLE_TEXT(sWndStyle, WS_EX_NOREDIRECTIONBITMAP);
        ADD_STYLE_TEXT(sWndStyle, WS_EX_LAYOUTRTL);
        ADD_STYLE_TEXT(sWndStyle, WS_EX_COMPOSITED);
        ADD_STYLE_TEXT(sWndStyle, WS_EX_NOACTIVATE);
    }
    return getStyleText(sWndStyle, style);
}
bool CWindowFinderDlg::UpdateStyleText(WindowInfo & wi)
{
    wi.bUpdated = GetWindowInfo().bUpdated;
    if (wi.bUpdated) {
        CString &text(wi.wndText);
        HWND hWnd(GetWindowInfo().hWnd);
        text = _T("Style: ") + getStyleText((DWORD)GetWindowLong(hWnd, GWL_STYLE));
        text += _T("Ex Style: ") + getExStyleText((DWORD)GetWindowLong(hWnd, GWL_EXSTYLE));
    }
    return wi.bUpdated;
}

static void ScreenToClient(_In_ HWND hWnd, _Inout_ LPRECT lpRect)
{
    LPPOINT pPoint((LPPOINT)lpRect);
    ::ScreenToClient(hWnd, pPoint);
    ::ScreenToClient(hWnd, pPoint+1);
}

#define TEXT_WINDOW_NOT_RESPONDING _T("(Window Not Responding...)")

bool CWindowFinderDlg::UpdateSelfText(WindowInfo& wi)
{
    wi.bUpdated = (wi.hWnd != mhWndCurrent);
    if (wi.bUpdated) {
        wi.hWnd = mhWndCurrent;
        CString &text(wi.wndText);
        text.Format(_T("%s0x%x\r\n"), LINK_TEXT_HANDLE, mhWndCurrent);
        bool bHang(false);
        CString titleText(IsCurrentWndHang() ? TEXT_WINDOW_NOT_RESPONDING : getWindowText(mhWndCurrent, bHang));
        text += _T("Title: ") + titleText + _T("\r\n");
        {
            TCHAR className[1024] = { 0 };
            GetClassName(mhWndCurrent, className, sizeof(className) / sizeof(className[0]));
            text += _T("Class: ") + CString(className) + _T("\r\n");
        }
        CRect cr;
        ::GetWindowRect(mhWndCurrent, cr);
        text += getRectText(cr, _T("Screen Rect"));
        HWND hWndPArent(::GetParent(mhWndCurrent));
        if (hWndPArent) {
            ::ScreenToClient(hWndPArent, cr);
            text += getRectText(cr, _T("Rect wrt Parent"));
        }
        ::GetClientRect(mhWndCurrent, cr);
        text += getRectText(cr, _T("Client Rect"));
    }
    return wi.bUpdated;
}

bool CWindowFinderDlg::UpdateParentText(WindowInfo& wi)
{
    wi.bUpdated = GetWindowInfo().bUpdated;
    if (wi.bUpdated) {
        HWND hWndPArent(mhWndCurrent ? ::GetParent(mhWndCurrent) : NULL);
        wi.bUpdated = (wi.hWnd != hWndPArent);
        if (wi.bUpdated) {
            wi.hWnd = hWndPArent;
            CString &text(wi.wndText);
            if (hWndPArent) {
                text.Format(_T("Parent %s0x%x\r\n"), LINK_TEXT_HANDLE, hWndPArent);
                bool bHang(false);
                text += _T("Parent Title: ") + getWindowText(hWndPArent, bHang) + _T("\r\n");
            }
            else
                text.Empty();
        }
    }
    return wi.bUpdated;
}

bool CWindowFinderDlg::UpdateChildrenText(WindowInfo& wi)
{
    bool bUpdated(false);
    CString &text(wi.wndText);
    if (IsTracking()) {// Tracking - no update
        if (wi.bUpdated) {
            wi.bUpdated = false;
            bUpdated = true;
            text.Empty();
        }
    }
    else {      // Not tracking
        text = LINK_TEXT_CHILD_ELEM _T("\r\n");
        if (!wi.bUpdated) { // First call after toggle tracking
            bUpdated = wi.bUpdated = true;      // Update text only
        }
        else if (wi.hWnd) {     // Second call when children found
            bUpdated = wi.bUpdated = true; // update children
            wi.hWnd = NULL;     // set to null so that subsequent call does not update again
            const CArray<CWindowEntry>& windowList(CRefreshWindowThread::GetInstance()->GetWindowList(false));
            for (INT_PTR i = 0; i < windowList.GetCount(); ++i) {
                CString childText;
                childText.Format(_T("   %d. Child %s%s\r\n"), i+1, LINK_TEXT_HANDLE, (LPCTSTR)windowList[i].GetDesc());
                text += childText;
            }
        }
    }
    return bUpdated;
}

bool CWindowFinderDlg::UpdateForegroundText(WindowInfo& wi)
{
    HWND hWndForeground = ::GetForegroundWindow();
    wi.bUpdated = (wi.hWnd != hWndForeground);
    if (wi.bUpdated) {
        wi.hWnd = hWndForeground;
        DWORD attachedThreaDID(-1);
        CString &text(wi.wndText);
        if (hWndForeground) {
            if (mAttachedThreaDID)
                AttachThreadInput(GetCurrentThreadId(), mAttachedThreaDID, FALSE);
            mAttachedThreaDID = GetWindowThreadProcessId(hWndForeground, NULL);
            BOOL bAttached = AttachThreadInput(GetCurrentThreadId(), mAttachedThreaDID, TRUE);
            if (!bAttached)
                mAttachedThreaDID = 0;
            DWORD le = GetLastError();
            text.Format(_T("Foregorund %s0x%x\r\n"), LINK_TEXT_HANDLE, hWndForeground);
            bool bHung(false);
            CString title = getWindowText(hWndForeground, bHung);
            if (bHung)
                title = TEXT_WINDOW_NOT_RESPONDING;
            text += _T("Foregorund Title: ") + title + _T("\r\n");
        }
        else
            text.Empty();
    }
    return wi.bUpdated;
}

bool CWindowFinderDlg::UpdateFocusText(WindowInfo& wi)
{
    HWND hWndFocus = ::GetFocus();
    wi.bUpdated = (wi.hWnd != hWndFocus);
    if (wi.bUpdated) {
        wi.hWnd = hWndFocus;
        CString &text(wi.wndText);
        if (hWndFocus) {
            text.Format(_T("Focus %s0x%x\r\n"), LINK_TEXT_HANDLE, hWndFocus);
            bool bHang(false);
            CString focusText(getWindowText(hWndFocus, bHang));
            TCHAR className[1024] = { 0 };
            GetClassName(hWndFocus, className, sizeof(className) / sizeof(className[0]));
            text += _T("Focus Class: ") + CString(className) + _T("\r\n");
            if (!bHang && hWndFocus != GetEditInfoWnd()
                && focusText.IsEmpty()) {
                IAccessibleHelper ih;
                ih.InitFromWindow(hWndFocus);
                TCHAR *feildsName[] = {
                    _T("Focus"),
                    _T("Value"),
                    _T("Name"),
                    _T("Description"),
                };
                for (auto name : feildsName) {
                    focusText = ih.GetValue(name).c_str();
                    if (!focusText.IsEmpty())
                        break;
                }
            }
            text += _T("Focus Title: ") + focusText + _T("\r\n");
        }
        else
            text.Empty();
    }
    return wi.bUpdated;
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
    SetTrackingChanged(false);
    return bUpdated;
}

CWindowFinderDlg::WindowInfo& CWindowFinderDlg::GetWindowInfo(UpdateTextProc inProc /* = NULL */)
{
    if (inProc == NULL && mWindowsInfo.GetCount() > 0)
        inProc = mWindowsInfo[0].fnUpdateTextProc;
    for (INT_PTR i = 0; i < mWindowsInfo.GetCount();++i) {
        if (inProc == mWindowsInfo[i].fnUpdateTextProc)
            return mWindowsInfo[i];
    }
    static WindowInfo wi;
    return wi;
}

void CWindowFinderDlg::UpdateChildItemLocation()
{
    if (IsCurrentWndHang()) {
        if (!mChildItemRect.IsRectEmpty())
            mChildItemRect.SetRectEmpty();
        return;
    }
    if ((GetTickCount() - mChildItemAccessibleUpdatedTime) > (TIMER_REFRESH_INTERVAL-10)) {
        mAccessibleHelper.InitFromPoint(mCurPoint.x, mCurPoint.y);
        mAccessibleHelper.Location();
        CRect oldRecr(mChildItemRect);
        mAccessibleHelper.GetRect(mChildItemRect);
        SetChildItemChanged((oldRecr != mChildItemRect) != FALSE);
        mChildItemAccessibleUpdatedTime = GetTickCount();
    }
}

CWindowFinderDlg::WindowInfo::WindowInfo(UpdateTextProc proc)
    : bUpdated(false), hWnd(NULL), fnUpdateTextProc(proc)
{

}
