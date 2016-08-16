#pragma once

typedef std::vector<HTREEITEM> HTREEITEMVec;

// CTreeCtrlDomain

class CTreeCtrlDomain : public CTreeCtrl
{
	DECLARE_DYNAMIC(CTreeCtrlDomain)

public:
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg BOOL OnDeviceChange(UINT nEventType, DWORD_PTR dwData);
	CTreeCtrlDomain();
	virtual ~CTreeCtrlDomain();
	CString GetFilePath(HTREEITEM hItem, bool bMakeUNCPath = true);
    // Get root path
    // GetFilePath - will return root path + computed path uptree
    CString SetRootPath(const CString &inNewRootPath);
    const CString& GetRootPath() const;
	BOOL SetCheck(HTREEITEM hItem, BOOL fCheck = TRUE, bool bNotify = false);
	BOOL SetCheckChildMatchingPattern(HTREEITEM hItem, BOOL fCheck, const CString& pattern, bool bNotify = false);
	BOOL SetCheckInRange(HTREEITEM hStartItem, HTREEITEM hEndItem, BOOL fCheck = TRUE, bool bNotify = false);
	HTREEITEM FindText(LPCTSTR text, HTREEITEM hStartItem = TVI_ROOT, bool bRecursive = true);
	HTREEITEM FindDomain(LPCTSTR text);
	HTREEITEM FindPath(const CString &path, HTREEITEM hStartItem = TVI_ROOT);
	HTREEITEM FindAddNetworkPath(const CString &path);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	HTREEITEM Expand(const CString &path, HTREEITEM hPrentItem = TVI_ROOT, UINT nCode = TVE_EXPAND);

	void RefreshDomainTree();
	void DeleteAllTreeItem(void);
	BOOL DeleteItem(HTREEITEM hItem);
	void OnTvnDeleteitem(HTREEITEM hItem);
	void OnTvnItemexpanding(NMHDR *pNMHDR, LRESULT *pResult);
	BOOL Expand(HTREEITEM hItem, UINT nCode = TVE_EXPAND, bool bThreaded = true);
	void GetCheckList(HTREEITEMVec &outCheckList);
    DEFINE_FUNCTION_SET_GET_FLAGBIT(m_uFlags, SearchInZip)
    DEFINE_FUNCTION_SET_GET_FLAGBIT(m_uFlags, FileListing)
	void AddRootDrive(LPCTSTR disk);
	void RemoveRootDrive(LPCTSTR disk);
	HTREEITEM InsertNewItem(HTREEITEM hItem, LPCTSTR name, DWORD_PTR itemData = NULL);
protected:
	void OnToggleITemCheck(UINT nFlags, HTREEITEM hItem, bool bNotify = false);
	BOOL UpdateCheckStatus(HTREEITEM hItem);
	BOOL GetStartAndEnd(HTREEITEM &hStartItem, HTREEITEM &hEndItem);
    DEFINE_FUNCTION_SET_GET_FLAGBIT(m_uFlags, UseThread)
	DECLARE_MESSAGE_MAP()

    enum FlagBit {
        UseThread,
        SearchInZip,
        FileListing,
    };
    UINT m_uFlags;
    CString mRootPath;
};

#define TVN_ITEMCHECK_STATE_CHANGED (TVN_FIRST-73)
typedef struct tagTVITEMCHECKSTATECHANGED {
    NMHDR hdr;
    HTREEITEM item;
	BOOL bChecked;
} NMTVITEMCHECKSTATECHANGED, *LPNMTVITEMCHECKSTATECHANGED;
