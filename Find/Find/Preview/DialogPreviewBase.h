#pragma once

#define TM_PREVIEW_THREAD_CLASS 1001

// CDialogPreviewBase dialog

#define WM_PBM_DATA_CHACNGED (WM_USER+0x601)

class CPreviewController;

class CDialogPreviewBase : public CDialog
{
	DECLARE_DYNAMIC(CDialogPreviewBase)

public:
	CDialogPreviewBase(UINT nIDTemplate, CWnd* pParent = NULL);   // standard constructor
	virtual void Init();
	virtual ~CDialogPreviewBase();
	virtual BOOL ShowPreview(const CString &path) = 0;
	virtual void StopPreview() {}
	virtual HBRUSH OnControlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	const CString& GetFileToPreview() const;
	virtual bool Load(LPCTSTR /*path*/) { return false; }
	virtual void DoResize() {}
	CPreviewController* GetPreviewContoller() const;
// Dialog Data
	enum { IDD = IDD_DIALOGPREVIEWBASE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
