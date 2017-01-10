#include "stdafx.h"
#include "ContentSearchManager.h"
#include "ThreadManager.h"
#include "FindServerDlg.h"
#include "MD5.h"
#include "WordParser.h"
#include "DataReader.h"

TableItertatorClass(ContentSearchManager);

ContentSearchManager::ContentSearchManager()
    : mContentDatabase(FDB_Words), m_dwContentSearchThreadIDManager(0), m_pDBCommitter(NULL),
    uFlags(0)
{
}


ContentSearchManager::~ContentSearchManager()
{
}

FindDataBase& ContentSearchManager::GetDatabase()
{
    if (!mContentDatabase.IsOpen())
        mContentDatabase.Open();
    return mContentDatabase;
}

enum ContentSearchThreadOperation {
    ManagerThread,
    ContentSearchThread
};
#define CM_THREAD_CLASS 0x389

bool ContentSearchManager::StartContentSearch(bool bCheckIfContentSearchRequired /* = false */)
{
    if (!IsSearchStarted()) {
        bool bStart(true);
        if (bCheckIfContentSearchRequired) {
            int hasFileEntries(0);
            ItrTableRowsCallbackData_ContentSearchManager iterFile(this, &ContentSearchManager::ItrFileTableRowsCallbackFn_CheckEntries, &hasFileEntries);
            bool bDBIsOpen(mContentDatabase.IsOpen());
            iterFile.IterateTableRows(GetDatabase(), "File");
            if (hasFileEntries == 0) {
                if (!bDBIsOpen)
                    GetDatabase().Close();
                bStart = false;
            }
        }
        if (bStart) {
            GetDBCommitter();
            SetSearchStarted();
            SetSearchFinished(false);
            m_dwContentSearchThreadIDManager = StartThreadOperation(ManagerThread, NULL);
        }
    }
    return IsSearchStarted();
}

void ContentSearchManager::StopContentSearch(bool bCancel /* = false */, bool bWaitForFinish /* = true */)
{
    if (bCancel)
        SetSearchStarted(false);
    if (bWaitForFinish)
        WaitForFinish();
}

void ContentSearchManager::WaitForFinish()
{
    SetSearchFinished();
    ThreadManager::GetInstance().WaitForThread(m_dwContentSearchThreadIDManager);
    DBCommiterManager::GetInstance().RemoveDBCommitter(&mContentDatabase);
    m_pDBCommitter = NULL;
}

void ContentSearchManager::AddFileEntry(const FileTableEntry &fte)
{
    // Add entry to content db
    CString query;
    bool bNewEntry(fte.fileID.IsEmpty());
    if (bNewEntry)
        query.Format(_T("INSERT OR IGNORE INTO File VALUES ('%s', '-', %I64d, 0, 0)"),
            fte.path, SystemUtils::TimeToInt(fte.fileModTime));
    else
        query.Format(_T("INSERT OR REPLACE INTO File VALUES ('%s', '%s', %I64d, %I64d, 0)"),
            fte.path, fte.fileID, SystemUtils::TimeToInt(fte.fileModTime), SystemUtils::TimeToInt(fte.lastUpdatedTime));
    GetDBCommitter()->AddDBQueryString(query);
    if (bNewEntry) {
        StartContentSearch();
        SetSearchScheduleImmediate();
    }
}

void ContentSearchManager::UpdateFileEntriesFromSourceDB(FindDataBase &inDB)
{
    if (inDB.GetType() == FDB_CacheDatabase) {
        ItrTableRowsCallbackData_ContentSearchManager iterCachedData(this, &ContentSearchManager::ItrCachedDataTableRowsCallbackFn_UpdateFileEntries);
        iterCachedData.IterateTableRows(inDB, "CachedData");
    }
}

void ContentSearchManager::RemoveFileEntry(LPCTSTR filePath, bool bsqlEscaped /* = false */)
{
    CString path(filePath);
    CString query, condition;
    {
        CString qpath(path);
        if (!bsqlEscaped)
            FindDataBase::MakeSQLString(qpath);
        condition.Format(_T(" WHERE Path='%s'"), qpath);
    }
    FileTableEntry fte(filePath);
    fte.GetFileID(true);
    query.Format(_T("DELETE FROM File%s"), condition);
    GetDBCommitter()->AddDBQueryString(query);
    // Delete word entries if md5 file count is zero
    query.Format(_T("||File| WHERE FileID='%s'|DELETE FROM Word WHERE FileID='%s'"), fte.fileID, fte.fileID);
    GetDBCommitter()->AddDBQueryString(query);
}

