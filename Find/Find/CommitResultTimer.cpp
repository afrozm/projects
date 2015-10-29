#include "StdAfx.h"
#include "CommitResultTimer.h"
#include "SystemUtils.h"
#include "SaveListResultCtrl.h"
#include "Path.h"
#include "FileMetaDataProvider.h"

CCommitResultTimer::CCommitResultTimer()
: mListCtrl(NULL),
miListIndexSaved(0), mLastSavedTime(0), mbInCommit(false)
{
}

CCommitResultTimer::~CCommitResultTimer(void)
{
}

bool CCommitResultTimer::DoCommit(unsigned int uFlags)
{
	if (mbInCommit)
		return false;
	mbInCommit = true;
	bool bCommited(false);
	bool bForce = (uFlags & CRTCF_FORCE) != 0;
	DWORD currentTickCount(GetTickCount());
	if (currentTickCount - mLastSavedTime > 1000*60 // 1 min
		|| bForce) {
		mLastSavedTime = currentTickCount;
		int listCount(mListCtrl->GetItemCount());
		if (listCount - miListIndexSaved > 0
			|| bForce) {
			int startIndex = miListIndexSaved;
			miListIndexSaved = listCount;
			FindDataBase db;
			if (db.Open() == 0) {
				if (uFlags & CRTCF_REMOVE) {
					db.QueryNonRows("DELETE FROM RecentSearch");
				}
				else {
					for (; startIndex < listCount; ++startIndex) {
						CString values;
						CString text(mListCtrl->GetItemText(startIndex, 1));
						FindDataBase::MakeSQLString(text);
						CListResItemData *pListData((CListResItemData *)mListCtrl->GetItemData(startIndex));
						ASSERT(pListData != NULL);
						values.Format(_T("'%s', '%s', '%I64d', '%I64d', '%I64d'"), text,
							SystemUtils::GetReadableSize(pListData->m_ullFileSize),
							SystemUtils::TimeToInt(pListData->mCreatedTime),
							SystemUtils::TimeToInt(pListData->mModifedTime),
							SystemUtils::TimeToInt(pListData->mAccessedTime));
						db.QueryNonRows("INSERT INTO RecentSearch VALUES (%s)",
							SystemUtils::UnicodeToUTF8(values).c_str());
					}
				}
				db.Commit();
				if (uFlags & CRTCF_REMOVE) {
					db.Vacuum();
				}
				bCommited = true;
			}
		}
	}
	mbInCommit = false;
	return bCommited;
}

TableItertatorClass(CCommitResultTimer);

int CCommitResultTimer::ItrRecentSearchTableRowsCallbackFn(sqlite3_stmt *statement, void * /*pUserData*/)
{
	Path path(SystemUtils::UTF8ToUnicodeCString((const char *)sqlite3_column_text(statement, RecentSearch_Path)));
	CString size(SystemUtils::UTF8ToUnicodeCString((const char *)sqlite3_column_text(statement, RecentSearch_Size)));
	int item = mListCtrl->InsertItem(0x7fffffff, path.FileName());
	CListResItemData *pListItemData = new CListResItemData(CCacheDataFileMetaDataProvider(statement, RecentSearch_Size, RecentSearch_CreatedTime, RecentSearch_ModifiedTime, RecentSearch_AccessedTime, -1));
	mListCtrl->SetItemData(item, (DWORD_PTR)pListItemData);
	mListCtrl->SetItemText(item, 1, path);
	mListCtrl->SetOptionalColumnItemText(item, ListColumns_Size, size);
	mListCtrl->UpdateOptionalDateColumns(item);
	return 0;
}

bool CCommitResultTimer::Load()
{
	bool bLoaded(false);
	FindDataBase db(FDB_PrefDatabase, true);
	if (db.Open() == 0) {
		mListCtrl->UpdateImageList();
		ItrTableRowsCallbackData_CCommitResultTimer itSHTable(this,
				&CCommitResultTimer::ItrRecentSearchTableRowsCallbackFn);
		itSHTable.IterateTableRows(db, "RecentSearch");
		bLoaded = true;
	}
	return bLoaded;
}