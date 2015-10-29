#pragma once

#include "sqlite3.h"

typedef int (*ItrTableRowsCallback)(sqlite3_stmt *statement, void *pUserData);

struct SelectData {
	SelectData() : bIsDistinct(false), columns(NULL) {}
	bool bIsDistinct;
	const char **columns;
};

class Database
{
public:
	Database(bool bReadOnly = false);
	~Database(void);
	int Open(const char *dbFile);
	bool IsOpen();
	int Close();
	int IterateTableRows(const char *tableName, ItrTableRowsCallback itcbFn, const char *conditions, void *pUserData, const SelectData *pData = NULL);
//	int GetTableColTexts(const char *tableName, const char *conditions, RIBS::VecRIBSStrings &outColTexts);
	int GetTableRowCount(const char *tableName, const char *conditions, int &outCount);
//	int GetTableNames(RIBS::VecRIBSStrings & outTableNamesVec);
	int QueryNonRows(const char *inQuery, ...);
	int QueryNonRows2(const char *inQuery);
	int Commit();
	int Rollback();
	int Vacuum();
	const char* GetErrorMessage() const;
private:
	int PrepareStmt(sqlite3_stmt **outStatement, const char *inQuery);
	int PrepareStmt(sqlite3_stmt **outStatement, const char *inQuery, va_list args);
	int PrepareStmtEx(sqlite3_stmt **outStatement, const char *inQuery, ...);
	int QueryNonRowsLoop(sqlite3_stmt *statement);
	sqlite3 *db;
	bool mbReadOnly;
};
