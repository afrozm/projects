#pragma once

#include "sqlite3.h"
#include <list>
#include <string>

typedef int (*ItrTableRowsCallback)(sqlite3_stmt *statement, void *pUserData);

struct SelectData {
	SelectData() : bIsDistinct(false), columns(NULL) {}
	bool bIsDistinct;
	const char **columns;
};

class Database
{
public:
    typedef std::list<std::string> ListString;
	Database(bool bReadOnly = false);
	~Database(void);
	int Open(const char *dbFile);
	bool IsOpen();
	int Close();
	int IterateTableRows(const char *tableName, ItrTableRowsCallback itcbFn, const char *conditions, void *pUserData, const SelectData *pData = NULL) const;
    int IterateTableRowsEx(const char *tableName, ItrTableRowsCallback itcbFn, void *pUserData, const SelectData *pData = NULL, const char *conditions = NULL, ...) const;
    int GetTableTexts(const char *tableName, ListString &outColTexts, const char **coulumNames = nullptr, int startIndex = 0, int endIndex = -1, const char *conditions = NULL, ...) const;
	int GetTableRowCount(const char *tableName, unsigned long long &outCount, const char *conditions = NULL, ...) const;
    bool TableHasEntry(const char *tableName, const char *conditions = NULL, ...) const;
	int GetTableNames(ListString & outTableNamesVec);
	int QueryNonRows(const char *inQuery, ...) const;
	int QueryNonRows2(const char *inQuery) const;
	int Commit();
	int Rollback();
	int Vacuum();
	const char* GetErrorMessage() const;
    bool IsReadOnly() const { return mbReadOnly; }
    // Effective only when db is closed. For already open db no effect.
    bool SetReadOnly(bool bReadOnly = true) { mbReadOnly = bReadOnly; }
private:
	int PrepareStmt(sqlite3_stmt **outStatement, const char *inQuery) const;
	int PrepareStmt(sqlite3_stmt **outStatement, const char *inQuery, va_list args) const;
	int PrepareStmtEx(sqlite3_stmt **outStatement, const char *inQuery, ...) const;
	int QueryNonRowsLoop(sqlite3_stmt *statement) const;
    sqlite3 *db;
	bool mbReadOnly;
};

#define TableItertatorClass(ClassName) \
static int ItrTableRowsCBFn_##ClassName(sqlite3_stmt *statement, void *pUserData);\
struct ItrTableRowsCallbackData_##ClassName {\
	ClassName *classPointer;		\
	int (ClassName::*ItrTableRowsCallbackFn)(sqlite3_stmt *statement, void *pUserData);\
	void *mpUserData;\
	ItrTableRowsCallbackData_##ClassName(ClassName *inCPt,\
		int (ClassName::*mCBFn)(sqlite3_stmt *statement, void *pUserData),\
		void *pUserData = NULL)\
		: classPointer(inCPt), ItrTableRowsCallbackFn(mCBFn), mpUserData(pUserData)\
	{\
	}\
	void SetCallbackFn(int (ClassName::*mCBFn)(sqlite3_stmt *statement, void *pUserData))\
	{\
		ItrTableRowsCallbackFn = mCBFn;\
	}\
	int IterateTableRows(Database &inDb, const char *tableName, const char *condition = "", const SelectData *pSelectData = NULL)\
	{\
		return inDb.IterateTableRows(tableName, ItrTableRowsCBFn_##ClassName, condition, this, pSelectData);\
	}\
};\
static int ItrTableRowsCBFn_##ClassName(sqlite3_stmt *statement, void *pUserData)\
{\
	ItrTableRowsCallbackData_##ClassName *pData = (ItrTableRowsCallbackData_##ClassName*)pUserData;\
	return (pData->classPointer->*(pData->ItrTableRowsCallbackFn))(statement, pData->mpUserData);\
}
