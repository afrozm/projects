#pragma once

#define DWF_TRANSPARENT_INSIDE 8 // Drawer is transparent only in inside parent window - if off then outside
#define DWF_TRANSPARENT 16 // Drawer is transparent

// CDrawer dialog

class CDrawer : public CDialog
{
	DECLARE_DYNAMIC(CDrawer)

public:
	CDrawer();   // standard constructor
	virtual ~CDrawer();
	BOOL Open(); // Open drawer window
	BOOL Close(); // Close drawer window
	BOOL IsOpen() const;
	enum DrawerPosition {
		DP_Left,
		DP_Top,
		DP_Right,
		DP_Bottom
	};
	BOOL Init(UINT nIDTemplate, CWnd* pParent, DrawerPosition dp = DP_Bottom);
	BOOL SetPosition(DrawerPosition dp);
	DrawerPosition GetPosition() const
	{
		return m_DrawerPosition;
	}
	BOOL IsMoving() const;
	LRESULT DrawerParentWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void SetTransparent(unsigned char transParentValue = 255, UINT uFlags = 0);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	BOOL StartMove();
	void OnTimerMove();
	virtual void OnOK() {}
	virtual void OnCancel() {}
	void DoTransparent(unsigned char transParentValue = 255);

	DECLARE_MESSAGE_MAP()
	DrawerPosition m_DrawerPosition;
	UINT m_uFlags;
	RECT m_DrawerRect;
	WNDPROC m_lpParentPrevWndFunc;
	unsigned char mTransparentValue;
};