struct ThreadData {
    ContentSearchManager *pContentSearchManager;
    ContentSearchThreadOperation threadOp;
    LPVOID pThreadData;
    ThreadData(ContentSearchManager *inpContentSearchManager, ContentSearchThreadOperation inthreadOp, LPVOID inpThreadData)
    {
        pContentSearchManager = inpContentSearchManager;
        threadOp = inthreadOp;
        pThreadData = inpThreadData;
    }
};

int ContentSearchManager::TMContentSearchManagerThreadProcFn(LPVOID pInThreadData)
{
    ThreadData *pThreadData((ThreadData *)pInThreadData);
    pThreadData->pContentSearchManager->DoThreadOperation(pThreadData);
    delete pThreadData;
    return 0;
}

static LPCTSTR GetNextDBState(const LPCTSTR inStr)
{
    static LPCTSTR states[]{
        _T("fresh"),
        _T("search")
    };
    if (inStr == NULL || *inStr == 0)
        return states[0];
    int i = 0;
    for (; i < _countof(states); ++i)
        if (!lstrcmpi(states[i], inStr))
            break;
    if (i >= _countof(states))
        i = 0;
    else
        i = (i + 1) % _countof(states);
    return states[i];
}
struct ItrFileTableCallbackData
{
    const bool& bIsTerminated;
    unsigned long long ullLastFileIndex, ullCurIndex;
};
#define IsSearchContinue() (IsSearchStarted() || bIsTerminated)
int ContentSearchManager::ManagerThreadProc(LPVOID /*pInThreadData*/)
{
    CountTimer ct;
    SetSearchScheduleImmediate();
    m_DBSearchState = GetDatabase().GetProperty(_T("SearchState"));
    const bool& bIsTerminated(ThreadManager::GetInstance().GetIsTerminatedFlag());
    ItrFileTableCallbackData cbData = { bIsTerminated,
        (unsigned long long)SystemUtils::StringToLongLong(GetDatabase().GetProperty(_T("LastFileIndex"))) };
    int noEntriesCount(0);
    SystemUtils::LogMessage(_T("Content Search Manager: start"));
    while (IsSearchContinue())
    {
        Sleep(1000);
        // Iterate Files table
        if (ct.UpdateTimeDuration())
        {
            cbData.ullCurIndex = 0;
            SetSearchScheduleImmediate(false);
            ItrTableRowsCallbackData_ContentSearchManager iterFile(this, &ContentSearchManager::ItrFileTableRowsCallbackFn_SearchContent, &cbData);
            int retVal(iterFile.IterateTableRows(GetDatabase(), "File"));
            if (501 == retVal // call did not finish completely
                || retVal == SQLITE_OK &&
                cbData.ullCurIndex == cbData.ullLastFileIndex) {// no entries iterated - continue again after 1 min
                if (retVal == SQLITE_OK && IsSearchFinished()) {  // main search is finished and waiting for us to finish
                    ++noEntriesCount;
                    if (noEntriesCount > 3)
                        break;
                }
                SystemUtils::LogMessage(_T("Content Search Manager: %s"),
                    501 == retVal ? _T("More jobs. Waiting for worker to finish") :
                    _T("No entry found. Waiting for entries"));
                SetSearchScheduleImmediate(true);
            }
            if (IsSearchContinue()) {
                if (IsSearchScheduleImmediate()) {
                    // Did not finished
                    ct.SetTimeUpdateDuration(1000 * 60); // reschedule after 1 min
                    SetSearchScheduleImmediate(false);
                }
                else { // Finished one cycle search
                    SystemUtils::LogMessage(_T("Content Search Manager: finished one state"));
                    ct.SetTimeUpdateDuration(1000 * 60 * 60); // reschedule after 1 hr
                    m_DBSearchState = GetNextDBState(m_DBSearchState);
                    if (m_DBSearchState == GetNextDBState(NULL)) // All cycles finished
                        break;
                }
            }
        }
    }
    // Wait for worker thread
    SystemUtils::LogMessage(_T("Content Search Manager: end: Waiting for workers"));
    while (ThreadManager::GetInstance().GetThreadCount(ContentSearchThread + CM_THREAD_CLASS) > 0) {
        Sleep(1000);
        if (!IsSearchContinue()) // if canceled, terminate worker threads
            ThreadManager::GetInstance().TerminateThreads(ContentSearchThread + CM_THREAD_CLASS, 1);
    }
    GetDatabase().SetProperty(_T("SearchState"), m_DBSearchState);
    if (IsSearchContinue())
        GetDatabase().RemoveProperty(_T("LastFileIndex"));
    else
        GetDatabase().SetProperty(_T("LastFileIndex"), SystemUtils::LongLongToString(cbData.ullCurIndex));
    m_dwContentSearchThreadIDManager = 0;
    SystemUtils::LogMessage(_T("Content Search Manager: finished. Time taken %s"), ct.GetString());
    SetSearchStarted(false);
    return 0;
}

