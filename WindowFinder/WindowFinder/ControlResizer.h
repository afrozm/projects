#pragma once

#include "STLUtils.h"

// ReSiZe Flags RSZF_
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

// Align Flafs
#define AF_LEFT 1
#define AF_TOP 2
#define AF_VCETNER 4
#define AF_HCENTER 8
#define AF_RIGHT 16
#define AF_BOTTOM 32
#define AF_AUTORESIZE 64
#define AF_INSIDE 128

class CBaseDlg;

struct ControlInfo {
	ControlInfo(int inControlID = -1);
	ControlInfo(CWnd *pWnd);
	int controlID;
	CWnd *pControl;
	CWnd* GetControl(CBaseDlg *pParent = NULL) const;
	bool operator == (const ControlInfo & prop) const;
	bool IsValid() const;
};

struct ResizeProp : public ControlInfo {
	ResizeProp(int inControlID = -1, UINT inuFlags = RSZF_RIGHT_FIXED | RSZF_BOTTOM_FIXED, int xMargin = -1, int yMargin = -1, const ResizeProp *marginControl = NULL);
	ResizeProp(CWnd *pWnd, UINT inuFlags = RSZF_RIGHT_FIXED | RSZF_BOTTOM_FIXED, int xMargin = -1, int yMargin = -1, const ResizeProp *marginControl = NULL);
	UINT uFlags;
	int controlRightMargin;
	int controlBottomMargin;
	ControlInfo marginControl;
};



class CControlResizer
{
public:
	CControlResizer(CBaseDlg *pDialog = NULL);
	~CControlResizer(void);
	void AddControl(int inControlID, UINT inuFlags = RSZF_RIGHT_FIXED | RSZF_BOTTOM_FIXED, int xMargin = -1, int yMargin = -1, int refrenceControl = -1);
	void AddControl(ResizeProp &inRszProp);
	void RemoveControl(const ResizeProp &inRszProp);
	void DoReSize(LPRECT clientRect = NULL) const;
	void operator=(const CControlResizer& objectSrc);
	void Append(const CControlResizer& objectSrc);
	void DoAlign(UINT afAligFlag = AF_VCETNER | AF_LEFT, LPRECT clientRect = NULL) const;
	bool AlignView(const ResizeProp &sourceView, const ResizeProp &dstView, UINT afEdgeFlag = AF_RIGHT, UINT afAlignFlag = AF_TOP, LPRECT clientRect = NULL);
	const ResizeProp* GetControl(const ResizeProp &inRszProp) const;
protected:
	void GetClientRect(LPRECT lpRect, LPRECT inOptClientRect = NULL) const;
	void DoResize(const ResizeProp &rszProp, const RECT &dialogClientRect) const;
	const ResizeProp* GetMarginControl(const ResizeProp *pMarginControl) const;
	CBaseDlg *m_pDialog;
	CArrayEx<ResizeProp> m_ResizePropArray;
};
