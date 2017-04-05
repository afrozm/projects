// FindDlg.h : header file
//

#pragma once
#include "SaveListResultCtrl.h"
#include "TreeCtrlDomain.h"
#include "ControlResizer.h"
#include "ResizeBar.h"
#include "FindOptionDlg.h"
#include "FindDataBase.h"
#include "CommitResultTimer.h"
#include "PreviewController.h"
#include "CountTimer.h"
#include "ComboBoxDragDrop.h"
#include "ContentSearchManager.h"
#include "TMessageHandler.h"

class CFindDlg;

class FindDlgContentMatchCallBack : public ContentMatchCallBack
{
public:
    typedef STLUtils::TFunction<int, int, CFindDlg> Callback;
    FindDlgContentMatchCallBack(Callback::ClassFunction cf, CFindDlg *pFindDlg);
    void MatchCallBack(MatchCallBackData &mcd);
    void SetMatchPattern(LPCTSTR matchPattern = nullptr);
    int GetMatchingFiles(LPCTSTR inOptFile = nullptr);
    const FileIDVsPath::FileList& GetFilePathFromFileID(const std::string &fileID);
    void Reset();
    void SetCurrentFileWeight(int iW) { m_iFileMatchingWeight = iW; }
    bool HasWordContent() { return mContentSearchManager.HasContent(); }
    bool HasMatchPattern() const;
    int StatusCheck(int iUpdate);
protected:
    LPCTSTR mMatchPattern;
    ContentSearchManager mContentSearchManager;
    CountTimer mLastStatusCheckTimer;
    int m_iLastUpdatedCount;
    Callback mCallback;
    int m_iFileMatchingWeight;
    FileIDVsPath mCachedFileIdVsPath;
};


#define FINDF_SEARCH_FILE_STARTED FLAGBIT(31)

// Thread op
typedef enum {
	THREAD_OP_NONE,
	THREAD_OP_EXPAND_TREE_NODE,
	THREAD_OP_START_SEARCH,
	THREAD_OP_FILL_COL_OPTIONAL,
	THREAD_OP_SAVE_SEARCH_RESULT,
	THREAD_OP_SEARCH_IN_NETWORK,
	THREAD_OP_DODRAG_DROP,
	THREAD_OP_FILTER_DUPLICATES,
	THREAD_OP_LOAD_LAST_RESULT,
    THREAD_OP_FIND_FILE_CONTENT,
    THREAD_OP_FIND_FILES_CONTENT // started explicitly by user from list result
} ThreadOperation;