int ContentSearchManager::ContentSearchThreadProc(LPVOID pInThreadData)
{
    FileTableEntry *fte = (FileTableEntry *)pInThreadData;
    Path filePath(fte->path);
    if (!filePath.Exists()) { // not valid - remove or update missed count entry
        if (filePath.IsUNC()) {
            if (fte->IncrementMissedCount() > 3)
                RemoveFileEntry(fte->path);
            else {
                fte->GetFileID(true);
                AddFileEntry(*fte); // update count
            }
        }
        else
            RemoveFileEntry(fte->path);
    }
    if (filePath.IsDir() || !CTextReader(fte->path).IsValidTextFile()) { // not valid - remove its entry
        RemoveFileEntry(fte->path);
    }
    else {
        bool bCompute(fte->fileID.IsEmpty() || fte->fileID == _T("-"));
        bool bUpdateFileTime(true);
        if (!bCompute) {
            bCompute = fte->UpdateFileModTime();
            bUpdateFileTime = false;
        }
        if (bCompute) {
            SystemUtils::LogMessage(_T("Indexing file: %s"), fte->path);
            const bool& bIsTerminated(ThreadManager::GetInstance().GetIsTerminatedFlag());
            // MD5 of path
            fte->GetFileID(true);
            FileDataReader fdr;
            BinaryData fileData(NULL, 64 * 1024 * 1024); // 64k buffer
            WordParser wp;
            WordCount wc;
            fdr.SetFile(fte->path);
            while (IsSearchContinue()) { // Count words
                if (fdr.ReadData(fileData) == 0)
                    break;
                std::vector<std::wstring> words;
                wp.ParseWords(fileData, words);
                wc.Count(words);
            }
            if (IsSearchContinue()) { // not canceled
                // Save
                const auto &wordList(wc.GetCount());
                CStringArray queryList;
                for (const auto &words : wordList) {
                    CString query, word(words.first.c_str());
                    FindDataBase::MakeSQLString(word);
                    int count(words.second);
                    query.Format(_T("INSERT OR REPLACE INTO Word VALUES ('%s', '%s', '%d')"), word, fte->fileID, count);
                    queryList.Add(query);
                    if (!IsSearchContinue())
                        break;
                }
                if (IsSearchContinue()) {
                    CDBCommiter *pDBComitter(GetDBCommitter());
                    pDBComitter->AddDBQueryStrings(queryList);
                    fte->lastUpdatedTime = CTime::GetCurrentTime();
                    if (bUpdateFileTime)
                        fte->UpdateFileModTime();
                    AddFileEntry(*fte);
                }
            }
        }
    }

    delete fte;
    return 0;
}
void FileTableEntry::UpdateFromSQliteEntry(sqlite3_stmt * statement)
{
    path = SystemUtils::UTF8ToUnicodeCString((const char *)sqlite3_column_text(statement, File_Path));
    fileID = SystemUtils::UTF8ToUnicodeCString((const char *)sqlite3_column_text(statement, File_FileID));
    fileModTime = SystemUtils::IntToTime(sqlite3_column_int64(statement, File_FileModifiedTime));
    lastUpdatedTime = SystemUtils::IntToTime(sqlite3_column_int64(statement, File_LastSearched));
    flags = sqlite3_column_int(statement, File_Flags);
}

FileTableEntry::FileTableEntry(LPCTSTR inpath /* = NULL */)
    : flags(0)
{
    if (inpath)
        path = inpath;
}

const CString& FileTableEntry::GetFileID(bool bComputeIfEmpty /*= false*/)
{
    if (bComputeIfEmpty && (fileID.IsEmpty() || fileID == _T("-")))
        fileID = CMD5().GetMD5(path, true);
    return fileID;
}

unsigned FileTableEntry::GetMissedCount() const
{
    return (flags & 0xf);
}

unsigned FileTableEntry::IncrementMissedCount()
{
    unsigned mc(GetMissedCount());
    mc++;
    mc &= 0xf;
    flags ^= 0xf;
    flags |= mc;
    return GetMissedCount();
}

