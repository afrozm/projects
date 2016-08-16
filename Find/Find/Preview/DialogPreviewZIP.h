#pragma once

#include "DialogPreviewBase.h"
#include "ControlResizer.h"
#include "TreeCtrlDomain.h"

// CDialogPreviewZIP dialog

class CDialogPreviewZIP : public CDialogPreviewBase
{
	DECLARE_DYNAMIC(CDialogPreviewZIP)
    CTreeCtrlDomain mTreeCtrl;
    CControlResizer mResizer;
public:
	CDialogPreviewZIP(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDialogPreviewZIP();
	BOOL ShowPreview(const CString &path);
	void DoResize()
	{
		mResizer.DoReSize();
	}
// Dialog Data
	enum { IDD = IDD_DIALOG_PREVIEW_ZIP };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnTvnItemexpandingTreeDomain(NMHDR *pNMHDR, LRESULT *pResult);

	DECLARE_MESSAGE_MAP()
};
