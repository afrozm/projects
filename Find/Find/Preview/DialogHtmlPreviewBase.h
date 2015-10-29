#pragma once

#ifdef _WIN32_WCE
#error "CDHtmlDialog is not supported for Windows CE."
#endif 

// CDialogHtmlPreviewBase dialog

class CDialogHtmlPreviewBase : public CDHtmlDialog
{
	DECLARE_DYNCREATE(CDialogHtmlPreviewBase)

public:
	CDialogHtmlPreviewBase(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDialogHtmlPreviewBase();
	BOOL ShowPreview(const CString &path, bool pathIsURL = false);
// Overrides

// Dialog Data
	enum { IDD = IDD_DIALOG_PREVIEW_HTML, IDH = 0 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	DECLARE_DHTML_EVENT_MAP()
};
