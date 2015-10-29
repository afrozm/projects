#include "StdAfx.h"
#include "SearchHistory.h"
#include "Path.h"
#include "SystemUtils.h"
#include "Find.h"
#include "FindServerDlg.h"


CArrayCString& GetQueryRemoveFromSearchHistory(const CString &searchHistory, CArrayCString &queryArray)
{
	CString srchHis(Path(searchHistory).GetRoot());
	FindDataBase::MakeSQLString(srchHis);
	CString query;
	query.Format(_T("DELETE FROM SearchHistory WHERE SearchKeys='%s'"), srchHis);
	queryArray.Add(query);
	return queryArray;
}
CArrayCString& GetQueryAddToSearchHistory(const CString &searchHistory, CArrayCString &queryArray, int flags, int misscount)
{
	CString srchPath(searchHistory);
	FindDataBase::MakeSQLString(srchPath);
	CString srchKeys(Path(searchHistory).GetRoot());
	srchPath = Path(srchPath).RemoveRoot();
	CString query;
	query.Format(_T("INSERT OR REPLACE INTO SearchHistory VALUES ('%s', %d, %d, '%s', %d)"),
		srchKeys,
		SystemUtils::TimeToInt(CTime::GetCurrentTime()),
		misscount, srchPath, flags);
	queryArray.Add(query);
	return queryArray;
}


CSearchHistory::CSearchHistory(void)
: miMissCount(0), miFlags(SHF_IS_AVAILABLE)
{
}
CSearchHistory::CSearchHistory(const CString &searchKey)
: miMissCount(0), miFlags(SHF_IS_AVAILABLE),
	mSearchKey(searchKey),
	mRoot(Path(searchKey).GetRoot())
{
}
CSearchHistory::~CSearchHistory(void)
{
}
int CSearchHistory::Compare(const CSearchHistory &searchHistory) const
{
	return mRoot.CollateNoCase(searchHistory.mRoot);
}
int CSearchHistory::Load(sqlite3_stmt *statement, const CTime &pLastSearchStartTime)
{
	mRoot = (SystemUtils::UTF8ToUnicodeCString((const char *)sqlite3_column_text(statement, SearchHistory_SearchKeys)));
	mLastUpdated = SystemUtils::IntToTime(sqlite3_column_int64(statement, SearchHistory_LastUpdated));
	miMissCount = sqlite3_column_int(statement, SearchHistory_MissCount);
	mSearchKey = Path(mRoot).Append((SystemUtils::UTF8ToUnicodeCString((const char *)sqlite3_column_text(statement, SearchHistory_LastSearchPath))));
	miFlags =  sqlite3_column_int(statement, SearchHistory_flags);
	if (miFlags & SHF_ALREADY_SEARCHED) {
		if (mLastUpdated < pLastSearchStartTime)
			miFlags &= ~SHF_ALREADY_SEARCHED; // Should include in search
	}
	return 0;
}
bool CSearchHistory::SetAvailable(bool bAvailable)
{
	bool bOldVal(IsAvailable());
	if (bAvailable)
		miFlags |= SHF_IS_AVAILABLE;
	else
		miFlags &= ~SHF_IS_AVAILABLE;
	return bOldVal;
}
static int CSearchHistoryCompare(const CSearchHistory *elem1, const CSearchHistory *elem2)
{
	return elem1->Compare(*elem2);
}