// CFindDlg dialog
class CFindDlg : public CDialog
{
// Construction
public:
	CFindDlg(CWnd* pParent = NULL);	// standard constructor
	~CFindDlg();
	int SavePrefToFile(FindDataBase &findDb, const CString &preferenceName);
	int LoadPrefFromFile(FindDataBase &findDb, const CString &preferenceName);
// Dialog Data
	enum { IDD = IDD_FIND_DIALOG };
	CTreeCtrlDomain *mTreeCtrlDomain;
	CSaveListResultCtrl *mListResult;
    CComboBoxDragDrop mComboBoxFindText;
	CResizeBar *mResizeBar;
	HTREEITEMVec mSearchList;

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

// Implementation
protected:
	HICON m_hIcon;
	HCURSOR m_hCurSorSizeWE;
	HACCEL m_hAccel;
	CString mFindText;
	CString mFileContentSearchText;
    FindDlgContentMatchCallBack mContentMatchCallBack;
	enum FlagBit {
		SearchStarted,
		SearchCancelled,
		Closed,
		DiplayFindOption,
		DisableThreadedoperation,
		StatusTimerStarted,
		ForceUpdateStatus,
	};
	UINT m_uFlags;
	DEFINE_FUNCTION_SET_FLAGBIT(m_uFlags, SearchStarted)
	DEFINE_FUNCTION_SET_FLAGBIT(m_uFlags, SearchCancelled)
	DEFINE_FUNCTION_SET_FLAGBIT(m_uFlags, Closed)
	DEFINE_FUNCTION_SET_GET_FLAGBIT(m_uFlags, DiplayFindOption)
	DEFINE_FUNCTION_SET_FLAGBIT(m_uFlags, DisableThreadedoperation)
	DEFINE_FUNCTION_SET_GET_FLAGBIT(m_uFlags, StatusTimerStarted)
	unsigned m_uDrapDropOpCount;
	CControlResizer mControlResizer;
	CFindOptionDlg mFindOptionDlg;
	CString mPreferenceName; // Currently loaded preference name
	CCommitResultTimer mCommitResultTimer;
	CPreviewController *m_pPreviewController;
	CountTimer mStatusUpdateTimer;
	CMutex mStatusLock, mListUpdateLock;
	CString mStatusMsg;
	// Generated message map functions
	virtual BOOL OnInitDialog();
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	virtual void OnCancel();
	afx_msg void OnSizing(UINT nSize, LPRECT lpRect);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	void GetFindList();
	HTREEITEM SearchForNetWorkPC(const CString &networkPath);
	bool CheckUncheckSearchNodes(const CString &pattern, HTREEITEM hItem = NULL);
	void Find();
	void FindInCache(CString findText);
    void FindFileContent(int nItems = 0);
	void FillColOptional(ListColumns optionalColumn);
	int SearchInNetwork(HTREEITEM hItem);
	void SaveSearchKeyWords();
	void LoadSearchKeyWords();
	void StartSearchInNetworkFoder(LPNETRESOURCE lpnRes);
	void TogglePreview();
	void ShowPreview();
	void SearchFileContent(CListResItemData * pItemData, StringMatcher *pStringMatcher = nullptr);
	int UpdateResultItem(CListResItemData *pListItemData, int itemIndex = -1);
	int GetIdleThreadCount();
    void AddResultItemToList(CListResItemData *pListItemData);
    int ContentMatchCallBack(const int &);
public:
	afx_msg void OnTvnItemexpandingTreeDomain(NMHDR *pNMHDR, LRESULT *pResult);
	void RefreshDomainTree(void);
	int SearchInFolder(CString pathToSearch);
	afx_msg void OnTvnKeydownTreeDomain(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTvnDeleteitemTreeDomain(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedOk();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	void ProcessMessageDuringDragDrop();
	int StartThreadOperation(ThreadOperation op, LPVOID threadData = NULL, int iThreadClass = 0);
	bool StartLimitedThreadOperation(ThreadOperation op, LPVOID threadData = NULL, int iThreadClass = 0, int iMaxThreadCount = FIND_MAX_THREAD_COUNT);
	int DoThreadOperation(ThreadOperation threadOp, LPVOID pThreadData);
	void DoPostThreadOperation();
	DEFINE_FUNCTION_IS_FLAGBIT_SET(m_uFlags, SearchCancelled)
	DEFINE_FUNCTION_IS_FLAGBIT_SET(m_uFlags, SearchStarted)
	DEFINE_FUNCTION_IS_FLAGBIT_SET(m_uFlags, Closed)
	DEFINE_FUNCTION_IS_FLAGBIT_SET(m_uFlags, DisableThreadedoperation)
	DEFINE_FUNCTION_SET_GET_FLAGBIT(m_uFlags, ForceUpdateStatus)
	void DeleteAllTreeItem(void);
	afx_msg void OnNMDblclkListResult(NMHDR *pNMHDR, LRESULT *pResult);
	bool StartThreadToSearchInNetwork(LPNETRESOURCE lpnRes);
	afx_msg void OnCbnSelchangeComboFind();
	void CFindDlg::SaveSearchResult(bool bFull = true);
	afx_msg void OnHdnItemclickListResult(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnBegindragListResult(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnItemchangedListResult(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnDeleteitemListResult(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnDeleteallitemsListResult(NMHDR *pNMHDR, LRESULT *pResult);
	void SetStatusMessage(LPCTSTR fmt = NULL, ...);
	void SetSearchStartedImpl();
	int FindFolderCallback(CFileFindEx *pFindFile, bool bFileMatched);
	afx_msg void OnFilePreferences();
	bool DisableThreadOp(bool bDisable = true);
	int ItrSearchhistoryTableRowsCallbackFn(sqlite3_stmt *statement, void *pUserData);
	int ItrPreferencesTableRowsCallbackFn(sqlite3_stmt *statement, void *pUserData);
	int ItrPrefSearchLocTableRowsCallbackFn(sqlite3_stmt *statement, void *pUserData);
	int ItrSearchCachedDataTableRowsCallbackFn(sqlite3_stmt *statement, void *pUserData);
	BOOL ShowWindow(int nCmdShow);
	afx_msg BOOL OnDeviceChange(UINT nEventType, DWORD_PTR dwData);

	void FileContentSearchCallback(CListResItemData *pItemData, LPCVOID pSrchData);
};

class CAutoDisableThreadOp {
public:
	CAutoDisableThreadOp(CFindDlg *pDlg)
		: m_pDlg(pDlg)
	{
		m_pDlg->DisableThreadOp();
	}
	~CAutoDisableThreadOp()
	{
		m_pDlg->DisableThreadOp(false);
	}
private:
	CFindDlg *m_pDlg;
};
