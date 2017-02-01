#pragma once

#include "TreeCtrlDomain.h"
#include "ControlResizer.h"
#include "ResizeBar.h"
#include "EmbedListCtrl.h"
#include "CmdEditCtrl.h"
#include "FindDataBase.h"
#include "ThreadManager.h"
#include "NetWorkFinder.h"
#include "SearchHistory.h"
#include "CountTimer.h"
#include "CombinedButton.h"
#include "ServerStatusDlg.h"
#include "ContentSearchManager.h"

// CFindServerDlg dialog

typedef enum {
	SST_NewSearch,
	SST_Verify,
	SST_QuickSearch,
	SST_Total
} ServerSearchStatus;

// Search flags
#define SF_SEARCH_STARTED FLAGBIT(0)
#define SF_SEARCH_CANCELLED FLAGBIT(1)
#define SF_DIALOG_CLOSED FLAGBIT(2)
#define SF_DIALOG_SHOW_MINIMIZED FLAGBIT(3)
#define SF_ENUM_IPs FLAGBIT(4)

class CFindServerDlg;

class CServerCombinedButtonContext : public CCombinedButtonContext {
public:
	CServerCombinedButtonContext(CFindServerDlg *pDlg);
	BOOL IsWindowVisible();
	void ShowWindow(int cmdShow = SW_SHOW);
private:
	CFindServerDlg *mDlg;
};

class CFindServerDlg : public CDialog
{
	DECLARE_DYNAMIC(CFindServerDlg)

public:
	CFindServerDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CFindServerDlg();
	bool IsSearchCancelled(bool bCheckThread = false);
	bool IsSearching() {return IsFlagSet(SF_SEARCH_STARTED);}
	void SetFlag(UINT uFlags, bool bSet = true);
	bool IsFlagSet(UINT uFlags) {return (m_uSearchFlags & uFlags) == uFlags;}
	bool StartLimitedThreadOperation(ServerThreadOperation op, LPVOID threadData = NULL);
	void StartSearchInNetworkFoder(LPNETRESOURCE lpnRes);
	const CString* FindInSearchHistory(const CString &inSearchHistory, int &retVal);
	int NetWorkFindShared(LPNETRESOURCE lpNetRes);
	const CTime& GetLastStartTime() const {return m_LastStartTime;}
	int IPEnumerator_Callback(void *ipData);
	void ShowServerMainControls(int cmdShow = SW_SHOW);

	void AddDBQueryString(const CString &queryString);
	void AddDBQueryString(LPCTSTR inStr, ...);
	void AddDBPropertyKeyValue(const CString& key, const CString &value);
	void AddDBQueryStrings(const CArrayCString &queryStrings);
	int GetMaxThreadCount() const {return m_iMaxThreadCount;}

