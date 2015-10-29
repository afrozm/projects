#pragma once

#include "FindDataBase.h"

#define FSH_SKIP_SEARCH 0
#define FSH_CONTINUE_SEARCH 1

#define SHF_ALREADY_SEARCHED FLAGBIT(0)
#define SHF_IS_AVAILABLE FLAGBIT(1) // Only runtime flag
#define SHF_IN_PROGRESS FLAGBIT(2) // Only runtime flag

CArrayCString& GetQueryRemoveFromSearchHistory(const CString &searchHistory, CArrayCString &queryArray);
CArrayCString& GetQueryAddToSearchHistory(const CString &searchHistory, CArrayCString &queryArray, int flags = 0, int misscount = 0);

class CSearchHistory
{
public:
	CSearchHistory(void);
	CSearchHistory(const CString &searchKey);
	~CSearchHistory(void);
	int Compare(const CSearchHistory &searchHistory) const;
	int Load(sqlite3_stmt *statement, const CTime &pLastSearchStartTime);
	const CString& GetSearchKey() const
	{	return mSearchKey;	}
	const CString& GetRootKey() const
	{	return mRoot;	}
	bool IsAlreadySearched(bool bCheckInProgress = false) const 
	{return (miFlags & SHF_ALREADY_SEARCHED) != 0 || bCheckInProgress && IS_FLAG_SET(miFlags, SHF_IN_PROGRESS);}
	bool IsAvailable() const 
	{return (miFlags & SHF_IS_AVAILABLE) != 0;}
	UINT GetMissCount() const {return miMissCount;}
	void ResetMissCount(UINT iMissCount = 0) {miMissCount=iMissCount;}
	bool SetAvailable(bool bAvailable = true);
	const CTime& GetLastupdated() const  {return mLastUpdated;}
	void SetAlreadySearched(bool bAlreadySearched = true) {SetUnsetFlag(SHF_ALREADY_SEARCHED, bAlreadySearched); SetUnsetFlag(SHF_IN_PROGRESS, false);}
	bool operator > (const CSearchHistory & inSH) const {return Compare(inSH) > 0;}
	bool operator < (const CSearchHistory & inSH) const {return Compare(inSH) < 0;}
private:
	void SetUnsetFlag(int flag, bool bSet = true)
	{
		if (bSet)
			miFlags |= flag;
		else
			miFlags &= ~flag;
	}
	CString mSearchKey;
	CString mRoot;
	CTime mLastUpdated;
	UINT miMissCount;
	int miFlags;
	friend class CSearchHistoryArray;
};
typedef CSortedArray<CSearchHistory> CSortedArrayCSearchHistory;

class CFindServerDlg;

#define SDF_CHECK_INPROGRESS FLAGBIT(0)

class CSearchHistoryArray {
public:
	CSearchHistoryArray(CFindServerDlg *pFindServerDlg);
	struct SearchData {
		UINT uSearchFlags; // SDF_ flags
	};
	const CSearchHistory* FindInSearchHistory(const CString &inSearchHistory, int &retVal, SearchData *pMoreSearchData = NULL);
	void Load(FindDataBase *pDataBase = NULL);
	int ItrSearchHistoryTableRowsCallbackFn(sqlite3_stmt *statement, void *pUserData);
	int ItrCachedDataTableRowsCallbackFn(sqlite3_stmt *statement, void *pUserData);
	INT_PTR Insert(const CString &searchPath);
	CSearchHistory* GetAt(INT_PTR pos);
	CSearchHistory* Find(const CSearchHistory &srchHistory);
	const CSortedArrayCSearchHistory& GetArray() const	{return mSearchHistory;}
	INT_PTR GetCount() const {return mSearchHistory.GetCount();}
private:
	void BuildSearchHistoryFromCacheData(FindDataBase *pDataBase);
	CSortedArrayCSearchHistory mSearchHistory;
	CFindServerDlg *m_pFindServerDlg;
};