#pragma once

#include "Database.h"
#include "Common.h"
#include "CountTimer.h"

class PrimeDatabase :
    public Database
{
public:
    PrimeDatabase(bool bReadOnly = false);
    ~PrimeDatabase();
    bool IsPrime(unsigned long long number, bool *bNotFound = nullptr);
    unsigned long long GetPrimeNumber(unsigned long long *inOutIndex = NULL, unsigned long long *outRoot = NULL);
    unsigned long long AddPrimeNumber(unsigned long long nextHighestPrimeNumber);
    unsigned long long TotalCount();

private:
    void Init();
    int IteratePrimeTable_Callback(sqlite3_stmt *statement, void *pUserData);
    int IteratePrimeTable_Callback_ComputeRootIndex(sqlite3_stmt *statement, void *pUserData);
    int GetTableColTexts(const char *tableName, std::vector<lstring> &outColTexts, const char *conditions = NULL, ...);
    lstring GetProperty(const lstring &propName);
    bool RemoveProperty(const lstring &propName);
    bool SetProperty(const lstring &propName, const lstring &propValue);
    int Commit(bool bForce = false);
    void UpdateTotalCountProprty();

    CountTimer mCommitTimer;
    unsigned long long mTotalCount;
};

