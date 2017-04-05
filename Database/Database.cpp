#include <stdio.h>
#include "Database.h"
#include <string>
/* Busy database parameters */
static const int kBusyTimeoutMs = 2000;

Database::Database(bool bReadOnly)
: db(NULL), mbReadOnly(bReadOnly)
{
}

Database::~Database(void)
{
	Close();
}

int Database::Close()
{
	int retVal = 0;
	if (db) {
		retVal = sqlite3_close(db);
		db = NULL;
	}
	return retVal;
}

int Database::Open(const char *dbFile)
{
	int retVal = 0;
	if (IsReadOnly())
		sqlite3_open_v2(dbFile, &db, SQLITE_OPEN_READONLY, NULL);
	else
		sqlite3_open(dbFile, &db);
	if (retVal) {
	}
	retVal = sqlite3_busy_timeout(this->db, kBusyTimeoutMs);
	if (SQLITE_OK != retVal)
	{
	}
	return retVal;
}

bool Database::IsOpen()
{
	return db != NULL;
}

int Database::IterateTableRows(const char *tableName, ItrTableRowsCallback itcbFn, const char *conditions, void *pUserData, const SelectData *pData) const
{
    if (conditions == nullptr)
        conditions = "";
	if (db == NULL)
		return SQLITE_ERROR;
	sqlite3_stmt *statement = NULL;
	std::string selectCore;
	if (pData != NULL) {
		if (pData->bIsDistinct)
			selectCore = "DISTINCT ";
        if (pData->columns) {
            const char **columns = pData->columns;
            while (*columns) {
                if (columns != pData->columns)
                    selectCore += ",";
                selectCore += *columns;
                ++columns;
            }
        }
	}
    if (selectCore.empty())
	    selectCore = "*";
	int sqlrv = PrepareStmtEx(&statement, "SELECT %s FROM %s %s", selectCore.c_str(), tableName, conditions);
	if (SQLITE_OK != sqlrv) {
		return sqlrv;
	}
	bool done = false;
	while (!done) {
		sqlrv = sqlite3_step(statement);
		switch (sqlrv)
		{
			case SQLITE_ROW:
				{
					sqlrv = itcbFn(statement, pUserData);
					done = sqlrv != 0;
				}
				break;
			case SQLITE_DONE:
				sqlrv = 0; // No error
			default:
				done = true;
				break;
		}
	}
	sqlite3_finalize(statement);
	return sqlrv;
}
int Database::IterateTableRowsEx(const char *tableName, ItrTableRowsCallback itcbFn, void *pUserData, const SelectData *pData /* = NULL */, const char *conditions /* = NULL */, ...) const
{
    va_list args;
    if (conditions == NULL)
        conditions = "";
    va_start(args, conditions);
    char *q = sqlite3_vmprintf(conditions, args);
    int retVal = IterateTableRows(tableName, itcbFn, q, pUserData, pData);
    sqlite3_free(q);
    va_end(args);
    return retVal;
}

int 
Database::GetTableNames(ListString &outTableNamesVec)
{
    outTableNamesVec.clear();
	if (db == NULL)
		return SQLITE_ERROR;
	sqlite3_stmt *statement = NULL;
	int sqlrv = PrepareStmtEx(&statement, "SELECT name FROM sqlite_master WHERE type='table'");
	if (SQLITE_OK != sqlrv) {
		return sqlrv;
	}
	bool done = false;
	while (!done) {
		sqlrv = sqlite3_step(statement);
		switch (sqlrv)
		{
			case SQLITE_ROW:
				{
					const char *p = reinterpret_cast<const char *>(sqlite3_column_text(statement, 0));
					if (p == NULL) {
						p = "";
					}
                    if (*p)
					    outTableNamesVec.push_back(p);
				}
				break;
			case SQLITE_DONE:
				sqlrv = 0; // No error
			default:
				done = true;
				break;
		}
	}
	sqlite3_finalize(statement);
	return sqlrv;
}
struct TableRowCallbackData
{
    Database::ListString &outColText;
    int startIndex, endIndex, currentIndex;
};
static int ItrTableRowsCallback_GetTableColTexts(sqlite3_stmt *statement, void *pUserData)
{
    TableRowCallbackData *pData = (TableRowCallbackData *)pUserData;
	int numColumns = sqlite3_column_count(statement);
	for (int col = 0; col < numColumns; ++col)
	{
		const char *p = reinterpret_cast<const char *>(sqlite3_column_text(statement, col));
		if (p == NULL) {
			p = "";
		}
		pData->outColText.push_back(p);
	}
    pData->currentIndex++;
	return pData->endIndex < 0 || pData->currentIndex < pData->endIndex ? 0 : 1;
}
int Database::GetTableTexts(const char *tableName, ListString &outColTexts, const char **coulumNames /* = nullptr */, int startIndex /* = 0 */, int endIndex /* = -1 */, const char *conditions /* = NULL */, ...) const
{
    if (conditions == nullptr)
        conditions = "";
    SelectData sd;
    sd.columns = coulumNames;
    TableRowCallbackData td = { outColTexts, startIndex, endIndex };
    va_list args;
    va_start(args, conditions);
    char *q = sqlite3_vmprintf(conditions, args);
    IterateTableRows(tableName, ItrTableRowsCallback_GetTableColTexts,
        q, &td, &sd);
    sqlite3_free(q);
    va_end(args);

    return td.currentIndex;
}

