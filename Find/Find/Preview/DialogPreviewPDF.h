#pragma once

#include "DialogPreviewBase.h"
#include "CAcroAXDocShim.h"
// CDialogPreviewPDF dialog

class CDialogPreviewPDF : public CDialogPreviewBase
{
	DECLARE_DYNAMIC(CDialogPreviewPDF)
	BOOL ShowPreview(const CString &path);
public:
	CDialogPreviewPDF(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDialogPreviewPDF();
	bool Load(LPCTSTR path);
// Dialog Data
	enum { IDD = IDD_DIALOG_PREVIEW_PDF };

protected:
	CAcroAXDocShim mPDFCtrl;
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg void OnSize(UINT nType, int cx, int cy);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnNMClickSyslinkGetreader(NMHDR *pNMHDR, LRESULT *pResult);
};
