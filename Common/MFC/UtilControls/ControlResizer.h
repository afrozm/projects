#pragma once

#define RSZF_RIGHT_FIXED 1
#define RSZF_BOTTOM_FIXED 2
#define RSZF_SIZEX_FIXED 4
#define RSZF_SIZEY_FIXED 8
#define RSZF_SIZE_FIXED (RSZF_SIZEY_FIXED|RSZF_SIZEX_FIXED)
#define RSZF_LEFT_FIXED 16
#define RSZF_TOP_FIXED 32
#define RSZF_RESIZE_OPPOSITE 64
#define RSZF_RESIZE_UPDATE 128 // Re pait the client area
#define RSZF_NO_RESIZE 256

struct ResizeProp {
	int controlID;
	UINT uFlags;
	int controlRightMargin;
	int controlBottomMargin;
	bool operator == (const ResizeProp & prop) const {return controlID == prop.controlID;}
};

class CControlResizer
{
public:
	CControlResizer(CDialog *pDialog = NULL);
	~CControlResizer(void);
	void AddControl(int id, UINT uFlags = RSZF_RIGHT_FIXED | RSZF_BOTTOM_FIXED);
	void RemoveControl(int id);
	void DoReSize();
	void DoReSize(int cx, int cy);
	void HideControls(int id, bool bHide = true, bool bLeft = true, bool bHorizontally = false);
	void ShowControls(int nCmdShow = SW_SHOW);
	void operator=(const CControlResizer& objectSrc);
	void Append(const CControlResizer& objectSrc);
private:
	void DoResize(const ResizeProp &rszProp, const RECT &dialogClientRect);
	const ResizeProp* GetControl(int id) const;
	int IsLeft(int baseID, int controlIDToCompare, bool bLeft = true, bool bHorizontally = false) const;
	bool IsHidden(int iControlID) const;
	CDialog *m_pDialog;
	CArrayEx<ResizeProp> m_ResizePropArray;
	CArray<int> m_ControlsHidden;
	int m_iHiddenLen;
};
