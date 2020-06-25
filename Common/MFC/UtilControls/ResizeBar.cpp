// ResizeBar.cpp : implementation file
//

#include "stdafx.h"
#include "ResizeBar.h"


// CResizeBar

#define RBF_HORIZONTAL FLAGBIT(0)
#define RBF_CONTROLS_HIDDEN FLAGBIT(1)
#define RBF_CONTROLS_HIDDEN_RIGHT FLAGBIT(2)
#define RBF_CONTROL_DRAGGED FLAGBIT(3)

IMPLEMENT_DYNAMIC(CResizeBar, CStatic)

CResizeBar::CResizeBar(CDialog *pDialog)
: m_iMinLeft(0), m_iMinRight(0), mDragControlResizer(pDialog), muFlags(0)
{
	m_hCursor = ::LoadCursor(NULL, IDC_SIZEWE);
}

CResizeBar::~CResizeBar()
{
}


BEGIN_MESSAGE_MAP(CResizeBar, CStatic)
	ON_WM_SETCURSOR()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

void CResizeBar::SetMinLeftRight(int ml, int mr)
{
	m_iMinLeft = ml;
	m_iMinRight = mr;
}

// CResizeBar message handlers


BOOL CResizeBar::OnSetCursor(CWnd *pWnd, UINT nHitTest, UINT message)
{
    UNREFERENCED_PARAMETER(pWnd);
    UNREFERENCED_PARAMETER(nHitTest);
    UNREFERENCED_PARAMETER(message);
	::SetCursor(m_hCursor);
	return TRUE;
}
void CResizeBar::OnParentResize()
{
	if (muFlags & RBF_CONTROLS_HIDDEN)
		return;
	int cx(0), cy(0);
	RECT cl;
	RECT controlRect;
	CWnd *pWnd = GetParent();
	pWnd->GetClientRect(&cl);
	GetWindowRect(&controlRect);
	::MapWindowPoints(NULL, pWnd->m_hWnd, (LPPOINT)&controlRect, 2);
	if (muFlags & RBF_HORIZONTAL) {
		if (controlRect.top - cl.top < m_iMinLeft) {
			cy = -(controlRect.top - cl.top - m_iMinLeft);
		}
		if (cl.bottom - controlRect.bottom < m_iMinRight) {
			if (cy == 0)
				cy = cl.bottom - controlRect.bottom - m_iMinRight;
			else cy = 0;
		}
	}
	else {
		if (controlRect.left - cl.left < m_iMinLeft) {
			cx = -(controlRect.left - cl.left - m_iMinLeft);
		}
		if (cl.right - controlRect.right < m_iMinRight) {
			if (cx == 0)
				cx = cl.right - controlRect.right - m_iMinRight;
			else cx = 0;
		}
	}
	if (cx || cy)
		mDragControlResizer.DoReSize(cx, cy);
}
#define MK_ESCAPE 0x8000000
void CResizeBar::OnMouseMove(UINT nFlags, CPoint point)
{
	CStatic::OnMouseMove(nFlags, point);
	if (nFlags & MK_ESCAPE) {
		CPoint tmp = point;
		MapWindowPoints(GetParent(), &tmp, 1);
		point.x = mOrgMousePointRelParent.x - tmp.x;
		ReleaseCapture();
	}
	else if (GetCapture() != this)
		return;
	RECT cl;
	RECT controlRect;
	CWnd *pWnd = GetParent();
	pWnd->GetClientRect(&cl);
	GetWindowRect(&controlRect);
	::MapWindowPoints(NULL, pWnd->m_hWnd, (LPPOINT)&controlRect, 2);
	int cx(0), cy(0);
	if (muFlags & RBF_HORIZONTAL) {
		cy = point.y-mOrgMousePoint.y;
		if (cy < 0) {
			if (controlRect.top - cl.top < m_iMinLeft)
				cy = 0;
		}
		else if (cl.bottom - controlRect.bottom < m_iMinRight)
			cy = 0;
	}
	else {
		cx = point.x-mOrgMousePoint.x;
		if (cx < 0) {
			if (controlRect.left - cl.left < m_iMinLeft)
				cx = 0;
		}
		else if (cl.right - controlRect.right < m_iMinRight)
			cx = 0;
	}
	if (cx || cy) {
		muFlags |= RBF_CONTROL_DRAGGED;
		mDragControlResizer.DoReSize(cx, cy);
	}
}

LRESULT CResizeBar::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_NCHITTEST:
		return HTCLIENT;
	case WM_NCLBUTTONDOWN:
	case WM_LBUTTONDOWN:
		if (muFlags & RBF_CONTROLS_HIDDEN)
			break;
		muFlags &= ~RBF_CONTROL_DRAGGED;
		SetCapture();
		SetCursor(m_hCursor);
		mOrgMousePoint.SetPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		mOrgMousePointRelParent = mOrgMousePoint;
		MapWindowPoints(GetParent(), &mOrgMousePointRelParent, 1);
		break;
	case WM_NCLBUTTONUP:
	case WM_LBUTTONUP:
		if (muFlags & RBF_CONTROLS_HIDDEN) {
			HideControl(false); // show control
			break;
		}
		ReleaseCapture();
		mOrgMousePoint.SetPoint(0, 0);
		if (!(muFlags & RBF_CONTROL_DRAGGED)) { // toggle
			HideControl(!(muFlags & RBF_CONTROLS_HIDDEN));
		}
		muFlags &= ~RBF_CONTROL_DRAGGED;
		break;
	case WM_CANCELMODE:
	case WM_KEYDOWN:
		if (message == WM_CANCELMODE || wParam == VK_ESCAPE)
			OnMouseMove(MK_ESCAPE, mOrgMousePoint);
		break;
	}
	return CStatic::WindowProc(message,wParam, lParam);
}

BOOL CResizeBar::SubclassDlgItem(UINT nID, CWnd* pParent)
{
	BOOL bRet(CStatic::SubclassDlgItem(nID, pParent));
	RECT rc;
	pParent->GetDlgItem(nID)->GetClientRect(&rc);
	int cx(SystemUtils::GetTranslatedDPIPixelX(5)), cy(rc.bottom-rc.top);
	if (rc.right-rc.left>rc.bottom-rc.top) {
		cy = cx;
		cx = rc.right-rc.left;
		muFlags |= RBF_HORIZONTAL;
		m_hCursor = ::LoadCursor(NULL, IDC_SIZENS);
		SetCursor(m_hCursor);
	}
	SetWindowPos(NULL, 0, 0, cx, cy, SWP_NOMOVE|SWP_NOZORDER);
	return bRet;
}
void CResizeBar::HideControl(bool bHide, bool bLeft)
{
	mDragControlResizer.HideControls(GetDlgCtrlID(), bHide, bLeft,
		(muFlags & RBF_HORIZONTAL) != 0);
	if (bHide) {
		muFlags |= RBF_CONTROLS_HIDDEN;
		if (!bLeft)
			muFlags |= RBF_CONTROLS_HIDDEN_RIGHT;
	}
	else {
		muFlags &= ~(RBF_CONTROLS_HIDDEN|RBF_CONTROLS_HIDDEN_RIGHT);
	}
}
