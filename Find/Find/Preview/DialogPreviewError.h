#pragma once

#include "DialogPreviewBase.h"
// CDialogPreviewError dialog

class CDialogPreviewError : public CDialogPreviewBase
{
	DECLARE_DYNAMIC(CDialogPreviewError)

public:
	CDialogPreviewError(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDialogPreviewError();
	BOOL ShowPreview(const CString &path);
// Dialog Data
	enum { IDD = IDD_DIALOG_PREVIEW_ERROR };
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
