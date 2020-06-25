#pragma once

#include "DialogPreviewBase.h"
#include "ControlResizer.h"
#include "StringMatcher.h"
// CDialogPreview dialog


class CDialogPreview;
class CPreviewContainer {
	CDialogPreviewBase *m_pPreviewDialog;
	CRegExpMatcher mStringMatcher;
public:
	CPreviewContainer();
	CPreviewContainer(const CString &matchPattern);
	~CPreviewContainer();
	bool CanShowPreview(const CString &fileName);
	CDialogPreviewBase* GetPreviewDialogObject(CDialogPreview *pDialogPreview, int iPreviewID);
};
typedef CArray<CPreviewContainer*> CArrayCPreviewContainer;

class CDialogPreview : public CDialog
{
	DECLARE_DYNAMIC(CDialogPreview)

public:
	CDialogPreview(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDialogPreview();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	BOOL ShowWindow(int nCmdShow);
// Dialog Data
	enum { IDD = IDD_DIALOG_PREVIEW };

	bool ShowPreview(const CString &path);
	const CString& GetFileToPreview() const {return m_FileToPreview;}
	CDialogPreviewBase* GetDialogPreviewsEx(int iPreviewID);
	CDialogPreviewBase* GetDialogPreviews(int iPreviewID);
	CDialogPreviewBase* GetCurrentPreview() {return m_pCurrentPreview;}
	CPreviewController* GetPreviewController() const {return m_pController;}
	void SetPreviewController(CPreviewController *pController) {m_pController=pController;}
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual void OnCancel();
	void HideCurrentPreview(CDialogPreviewBase *pNewPreview);
	void ShowCurrentPreview(CDialogPreviewBase *pNewPreview = NULL);
	void ShowDialogControls(int iCmdShow = SW_SHOW);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	DECLARE_MESSAGE_MAP()

private:
	CArrayCPreviewContainer m_CArrayCPreviewContainer;
	CDialogPreviewBase *m_pCurrentPreview;
	CControlResizer mControlResizer;
	int m_iChildControlID;
	CString m_FileToPreview;
	CPreviewController *m_pController;
};
