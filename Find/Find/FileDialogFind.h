#pragma once


// CFileDialogFind

class CFileDialogFind : public CFileDialog
{
	DECLARE_DYNAMIC(CFileDialogFind)

public:
	CFileDialogFind(BOOL bOpenFileDialog, // TRUE for FileOpen, FALSE for FileSaveAs
		LPCTSTR lpszDefExt = NULL,
		LPCTSTR lpszFileName = NULL,
		DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		LPCTSTR lpszFilter = NULL,
		CWnd* pParentWnd = NULL);
	virtual ~CFileDialogFind();
	UINT IsAppendChecked(void) {return m_uAppendChecked;}
protected:
	void OnInitDone();
	BOOL OnFileNameOK();
	DECLARE_MESSAGE_MAP()
private:
	UINT m_uAppendChecked;
};