bool FileTableEntry::UpdateFileModTime()
{
    bool bUpdated(false);
    // Get file time
    FILETIME modTime = { 0 };
    Path(path).GetFileTime(NULL, NULL, &modTime);
    CTime cModTime(modTime);
    if (cModTime != fileModTime) {
        fileModTime = cModTime;
        bUpdated = true;
    }
    return bUpdated;
}

int ContentSearchManager::ItrFileTableRowsCallbackFn_SearchContent(sqlite3_stmt *statement, void * pUserData)
{
    ItrFileTableCallbackData *cbData((ItrFileTableCallbackData*)pUserData);
    const bool& bIsTerminated(cbData->bIsTerminated);
    bool bSearchContent(false);
    cbData->ullCurIndex++;
    if (cbData->ullCurIndex < cbData->ullLastFileIndex) {
        // Skip till last index
        return IsSearchContinue() ? 0 : 1;
    }
    if (m_DBSearchState.CompareNoCase(_T("fresh")) == 0)
    {
        CString line(
            SystemUtils::UTF8ToUnicodeCString((const char *)sqlite3_column_text(statement, File_FileID)));
        if (line.IsEmpty() || line == _T("-"))
            bSearchContent = true;
    }
    else // Search content
    {
        CTime time = SystemUtils::IntToTime(sqlite3_column_int64(statement, File_LastSearched));
        CTimeSpan timeDiff = CTime::GetCurrentTime() - time;
        bSearchContent = timeDiff.GetDays() > 1;
    }
    if (bSearchContent) {
        FileTableEntry *fte = new FileTableEntry;
        fte->UpdateFromSQliteEntry(statement);
        if (!StartLimitedThreadOperation(ContentSearchThread, (LPVOID)fte, 3)) {
            cbData->ullLastFileIndex = cbData->ullCurIndex;
            delete fte;
            return 501; // Close current iteration.
        }
    }
    return IsSearchContinue() ? 0 : 1;
}

int ContentSearchManager::ItrFileTableRowsCallbackFn_CheckEntries(sqlite3_stmt * /*statement*/, void *pUserData)
{
    *(int *)pUserData = 1;
    return 1;
}

int ContentSearchManager::ItrCachedDataTableRowsCallbackFn_UpdateFileEntries(sqlite3_stmt *statement, void * /*pUserData*/)
{
    FileTableEntry fte((LPCTSTR)SystemUtils::UTF8ToUnicodeCString((const char *)sqlite3_column_text(statement, CachedData_Path)));
    fte.fileModTime = SystemUtils::IntToTime(sqlite3_column_int64(statement, CachedData_ModifiedTime));
    AddFileEntry(fte);
    return IsSearchStarted() ? 0 : 1;
}

CDBCommiter* ContentSearchManager::GetDBCommitter()
{
    if (m_pDBCommitter == NULL)
        m_pDBCommitter = DBCommiterManager::GetInstance().AddDbCommiter(&GetDatabase());
    return m_pDBCommitter;
}

bool ContentSearchManager::StartLimitedThreadOperation(int op, LPVOID threadData, int nTries /* = -1 */)
{
    // Wait till one on the thread is freed.
    while (ThreadManager::GetInstance().GetThreadCount(op+ CM_THREAD_CLASS) > 10 && IsSearchStarted()) {
        if (nTries == 0)
            return false;
        else if (nTries > 0)
            --nTries;
        Sleep(1000);
    }
    if (IsSearchStarted()) {
        StartThreadOperation(op, threadData);
        return true;
    }
    return false;
}
static LPCTSTR GetThreadName(ContentSearchThreadOperation threadOp)
{
    switch (threadOp)
    {
    case ManagerThread:
        return _T("CSManager");
    case ContentSearchThread:
        return _T("CSWorker");
    }
    return NULL;
}
int ContentSearchManager::StartThreadOperation(int op, LPVOID threadData)
{
    DWORD threadID(0);
    ThreadData *td = new ThreadData(this, (ContentSearchThreadOperation)op, threadData);
    ThreadManager::GetInstance().CreateThread(TMContentSearchManagerThreadProcFn, td, op+CM_THREAD_CLASS, &threadID, GetThreadName(ContentSearchThreadOperation(op)));
    return threadID;
}
int ContentSearchManager::DoThreadOperation(LPVOID pInThreadData)
{
    ThreadData *pThreadData((ThreadData *)pInThreadData);
    switch (pThreadData->threadOp)
    {
    case ManagerThread:
        return ManagerThreadProc(pThreadData->pThreadData);
    case ContentSearchThread:
        return ContentSearchThreadProc(pThreadData->pThreadData);
    default:
        break;
    }
    return 0;
}