CSearchHistoryArray::CSearchHistoryArray(CFindServerDlg *pFindServerDlg)
: mSearchHistory((GenericCompareFn)CSearchHistoryCompare), m_pFindServerDlg(pFindServerDlg)
{
}
int CSearchHistoryArray::ItrSearchHistoryTableRowsCallbackFn(sqlite3_stmt *statement, void *pUserData)
{
    UNREFERENCED_PARAMETER(pUserData);
	CSearchHistory newCSearchHistory;
	newCSearchHistory.Load(statement, m_pFindServerDlg->GetLastStartTime());
	mSearchHistory.Insert(newCSearchHistory);
	return 0;
}
int CSearchHistoryArray::ItrCachedDataTableRowsCallbackFn(sqlite3_stmt *statement, void *pUserData)
{
    UNREFERENCED_PARAMETER(pUserData);
	CSearchHistory newCSearchHistory(Path(SystemUtils::UTF8ToUnicodeCString((const char *)sqlite3_column_text(statement, 0))).GetRoot());
	if (Find(newCSearchHistory) == NULL) {
		mSearchHistory.Insert(newCSearchHistory);
		CArrayCString queryArray;
		GetQueryAddToSearchHistory(newCSearchHistory.GetRootKey(), queryArray);
		m_pFindServerDlg->AddDBQueryStrings(queryArray);
		SystemUtils::LogMessage(_T("Adding missing entry (%s) in search history"), newCSearchHistory.GetRootKey());
	}
	return 0;
}
TableItertatorClass(CSearchHistoryArray);
void CSearchHistoryArray::BuildSearchHistoryFromCacheData(FindDataBase *pDataBase)
{
	if (mSearchHistory.HasDuplicates() >= 0) {
		mSearchHistory.RemoveAll();
		m_pFindServerDlg->AddDBQueryString(_T("DELETE FROM SearchHistory"));
		LoggerFacory::GetInstance().GetLogger(_T("")).Log(Logger::kLogLevelWarning, _T("Rebuilding search history - duplicate found"));
	}
	ItrTableRowsCallbackData_CSearchHistoryArray itSHTable(this,
		&CSearchHistoryArray::ItrCachedDataTableRowsCallbackFn,
		NULL);
	SelectData selectData;
	selectData.bIsDistinct = true;
	const char *columns[] = {"Root", NULL};
	selectData.columns = columns;
	itSHTable.IterateTableRows(*pDataBase, "CachedData", "", &selectData);

}
void CSearchHistoryArray::Load(FindDataBase *pDataBase)
{
	// Clear all 
	mSearchHistory.RemoveAll();
	if (pDataBase != NULL) {
		ItrTableRowsCallbackData_CSearchHistoryArray itSHTable(this,
			&CSearchHistoryArray::ItrSearchHistoryTableRowsCallbackFn,
			NULL);
		itSHTable.IterateTableRows(*pDataBase, "SearchHistory");
		BuildSearchHistoryFromCacheData(pDataBase);
		mSearchHistory.Sort();
	}
}
INT_PTR CSearchHistoryArray::Insert(const CString &searchPath)
{
	return mSearchHistory.Insert(CSearchHistory(searchPath));
}
CSearchHistory* CSearchHistoryArray::GetAt(INT_PTR pos)
{
	return &mSearchHistory.GetAt(pos);
}
CSearchHistory* CSearchHistoryArray::Find(const CSearchHistory &srchHistory)
{
	INT_PTR pos(mSearchHistory.Find(srchHistory));
	if (pos >= 0)
		return &mSearchHistory.GetAt(pos);
	return NULL;
}
const CSearchHistory* CSearchHistoryArray::FindInSearchHistory(const CString &inSearchHistory, int &retVal, SearchData *pMoreSearchData /* = NULL */)
{
	retVal = FSH_CONTINUE_SEARCH;
	CSearchHistory *outSearchHistory(NULL);
	if (mSearchHistory.GetCount() > 0) {
		const CSearchHistory searchHis(inSearchHistory);
		const CString &searchRoot(searchHis.GetRootKey());
		outSearchHistory = (CSearchHistory *)Find(searchHis);
		// Check in search history
		if (outSearchHistory != NULL) {
			outSearchHistory->miFlags |= SHF_IS_AVAILABLE;
			bool bCheckInProgress(pMoreSearchData && IS_FLAG_SET(pMoreSearchData->uSearchFlags, SDF_CHECK_INPROGRESS));
			if (outSearchHistory->IsAlreadySearched(bCheckInProgress))
				retVal = FSH_SKIP_SEARCH;
			else if (searchRoot != inSearchHistory) { // if it is path - then check if it smaller - if so skip it
				if (ComparePath(inSearchHistory, outSearchHistory->GetSearchKey()) < 0)
					retVal = FSH_SKIP_SEARCH;
			}
			if (retVal == FSH_CONTINUE_SEARCH && bCheckInProgress)
				outSearchHistory->miFlags |= SHF_IN_PROGRESS;
		}
	}
	return outSearchHistory;
}
