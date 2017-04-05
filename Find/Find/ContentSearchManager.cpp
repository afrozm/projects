#include "stdafx.h"
#include "ContentSearchManager.h"
#include "ThreadManager.h"
#include "FindServerDlg.h"
#include "cMD5.h"
#include "WordParser.h"
#include "DataReader.h"


FileIDVsPath::FileIDVsPath()
    : mClearTimer(1000*60*60) // 1hr
{

}

const FileIDVsPath::FileList& FileIDVsPath::GetFilesFromFileId(const std::string &fileID) const
{
    static FileList sEmptyFileList;
    auto cit(mMapFildIdVsPaths.find(fileID));
    if (cit != mMapFildIdVsPaths.end())
        return cit->second;
    return sEmptyFileList;
}


void FileIDVsPath::AddFileForFileID(const std::string &fileID, const std::string &file)
{
    mClearTimer.UpdateTimeDuration(true);
    mMapFildIdVsPaths[fileID].insert(file);
}


void FileIDVsPath::Reset()
{
    if (mClearTimer.UpdateTimeDuration(mMapFildIdVsPaths.size() > 10000))
        mMapFildIdVsPaths.clear();
}

ContentMatchCallBack::MatchCallBackData::MatchCallBackData()
    : matchCount(0), matchWeight(0)
{

}



bool ContentMatchCallBack::MatchCallBackData::operator<(const MatchCallBackData &other) const
{
    int diff = fileID.compare(other.fileID);
    if (!diff)
        diff = matchWeight - other.matchWeight;
    if (!diff)
        diff = matchCount - other.matchCount;
    if (!diff)
        diff = matchWord.compare(other.matchWord);
    return diff < 0;
}

ContentMatchCallBack::ContentMatchCallBack()
    : m_pMatcher(nullptr)
{
}

ContentMatchCallBack::~ContentMatchCallBack()
{
    Reset();
}

void ContentMatchCallBack::SetMatchPattern(LPCTSTR matchPattern /*= nullptr*/)
{
    if (m_pMatcher)
        delete m_pMatcher;
    m_pMatcher = nullptr;
    if (matchPattern && *matchPattern)
        m_pMatcher = StringMatcher_GetStringMatcher(matchPattern);
}

void ContentMatchCallBack::MatchCallBack(MatchCallBackData &mcd)
{
    if (m_pMatcher)
        mcd.matchWeight += m_pMatcher->GetMatchWeight();
    mResult.push_back(new MatchCallBackData(mcd));
}

bool ContentMatchCallBack::Match(const lstring &inWord)
{
    if (m_pMatcher)
        return m_pMatcher->Match(inWord.c_str());
    return true;
}

void ContentMatchCallBack::Reset()
{
    SetMatchPattern();
    for (auto it : mResult)
        delete it;
    mResult.clear();
}

struct ResultGroupData // same file ID
{
    ResultGroupData(int s = 0, int e = 0, int w = 0, int mc = 0) : startPos(s), endPos(e), wt(w), matchCountInFile(mc) {}
    int Count() const { return endPos - startPos; }
    bool operator<(const ResultGroupData& r)const // defined a oppsite way in order to do reverse sort
    {
        int diff(Count() - r.Count());      // Check count
        if (!diff)
            diff = wt - r.wt;           // check weight
        if (!diff)                      // check match count in file
            diff = matchCountInFile - r.matchCountInFile;
        return diff > 0;
    }
    int startPos, endPos, wt, matchCountInFile;
};

static bool CompareMatchCallBackData(const ContentMatchCallBack::MatchCallBackData *first, const ContentMatchCallBack::MatchCallBackData *second)
{
    return !(*first < *second); // reverse sort
}

