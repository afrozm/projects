#include "stdafx.h"
#include "PrimeDatabase.h"
#include "Path.h"
#include "Shlobj.h"
#include "StringUtils.h"
#include "STLUtils.h"

#define PRIME_TABLE_NAME "Prime"

TableItertatorClass(PrimeDatabase)

PrimeDatabase::PrimeDatabase(bool bReadOnly /* = false */)
    : Database(bReadOnly), mCommitTimer(false, 5*1000UL), mTotalCount(0)
{
    Init();
}


PrimeDatabase::~PrimeDatabase()
{
    Commit(true);
}

bool PrimeDatabase::IsPrime(unsigned long long number, bool *bNotFound /* = nullptr */)
{
    if (number > GetPrimeNumber()) {
        if (bNotFound)
            *bNotFound = true;
        return false;
    }
    unsigned long long count(0);
    GetTableRowCount(PRIME_TABLE_NAME, count, "WHERE PrimeNumber = %llu", number);
    return count == 1;
}

struct ComputeRootIndexData {
    unsigned long long pn;
    unsigned long long rootPN;
    unsigned long long rootIndex;
};
unsigned long long PrimeDatabase::GetPrimeNumber(unsigned long long * inOutIndex, unsigned long long * outRoot)
{
    unsigned long long pn =  0 ;
    ItrTableRowsCallbackData_PrimeDatabase itcb(this, &PrimeDatabase::IteratePrimeTable_Callback, &pn);
    unsigned long long idx(inOutIndex && *inOutIndex != -1 ? *inOutIndex : TotalCount()-1);
    std::string cond(StringUtils::UnicodeToUTF8(StringUtils::Format(_T("LIMIT 1 OFFSET %llu"), idx).c_str()));
    itcb.IterateTableRows(*this, PRIME_TABLE_NAME, cond.c_str());
    if (inOutIndex)
        *inOutIndex = idx;
    if (outRoot) {
        ComputeRootIndexData cData = { pn, 0, 0 };
        itcb.ItrTableRowsCallbackFn = &PrimeDatabase::IteratePrimeTable_Callback_ComputeRootIndex;
        itcb.mpUserData = &cData;
        itcb.IterateTableRows(*this, PRIME_TABLE_NAME);
        *outRoot = cData.rootIndex;
    }
    return pn;
}

unsigned long long PrimeDatabase::AddPrimeNumber(unsigned long long nextHighestPrimeNumber)
{
#ifdef _DEBUG
    assert(nextHighestPrimeNumber > GetPrimeNumber());
#endif // _DEBUG

    QueryNonRows("INSERT OR REPLACE INTO Prime VALUES(%llu)", nextHighestPrimeNumber);
    Commit();
    return mTotalCount++;
}

unsigned long long PrimeDatabase::TotalCount()
{
    if (mTotalCount == 0) {
        mTotalCount = StringUtils::getLLfromStr(GetProperty(_T("TotalCount")).c_str());
        if (mTotalCount == 0) {
            GetTableRowCount(PRIME_TABLE_NAME, mTotalCount);
            UpdateTotalCountProprty();
        }
    }
    return mTotalCount;
}

static int ItrTableRowsCallback_GetTableColTexts(sqlite3_stmt *statement, void *pUserData)
{
    std::vector<lstring> *pOutColTexts = (std::vector<lstring>*)pUserData;
    int numColumns = sqlite3_column_count(statement);
    for (int col = 0; col < numColumns; ++col)
    {
        const char *p = reinterpret_cast<const char *>(sqlite3_column_text(statement, col));
        if (p == NULL) {
            p = "";
        }
        pOutColTexts->push_back(StringUtils::UTF8ToUnicode(p));
    }
    return 0;
}

