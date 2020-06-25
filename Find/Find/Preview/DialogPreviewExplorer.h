#pragma once

#include "DialogPreviewBase.h"
#include "ExplorerPreviewManager.h"

// CDialogPreviewExplorer dialog

class CDialogPreviewExplorer : public CDialogPreviewBase
{
	DECLARE_DYNAMIC(CDialogPreviewExplorer)

public:
	CDialogPreviewExplorer(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDialogPreviewExplorer();
	BOOL ShowPreview(const CString &path);
	bool Load(LPCTSTR path);
	void StopPreview();
	void DoResize();
// Dialog Data
	enum { IDD = IDD_DIALOG_PREVIEW_EXPLORER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg void OnSize(UINT nType, int cx, int cy);
	DECLARE_MESSAGE_MAP()

	CExplorerPreviewManager::CExpPreviewHandler *m_pPreviewExp;
};