static int ItrTableRowsCallback_GetTableRowCount(sqlite3_stmt *statement, void *pUserData)
{
    ++(*(unsigned long long *)pUserData);
    return 0;
}
static int ItrTableRowsCallback_GetTableRowCount2(sqlite3_stmt *statement, void *pUserData)
{
    (*(unsigned long long *)pUserData) = sqlite3_column_int64(statement, 0);
    return 0;
}

int Database::GetTableRowCount(const char *tableName, unsigned long long &outCount, const char *conditions /* = NULL */, ...) const
{
    va_list args;
    if (conditions == NULL)
        conditions = "";
    int retVal(0);
    if (*conditions) {
        va_start(args, conditions);
        char *q = sqlite3_vmprintf(conditions, args);
        retVal = IterateTableRows(tableName, ItrTableRowsCallback_GetTableRowCount,
            q, &outCount);
        sqlite3_free(q);
        va_end(args);
    }
    else {
        SelectData sd;
        char* col[] = { "COUNT(*)", NULL };
        sd.columns = (const char **)col;
        retVal = IterateTableRows(tableName, ItrTableRowsCallback_GetTableRowCount2,
            conditions, &outCount, &sd);
    }
    return retVal;
}

bool Database::TableHasEntry(const char *tableName, const char *conditions /*= NULL*/, ...) const
{
    va_list args;
    if (conditions == NULL)
        conditions = "";
    va_start(args, conditions);
    char *q = sqlite3_vmprintf(conditions, args);
    ListString texts;
    int count(GetTableTexts(tableName, texts, nullptr, 0, 1, "%s", q));
    sqlite3_free(q);
    va_end(args);
    return count > 0;
}

/*! Undo the changes made in the databse. */
int Database::Rollback()
{
	if (db == NULL)
		return SQLITE_ERROR;
	if (sqlite3_get_autocommit(db))
		return SQLITE_OK; // NOP in auto-commit mode

	return QueryNonRows("ROLLBACK");
}


/*! Commit the changes made in the databse. */
int Database::Commit()
{
	if (db == NULL)
		return SQLITE_ERROR;
	if (sqlite3_get_autocommit(db))
		return SQLITE_OK; // NO-OP in auto-commit mode

	return QueryNonRows("COMMIT");
}
int Database::Vacuum()
{
	if (db == NULL)
		return SQLITE_ERROR;

	return QueryNonRows("VACUUM");
}

int Database::QueryNonRows(const char *inQuery, ...) const
{
	int sqlrv = SQLITE_OK;
	sqlite3_stmt *statement = NULL;

	va_list args;
	va_start(args, inQuery);
	sqlrv = PrepareStmt(&statement, inQuery, args);
	va_end(args);
	if (SQLITE_OK != sqlrv) {
		return sqlrv;
	}
	sqlrv = QueryNonRowsLoop(statement);
	return sqlrv;
}
int Database::QueryNonRows2(const char *inQuery) const
{
	int sqlrv = SQLITE_OK;
	sqlite3_stmt *statement = NULL;

	sqlrv = PrepareStmt(&statement, inQuery);
	if (SQLITE_OK != sqlrv) {
		return sqlrv;
	}
	sqlrv = QueryNonRowsLoop(statement);
	return sqlrv;
}
int Database::QueryNonRowsLoop(sqlite3_stmt *statement) const
{
	int sqlrv = SQLITE_OK;
	// Execute...
	bool done = false;
	while (!done)
	{
		sqlrv = sqlite3_step(statement);
		switch (sqlrv)
		{
			case SQLITE_DONE:
				sqlrv = SQLITE_OK;
				done = true;
				break;
				
			case SQLITE_ROW:
				break;

			case SQLITE_ERROR:
				// Meaningful errors are reported at finalize, not here.
				break;

			default:
				done = true;
				break;
		}
	}

	// Cleanup...
	sqlrv = sqlite3_finalize(statement);
	if (SQLITE_OK != sqlrv)
	{
	}

	return sqlrv;
}
int Database::PrepareStmtEx(sqlite3_stmt **outStatement, const char *inQuery, ...) const
{
	va_list args;
	va_start(args, inQuery);
	int sqlrv = PrepareStmt(outStatement, inQuery, args);
	va_end(args);
	return sqlrv;
}

int Database::PrepareStmt(sqlite3_stmt **outStatement, const char *inQuery, va_list args) const
{
	int sqlrv = SQLITE_ERROR;
	if (db == NULL)
		return sqlrv;
	char *q = sqlite3_vmprintf(inQuery, args);

	sqlrv = PrepareStmt(outStatement, q);
	sqlite3_free(q);
	return sqlrv;
}
int Database::PrepareStmt(sqlite3_stmt **outStatement, const char *inQuery) const
{
	int sqlrv = SQLITE_ERROR;
	if (db == NULL)
		return sqlrv;
	const char *q = inQuery;

	// If we are in auto-commit mode and this is a mutating operation, start a transaction
	if (sqlite3_get_autocommit(db))
	{
		// Ain't this sophisticated!
		if ((strstr(q, "INSERT ") == q) || (strstr(q, "UPDATE ") == q) || (strstr(q, "DELETE ") == q)) 
		{
			sqlrv = QueryNonRows("BEGIN");
			if (SQLITE_OK != sqlrv)
			{
				return sqlrv;
			}
		}
	}

	const char *tail = NULL;
	sqlrv = sqlite3_prepare(db, q, static_cast<int>(strlen(q)), outStatement, &tail);
	if (SQLITE_OK != sqlrv)
	{
	}

	return sqlrv;
}

const char* Database::GetErrorMessage() const
{
	const char *errMesg("");
	if (db != NULL)
		errMesg = sqlite3_errmsg(db);
	if (errMesg == NULL)
		errMesg = "";
	return errMesg;
}