int PrimeDatabase::GetTableColTexts(const char * tableName, std::vector<lstring>& outColTexts, const char * conditions, ...)
{
    if (conditions == NULL)
        conditions = "";
    va_list args;
    va_start(args, conditions);
    char *q = sqlite3_vmprintf(conditions, args);
    int retVal = IterateTableRows(tableName, ItrTableRowsCallback_GetTableColTexts,
        q, &outColTexts);
    sqlite3_free(q);
    va_end(args);
    return retVal;
}
lstring PrimeDatabase::GetProperty(const lstring &propName)
{
    lstring retVal;
    std::vector<lstring> arrString;
    lstring condition;
    GetTableColTexts("Property", arrString, "WHERE Name = '%s'", StringUtils::UnicodeToUTF8(propName.c_str()).c_str());
    if (arrString.size() > 1)
        retVal = arrString[1];
    return retVal;
}
bool PrimeDatabase::RemoveProperty(const lstring &propName)
{
    return QueryNonRows("DELETE FROM Property WHERE Name = '%s'", StringUtils::UnicodeToUTF8(propName.c_str()).c_str()) == 0;
}
bool PrimeDatabase::SetProperty(const lstring &propName, const lstring &propValue)
{
    lstring propVals(propValue);
    StringUtils::Replace<wchar_t>(propVals, _T("'"), _T("''"));
    propVals = _T("'") + propName + _T("','") + propVals + _T("'");
    return QueryNonRows("INSERT OR REPLACE INTO Property VALUES(%s)",
        StringUtils::UnicodeToUTF8(propVals.c_str()).c_str()) == 0;
}

int PrimeDatabase::Commit(bool bForce /* = false */)
{
    if (mCommitTimer.UpdateTimeDuration() || bForce) {
        UpdateTotalCountProprty();
        __super::Commit();
    }
    return 0;
}

void PrimeDatabase::UpdateTotalCountProprty()
{
    SetProperty(_T("TotalCount"), STLUtils::ChangeType<unsigned long long, lstring>(mTotalCount));
}

void PrimeDatabase::Init()
{
    if (IsOpen())
        return;
    Path dbPath(Path::GetSpecialFolderPath(CSIDL_APPDATA, true).Append(_T("Prime")));
    dbPath.CreateDir();
    dbPath = dbPath.Append(_T("prime.db"));
    if (dbPath.Exists())
        Open(StringUtils::UnicodeToUTF8(dbPath.c_str()).c_str());
    if (IsOpen())
        return;

    const char *kPrimeDatabaseSchema[] = {
        "CREATE TABLE `Prime` (\
            `PrimeNumber`	INTEGER NOT NULL UNIQUE,\
            PRIMARY KEY(PrimeNumber) );",
        "CREATE TABLE Property ( Name TEXT NOT NULL,Value TEXT NOT NULL ,PRIMARY KEY (Name) );",
    };

    if (0 == Open(StringUtils::UnicodeToUTF8(dbPath.c_str()).c_str())) {
        for (int i = 0; i < _countof(kPrimeDatabaseSchema); ++i)
            QueryNonRows(kPrimeDatabaseSchema[i]);
        long long initialPrime[] = { 1, 2, 3, 5, 7 };
        for (int i = 0; i < _countof(initialPrime); i++)
            AddPrimeNumber(initialPrime[i]);
        Commit(true);
    }
}

int PrimeDatabase::IteratePrimeTable_Callback(sqlite3_stmt * statement, void * pUserData)
{
    unsigned long long *pULLNumber((unsigned long long *)pUserData);
    *pULLNumber = sqlite3_column_int64(statement, 0);
    return 1;
}

int PrimeDatabase::IteratePrimeTable_Callback_ComputeRootIndex(sqlite3_stmt *statement, void *pUserData)
{
    ComputeRootIndexData *pData((ComputeRootIndexData *)pUserData);
    unsigned long long curPN = sqlite3_column_int64(statement, 0);
    if (curPN * curPN >= pData->pn) {
        pData->rootPN = curPN;
        return 1; // stop
    }
    pData->rootIndex++;
    return 0;
}
