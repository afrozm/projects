#pragma once

#include "DialogPreviewBase.h"
#include "TextReader.h"
#include "ControlResizer.h"
#include "EditFindCtrl.h"

// CDialogPreviewText dialog

class CDialogPreviewText : public CDialogPreviewBase
{
	DECLARE_DYNAMIC(CDialogPreviewText)
	CTextReader mTextReader;
	bool mFileLoaded;
	CMutex mLock;
	CControlResizer mResizer;
	CString mText;
	CEditFindCtrl mEditCtrl;
public:
	CDialogPreviewText(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDialogPreviewText();
	BOOL ShowPreview(const CString &path);
	int LoadFile();
	void StopPreview();
	void DoResize()
	{
		mResizer.DoReSize();
	}
// Dialog Data
	enum { IDD = IDD_DIALOG_PREVIEW_TEXT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnDataChanged(WPARAM, LPARAM);
	DECLARE_MESSAGE_MAP()
	bool UpdateText();
};
