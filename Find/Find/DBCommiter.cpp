#include "stdafx.h"
#include "DBCommiter.h"
#include "AutoLock.h"
#include "Path.h"
#include "ThreadManager.h"
#include "ServerStatusDlg.h"

CDBCommiter::CDBCommiter(FindDataBase *pDB)
    : m_pDB(pDB), lastVacuumTime(1000 * 60 * 60), // 1 hr
    mbDeleteFound(false), mbFinished(false)
{
    mDBName = Path(m_pDB->GetDBPath()).FileName().RenameExtension();
}


CDBCommiter::~CDBCommiter()
{
}

void CDBCommiter::AddDBQueryString(LPCTSTR inStr, ...)
{
    va_list args;
    // retrieve the variable arguments
    va_start(args, inStr);
    CString str;
    str.FormatV(inStr, args);
    va_end(args);
    AddDBQueryString(str);
}
void CDBCommiter::AddDBQueryString(const CString & queryString)
{
    CAutoLock autoLock(mLockerDBQueryString);
    mDBQueryString.Add(queryString);
}
void CDBCommiter::AddDBQueryStrings(const CArrayCString &queryStrings)
{
    if (queryStrings.GetCount() <= 0)
        return;
    CAutoLock autoLock(mLockerDBQueryString);
    for (int i = 0; i < queryStrings.GetCount(); ++i)
        mDBQueryString.Add(queryStrings.GetAt(i));
}

static bool StrRemoveLastCharFromString(CString &inOutStr, TCHAR lastChar)
{
    bool bHas(false);
    if (inOutStr.GetLength() > 0 && inOutStr[inOutStr.GetLength() - 1] == lastChar) {
        inOutStr = inOutStr.Left(inOutStr.GetLength() - 1);
        bHas = true;
    }
    return bHas;
}

static Logger& GetLogger()
{
    return GetServerLogger();
}

void CDBCommiter::DoCommit()
{
    CArrayCString queryStrings;
    INT_PTR queryCount(0);
    // save and empty current query
    {
        CAutoLock autoLock(mLockerDBQueryString);
        queryCount = mDBQueryString.GetCount();
        if (queryCount > 0) {
            queryStrings.Append(mDBQueryString);
            mDBQueryString.RemoveAll();
        }
    }
    if (queryCount > 0) {
        // Commit to db
        ThreadManager::GetInstance().SetThreadStatusStr(_T("DBCommit: commit %sdb %d queries: Start"), (LPCTSTR)mDBName, queryCount);
        for (INT_PTR i = 0; i < queryCount; ++i) {
            const CString &queryString(queryStrings[i]);
            if (queryString.Find(_T("DELETE FROM")) == 0)
                mbDeleteFound = true;
            CArrayCString arrQueryStr;
            LPCTSTR pStrQuery(queryString);
            if (queryString[0] == '|') {
                SystemUtils::SplitString(queryString, arrQueryStr, _T("|"));
                if (StrRemoveLastCharFromString(arrQueryStr[2], '.')) { // save
                    m_pDB->GetTableColTexts(SystemUtils::UnicodeToUTF8(arrQueryStr[2]).c_str(),
                        SystemUtils::UnicodeToUTF8(arrQueryStr[3]).c_str(), mSavedRowValue);
                }
                else if (StrRemoveLastCharFromString(arrQueryStr[2], '?')) { // replace
                    for (INT_PTR j = 0; i < mSavedRowValue.GetCount(); ++j) {
                        CString repStr;
                        repStr.Format(_T("[%d]"), j);
                        arrQueryStr[1].Replace(repStr, mSavedRowValue[j]);
                        arrQueryStr[3].Replace(repStr, mSavedRowValue[j]);
                        arrQueryStr[4].Replace(repStr, mSavedRowValue[j]);
                    }
                    mSavedRowValue.RemoveAll();
                }
                if (m_pDB->GetTableRowCount(arrQueryStr[2], arrQueryStr[3]) == 0) // No rows
                    pStrQuery = arrQueryStr[4]; // execute query 4, else query 1 will be executed
                else
                    pStrQuery = arrQueryStr[1];
                if (*pStrQuery == 0) // empty
                    continue;
            }
            int retVal = m_pDB->QueryNonRows2(SystemUtils::UnicodeToUTF8(pStrQuery).c_str());
            if (retVal != SQLITE_OK) {
                CString errMesg(SystemUtils::UTF8ToUnicodeCString(m_pDB->GetErrorMessage()));
                GetLogger().Log(Logger::kLogLevelError, _T("DBCommit: ErrorCode:%d-%s    %s"), retVal, (LPCTSTR)errMesg, pStrQuery);
            }
        }
        m_pDB->Commit();
        if (mbDeleteFound) {
            if (lastVacuumTime.UpdateTimeDuration(mbFinished)) { // 1 hrs have been passed
                ThreadManager::GetInstance().SetThreadStatusStr(_T("DBCommit: performing vacuum: Start"));
                m_pDB->Vacuum();
                mbDeleteFound = false;
                ThreadManager::GetInstance().SetThreadStatusStr(_T("DBCommit: performing vacuum: End"));
            }
        }
        ThreadManager::GetInstance().SetThreadStatusStr(_T("DBCommit: commit %sdb %d queries: End"), (LPCTSTR)mDBName, queryCount);
    }
}


