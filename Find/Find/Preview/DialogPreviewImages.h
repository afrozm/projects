#pragma once

#include "DialogPreviewBase.h"
// CDialogPreviewImages dialog

class CDialogPreviewImages : public CDialogPreviewBase
{
	DECLARE_DYNAMIC(CDialogPreviewImages)

public:
	CDialogPreviewImages(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDialogPreviewImages();
	BOOL ShowPreview(const CString &path);
	void StopPreview();
// Dialog Data
	enum { IDD = IDD_DIALOG_PREVIEW_IMAGES };
	afx_msg void OnPaint( );
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	void *m_pImage;
};
