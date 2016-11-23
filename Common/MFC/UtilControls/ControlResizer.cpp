#include "StdAfx.h"
#include "ControlResizer.h"

CControlResizer::CControlResizer(CDialog *pDialog)
: m_pDialog(pDialog), m_iHiddenLen(0)
{
}

CControlResizer::~CControlResizer(void)
{
}
void CControlResizer::RemoveControl(int id)
{
	ResizeProp rszProp = {id};
	m_ResizePropArray.Remove(rszProp);
}
void CControlResizer::AddControl(int id, UINT uFlags)
{
	ResizeProp rszProp = {id, uFlags};
	RECT clR;
	RECT controlRect;
	m_pDialog->GetClientRect(&clR);
	m_pDialog->GetDlgItem(id)->GetWindowRect(&controlRect);
	MapWindowPoints(NULL, m_pDialog->m_hWnd, (LPPOINT)&controlRect, 2);
	rszProp.controlRightMargin = clR.right - controlRect.right;
	rszProp.controlBottomMargin = clR.bottom - controlRect.bottom;
	m_ResizePropArray.AddUnique(rszProp);
}
void CControlResizer::DoResize(const ResizeProp &rszProp, const RECT &clR)
{
	RECT controlRect;
	CWnd *pControlWnd = m_pDialog->GetDlgItem(rszProp.controlID);
	pControlWnd->GetWindowRect(&controlRect);
	MapWindowPoints(NULL, m_pDialog->m_hWnd, (LPPOINT)&controlRect, 2);
	int widht = controlRect.right - controlRect.left;
	int height = controlRect.bottom - controlRect.top;
	if (rszProp.uFlags & RSZF_BOTTOM_FIXED) {
		if (rszProp.uFlags & RSZF_SIZEY_FIXED)
			controlRect.top = clR.bottom - rszProp.controlBottomMargin - height;
		controlRect.bottom = clR.bottom - rszProp.controlBottomMargin;
	}
	if (rszProp.uFlags & RSZF_RIGHT_FIXED) {
		if (rszProp.uFlags & RSZF_SIZEX_FIXED)
			controlRect.left = clR.right - rszProp.controlRightMargin - widht;
		controlRect.right = clR.right - rszProp.controlRightMargin;
	}
	widht = controlRect.right - controlRect.left;
	height = controlRect.bottom - controlRect.top;
	pControlWnd->SetWindowPos(NULL, controlRect.left, controlRect.top, widht, height, SWP_NOZORDER);
}
void CControlResizer::DoReSize()
{
	RECT clR;
	m_pDialog->GetClientRect(&clR);
	INT_PTR arrayCount  = m_ResizePropArray.GetCount();
	for (INT_PTR i = 0; i < arrayCount; i++) {
		DoResize(m_ResizePropArray.GetAt(i), clR);
	}
}
const ResizeProp* CControlResizer::GetControl(int id) const
{
	INT_PTR arrayCount  = m_ResizePropArray.GetCount();
	for (INT_PTR i = 0; i < arrayCount; i++) {
		const ResizeProp *rszProp(&m_ResizePropArray.GetAt(i));
		if (rszProp->controlID == id)
			return rszProp;
	}
	return NULL;
}
int CControlResizer::IsLeft(int baseID, int controlIDToCompare, bool bLeft, bool bHorizontally) const
{
	RECT baseRect, controlRect;
	m_pDialog->GetDlgItem(baseID)->GetWindowRect(&baseRect);
	m_pDialog->GetDlgItem(controlIDToCompare)->GetWindowRect(&controlRect);
	int iLeftLen(0);
	if (bHorizontally) {
		if (bLeft) {
			if (controlRect.bottom <= baseRect.top) // top
				iLeftLen = controlRect.top - baseRect.top;
		}
		else if (controlRect.top >= baseRect.bottom) // bottom
			iLeftLen = controlRect.bottom - baseRect.top;
	}
	else {
		if (bLeft) {
			if (controlRect.right <= baseRect.left) // left
				iLeftLen = controlRect.left - baseRect.left;
		}
		else if (controlRect.left >= baseRect.right) // right
			iLeftLen = controlRect.right - baseRect.left;
	}
	return iLeftLen;
}
void CControlResizer::HideControls(int id, bool bHide, bool bLeft, bool bHorizontally)
{
	const ResizeProp *rszBaseControl(GetControl(id));
	if (rszBaseControl == NULL)
		return;
	if (bHide) {
		if (m_iHiddenLen) { // already hidden
			if (m_iHiddenLen < 0 && bLeft) // already hidden in left/top
				return;
			// First show - then hide
			HideControls(id, !bHide, bLeft, bHorizontally);
		}
	}
	if (bHide) {
		m_iHiddenLen = 0;
		// first hide all controls left
		m_ControlsHidden.RemoveAll();
		INT_PTR arrayCount  = m_ResizePropArray.GetCount();
		for (INT_PTR i = 0; i < arrayCount; i++) {
			const ResizeProp &rszProp(m_ResizePropArray.GetAt(i));
			int leftLen = IsLeft(rszBaseControl->controlID, rszProp.controlID, bLeft, bHorizontally);
			if (leftLen) {
				m_pDialog->GetDlgItem(rszProp.controlID)->ShowWindow(SW_HIDE);
				m_ControlsHidden.Add(rszProp.controlID);
				if (leftLen < 0) {
					if (leftLen < m_iHiddenLen)
						m_iHiddenLen = leftLen;
				}
				else {
					if (leftLen > m_iHiddenLen)
						m_iHiddenLen = leftLen;
				}
			}
		}
		int cx(0), cy(0);
		if (bHorizontally)
			cy = m_iHiddenLen;
		else
			cx = m_iHiddenLen;
		DoReSize(cx, cy);
	}
	else { // show the control
		// first resize
		int cx(0), cy(0);
		m_iHiddenLen = -m_iHiddenLen;
		if (bHorizontally)
			cy = m_iHiddenLen;
		else
			cx = m_iHiddenLen;
		m_iHiddenLen = 0;
		DoReSize(cx, cy);
		// then show all hidden control
		INT_PTR arrayCount  = m_ControlsHidden.GetCount();
		for (INT_PTR i = 0; i < arrayCount; i++) {
			m_pDialog->GetDlgItem(m_ControlsHidden[i])->ShowWindow(SW_SHOW);
		}
		m_ControlsHidden.RemoveAll();
	}
}
bool CControlResizer::IsHidden(int iControlID) const
{
	INT_PTR arrayCount  = m_ControlsHidden.GetCount();
	for (INT_PTR i = 0; i < arrayCount; i++) {
		if (m_ControlsHidden[i] == iControlID)
			return true;
	}
	return false;
}
void CControlResizer::DoReSize(int incrX, int incrY)
{
	INT_PTR arrayCount  = m_ResizePropArray.GetCount();
	CRgn updateRgn;
	updateRgn.CreateRectRgn(0, 0, 0, 0);
	bool bIsEmptyUpdateRgn(true);
	for (INT_PTR i = 0; i < arrayCount; i++) {
		const ResizeProp &rszProp(m_ResizePropArray.GetAt(i));
		if (IsHidden(rszProp.controlID)) continue;
		if (rszProp.uFlags & RSZF_NO_RESIZE) continue;
		RECT controlRect;
		CWnd *pControlWnd = m_pDialog->GetDlgItem(rszProp.controlID);
		pControlWnd->GetWindowRect(&controlRect);
		MapWindowPoints(NULL, m_pDialog->m_hWnd, (LPPOINT)&controlRect, 2);
		CRgn srcRgn;
		if (rszProp.uFlags & RSZF_RESIZE_UPDATE) {
			srcRgn.CreateRectRgn(controlRect.left, controlRect.top, controlRect.right, controlRect.bottom);
		}
		int widht = controlRect.right - controlRect.left;
		int height = controlRect.bottom - controlRect.top;
		int cx = incrX;
		int cy = incrY;
		if (rszProp.uFlags & RSZF_RESIZE_OPPOSITE) {
			cx = -cx;
			cy = -cy;
		}
		if (cx) {
			if (rszProp.uFlags & RSZF_SIZEX_FIXED) {
				OffsetRect(&controlRect, cx, 0);
			}
			else if (rszProp.uFlags & RSZF_RIGHT_FIXED) {
				controlRect.left = controlRect.right - widht - cx;
			}
			else if (rszProp.uFlags & RSZF_LEFT_FIXED) {
				controlRect.right = controlRect.left + widht + cx;
			}
		}
		if (cy) {
			if (rszProp.uFlags & RSZF_SIZEY_FIXED) {
				OffsetRect(&controlRect, 0, cy);
			}
			else if (rszProp.uFlags & RSZF_BOTTOM_FIXED) {
				controlRect.top = controlRect.bottom - height - cy;
			}
			else if (rszProp.uFlags & RSZF_TOP_FIXED) {
				controlRect.bottom = controlRect.top + height + cy;
			}
		}
		if (rszProp.uFlags & RSZF_RESIZE_UPDATE) {
			CRgn dst;
			dst.CreateRectRgn(controlRect.left, controlRect.top, controlRect.right, controlRect.bottom);
			updateRgn.CombineRgn(&srcRgn, &dst, RGN_OR);
			bIsEmptyUpdateRgn = false;
		}
		widht = controlRect.right - controlRect.left;
		height = controlRect.bottom - controlRect.top;
		pControlWnd->SetWindowPos(NULL, controlRect.left, controlRect.top, widht, height, SWP_NOZORDER);
	}
	if (!bIsEmptyUpdateRgn) {
		m_pDialog->InvalidateRgn(&updateRgn);
		m_pDialog->UpdateWindow();
	}
}
void CControlResizer::ShowControls(int nCmdShow)
{
	INT_PTR arrayCount(m_ResizePropArray.GetCount());
	for (INT_PTR i = 0; i < arrayCount; i++) {
		const ResizeProp &rszProp(m_ResizePropArray.GetAt(i));
		m_pDialog->GetDlgItem(rszProp.controlID)->ShowWindow(nCmdShow);
	}
}
void CControlResizer::operator=(const CControlResizer& objectSrc)
{
	m_pDialog = objectSrc.m_pDialog;
	m_ResizePropArray.Copy(objectSrc.m_ResizePropArray);
	m_ControlsHidden.RemoveAll();
	m_iHiddenLen = 0;
}
void CControlResizer::Append(const CControlResizer& objectSrc)
{
	INT_PTR arrayCount(objectSrc.m_ResizePropArray.GetCount());
	for (INT_PTR i = 0; i < arrayCount; i++) {
		const ResizeProp &rszProp(objectSrc.m_ResizePropArray.GetAt(i));
		m_ResizePropArray.AddUnique(rszProp);
	}
}