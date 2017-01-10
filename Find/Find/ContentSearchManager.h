#pragma once

#include "FindDataBase.h"
#include "DBCommiter.h"


struct FileTableEntry
{
    CString path, fileID;
    CTime fileModTime, lastUpdatedTime;
    unsigned flags;
    void UpdateFromSQliteEntry(sqlite3_stmt *statement);
    FileTableEntry(LPCTSTR inpath = NULL);
    const CString& GetFileID(bool bComputeIfEmpty = false);
    unsigned GetMissedCount() const;
    unsigned IncrementMissedCount();
    bool UpdateFileModTime();
};


class ContentSearchManager
{
public:
    ContentSearchManager();
    ~ContentSearchManager();
    bool StartContentSearch(bool bCheckIfContentSearchRequired = false);
    void StopContentSearch(bool bCancel = false, bool bWaitForFinish = true);
    void AddFileEntry(const FileTableEntry &fte);
    void UpdateFileEntriesFromSourceDB(FindDataBase &inDB);
    void RemoveFileEntry(LPCTSTR filePath, bool bsqlEscaped = false);
    DEFINE_FUNCTION_IS_FLAGBIT_SET(uFlags, SearchStarted);
private:
    void WaitForFinish();
    FindDataBase& GetDatabase();
    bool StartLimitedThreadOperation(int op, LPVOID threadData, int nTries = -1);
    int StartThreadOperation(int op, LPVOID threadData);
    int DoThreadOperation(LPVOID pInThreadData);
    static int TMContentSearchManagerThreadProcFn(LPVOID pInThreadData);
    int ManagerThreadProc(LPVOID pInThreadData);
    int ContentSearchThreadProc(LPVOID pInThreadData);
    int ItrFileTableRowsCallbackFn_SearchContent(sqlite3_stmt *statement, void *pUserData);
    int ItrFileTableRowsCallbackFn_CheckEntries(sqlite3_stmt *statement, void *pUserData);
    int ItrCachedDataTableRowsCallbackFn_UpdateFileEntries(sqlite3_stmt *statement, void *pUserData);
    CDBCommiter* GetDBCommitter();
    FindDataBase mContentDatabase;
    enum Flags {
        SearchStarted,
        SearchFinished,
        SearchScheduleImmediate,
    };
    DEFINE_FUNCTION_SET_FLAGBIT(uFlags, SearchStarted);
    DEFINE_FUNCTION_SET_GET_FLAGBIT(uFlags, SearchFinished);
    DEFINE_FUNCTION_SET_GET_FLAGBIT(uFlags, SearchScheduleImmediate);
    unsigned uFlags;
    DWORD m_dwContentSearchThreadIDManager;
    CDBCommiter *m_pDBCommitter;
    CString m_DBSearchState; // fresh, search
};

