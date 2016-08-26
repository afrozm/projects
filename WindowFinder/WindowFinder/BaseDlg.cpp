// ImageDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BaseDlg.h"

// CBaseDlg dialog

IMPLEMENT_DYNAMIC(CBaseDlg, CMFCBaseDlg)

CBaseDlg::CBaseDlg(UINT nIDTemplate, CWnd* pParent /*=NULL*/)
	: CMFCBaseDlg(nIDTemplate, pParent), mControlResizer(this)
{

}

CBaseDlg::~CBaseDlg()
{
}

BOOL CBaseDlg::CreateView(CWnd* pParent /* = NULL */)
{
	CWnd *pParentWnd(pParent ? pParent : m_pParentWnd);
	if (m_pParentWnd != pParentWnd)
		m_pParentWnd = pParentWnd;
	return Create(m_lpszTemplateName, m_pParentWnd);
}

void CBaseDlg::GetClientRect2(LPRECT outClientRect) const
{
	GetClientRect(outClientRect);
}

void CBaseDlg::GetClientRect2( HWND hWnd, LPRECT outClientRect )
{
	if (!::SendMessage(hWnd, WM_GET_CLIENT_RECT2, (WPARAM)outClientRect, 0))
		::GetClientRect(hWnd, outClientRect);
}

BEGIN_MESSAGE_MAP(CBaseDlg, CMFCBaseDlg)
	ON_WM_SIZE()
	ON_MESSAGE(WM_GET_CLIENT_RECT2, OnGetClientRect2)
END_MESSAGE_MAP()


// CBaseDlg message handlers
afx_msg void CBaseDlg::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);
	mControlResizer.DoReSize();
}

void CBaseDlg::AdjustWidthHeightWithParent(BOOL bAdjustWidht /* = TRUE */, BOOL bAdjustHeight /* = TRUE */)
{
	CRect wr;
	GetWindowRect(wr);
	CRect pcr;
	CWnd *pParent(m_pParentWnd ? m_pParentWnd : GetParent());
	ASSERT(pParent->GetSafeHwnd() != NULL);
	GetClientRect2(pParent->GetSafeHwnd(), pcr);
	pParent->ScreenToClient(wr);
	if (bAdjustWidht) {
		wr.left = pcr.left;
		wr.right = pcr.right;
	}
	if (bAdjustHeight) {
		wr.top = pcr.top;
		wr.bottom = pcr.bottom;
	}
	if (bAdjustWidht || bAdjustHeight)
		SetWindowPos(NULL, wr.left, wr.top, wr.Width(), wr.Height(), SWP_NOZORDER);
}

void CBaseDlg::GetControlRect( int controlID, LPRECT outControlRect )
{
	GetControlRect(GetDlgItem(controlID), outControlRect);
}

void CBaseDlg::GetControlRect( CWnd *pChildControl, LPRECT outControlRect )
{
	ASSERT(pChildControl->GetSafeHwnd() != NULL);
	pChildControl->GetWindowRect(outControlRect);
	ScreenToClient(outControlRect);
}

bool CBaseDlg::SetWindowSize( int width, int height )
{
	CRect wr;
	GetWindowRect(wr);
	if (wr.Height() != height || wr.Width() != width) {
		SetWindowPos(NULL, 0, 0, width, height, SWP_NOZORDER | SWP_NOMOVE);
		return true;
	}
	return false;
}

bool CBaseDlg::SetWindowPosition( int x, int y )
{
	CRect cr;
	GetWindowRect(cr);
	CWnd *pParent(m_pParentWnd ? m_pParentWnd : GetParent());
	if (pParent)
		pParent->ScreenToClient(cr);
	if (x != cr.left || y != cr.top) {
		SetWindowPos(NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
		return true;
	}
	return false;
}

bool CBaseDlg::SetWindowRect( LPRECT newRect )
{
	if (newRect == NULL)
		return false;
	CRect cr, newWr(newRect);
	GetWindowRect(cr);
	CWnd *pParent(m_pParentWnd ? m_pParentWnd : GetParent());
	if (pParent)
		pParent->ScreenToClient(cr);
	if (cr != newWr) {
		SetWindowPos(NULL, newWr.left, newWr.top, newWr.Width(), newWr.Height(), SWP_NOZORDER);
		return true;
	}
	return false;
}

LRESULT CBaseDlg::OnGetClientRect2( WPARAM wParam, LPARAM lParam )
{
	LPRECT outRect((LPRECT)wParam);
	GetClientRect2(outRect);
	return TRUE;
}
