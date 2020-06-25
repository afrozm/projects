#pragma once

#include "ControlResizer.h"

// CResizeBar

class CResizeBar : public CStatic
{
	DECLARE_DYNAMIC(CResizeBar)

public:
	CResizeBar(CDialog *pDialog = NULL);
	virtual ~CResizeBar();
	BOOL OnSetCursor(CWnd *pWnd, UINT nHitTest, UINT message);
	void OnMouseMove(UINT nFlags, CPoint point);
	void SetMinLeftRight(int ml, int mr);
	void OnParentResize();
	BOOL SubclassDlgItem(UINT nID, CWnd* pParent);
	CControlResizer& GetControlResizer() {return mDragControlResizer;}
	void HideControl(bool bHide = true, bool bLeft = true);
protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	HCURSOR m_hCursor;
	CPoint mOrgMousePoint;
	CPoint mOrgMousePointRelParent;
	int m_iMinLeft;
	int m_iMinRight;
	unsigned muFlags;
	CControlResizer mDragControlResizer;
	DECLARE_MESSAGE_MAP()
};


