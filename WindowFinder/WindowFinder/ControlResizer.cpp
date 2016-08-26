#include "StdAfx.h"
#include "ControlResizer.h"
#include "BaseDlg.h"
//#include "ScrollDlg.h"

ControlInfo::ControlInfo(int inControlID /* = -1 */)
	: controlID(inControlID), pControl(NULL)
{

}
ControlInfo::ControlInfo(CWnd *pWnd)
	: controlID(-1), pControl(pWnd)
{

}
bool ControlInfo::operator==(const ControlInfo & prop) const
{
	if (pControl == NULL)
		return controlID == prop.controlID;
	return pControl == prop.pControl;
}
CWnd* ControlInfo::GetControl(CBaseDlg *pParent /* = NULL */) const
{
	if (pControl != NULL)
		return pControl;
	return controlID >= 0 ? pParent->GetDlgItem(controlID) : NULL;
}

bool ControlInfo::IsValid() const
{
	return pControl != NULL || controlID >= 0;
}

ResizeProp::ResizeProp(int inControlID /* = -1 */, UINT inuFlags /* = RSZF_RIGHT_FIXED | RSZF_BOTTOM_FIXED */,
	int xMargin /* = -1 */, int yMargin /* = -1 */, const ResizeProp *marginControl /* = NULL */)
	: ControlInfo(inControlID), uFlags(inuFlags), controlRightMargin(xMargin), controlBottomMargin(yMargin)
{

}
ResizeProp::ResizeProp(CWnd *pWnd, UINT inuFlags /* = RSZF_RIGHT_FIXED | RSZF_BOTTOM_FIXED */,
	int xMargin /* = -1 */, int yMargin /* = -1 */, const ResizeProp *marginControl /* = NULL */)
	: ControlInfo(pWnd), uFlags(inuFlags), controlRightMargin(xMargin), controlBottomMargin(yMargin)
{

}

CControlResizer::CControlResizer(CBaseDlg *pDialog)
: m_pDialog(pDialog)
{
}

CControlResizer::~CControlResizer(void)
{
}
void CControlResizer::RemoveControl(const ResizeProp &inRszProp)
{
	m_ResizePropArray.Remove(inRszProp);
}

