// DialogMediaControl.cpp : implementation file
//

#include "stdafx.h"
#include "Find.h"
#include "DialogMediaControl.h"
#include "DialogPreviewMedia.h"


// CDialogMediaControl dialog

IMPLEMENT_DYNAMIC(CDialogMediaControl, CDrawer)

CDialogMediaControl::CDialogMediaControl(CWnd* pParent /*=NULL*/)
: m_VLCPlayer(NULL), m_bManualPositioning(false), mTotalTime(0),
m_bShowTimeInReverse(false), m_bPaused(false), m_iTimeOutCount(-1),
mControlResizer(this), mLastUpdateTime(0), m_bFinished(false), m_pDialogPreviewMedia((CDialogPreviewMedia *)pParent),
m_bFullScreen(false)
{

}

CDialogMediaControl::~CDialogMediaControl()
{
}

void CDialogMediaControl::DoDataExchange(CDataExchange* pDX)
{
	CDrawer::DoDataExchange(pDX);
}
static CString GetStringTime(__int64 ms)
{
	__int64 h(ms/(1000*60*60));
	__int64 mins(ms-h*1000*60*60);
	__int64 m(mins/(1000*60));
	__int64 s(mins-m*1000*60);
	s /= 1000;
	CString t;
	t.Format(_T("%02d:%02d:%02d"), (int)h,(int)m,(int)s);
	return t;
}
void CDialogMediaControl::UpdateTimeControls()
{
	__int64 curTime(m_VLCPlayer->GetTime());
	int newPos(0);
	if (mTotalTime)
		newPos = (int)((curTime * 1024) / mTotalTime);
	CSliderCtrl *pSlider((CSliderCtrl *)GetDlgItem(IDC_SLIDER_MEDIA_POS));
	pSlider->SetPos(newPos);
	CString t;
	if (m_bShowTimeInReverse) {
		curTime = mTotalTime - curTime;
		t += _T("-");
	}
	t += GetStringTime(curTime) + mStrTotalTime;
	SetDlgItemText(IDC_STATIC_MEDIA_TIME, t);
}
void CDialogMediaControl::UpdatePosition()
{
	if (m_hWnd == NULL)
		return;
	if (IsOpen()) {
		DWORD currentTime(GetTickCount());
		if (mTotalTime == 0) {
			mTotalTime = m_VLCPlayer->GetLength();
			CSliderCtrl *pSlider((CSliderCtrl *)GetDlgItem(IDC_SLIDER_MEDIA_VOLUME));
			pSlider->SetPos(m_VLCPlayer->GetVolume());
			SetDlgItemText(IDC_BUTTON_MEDIA_PLAY_PAUSE, _T("[]"));
			mStrTotalTime = _T("/") + GetStringTime(mTotalTime);
			mLastUpdateTime = currentTime;
		}
		if (currentTime - mLastUpdateTime >= 1000) {
			mLastUpdateTime = currentTime;
			UpdateTimeControls();
		}
	}
}
#define DPM_TIMER_SHOWHIDE_CONTROL 101
void CDialogMediaControl::ResetConrols()
{
	if (m_hWnd == NULL)
		return;
	SetTimer(DPM_TIMER_SHOWHIDE_CONTROL, 300, NULL);
	m_bManualPositioning = false;
	mTotalTime = 0;
	m_bShowTimeInReverse = false;
	m_bPaused = false;
	m_bFinished = false;
	m_iTimeOutCount = -1;
	mLastUpdateTime = 0;
	CSliderCtrl *pSlider((CSliderCtrl *)GetDlgItem(IDC_SLIDER_MEDIA_POS));
	pSlider->SetRange(0, 1024);
	pSlider->SetPos(0);
	pSlider = (CSliderCtrl *)GetDlgItem(IDC_SLIDER_MEDIA_VOLUME);
	pSlider->SetRange(0, 100);
	pSlider->SetPos(0);
	pSlider->SetPos(m_VLCPlayer->GetVolume());
	SetDlgItemText(IDC_BUTTON_MEDIA_PLAY_PAUSE, _T(">"));
	SetDlgItemText(IDC_STATIC_MEDIA_TIME, _T(""));
}
void CDialogMediaControl::StopControls()
{
	if (m_hWnd == NULL)
		return;
	KillTimer(DPM_TIMER_SHOWHIDE_CONTROL);
}

