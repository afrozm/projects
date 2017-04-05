#pragma once
#include "FindReplaceDialogEx.h"
#include "FindOptionDlg.h"
#include "ListCtrlUtil.h"

class CFindDlg;

class CListResItemData
{
public:
	CListResItemData(const CFileMetaDataProvider &fileMetaDataProvider, int matchWeight = 0, LPCTSTR path = nullptr)
		: m_ullFileSize(fileMetaDataProvider.GetFileSize()), mMatchWeight(matchWeight), mPath(path?path:L"")
	{
		try {fileMetaDataProvider.GetCreationTime(mCreatedTime);}catch(...) {}
		try {fileMetaDataProvider.GetLastWriteTime(mModifedTime);}catch(...) {}
		try {fileMetaDataProvider.GetLastAccessTime(mAccessedTime);}catch(...) {}
	}
	bool HasSize() const {return m_ullFileSize >= 0;}
	LONGLONG m_ullFileSize;
	CTime mCreatedTime;
	CTime mModifedTime;
	CTime mAccessedTime;
	int mMatchWeight;
	CString mPath;
    CString mName;
};

// CSaveListResultCtrl

enum ListColumns {
	ListColumns_Size,
	ListColumns_CreatedTime,
	ListColumns_ModifiedTime,
	ListColumns_AccessedTime,
	ListColumns_FileIcon,
	// Add here for more
	ListColumns_OptianlCount
};

class CSaveListResultCtrl : public CListCtrlUtil
{
	DECLARE_DYNAMIC(CSaveListResultCtrl)
	int m_iColSorted;
	CMutex m_Mutext;
	class CFindOption {
	public:
		CFindOption() : m_pFindDialog(NULL), m_bMatchCaseInSen(true), m_bMatchFullPath(false),
			mCurrentStringType(CFindReplaceDialogEx::PlainText)
		{}
		void Delete() {
			GetFindStringType();
			m_pFindDialog = NULL;
		}
		void CreateFindDialog(CSaveListResultCtrl *pList);
		CString& GetFindString();
		bool MatchCaseInSensitive() {return m_bMatchCaseInSen;}
		bool MatchFullPath() {return m_bMatchFullPath;}
		CFindReplaceDialogEx::FindStringType GetFindStringType();
	private:
		CFindReplaceDialogEx *m_pFindDialog;
		CString m_StrFind;
		bool m_bMatchCaseInSen;
		bool m_bMatchFullPath;
		CFindReplaceDialogEx::FindStringType mCurrentStringType;
	} m_FindOption;
	CFindDlg *m_pFindDlg;
	HMENU m_hExplorerContextMenu;
	CMenu *mContextMenu;
	IContextMenu *pExplorerContextMenu;
	bool mbDrawingDisabled;
	int m_iOptionalColumns[ListColumns_OptianlCount];
public:
	CSaveListResultCtrl(CFindDlg *pFindDlg);
	virtual ~CSaveListResultCtrl();
	void OnContextMenu(CWnd *pWnd, CPoint pos);
	void SortItemsEx(int nCol, bool bForce = false);
	void MoveItem(int nSrcItem, int nDstIndex);
	void CopyFilesToClipBoard();
	void RemoveSelectedFiles(bool bDelete = false);
	void DoDragDrop();
	void CopyPath();
	void FilterDuplicates(CDialog *pParentDialog);
	void OnLVNDeleteItem(int nItem);
	void OnLVNDeleteAllItem();
	LONGLONG GetFileSize(int nItem);
	CSyncObject *GetSyncObj() {return &m_Mutext;}
	void Find();
	void FindNext(bool bUp = false, bool bAll = false);
	CFindDlg* GetFindDlg() const {return m_pFindDlg;}
	bool DisableNotification(bool bDisable = true);
	void InitExplorerContextMenu(HMENU hMenu);
	void DisablePaint(bool bDisablePaint = false) {mbDrawingDisabled=bDisablePaint;}
	bool AddOptionalColumn(ListColumns optionalComun);
	bool RemoveOptionalColumn(ListColumns optionalComun);
	bool IsOptionalColumnPresent(ListColumns optionalComun) const;
	int GetOptionalColumnsIndex(ListColumns optionalComun) const;
	BOOL SetOptionalColumnItemText(int item, ListColumns optionalComun, LPCTSTR text);
	BOOL UpdateOptionalDateColumns(int item);
	BOOL UpdateOptionalColumn(int item, ListColumns optionalComun);
	int GetColumnDataIndex(int iCol);
	int InsertItem(int nItem, LPCTSTR lpszItem, bool isFolder = false);
	bool UpdateImageList();
	void SaveDefault(FindDataBase &findDb);
	void LoadDefault(FindDataBase &findDb);
	int GetItemMatchingWeight(int matchWeight);
protected:
	int GetIconIndex(LPCTSTR itemText = NULL);
	HMENU GetExplorerContextMenu(IContextMenu **pExplorerContextMenu);
	HANDLE GetHDrop();
	void OnKeyDown(UINT nChar, UINT nReptCnt, UINT nFlags);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg LRESULT OnFindReplace(WPARAM wParam, LPARAM lParam);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
	static UINT WM_FINDREPLACE;
};