void CControlResizer::AddControl(int inControlID, UINT inuFlags /* = RSZF_RIGHT_FIXED | RSZF_BOTTOM_FIXED */, int xMargin /* = -1 */, int yMargin /* = -1 */, int refrenceControl /* = -1 */)
{
	ResizeProp controlToAdd(inControlID, inuFlags, xMargin, yMargin);
	if (refrenceControl > 0 && refrenceControl != inControlID)
		controlToAdd.marginControl = ControlInfo(refrenceControl);
	AddControl(controlToAdd);
}
void CControlResizer::AddControl(ResizeProp &inRszProp)
{
	if (inRszProp.controlBottomMargin < 0 || inRszProp.controlRightMargin < 0) {
		RECT clR;
		RECT controlRect;
		if (inRszProp.marginControl.IsValid()) {
			m_pDialog->GetControlRect(inRszProp.marginControl.GetControl(m_pDialog), &clR);
			clR.right = clR.left;
			clR.bottom = clR.top;
		}
		else
			GetClientRect(&clR);
		m_pDialog->GetControlRect(inRszProp.GetControl(m_pDialog), &controlRect);
		inRszProp.controlRightMargin = clR.right - controlRect.right;
		inRszProp.controlBottomMargin = clR.bottom - controlRect.bottom;
	}
	m_ResizePropArray.AddUnique(inRszProp);
}
void CControlResizer::DoResize(const ResizeProp &rszProp, const RECT &clientRect) const
{
	RECT controlRect;
	RECT clR = clientRect;
	if (rszProp.marginControl.IsValid()) {
		m_pDialog->GetControlRect(rszProp.marginControl.GetControl(m_pDialog), &clR);
		clR.right = clR.left;
		clR.bottom = clR.top;
	}
	CWnd *pControlWnd = rszProp.GetControl(m_pDialog);
	m_pDialog->GetControlRect(rszProp.GetControl(m_pDialog), &controlRect);
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
void CControlResizer::DoReSize(LPRECT clientRect /* = NULL */) const
{
	RECT clR;
	GetClientRect(&clR, clientRect);
	INT_PTR arrayCount  = m_ResizePropArray.GetCount();
	for (INT_PTR i = 0; i < arrayCount; i++) {
		DoResize(m_ResizePropArray.GetAt(i), clR);
	}
}
const ResizeProp* CControlResizer::GetControl(const ResizeProp &inRszProp) const
{
	INT_PTR arrayCount  = m_ResizePropArray.Find(inRszProp);
	if (arrayCount >= 0)
		return &m_ResizePropArray.GetAt(arrayCount);

	return NULL;
}
void CControlResizer::operator=(const CControlResizer& objectSrc)
{
	m_pDialog = objectSrc.m_pDialog;
	m_ResizePropArray.Copy(objectSrc.m_ResizePropArray);
}
void CControlResizer::Append(const CControlResizer& objectSrc)
{
	INT_PTR arrayCount(objectSrc.m_ResizePropArray.GetCount());
	for (INT_PTR i = 0; i < arrayCount; i++) {
		const ResizeProp &rszProp(objectSrc.m_ResizePropArray.GetAt(i));
		m_ResizePropArray.AddUnique(rszProp);
	}
}

void CControlResizer::DoAlign( UINT afAligFlag /*= AF_VCETNER | AF_LEFT*/, LPRECT clientRect /*= NULL*/ ) const
{
	CRect cr;

	if (clientRect == NULL)
		GetClientRect(cr);
	else
		cr = *clientRect;
	INT_PTR arrayCount  = m_ResizePropArray.GetCount();
	CRect totalRect;
	for (INT_PTR i = 0; i < arrayCount; i++) {
		CRect controlRect;
		const ResizeProp &rsProp(m_ResizePropArray.GetAt(i));
		m_pDialog->GetControlRect(rsProp.GetControl(m_pDialog), controlRect);
		totalRect.UnionRect(totalRect, controlRect);
	}
	CSize topLeft(totalRect.Width(), totalRect.Height());

	if (afAligFlag & AF_AUTORESIZE) {
		CRect viewRect;
		GetClientRect(viewRect);
		CSize newSize(viewRect.Width() + (afAligFlag & AF_HCENTER) ? (topLeft.cx-cr.Width()) : 0,
			viewRect.Height() + (afAligFlag & AF_VCETNER) ? (topLeft.cy-cr.Height()) : 0);

		m_pDialog->SetWindowPos(NULL, 0, 0, newSize.cx, newSize.cy, SWP_NOZORDER | SWP_NOMOVE);
	}
	if (afAligFlag & AF_VCETNER)
		topLeft.cy = ((cr.Height()-topLeft.cy) >> 1);
	else if (afAligFlag & AF_BOTTOM)
		topLeft.cy = cr.bottom - topLeft.cy;
	else
		topLeft.cy = 0;
	if (afAligFlag & AF_HCENTER)
		topLeft.cx = ((cr.Width()-topLeft.cx) >> 1);
	else if (afAligFlag & AF_RIGHT)
		topLeft.cx = cr.right - topLeft.cx;
	else
		topLeft.cx = 0;

	if (topLeft.cy < 0)
		topLeft.cy = 0;
	if (topLeft.cx < 0)
		topLeft.cx = 0;
	for (INT_PTR i = 0; i < arrayCount; i++) {
		CRect controlRect;
		const ResizeProp &rsProp(m_ResizePropArray.GetAt(i));
		m_pDialog->GetControlRect(rsProp.GetControl(m_pDialog), controlRect);
		rsProp.GetControl(m_pDialog)->SetWindowPos(NULL, controlRect.left + topLeft.cx, controlRect.top + topLeft.cy, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	}
}

void CControlResizer::GetClientRect(LPRECT lpRect, LPRECT inOptClientRect /* = NULL */) const
{
	if (inOptClientRect)
		*lpRect = *inOptClientRect;
	else
		m_pDialog->GetClientRect2(lpRect);
}

bool CControlResizer::AlignView(const ResizeProp &sourceView, const ResizeProp &dstView, UINT afEdgeFlag /* = AF_RIGHT */, UINT afAlignFlag /* = AF_TOP */, LPRECT clientRect /* = NULL */)
{
	CRect sourceRect;
	CWnd *pSourceControl(sourceView.GetControl(m_pDialog));
	bool bSourceIsClientRect(pSourceControl == NULL);
	CRect dstRect;
	int dstBottonMargin(0), dstRightMargin(0);
	m_pDialog->GetControlRect(dstView.GetControl(m_pDialog), dstRect);
	if (!bSourceIsClientRect)
		m_pDialog->GetControlRect(pSourceControl, sourceRect);
	else {
		GetClientRect(sourceRect, clientRect);
		afAlignFlag |= AF_INSIDE;
	}
	if (afAlignFlag & AF_INSIDE) {
		dstBottonMargin = -(dstRect.Height() + dstView.controlBottomMargin);
		dstRightMargin = -(dstRect.Width() + dstView.controlRightMargin);
	}
	CSize leftTop(sourceRect.left, sourceRect.top);
	bool bAligned(false);
	bool bCheckCutoffMargin(!(afEdgeFlag & AF_AUTORESIZE) && !bSourceIsClientRect && (sourceView.controlBottomMargin > 0 || sourceView.controlRightMargin > 0));
	if (afEdgeFlag & (AF_TOP|AF_BOTTOM)) {
		if (afEdgeFlag & AF_TOP)
			leftTop.cy -= dstRect.Height() + (dstBottonMargin ? dstBottonMargin : dstView.controlBottomMargin);
		else
			leftTop.cy = sourceRect.bottom + (dstBottonMargin ? dstBottonMargin : dstView.controlBottomMargin);
		if (afAlignFlag & AF_RIGHT)
			leftTop.cx = sourceRect.right - dstRect.Width() - dstView.controlRightMargin;
		else if (afAlignFlag & AF_HCENTER)
			leftTop.cx += ((sourceRect.Width()-dstRect.Width()) >> 1);
		else if (afAlignFlag & AF_LEFT) // left align
			leftTop.cx += dstView.controlRightMargin;
		else
			leftTop.cx = dstRect.left + dstView.controlRightMargin;
		bAligned = true;
	}
	if (afEdgeFlag & (AF_LEFT|AF_RIGHT)) {
		if (afEdgeFlag & AF_LEFT)
			leftTop.cx -= dstRect.Width() + (dstRightMargin ? dstRightMargin : dstView.controlRightMargin);
		else
			leftTop.cx = sourceRect.right + (dstRightMargin ? dstRightMargin : dstView.controlRightMargin);
		if (afAlignFlag & AF_BOTTOM)
			leftTop.cy = sourceRect.bottom - dstRect.Height() - dstView.controlBottomMargin;
		else if (afAlignFlag & AF_VCETNER)
			leftTop.cy += ((sourceRect.Height()-dstRect.Height()) >> 1);
		else if (afAlignFlag & AF_TOP) // top align
			leftTop.cy += dstView.controlBottomMargin;
		else
			leftTop.cy = dstRect.top + dstView.controlBottomMargin;
		bAligned = true;
	}
	if (!bAligned && bSourceIsClientRect) {
		if (afEdgeFlag & AF_VCETNER)
			leftTop.cy += ((sourceRect.Height()-dstRect.Height()) >> 1);
		if (afEdgeFlag & AF_HCENTER)
			leftTop.cx += ((sourceRect.Width()-dstRect.Width()) >> 1);
	}
	if (bCheckCutoffMargin) {
		bCheckCutoffMargin = false;
		if (afEdgeFlag & AF_TOP)
			bCheckCutoffMargin = leftTop.cy < sourceView.controlBottomMargin;
		else if (afEdgeFlag & AF_BOTTOM)
			bCheckCutoffMargin = (leftTop.cy + dstRect.Height()) > sourceView.controlBottomMargin;
		if (afEdgeFlag & AF_LEFT)
			bCheckCutoffMargin = leftTop.cx < sourceView.controlRightMargin;
		else if (afEdgeFlag & AF_RIGHT)
			bCheckCutoffMargin = (leftTop.cx + dstRect.Width()) > sourceView.controlRightMargin;
	}
	bCheckCutoffMargin = !bCheckCutoffMargin;
	bool bReposed(false);
	if (bCheckCutoffMargin) {
		if (leftTop.cx != dstRect.left || leftTop.cy != dstRect.top) {
			dstView.GetControl(m_pDialog)->SetWindowPos(NULL, leftTop.cx, leftTop.cy, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
			dstRect.right = leftTop.cx + dstRect.Width();
			dstRect.bottom = leftTop.cy + dstRect.Height();
			bReposed = true;
		}
	}

	if (bReposed && (afEdgeFlag & AF_AUTORESIZE)) {
		CRect cr;
		GetClientRect(cr, clientRect);
		CRect viewRect;
		GetClientRect(viewRect);
		CSize newSize(viewRect.Width(), viewRect.Height());
		CSize diffSize;
		if (afEdgeFlag & AF_BOTTOM)
			diffSize.cy = dstRect.bottom - cr.bottom;
		if (afEdgeFlag & AF_RIGHT)
			diffSize.cx = dstRect.right - cr.right;
		if (diffSize.cx  || diffSize.cy ) {
			newSize += diffSize;
			if (newSize.cx < sourceView.controlRightMargin)
				newSize.cx = sourceView.controlRightMargin;
			if (newSize.cy < sourceView.controlBottomMargin)
				newSize.cy = sourceView.controlBottomMargin;
			if (newSize.cx != viewRect.Width() || newSize.cy != viewRect.Height()) {
				//if (m_pDialog->IsKindOf(RUNTIME_CLASS(CScrollDlg)))
				//	((CScrollDlg*)m_pDialog)->SetDisplaySize(newSize.cx, newSize.cy);
				//else
					m_pDialog->SetWindowSize(newSize.cx, newSize.cy);
			}
		}
	}
	return bCheckCutoffMargin;
}