BEGIN_MESSAGE_MAP(CDialogMediaControl, CDrawer)
	ON_BN_CLICKED(IDC_BUTTON_MEDIA_PLAY_PAUSE, &CDialogMediaControl::OnBnClickedButtonMediaPlayPause)
	ON_WM_HSCROLL()
	ON_STN_CLICKED(IDC_STATIC_MEDIA_TIME, &CDialogMediaControl::OnStnClickedStaticMediaTime)
	ON_WM_TIMER()
	ON_WM_SIZE()
	ON_WM_SIZING()
	ON_BN_CLICKED(IDC_BUTTON_MEDIA_FULLSCREEN, &CDialogMediaControl::OnBnClickedButtonMediaFullscreen)
END_MESSAGE_MAP()


// CDialogMediaControl message handlers

void CDialogMediaControl::OnBnClickedButtonMediaPlayPause()
{
	CWnd *pCtrl(GetDlgItem(IDC_BUTTON_MEDIA_PLAY_PAUSE));

	m_iTimeOutCount = 0;
	m_bPaused = !m_bPaused;
	if (m_bPaused) {
		m_VLCPlayer->Pause();
		SetDlgItemText(IDC_BUTTON_MEDIA_PLAY_PAUSE, _T(">"));
		m_ToolTip.UpdateTipText(_T("Play"), pCtrl, IDC_BUTTON_MEDIA_PLAY_PAUSE);
	}
	else {
		if (m_bFinished) {
			m_bFinished = false;
			m_pDialogPreviewMedia->ShowPreview(m_pDialogPreviewMedia->GetFileToPreview());
			m_ToolTip.UpdateTipText(_T("Play"), pCtrl, IDC_BUTTON_MEDIA_PLAY_PAUSE);
		}
		else {
			m_VLCPlayer->Play();
			SetDlgItemText(IDC_BUTTON_MEDIA_PLAY_PAUSE, _T("[]"));
			m_ToolTip.UpdateTipText(_T("Pause"), pCtrl, IDC_BUTTON_MEDIA_PLAY_PAUSE);
		}
	}
}