	typedef int (CFindServerDlg::*SearchFolderCallback)(CFileFindEx *pFindFile, bool, void *pUserData);
	struct SearchInFolderData {
		SearchInFolderData(bool bInRecursive = true, SearchFolderCallback inCallbackFn = NULL, void *pInUserData = NULL)
			: bRecursive(bInRecursive), callbackFn(inCallbackFn), pUserData(pInUserData) {}
		bool bRecursive;
		SearchFolderCallback callbackFn;
		void *pUserData;
	};

// Dialog Data
	enum { IDD = IDD_FIND_DIALOG_SERVER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
	afx_msg void OnSizing(UINT nSize, LPRECT lpRect);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	afx_msg void OnDestroy();
	virtual void OnCancel();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	void LoadDefault();
	void LoadFromDB();
	void CommitToDB();
	void Execute(LPCTSTR commandsToExecute = NULL);
	void SetSearchStarted(bool bSearchStarted = true);
	void Find(bool bForce = false);
	int StartThreadOperation(ServerThreadOperation op, LPVOID threadData = NULL);
	void DisableControls(bool bDisable = true);
	void InitCatagotyList(bool bFill = true);
	int SearchInNetwork(HTREEITEM hItem);
	int SearchInNetwork(const CString &networkPath);
	bool RemoveObsoleteSearchHistory(const CSearchHistory &searchHistory);
	void Verify(CSearchHistory *rootPath, bool bRemove = false);
	void SetTitle(LPCTSTR newTtile);
	void StartFind(bool bForce = false);
	void EnumerateIps(DWORD_PTR ipRange);
	bool PushIpHostname(const CString& hostName);
	CString PopHostName();
	bool StartEnumerateIps(DWORD startIp, DWORD endIp);
	void UpdateIpEnumStatusInDB(LPCTSTR propertyName);
	void StartIpEnum();
	void AddThreadStatusPanel(bool bAdd = true);
	bool IsMirrorServer(const CString &inServerName);
	const CSearchHistory* PreSearchCheck(const CString &inSearchHistory, int &retVal, UINT uFlags = SDF_CHECK_INPROGRESS);
	// Memeber variables
	CTreeCtrlDomain *mTreeCtrlDomain;
	CEmbedListCtrl *mCatagoryEmbedListCtrl;
	CResizeBar *mResizeBar;
	CCmdEditCtrl *mCmdEditCtrl;
	CControlResizer mControlResizer;
	CControlResizer mServerMainControls;
	HICON m_hIcon;
	UINT m_uSearchFlags;
	ServerSearchStatus mServerSearchStatus;
	struct FindCatagory {
		FindCatagory()
			: catagoryNum(0), sizeCond(0), uFlags(0)
		{
		}
		~FindCatagory();
		int catagoryNum;
		CRegExpMatcher mStringMatcher;
		LONGLONG mSizeMin;
		LONGLONG mSizeMax;
		int sizeCond;
        enum {
            FindContent
        };
        DEFINE_FUNCTION_SET_GET_FLAGBIT(uFlags, FindContent);
        unsigned uFlags;
		void Init(int num, CEmbedListCtrl *pList);
		bool Match(const CFileFindEx *pFileFile);
	};
	typedef CArray<FindCatagory*> CArrayFindCatagory;
	CArrayFindCatagory mArrayFindCatagory;
	CSearchHistoryArray mSearchHistory;
	CArrayCString mIPEnumHostNames;
	CMutex mLockerIPEnumHostNames;
	CountTimer mIPEnumTime;
	FindDataBase mDataBase;
	CTime m_LastStartTime;
	CCombinedButton mCombinedBotton;
	CServerCombinedButtonContext mFindServerDlgContext;
	CServerStatusDlg *m_pStatusDlg;
	struct MirrorInfo {
		CRegExpMatcher mStringMatcher;
		CString mMirrorMachine;
	};
	typedef CArray<MirrorInfo> CArrayMirrorInfo;
	CArrayMirrorInfo mArrMirrorInfo;
	CArrayCString mHostIPsToEnumerate; // ip:[hostname] list of hostname
	int m_iMaxThreadCount;
    ContentSearchManager mContentSearchManager;
    CDBCommiter *m_pDBCommitter;

	int SearchInFolder(const CString &pathToSearch, SearchInFolderData *pExtraData = NULL);
	int QuickSearch(const Path& remoteHostPath);
	int FindFolderCallback(CFileFindEx *pFindFile, bool, void *pUserData);
	int FindFolderCallbackForQuickSearch(CFileFindEx *pFindFile, bool, void *pUserData);
public:
	afx_msg void OnTvnItemexpandingTreeDomain(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonAdd();
	afx_msg void OnBnClickedButtonExecute();
	int ItrCatagoryTableRowsCallbackFn(sqlite3_stmt *statement, void *pUserData);
	int ItrVerifyCacheDataTableRowsCallbackFn(sqlite3_stmt *statement, void *pUserData);
	int ItrGetFolderListCacheDataTableRowsCallbackFn(sqlite3_stmt *statement, void *pUserData);
	int TreeNetWorkIteratorCallBack(void *pData, void *pUserParam);
	int DoThreadOperation(LPVOID pThreadData);
	afx_msg void OnBnClickedButtonLoaddefault();
	afx_msg void OnBnClickedOk();
	afx_msg void OnTvnItemCheckStateChangedTreeDomain(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonDelete();
	afx_msg BOOL OnDeviceChange(UINT nEventType, DWORD_PTR dwData);
	afx_msg void OnBnClickedButtonLeft();
	afx_msg void OnBnClickedButtonRight();
	afx_msg LRESULT OnAddThreadStatusPanel(WPARAM wParam, LPARAM lParam);
};