const ContentMatchCallBack::ListResultData& ContentMatchCallBack::Result()
{
    mResult.sort(CompareMatchCallBackData);
    std::list<ResultGroupData> listGroup;
    int iStart = 0, iCurrent = 0, iWeight = 0, iMC = 0;
    std::string *pPreviousFile = nullptr;
    for (auto & l : mResult) {
        if (pPreviousFile && *pPreviousFile != l->fileID) {
            listGroup.push_back(ResultGroupData(iStart, iCurrent, iWeight, iMC));
            iStart = iCurrent;
            iWeight = 0;
            iMC = 0;
        }
        iWeight += l->matchWeight;
        iMC += l->matchCount;
        pPreviousFile = &l->fileID;
        ++iCurrent;
    }
    if (pPreviousFile) // push last group
        listGroup.push_back(ResultGroupData(iStart, iCurrent, iWeight, iMC));
    listGroup.sort();       // This does reverse sort due to < operator defined as opposite way
    ListResultData copyData;
    for (auto &lg : listGroup) {
        auto sit(mResult.begin());
        auto eit(sit);
        std::advance(sit, lg.startPos);
        std::advance(eit, lg.endPos);
        copyData.insert(copyData.end(), sit, eit);
    }
    mResult = copyData;
    return mResult;
}

TableItertatorClass(ContentSearchManager);

ContentSearchManager::ContentSearchManager()
    : mContentDatabase(FDB_Words), m_dwContentSearchThreadIDManager(0), m_pDBCommitter(NULL),
    m_uFlags(0)
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

