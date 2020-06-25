// Drawer.cpp : implementation file
//

#include "stdafx.h"
#include "Drawer.h"


// CDrawer dialog

// Drawer window flag
#define DWF_MOVING 1 // Drawer is either closing or opening
#define DWF_CLOSE 2 // Drawer is being closing
#define DWF_INSIDE 4 // Drawer is inside parent window


IMPLEMENT_DYNAMIC(CDrawer, CDialog)

CDrawer::CDrawer()
	: m_DrawerPosition(CDrawer::DP_Bottom),
	m_uFlags(DWF_CLOSE), m_lpParentPrevWndFunc(NULL), mTransparentValue(255)
{

}

CDrawer::~CDrawer()
{
}

void CDrawer::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}
static LRESULT CALLBACK DrawerParentWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CDrawer *pDrawer((CDrawer *)GetWindowLongPtr(hWnd, GWLP_USERDATA));
	return pDrawer->DrawerParentWindowProc(hWnd, msg, wParam, lParam);
}
LRESULT CDrawer::DrawerParentWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT res = m_lpParentPrevWndFunc(hWnd, msg, wParam, lParam);
	switch (msg) {
	case WM_MOVE:
	case WM_SIZE:
		SetPosition(GetPosition());
		break;
	}
	return res;
}
BOOL CDrawer::Init(UINT nIDTemplate, CWnd* pParent, DrawerPosition dp)
{
	BOOL bCreated(TRUE);
	if (m_hWnd == NULL) {
		bCreated = Create(nIDTemplate, pParent);
		HWND hWndParentRoot(pParent->GetAncestor(GA_ROOT)->GetSafeHwnd());
		SetWindowLongPtr(hWndParentRoot, GWLP_USERDATA, (LONG_PTR)this);
		m_lpParentPrevWndFunc = (WNDPROC)GetWindowLongPtr(hWndParentRoot, GWLP_WNDPROC);
		SetWindowLongPtr(hWndParentRoot, GWLP_WNDPROC, (LONG_PTR)::DrawerParentWindowProc);
		SetPosition(dp);
		GetWindowRect(&m_DrawerRect);
		if (IsOpen())
			Open();
	}
	return bCreated;
}
#define DW_MARGIN 5

