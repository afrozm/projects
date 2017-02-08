#pragma once

#include "FindDataBase.h"
#include "DBCommiter.h"


struct FileTableEntry
{
    CString path;
    CTime fileModTime, lastUpdatedTime;
    void UpdateFromSQliteEntry(sqlite3_stmt *statement);
    FileTableEntry(LPCTSTR inpath = NULL);
    const std::string& GetFileID() const { return fileID; }
    bool SetFileID(const std::string &fileID);
    unsigned GetMissedCount() const;
    unsigned IncrementMissedCount();
    bool UpdateFileModTime();
    bool IsUpdated() const { return bUpdated; }
    bool IsFileIDEmpty() const { return fileID.empty() || fileID == "-"; }
private:
    std::string fileID;
    unsigned flags;
    bool bUpdated;
};


class ContentSearchManager
{
public:
    ContentSearchManager();
    ~ContentSearchManager();
    enum ContentSearchFlags
    {
        CheckIfContentSearchRequired = FLAGBIT(1),
        ForceSearch = FLAGBIT(2)
    };
    bool StartContentSearch(unsigned uFlags = 0);
    void StopContentSearch(bool bCancel = false, bool bWaitForFinish = true);
    void AddFileEntry(const FileTableEntry &fte);
    void UpdateFileEntriesFromSourceDB(FindDataBase &inDB);
    void RemoveFileEntry(LPCTSTR filePath, bool bsqlEscaped = false, const char *fileID = nullptr);
    void RemoveFileEntry(const FileTableEntry &fte);
    DEFINE_FUNCTION_IS_FLAGBIT_SET(m_uFlags, SearchStarted);
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
        SearchForce,
    };
    DEFINE_FUNCTION_SET_FLAGBIT(m_uFlags, SearchStarted);
    DEFINE_FUNCTION_SET_GET_FLAGBIT(m_uFlags, SearchFinished);
    DEFINE_FUNCTION_SET_GET_FLAGBIT(m_uFlags, SearchScheduleImmediate);
    DEFINE_FUNCTION_SET_GET_FLAGBIT(m_uFlags, SearchForce);
    unsigned m_uFlags;
    DWORD m_dwContentSearchThreadIDManager;
    CDBCommiter *m_pDBCommitter;
    CString m_DBSearchState; // fresh, search
};