bool ContentSearchManager::StartContentSearch(unsigned uFlags /* = 0 */)
{
    if (!IsSearchStarted()) {
        bool bStart(true);
        if (IS_FLAG_SET(uFlags, CheckIfContentSearchRequired)) {
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
            SetSearchForce(IS_FLAG_SET(uFlags, ForceSearch));
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
    SetSearchForce(false);
    ThreadManager::GetInstance().WaitForThread(m_dwContentSearchThreadIDManager);
    DBCommiterManager::GetInstance().RemoveDBCommitter(&mContentDatabase);
    m_pDBCommitter = NULL;
}

void ContentSearchManager::AddFileEntry(const FileTableEntry &fte)
{
    // Add entry to content db
    CString query;
    bool bNewEntry(fte.IsFileIDEmpty());
    if (bNewEntry)
        query.Format(_T("INSERT OR IGNORE INTO File VALUES ('%s', '-', %I64d, 0, 0)"),
            fte.path, SystemUtils::TimeToInt(fte.fileModTime));
    else
        query.Format(_T("INSERT OR REPLACE INTO File VALUES ('%s', '%s', %I64d, %I64d, 0)"),
            fte.path, UTF8_TO_UNICODE(fte.GetFileID()).c_str(), SystemUtils::TimeToInt(fte.fileModTime), SystemUtils::TimeToInt(fte.lastUpdatedTime));
    GetDBCommitter()->AddDBQueryString(query);
    if (bNewEntry) {
        SetSearchHasJob();
        StartContentSearch();
    }
}

void ContentSearchManager::RemoveFileEntry(const FileTableEntry & fte)
{
    RemoveFileEntry(fte.path, false, fte.GetFileID().c_str());
}

void ContentSearchManager::RemoveFileEntry(LPCTSTR filePath, bool bsqlEscaped /* = false */, const char *fileID /* = nullptr */)
{
    CString path(filePath);
    CString query, condition;
    {
        CString qpath(path);
        if (!bsqlEscaped)
            FindDataBase::MakeSQLString(qpath);
        condition.Format(_T("WHERE Path='%s'"), qpath);
    }
    FileTableEntry fte(filePath);
    fte.SetFileID(fileID ? fileID : "");
    if (fileID == NULL) {
        CArrayCString queries;
        // save file id and delete file entry
        query.Format(_T("|DELETE FROM File%s|File.|%s"), condition, condition);
        queries.Add(query);
        // delete word entries for saved file id.
        query.Format(_T("||File?|WHERE FileID='[1]'|DELETE FROM Word WHERE FileID='[1]'"));
        queries.Add(query);
        GetDBCommitter()->AddDBQueryStrings(queries);
    }
    else {
        query.Format(_T("DELETE FROM File%s"), condition);
        GetDBCommitter()->AddDBQueryString(query);
        if (!fte.IsFileIDEmpty()) {// Delete word entries if md5 file count is zero
            CString csFileID(StringUtils::UTF8ToUnicode(fte.GetFileID()).c_str());
            query.Format(_T("||File|WHERE FileID='%s'|DELETE FROM Word WHERE FileID='%s'"), csFileID, csFileID);
            GetDBCommitter()->AddDBQueryString(query);
        }
    }
}

int ContentSearchManager::ItrWordTableRowsCallbackFn_SearchWord(sqlite3_stmt *statement, void *pUserData)
{
    if (!IsSearchStarted())
        return 1;
    ContentMatchCallBack *pCallbackData((ContentMatchCallBack*)pUserData);
    lstring word = UTF8_TO_UNICODE(((const char *)sqlite3_column_text(statement, Word_Word)));
    int iUpdate(0);
    if (pCallbackData->Match(word)) { // matched
        ContentMatchCallBack::MatchCallBackData mcd;
        mcd.fileID = ((const char *)sqlite3_column_text(statement, Word_FileID));
        mcd.matchCount = sqlite3_column_int(statement, Word_Count);
        mcd.matchWord = word;
        pCallbackData->MatchCallBack(mcd);
        iUpdate = 1;
    }
    return pCallbackData->StatusCheck(iUpdate);
}

const FileIDVsPath::FileList& ContentSearchManager::GetFilePathFromFileID(const std::string &fileID, FileIDVsPath *cachedFileIDVsPath /* = nullptr */, FileIDVsPath::FileList *outList /* = nullptr */)
{
    if (outList == nullptr && cachedFileIDVsPath == nullptr)
        return FileIDVsPath().GetFilesFromFileId(fileID); // return empty
    if (cachedFileIDVsPath) { // check cache
        const FileIDVsPath::FileList &fileList = cachedFileIDVsPath->GetFilesFromFileId(fileID);
        if (!fileList.empty())
            return fileList;
    }
    Database::ListString filePath;
    const char *columns[] = { "Path", nullptr };
    GetDatabase().GetTableTexts("File", filePath, columns, 0, -1, "WHERE FileID='%s'", fileID.c_str());
    if (filePath.size() > 0) {
        for (auto &path : filePath) {
            if (cachedFileIDVsPath)
                cachedFileIDVsPath->AddFileForFileID(fileID, path);
            else
                outList->insert(path);
        }
    }
    return cachedFileIDVsPath ? cachedFileIDVsPath->GetFilesFromFileId(fileID) : *outList;
}

bool ContentSearchManager::HasContent()
{
    return GetDatabase().TableHasEntry("Word");
}

std::string ContentSearchManager::GetFileIDFromFilePath(const std::string &inFilePath)
{
    Database::ListString filePath;
    const char *columns[] = { "FileID", nullptr };
    GetDatabase().GetTableTexts("File", filePath, columns, 0, 1, "WHERE Path = '%s'", inFilePath.c_str());
    if (filePath.size() > 0)
        return filePath.front();
    return "";
}

int ContentSearchManager::GetMatchingFiles(ContentMatchCallBack &matchCallback, LPCTSTR inOptWordToMatch /* = nullptr */, LPCTSTR inOptFile /* = nullptr */)
{
    SetSearchStarted(true);
    ItrTableRowsCallbackData_ContentSearchManager iterFile(this, &ContentSearchManager::ItrWordTableRowsCallbackFn_SearchWord, &matchCallback);
    std::string cond;
    if (inOptWordToMatch && *inOptWordToMatch)
        cond = "WHERE Word LIKE '%" + UNICODE_TO_UTF8(inOptWordToMatch) + "%";
    if (inOptFile) {
        std::string fileID(GetFileIDFromFilePath(UNICODE_TO_UTF8(inOptFile)));
        if (!fileID.empty()) {
            cond = cond.empty() ? "WHERE " : cond + " AND ";
            cond += "FileID='" + fileID + "'";
        }
    }
    int retVal(iterFile.IterateTableRows(GetDatabase(), "Word", cond.c_str()));
    SetSearchStarted(false);
    return retVal;
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


struct ItrFileTableCallbackData
{
    const bool& bIsTerminated;
    unsigned long long ullStartIndex, ullEndIndex;
    unsigned long long GetCurrenCount() const { return ullEndIndex - ullStartIndex; }
};
#define IsSearchContinue() (IsSearchStarted() && !bIsTerminated)
int ContentSearchManager::ManagerThreadProc(LPVOID /*pInThreadData*/)
{
    CountTimer ct(2000 * 60); // 2 min
    mSearchStartTime = SystemUtils::StringToLongLong(GetDatabase().GetProperty(_T("SearchStartTime")));
    if (!mSearchStartTime.GetTime())
        mSearchStartTime = CTime::GetCurrentTime();
    const bool& bIsTerminated(ThreadManager::GetInstance().GetIsTerminatedFlag());
    ItrFileTableCallbackData cbData = { bIsTerminated, 0, 0 };

    SystemUtils::LogMessage(_T("Content Search Manager: start"));
    unsigned finishCount(0);
    while (IsSearchContinue())
    {
        Sleep(1000);
        // Iterate Files table
        if (ct.UpdateTimeDuration())
        {
            if (IsSearchHasJob())
                finishCount = 0;
            cbData.ullStartIndex = 0;
            ItrTableRowsCallbackData_ContentSearchManager iterFile(this, &ContentSearchManager::ItrFileTableRowsCallbackFn_SearchContent, &cbData);
            int retVal(iterFile.IterateTableRows(GetDatabase(), "File"));
            if (501 == retVal) // call did not finish completely
                SystemUtils::LogMessage(_T("Content Search Manager: More jobs. Waiting for workers to finish"));
            else if (IsSearchHasJob())
                SystemUtils::LogMessage(_T("Content Search Manager: More jobs to do."));
            else {
                cbData.ullEndIndex = 0;
                ++finishCount;
                SystemUtils::LogMessage(_T("Content Search Manager: finished searching"));
            }
        }
        else if (!IsSearchHasJob() && IsSearchFinished() && finishCount > 1)
            break;
    }
    // Wait for worker thread
    SystemUtils::LogMessage(_T("Content Search Manager: end: Waiting for workers"));
    while (ThreadManager::GetInstance().GetThreadCount(ContentSearchThread + CM_THREAD_CLASS) > 0) {
        Sleep(1000);
        if (!IsSearchContinue()) // if canceled, terminate worker threads
            ThreadManager::GetInstance().TerminateThreads(ContentSearchThread + CM_THREAD_CLASS, 1);
    }
    if (IsSearchContinue()) // not canceled
        GetDatabase().RemoveProperty(_T("SearchStartTime"));
    else
        GetDatabase().SetProperty(_T("SearchStartTime"), SystemUtils::LongLongToString(mSearchStartTime.GetTime()));
    GetDatabase().Commit();
    m_dwContentSearchThreadIDManager = 0;
    SystemUtils::LogMessage(_T("Content Search Manager: finished. Time taken %s"), ct.GetString());
    SetSearchStarted(false);
    return 0;
}

class ContentMD5Callback : public MD5Callback
{
public:
    ContentMD5Callback(ContentSearchManager *pCSM, const bool &bInIsTerminated)
        : m_pCSM(pCSM), bIsTerminated(bInIsTerminated) {}

    virtual int Status() override
    {
        return IsSearchContinue() ? 0 : 1;
    }

private:
    bool IsSearchStarted() const { return m_pCSM->IsSearchStarted(); }
    ContentSearchManager *m_pCSM;
    const bool& bIsTerminated;
};


int ContentSearchManager::ContentSearchThreadProc(LPVOID pInThreadData)
{
    FileTableEntry *fte = (FileTableEntry *)pInThreadData;
    Path filePath(fte->path);
    if (!filePath.Exists()) { // not valid - remove or update missed count entry
        if (filePath.IsUNC()) {
            if (fte->IncrementMissedCount() > 3)
                RemoveFileEntry(*fte);
            else
                AddFileEntry(*fte); // update count
        }
        else
            RemoveFileEntry(*fte);
    }
    else if (filePath.IsDir() || filePath.GetSize() == 0 || !CTextReader(fte->path).IsValidTextFile()) { // not valid - remove its entry
        RemoveFileEntry(*fte);
    }
    else {
        const bool& bIsTerminated(ThreadManager::GetInstance().GetIsTerminatedFlag());
        bool bCompute(false);
        FileTableEntry oldEntry(*fte);
        if (fte->IsFileIDEmpty() || fte->UpdateFileModTime())
        { // compute MD5 first
            ContentMD5Callback callback(this, bIsTerminated);
            cMD5 md5(&callback);
            std::string fileID =  md5.CalcMD5FromFile(fte->path);
            if (!fileID.empty())
                bCompute = fte->SetFileID(fileID);
        }
        if (bCompute) {
            SystemUtils::LogMessage(_T("Indexing file: %s"), fte->path);
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
                CString fileID(UTF8_TO_UNICODE(fte->GetFileID()).c_str());
                for (const auto &words : wordList) {
                    CString query, word(words.first.c_str());
                    FindDataBase::MakeSQLString(word);
                    int count(words.second);
                    query.Format(_T("INSERT OR REPLACE INTO Word VALUES ('%s', '%s', '%d')"), word, fileID, count);
                    queryList.Add(query);
                    if (!IsSearchContinue())
                        break;
                }
                if (IsSearchContinue()) {
                    CDBCommiter *pDBComitter(GetDBCommitter());
                    pDBComitter->AddDBQueryStrings(queryList);
                    fte->lastUpdatedTime = CTime::GetCurrentTime();
                }
            }
        }
        if (IsSearchContinue()) {
            if (bCompute || fte->IsUpdated()) {
                fte->UpdateFileModTime();
                if (!oldEntry.IsFileIDEmpty()
                    && oldEntry.GetFileID() != fte->GetFileID()) // remove old entry if file is updated
                    RemoveFileEntry(oldEntry);
                AddFileEntry(*fte); // Add updated file entry
            }
        }
    }

    delete fte;
    return 0;
}
void FileTableEntry::UpdateFromSQliteEntry(sqlite3_stmt * statement)
{
    path = SystemUtils::UTF8ToUnicodeCString((const char *)sqlite3_column_text(statement, File_Path));
    fileID = (const char *)sqlite3_column_text(statement, File_FileID);
    fileModTime = SystemUtils::IntToTime(sqlite3_column_int64(statement, File_FileModifiedTime));
    lastUpdatedTime = SystemUtils::IntToTime(sqlite3_column_int64(statement, File_LastSearched));
    flags = sqlite3_column_int(statement, File_Flags);
}

FileTableEntry::FileTableEntry(LPCTSTR inpath /* = NULL */)
    : flags(0), bUpdated(false)
{
    if (inpath)
        path = inpath;
}

bool FileTableEntry::SetFileID(const std::string &infileID)
{
    bool bFileIDUpdated(false);
    if (infileID != fileID) {
        bUpdated = true;
        fileID = infileID;
        bFileIDUpdated = true;
    }
    return bFileIDUpdated;
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
    bUpdated = true;
    return GetMissedCount();
}

bool FileTableEntry::UpdateFileModTime()
{
    bool bModTimeUpdated(false);
    // Get file time
    FILETIME modTime = { 0 };
    Path(path).GetFileTime(NULL, NULL, &modTime);
    CTime cModTime(modTime);
    if (cModTime != fileModTime) {
        fileModTime = cModTime;
        bModTimeUpdated = true;
        bUpdated = true;
    }
    return bModTimeUpdated;
}

int ContentSearchManager::ItrFileTableRowsCallbackFn_SearchContent(sqlite3_stmt *statement, void * pUserData)
{
    ItrFileTableCallbackData *cbData((ItrFileTableCallbackData*)pUserData);
    const bool& bIsTerminated(cbData->bIsTerminated);
    bool bSearchContent(IsSearchForce());
    cbData->ullStartIndex++;
    if (cbData->ullStartIndex < cbData->ullEndIndex)
        return IsSearchContinue() ? 0 : 1;
    SetSearchHasJob(false);
    {
        CTime lastUpdatedTime = SystemUtils::IntToTime(sqlite3_column_int64(statement, File_LastSearched));
        if (mSearchStartTime < lastUpdatedTime) // Already updated
            return IsSearchContinue() ? 0 : 1;
    }
    if (!bSearchContent) {
        CString line(
            SystemUtils::UTF8ToUnicodeCString((const char *)sqlite3_column_text(statement, File_FileID)));
        if (line.IsEmpty() || line == _T("-"))
            bSearchContent = true;
    }
    if (!bSearchContent) {
        CTime time = SystemUtils::IntToTime(sqlite3_column_int64(statement, File_LastSearched));
        CTimeSpan timeDiff = CTime::GetCurrentTime() - time;
        bSearchContent = timeDiff.GetDays() >= 1;
    }
    if (bSearchContent) {
        FileTableEntry *fte = new FileTableEntry;
        fte->UpdateFromSQliteEntry(statement);
        if (!StartLimitedThreadOperation(ContentSearchThread, (LPVOID)fte, 3)) {
            cbData->ullEndIndex = cbData->ullStartIndex;
            SetSearchHasJob();
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
