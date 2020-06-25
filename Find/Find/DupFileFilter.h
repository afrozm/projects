#pragma once
#include "FindDlg.h"
#include "SaveListResultCtrl.h"

class CDupFileFilter
{
public:
	CDupFileFilter(CSaveListResultCtrl *pListCtrl, CDialog *pFindDlg);
	~CDupFileFilter(void);
	void ApplyFilter();
	CFindDlg* GetDlg() const { return m_pDialog;}
private:
	__int64 CompareFiles(HANDLE pFiles[], unsigned int nFiles);
	__int64 CompareFiles(int nItem1, int nItem2);
	bool FilePartialMatch(int nItem1, int nItem2);
	const CString& GetFileMD5(const CString &filePath);
	CSaveListResultCtrl *mListCtrl;
	CFindDlg *m_pDialog;
	UINT m_uFlags;
	CMapStringToString mFileHash;
};