BOOL CDrawer::SetPosition(DrawerPosition dp)
{
	if (m_uFlags & DWF_MOVING)
		return FALSE;
	m_DrawerPosition = dp;
	BOOL bSupportsResizing((GetStyle()&WS_THICKFRAME)!=0);
	RECT parentWindowRect, windowRect, desktopWindowRect;
	GetWindowRect(&windowRect);
	GetParent()->GetWindowRect(&parentWindowRect);
	//LONG parentWindowWidth(parentWindowRect.right-parentWindowRect.left);
	//LONG parentWindowHeight(parentWindowRect.bottom-parentWindowRect.top);
	LONG windowWidth(windowRect.right-windowRect.left);
	LONG windowHeight(windowRect.bottom-windowRect.top);
	GetDesktopWindow()->GetWindowRect(&desktopWindowRect);
	const CWnd *pWndInsertAfter(NULL);
	switch (m_DrawerPosition) {
	case DP_Left:
		{
			if (parentWindowRect.left-desktopWindowRect.left >= windowWidth) { // fits in - keep outside
				windowRect.right = parentWindowRect.left;
				windowRect.left = windowRect.right - windowWidth;
			}
			else { // does not fit - adjust inside
				windowRect.left = parentWindowRect.left;
				windowRect.right = windowRect.left + windowWidth;
				pWndInsertAfter = &CWnd::wndTop;
			}
			windowRect.top = parentWindowRect.top + DW_MARGIN;
			if (bSupportsResizing)
				windowRect.bottom = parentWindowRect.bottom - DW_MARGIN;
			else
				windowRect.bottom = windowRect.top + windowHeight;
		}
		break;
	case DP_Top:
		{
			if (parentWindowRect.top-desktopWindowRect.top >= windowHeight) { // fits in - keep outside
				windowRect.bottom = parentWindowRect.top;
				windowRect.top = windowRect.bottom - windowHeight;
			}
			else { // does not fit - adjust inside
				windowRect.top = parentWindowRect.top;
				windowRect.bottom = windowRect.top + windowHeight;
				pWndInsertAfter = &CWnd::wndTop;
			}
			windowRect.left = parentWindowRect.left + DW_MARGIN;
			if (bSupportsResizing)
				windowRect.right = parentWindowRect.right - DW_MARGIN;
			else
				windowRect.right = windowRect.left + windowWidth;
		}
		break;
	case DP_Right:
		{
			if (desktopWindowRect.right-parentWindowRect.right >= windowWidth) { // fits in - keep outside
				windowRect.left = parentWindowRect.right;
				windowRect.right = windowRect.left+windowWidth;
			}
			else { // does not fit - adjust inside
				windowRect.right = parentWindowRect.right;
				windowRect.left = windowRect.right - windowWidth;
				pWndInsertAfter = &CWnd::wndTop;
			}
			windowRect.top = parentWindowRect.top + DW_MARGIN;
			if (bSupportsResizing)
				windowRect.bottom = parentWindowRect.bottom - DW_MARGIN;
			else
				windowRect.bottom = windowRect.top + windowHeight;
		}
		break;
	case DP_Bottom:
		{
			if (desktopWindowRect.bottom-parentWindowRect.bottom >= windowHeight) { // fits in - keep outside
				windowRect.top = parentWindowRect.bottom;
				windowRect.bottom = windowRect.top + windowHeight;
			}
			else { // does not fit - adjust inside
				windowRect.bottom = parentWindowRect.bottom;
				windowRect.top = windowRect.bottom - windowHeight;
				pWndInsertAfter = &CWnd::wndTop;
			}
			windowRect.left = parentWindowRect.left + DW_MARGIN;
			if (bSupportsResizing)
				windowRect.right = parentWindowRect.right - DW_MARGIN;
			else
				windowRect.right = windowRect.left + windowWidth;
		}
		break;
	}
	if (pWndInsertAfter == &CWnd::wndTop)
		m_uFlags |= DWF_INSIDE;
	else
		m_uFlags &= ~DWF_INSIDE;
	BOOL bSuccess(SetWindowPos(pWndInsertAfter, windowRect.left, windowRect.top,
		windowRect.right-windowRect.left, windowRect.bottom-windowRect.top,
		SWP_NOACTIVATE));
	if (mTransparentValue != 255) {
		unsigned char tVal = mTransparentValue;
		if (!(m_uFlags & DWF_TRANSPARENT)) {
			if (!(m_uFlags & DWF_TRANSPARENT_INSIDE) ^ !(m_uFlags & DWF_INSIDE))
				tVal = 255;
		}
		DoTransparent(tVal);
	}
	return bSuccess;
}

BOOL CDrawer::Open() // Open drawer window
{
	m_uFlags &= ~DWF_CLOSE;
	return StartMove();
}
BOOL CDrawer::Close() // Close drawer window
{
	m_uFlags |= DWF_CLOSE;
	return StartMove();
}

BOOL CDrawer::IsOpen() const
{
	return !(m_uFlags & DWF_CLOSE);
}

// Drawer timers
#define DT_MOVE 1

BOOL CDrawer::StartMove()
{
	if (!IsMoving()) {
		m_uFlags |= DWF_MOVING;
		if (IsWindowVisible())
			GetWindowRect(&m_DrawerRect);
		else if (IsOpen())
			ShowWindow(SW_SHOW);
		SetTimer(DT_MOVE, 10, NULL); // Start Timer
	}
	return TRUE;
}

