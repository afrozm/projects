#pragma once

#include "FindDataBase.h"
#include "DBCommiter.h"
#include <set>
#include "StringMatcher.h"
#include <list>

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

class FileIDVsPath
{
public:
    FileIDVsPath();
    typedef std::set<std::string> FileList;
    const FileList& GetFilesFromFileId(const std::string &fileID) const;
    void AddFileForFileID(const std::string &fileID, const std::string &file);
    void Reset();
private:
    typedef std::map<std::string, FileList> MapFildIdVsPaths;
    MapFildIdVsPaths mMapFildIdVsPaths;
    CountTimer mClearTimer;
};

class ContentMatchCallBack
{
public:
    ContentMatchCallBack();
    ~ContentMatchCallBack();
    void SetMatchPattern(LPCTSTR matchPattern = nullptr);
    struct MatchCallBackData
    {
        MatchCallBackData();
        bool MatchCallBackData::operator<(const MatchCallBackData &other) const;
        std::string fileID;
        lstring matchWord;
        int matchCount, matchWeight;
    };
    virtual void MatchCallBack(MatchCallBackData &mcd);
    bool Match(const lstring &inWord);
    void Reset();
    typedef std::list<MatchCallBackData*> ListResultData;
    const ListResultData& Result();
    virtual int StatusCheck(int /*iUpdate*/) { return 0; }
    bool HasResult() const { return !mResult.empty(); }
protected:
    ListResultData mResult;
    StringMatcher *m_pMatcher;
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
    void RemoveFileEntry(LPCTSTR filePath, bool bsqlEscaped = false, const char *fileID = nullptr);
    void RemoveFileEntry(const FileTableEntry &fte);
    
    int GetMatchingFiles(ContentMatchCallBack &matchCallback, LPCTSTR inOptWordToMatch = nullptr, LPCTSTR inOptFile = nullptr);
    const FileIDVsPath::FileList& GetFilePathFromFileID(const std::string &fileID, FileIDVsPath *cachedFileIDVsPath = nullptr, FileIDVsPath::FileList *outList = nullptr);
    DEFINE_FUNCTION_IS_FLAGBIT_SET(m_uFlags, SearchStarted);

    bool HasContent();
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
    int ItrWordTableRowsCallbackFn_SearchWord(sqlite3_stmt *statement, void *pUserData);
    std::string GetFileIDFromFilePath(const std::string &inFilePath);
    CDBCommiter* GetDBCommitter();
    FindDataBase mContentDatabase;
    enum Flags {
        SearchStarted,
        SearchFinished,
        SearchHasJob,
        SearchForce,
    };
    DEFINE_FUNCTION_SET_FLAGBIT(m_uFlags, SearchStarted);
    DEFINE_FUNCTION_SET_GET_FLAGBIT(m_uFlags, SearchFinished);
    DEFINE_FUNCTION_SET_GET_FLAGBIT(m_uFlags, SearchHasJob);
    DEFINE_FUNCTION_SET_GET_FLAGBIT(m_uFlags, SearchForce);
    unsigned m_uFlags;
    DWORD m_dwContentSearchThreadIDManager;
    CDBCommiter *m_pDBCommitter;
    CTime mSearchStartTime;
};

