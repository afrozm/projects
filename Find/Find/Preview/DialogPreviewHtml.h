#pragma once

#include "DialogPreviewBase.h"
#include "DialogHtmlPreviewBase.h"
#include "ControlResizer.h"
// CDialogPreviewHtml dialog

class CDialogPreviewHtml : public CDialogPreviewBase
{
	DECLARE_DYNAMIC(CDialogPreviewHtml)

public:
	CDialogPreviewHtml(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDialogPreviewHtml();
	BOOL ShowPreview(const CString &path);
	BOOL OnInitDialog();

// Dialog Data
	enum { IDD = IDD_DIALOG_PREVIEW_HTMLC };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg void OnSize(UINT nType, int cx, int cy);
	DECLARE_MESSAGE_MAP()

	CDialogHtmlPreviewBase m_HtmlDialog;
	CControlResizer mControlResizer;
};