BOOL CDrawer::IsMoving() const
{
	return (m_uFlags & DWF_MOVING) != 0;
}
void CDrawer::SetTransparent(unsigned char transParentValue, UINT uFlags)
{
	uFlags &= DWF_TRANSPARENT|DWF_TRANSPARENT_INSIDE;
	m_uFlags |= uFlags;
	mTransparentValue = transParentValue;
	DoTransparent(mTransparentValue);
}
void CDrawer::DoTransparent(unsigned char transParentValue)
{
	BYTE alpha = 255;
	GetLayeredWindowAttributes(NULL, &alpha, NULL);
	if (alpha == transParentValue)
		return;
	LONG exStyle = GetWindowLong(m_hWnd, GWL_EXSTYLE);
	if (transParentValue == 255) { // Switch Off
		if (exStyle & WS_EX_LAYERED) {
			exStyle &= ~WS_EX_LAYERED;
			SetWindowLong(m_hWnd, GWL_EXSTYLE, exStyle);
		}
	}
	else {
		// Switch on and set transparentcy
		if (!(exStyle & WS_EX_LAYERED))
			SetWindowLong(m_hWnd, GWL_EXSTYLE, WS_EX_LAYERED | exStyle);
		SetLayeredWindowAttributes(0, transParentValue, LWA_ALPHA);
	}
}

BEGIN_MESSAGE_MAP(CDrawer, CDialog)
	ON_WM_TIMER()
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()


// CDrawer message handlers
void CDrawer::OnTimer(UINT_PTR nIDEvent)
{
	switch (nIDEvent) {
	case DT_MOVE:
		OnTimerMove();
		break;
	}
}
#define DRAWER_MOVE_PIXEL 20
#define DRAWER_MIN_WIDTH 15
void CDrawer::OnTimerMove()
{
	int size = DRAWER_MOVE_PIXEL;
	if (m_uFlags & DWF_CLOSE)
		size = -DRAWER_MOVE_PIXEL;
	RECT windowRect;
	GetWindowRect(&windowRect);
	BOOL bFinished(FALSE);
	switch (m_DrawerPosition) {
	case DP_Left:
		{
			if (m_uFlags & DWF_INSIDE)
				windowRect.right += size;
			else
				windowRect.left -= size;
			LONG width = windowRect.right-windowRect.left;
			bFinished = size > 0 && width >= (m_DrawerRect.right-m_DrawerRect.left)
				|| width < DRAWER_MIN_WIDTH;
		}
		break;
	case DP_Top:
		{
			if (m_uFlags & DWF_INSIDE)
				windowRect.bottom += size;
			else
				windowRect.top -= size;
			LONG height = windowRect.bottom-windowRect.top;
			bFinished = size > 0 && height >= (m_DrawerRect.bottom-m_DrawerRect.top)
				|| height < DRAWER_MIN_WIDTH;
		}
		break;
	case DP_Right:
		{
			if (m_uFlags & DWF_INSIDE)
				windowRect.left -= size;
			else
				windowRect.right += size;
			LONG width = windowRect.right-windowRect.left;
			bFinished = size > 0 && width >= (m_DrawerRect.right-m_DrawerRect.left)
				|| width < DRAWER_MIN_WIDTH;
		}
		break;
	case DP_Bottom:
		{
			if (m_uFlags & DWF_INSIDE)
				windowRect.top -= size;
			else
				windowRect.bottom += size;
			LONG height = windowRect.bottom-windowRect.top;
			bFinished = size > 0 && height >= (m_DrawerRect.bottom-m_DrawerRect.top)
				|| height < DRAWER_MIN_WIDTH;
		}
		break;
	}
	if (bFinished) {
		KillTimer(DT_MOVE);
		m_uFlags &= ~DWF_MOVING;
		if (size < 0) {// close
			ShowWindow(SW_HIDE);
		}
		else {
			windowRect = m_DrawerRect;
		}
	}
	const CWnd *pWndInsertAfter(NULL);
	if (m_uFlags & DWF_INSIDE)
		pWndInsertAfter = &CWnd::wndTop;
	SetWindowPos(pWndInsertAfter, windowRect.left, windowRect.top,
		windowRect.right-windowRect.left, windowRect.bottom-windowRect.top,
		SWP_NOACTIVATE);
	if (bFinished)
		SetPosition(m_DrawerPosition);
}
void CDrawer::OnShowWindow(BOOL bShow, UINT nStatus)
{
	if (bShow && !IsOpen() && nStatus) // If it already hidden and restored to show - do not show
		return;
	CDialog::OnShowWindow(bShow, nStatus);
}