DBCommiterManager & DBCommiterManager::GetInstance()
{
    static DBCommiterManager sDBCommiterManager;
    return sDBCommiterManager;
}

CDBCommiter * DBCommiterManager::AddDbCommiter(FindDataBase * pDB)
{
    CAutoLock autoLock(mLockerDBCommitters);

    // check if Added already
    for (INT_PTR i = 0; i < mDBCommiters.GetCount(); ++i)
        if (mDBCommiters[i]->GetDBType() == pDB->GetType())
            return mDBCommiters[i];
    INT_PTR dbIndex(mDBCommiters.Add(new CDBCommiter(pDB)));
    // Start DB commiter thread
    if (!mbThreadStarted) {
        mbThreadStarted = true;
        ThreadManager::GetInstance().CreateThread(ThreadProcFn_Commiter, this, SERVER_THREAD_OP_START_DBCOMMITER, NULL, _T("DBCommiter"));
    }
    return mDBCommiters[dbIndex];
}

void DBCommiterManager::RemoveDBCommitter(FindDataBase *pDB)
{
    bool bRemoveAll(((FindDataBase*)-1) == pDB);
    {
        CAutoLock autoLock(mLockerDBCommitters);
        for (INT_PTR i = 0; i < mDBCommiters.GetCount(); ++i) {
            if (bRemoveAll || mDBCommiters[i]->GetDBType() == pDB->GetType()) {
                mDBCommiters[i]->SetFinish();
                if (!bRemoveAll)
                    break;
            }
        }
        INT_PTR i = 0;
        for (; i < mDBCommiters.GetCount(); ++i)
            if (!mDBCommiters[i]->IsFinished())
                break;
        if (i == mDBCommiters.GetCount())
            bRemoveAll = true;
    }
    if (bRemoveAll) {
        // Wait for finish
        while (ThreadManager::GetInstance().GetThreadCount(SERVER_THREAD_OP_START_DBCOMMITER) == 1) {
            mbThreadStarted = false; // make it close thread
            Sleep(1000);
        }
    }
}

DBCommiterManager::DBCommiterManager()
    : mbThreadStarted(false)
{
}

DBCommiterManager::~DBCommiterManager()
{
    RemoveDBCommitter((FindDataBase*)-1);
}

int DBCommiterManager::ThreadProcFn_Commiter(LPVOID pThreadData)
{
    return ((DBCommiterManager*)pThreadData)->DoCommitterThreadFn();
}

int DBCommiterManager::DoCommitterThreadFn()
{
    CountTimer lastCommitTime(1000 * 60); // 1 min
    SystemUtils::LogMessage(_T("DBCommit: Start"));
    while (mDBCommiters.GetCount() > 0)
    {
        Sleep(1000);
        bool bFinished(!mbThreadStarted);
        if (lastCommitTime.UpdateTimeDuration(bFinished))
        {
            CAutoLock autoLock(mLockerDBCommitters);
            for (INT_PTR i = 0; i < mDBCommiters.GetCount(); ++i)
                mDBCommiters[i]->DoCommit();
            // Check and remove finished ones
            for (INT_PTR i = 0; i < mDBCommiters.GetCount(); ++i) {
                if (bFinished || mDBCommiters[i]->IsFinished()) {
                    mDBCommiters[i]->DoCommit(); // Commit remaining
                    delete mDBCommiters[i];
                    mDBCommiters.RemoveAt(i);
                    if (i > 0) --i;
                }
            }
        }
    }
    mbThreadStarted = false;
    SystemUtils::LogMessage(_T("DBCommit: End"));
    return 0;
}