void CDialogMediaControl::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    UNREFERENCED_PARAMETER(nPos);
	if (nSBCode == SB_ENDSCROLL)
		return;
	if (pScrollBar != NULL) {
		CSliderCtrl *pSlider((CSliderCtrl*)pScrollBar);
		switch (pSlider->GetDlgCtrlID()) {
		case IDC_SLIDER_MEDIA_POS:
			{
				__int64 curTime(m_VLCPlayer->GetTime());
				__int64 newTime = curTime;
				switch (nSBCode) {
				case SB_LINERIGHT:
						newTime = curTime + 5000; // 5 secs
					break;
				case SB_LINELEFT:
						newTime = curTime - 5000;
					break;
				case SB_PAGELEFT:
						newTime = curTime - 5*1000*60; // 5 mins
					break;
				case SB_PAGERIGHT:
						newTime = curTime + 5*1000*60;
					break;
				}
				if (newTime == curTime) {
					newTime = pSlider->GetPos();
					newTime = (newTime * mTotalTime) / 1024;
				}
				m_iTimeOutCount = 0;
				m_bManualPositioning = true;
				m_VLCPlayer->SetTime(newTime);
				UpdateTimeControls();
			}
			break;
		case IDC_SLIDER_MEDIA_VOLUME:
			m_VLCPlayer->SetVolume(pSlider->GetPos());
			break;
		}
	}
}
void CDialogMediaControl::OnStnClickedStaticMediaTime()
{
	m_bShowTimeInReverse = !m_bShowTimeInReverse;
	UpdateTimeControls();
}
void CDialogMediaControl::OnTimer(UINT_PTR nIDEvent)
{
	switch (nIDEvent) {
	case DPM_TIMER_SHOWHIDE_CONTROL:
		OnTimerShowHideControls();
		break;
	}
	CDrawer::OnTimer(nIDEvent);
}
void CDialogMediaControl::OnTimerShowHideControls()
{
	do {
		if (GetCapture())
			break;
		POINT cursorPos;
		if (!GetCursorPos(&cursorPos))
			break;
		HWND hWnd = ::WindowFromPoint(cursorPos);
		if (hWnd == NULL)
			break;
		hWnd = ::GetAncestor(hWnd, GA_ROOT);
		// Mouse on preview dialog or control dialog
		bool bMouseOnOurWindow(hWnd == GetParent()->GetAncestor(GA_ROOT)->GetSafeHwnd() || hWnd == GetSafeHwnd());
		++m_iTimeOutCount;
		if (m_iTimeOutCount) {
			if (IsOpen()) { // Already open - close it
				if (!bMouseOnOurWindow || mMouseHoverPoint == cursorPos) {
					if (m_iTimeOutCount > 9)
						Close();
				}
				else m_iTimeOutCount = 0;
			}
			else if (bMouseOnOurWindow) {
				if (mMouseHoverPoint != cursorPos) {
					Open();
					m_iTimeOutCount = 0;
				}
			}
			else m_iTimeOutCount = 0;
		}
		mMouseHoverPoint = cursorPos;
	} while (0);
}
BOOL CDialogMediaControl::OnInitDialog()
{
	BOOL bRet(CDrawer::OnInitDialog());

	mControlResizer.AddControl(IDC_SLIDER_MEDIA_POS, RSZF_RIGHT_FIXED|RSZF_SIZEY_FIXED);
	mControlResizer.AddControl(IDC_SLIDER_MEDIA_VOLUME, RSZF_RIGHT_FIXED);
	mControlResizer.AddControl(IDC_STATIC_MEDIA_TIME, RSZF_RIGHT_FIXED);
	mControlResizer.AddControl(IDC_BUTTON_MEDIA_FULLSCREEN, RSZF_SIZE_FIXED|RSZF_BOTTOM_FIXED);
	HICON hIcon(::AfxGetApp()->LoadIcon(IDI_ICON_FULLSCREEN));
	((CButton*)GetDlgItem(IDC_BUTTON_MEDIA_FULLSCREEN))->SetIcon(hIcon);
	SetTransparent(150, DWF_TRANSPARENT_INSIDE);
	m_ToolTip.Create(this);
	int ids[] = {IDC_BUTTON_MEDIA_FULLSCREEN, IDC_BUTTON_MEDIA_PLAY_PAUSE};
	LPCTSTR tips[] = {_T("Toggle Fullscreen"), _T("Pause")};
	for (int i = 0; i < sizeof(ids)/sizeof(ids[0]); i++) {
		RECT cr;
		CWnd *pCtrl(GetDlgItem(ids[i]));
		pCtrl->GetClientRect(&cr);
		m_ToolTip.AddTool(pCtrl, tips[i], &cr, ids[i]);
	}
	m_ToolTip.Activate(TRUE);
	return bRet;
}
void CDialogMediaControl::OnSize(UINT nType, int cx, int cy)
{
	CDrawer::OnSize(nType, cx, cy);
	switch (nType) {
	case SIZE_MINIMIZED:
		break;
	default:
		mControlResizer.DoReSize();
		InvalidateRect(NULL, FALSE);
		break;
	}
}
void CDialogMediaControl::OnSizing(UINT nSize, LPRECT lpRect)
{
	if (lpRect->right - lpRect->left < 180) {
		switch (nSize) {
		case WMSZ_BOTTOMRIGHT:
		case WMSZ_RIGHT:
		case WMSZ_TOPRIGHT:
			lpRect->right = lpRect->left + 180;
			break;
		case WMSZ_BOTTOMLEFT:
		default:
			lpRect->left = lpRect->right - 180;
		}
	}
	if (lpRect->bottom - lpRect->top < 120
		|| lpRect->bottom - lpRect->top > 120) {
		switch (nSize) {
		case WMSZ_BOTTOM:
		case WMSZ_BOTTOMLEFT:
		case WMSZ_BOTTOMRIGHT:
			lpRect->bottom = lpRect->top + 120;
			break;
		default:
			lpRect->top = lpRect->bottom - 120;
		}
	}
	CDrawer::OnSizing(nSize, lpRect);
}
BOOL CDialogMediaControl::PreTranslateMessage(MSG* pMsg)
{
	m_ToolTip.RelayEvent(pMsg);
	switch (pMsg->message) {
	case WM_KEYDOWN:
		if (pMsg->wParam == VK_SPACE)
			theApp.GetMainWnd()->PostMessage(WM_TOGGLE_PREVIEW);
		break;
	}
	return CDrawer::PreTranslateMessage(pMsg);
}

void CDialogMediaControl::OnBnClickedButtonMediaFullscreen()
{
	m_bFullScreen = !m_bFullScreen;
	CRect newWindowSize;
	CWnd *pDlg(m_pDialogPreviewMedia->GetParent());
	if (m_bFullScreen) {
		// Save current rect
		pDlg->GetWindowRect(&m_WindowSize);
		GetDesktopWindow()->GetWindowRect(&newWindowSize);
	}
	else {
		newWindowSize = m_WindowSize;
	}
	pDlg->SetWindowPos(&wndTopMost, newWindowSize.left, newWindowSize.top,
		newWindowSize.right-newWindowSize.left, newWindowSize.bottom-newWindowSize.top, 0);
		
}
