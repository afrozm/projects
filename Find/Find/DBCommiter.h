#pragma once
#include "CountTimer.h"
#include "FindDataBase.h"

class CDBCommiter
{
public:
    CDBCommiter(FindDataBase *pDB);
    ~CDBCommiter();
    void AddDBQueryString(LPCTSTR inStr, ...);
    void AddDBQueryString(const CString &queryString);
    void AddDBQueryStrings(const CArrayCString &queryStrings);
    void DoCommit();
    FDB_Database GetDBType() const { return m_pDB->GetType(); }
    void SetFinish() { mbFinished = true; }
    bool IsFinished() const { return mbFinished; }
private:
    CountTimer lastVacuumTime; // 1 hr
    FindDataBase *m_pDB;
    CArrayCString mDBQueryString, mSavedRowValue;
    CMutex mLockerDBQueryString;
    CString mDBName;
    bool mbDeleteFound, mbFinished;
};

class DBCommiterManager
{
public:
    static DBCommiterManager& GetInstance();

    CDBCommiter* AddDbCommiter(FindDataBase *pDB);
    void RemoveDBCommitter(FindDataBase *pDB);

private:
    DBCommiterManager();
    ~DBCommiterManager();
    static int ThreadProcFn_Commiter(LPVOID pThreadData);
    int DoCommitterThreadFn();
    CArray<CDBCommiter*> mDBCommiters;
    CMutex mLockerDBCommitters;
    bool mbThreadStarted;
};
