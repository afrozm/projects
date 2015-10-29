#pragma once

#include "DialogPreview.h"
#include "ThreadManager.h"
#include "RefCountObj.h"


class CPreviewController
{
public:
	CPreviewController();
	~CPreviewController(void);
	CDialogPreview* GetPreviewDialog(CDialog *pParentDialog);
	BOOL ShowPreview(const CString &fileToPreview);
	BOOL HidePreview();
	int PreviewControlHandler();
	BOOL IsPreviewVisible()  const;
	enum ControllerAction {
		CA_None,
		CA_ShowPreview,
		CA_HidePreview,
		CA_ReSize,
		CA_Terminate,
		CA_DataChanged,
	};
	BOOL PerformAction(ControllerAction ca);
	BOOL IsPreviewTerminated();
	void SetExtraData(CRefCountObj *pExtraData = NULL);
	CRefCountObj* GetExtraData() const {return m_pExtraData;}
private:
	CString GetFileToPreview();
	ControllerAction GetAction();
	CDialogPreview *m_pDialogPreview;
	CString mFileToPreview;
	DWORD m_dwThreadControlID;
	CMutex mLockFile;
	ControllerAction mAction;
	CRefCountObj *m_pExtraData;
	bool mbUpdateData;
};

class CPreviewExtraDataFileContentSearch : CRefCountObj {
public:
	INT_PTR nLineToScroll;
	CString textToSel;
};