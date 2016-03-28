#include "StdAfx.h"
#include "FindDataBase.h"
#include "Path.h"
#include "Database.h"
#include "SystemUtils.h"

const char *kFindDatabaseSchema[] = {
	"CREATE TABLE SearchHistory ( SearchKeys TEXT NOT NULL, PRIMARY KEY (SearchKeys) );",
	"CREATE TABLE Preferences ( PreferenceName TEXT NOT NULL,PreferenceKey TEXT,SizeMin TEXT,SizeMax TEXT,SizeCondition INT,DateFrom  TEXT,DateTo TEXT,DateCondition INT,DateType INT ,PRIMARY KEY (PreferenceName) );",
	"CREATE TABLE PrefSearchLocations ( PreferenceName TEXT NOT NULL, SearchLocation TEXT NOT NULL,PRIMARY KEY (PreferenceName,SearchLocation) );",
	"CREATE TABLE RecentSearch ( Path TEXT NOT NULL,Size TEXT NOT NULL, CreatedTime INT, ModifiedTime INT, AccessedTime INT ,PRIMARY KEY (Path) );",
	"CREATE TABLE Property ( Name TEXT NOT NULL,Value TEXT NOT NULL ,PRIMARY KEY (Name) );",
	"CREATE TRIGGER ctd_Preferences BEFORE DELETE ON Preferences FOR EACH ROW BEGIN DELETE FROM PrefSearchLocations WHERE PreferenceName = OLD.PreferenceName; END;"
};

const char *kCacheDatabaseSchema[] = {
	"CREATE TABLE Category ( CatagoryNumber INT NOT NULL,Name TEXT NOT NULL, SearchKeys TEXT, ExceptKeys TEXT, SizeMin TEXT,SizeMax TEXT, SizeCondition TEXT,Flags INT,PRIMARY KEY (CatagoryNumber) );",
	"CREATE TABLE CachedData (Path TEXT NOT NULL,Size TEXT NOT NULL , LastUpdated INT NOT NULL, MissCount INT, CatagoryNumber INT NOT NULL, Root TEXT NOT NULL COLLATE NOCASE, CreatedTime INT, ModifiedTime INT, AccessedTime INT, AddeddTime INT, PRIMARY KEY (Path) );",
	"CREATE TABLE Property ( Name TEXT NOT NULL,Value TEXT NOT NULL ,PRIMARY KEY (Name) );",
	"CREATE TABLE SearchHistory ( SearchKeys TEXT NOT NULL COLLATE NOCASE, LastUpdated INT NOT NULL, MissCount INT, LastSearchPath TEXT, flags INT NOT NULL, PRIMARY KEY (SearchKeys) );",
	"CREATE TRIGGER ctd_SearchHistory BEFORE DELETE ON SearchHistory FOR EACH ROW BEGIN DELETE FROM CachedData WHERE Root = OLD.SearchKeys; END;"
};

FindDataBase::FindDataBase(FDB_Database dataBaseType, bool bReadOnly)
						   : mFDB_DatabaseType(dataBaseType), Database(bReadOnly)
{
}

FindDataBase::~FindDataBase(void)
{
}
static CString sCacheDBPath;
bool FindDataBase::SetCacheDBPath(LPCTSTR path)
{
	bool bSuccess(true);
	if (path == NULL || *path == 0)
		sCacheDBPath.Empty();
	else {
		CString oldCacheDBPath(sCacheDBPath);
		sCacheDBPath = path;
		if (SGetProperty(_T("ServerSearchStatus"), FDB_CacheDatabase).IsEmpty()) {
			sCacheDBPath = oldCacheDBPath;
			bSuccess = false;
		}
	}
	return bSuccess;
}
const CString& FindDataBase::GetCacheDBPath()
{
	return sCacheDBPath;
}
CString FindDataBase::GetDBPath()
{
	Path prefDatabasePath(GetPreferencesFolderPahth());
	switch (mFDB_DatabaseType) {
		case FDB_PrefDatabase:
			prefDatabasePath = prefDatabasePath.Append(CString(_T("FindPref.db")));
			break;
		case FDB_CacheDatabase:
			if (sCacheDBPath.IsEmpty()) {
				prefDatabasePath = prefDatabasePath.Parent();
				prefDatabasePath = prefDatabasePath.Append(CString(_T("FindCache.db")));
			}
			else
				prefDatabasePath = sCacheDBPath;
			break;
	}
	return prefDatabasePath;
}

int FindDataBase::Open()
{
	if (IsOpen())
		return 0;
	Path dbPath(GetDBPath());
	return Database::Open(SystemUtils::UnicodeToUTF8(dbPath).c_str());
}
void FindDataBase::LoadSchema()
{
	Path prefDatabasePath(GetDBPath());
	if (prefDatabasePath.Exists())
		return;
	if (Open() == 0) {
		const char **kDataBaseSchemas[] = {kFindDatabaseSchema, kCacheDatabaseSchema};
		const int kSize[] = {sizeof(kFindDatabaseSchema)/sizeof(const char *), 
			sizeof(kCacheDatabaseSchema)/sizeof(const char *)};
		for (int i = 0 ; i < kSize[mFDB_DatabaseType]; ++i) {
			QueryNonRows(kDataBaseSchemas[mFDB_DatabaseType][i]);
		}
		Commit();
	}
}

CString FindDataBase::GetPreferencesFolderPahth()
{
	std::wstring appPath;
	SystemUtils::GetSpecialFolderPath(CSIDL_APPDATA, true, appPath);
	CString prefFolderPath(appPath.c_str());
	prefFolderPath.Append(_T("\\Find\\Preferences"));
	::SHCreateDirectoryEx(NULL, prefFolderPath, NULL);
	return prefFolderPath;
}
static int ItrTableRowsCallback_GetTableColTexts(sqlite3_stmt *statement, void *pUserData)
{
	CArrayCString *pOutColTexts = (CArrayCString*)pUserData;
	int numColumns = sqlite3_column_count(statement);
	for (int col = 0; col < numColumns; ++col)
	{
		const char *p = reinterpret_cast<const char *>(sqlite3_column_text(statement, col));
		if (p == NULL) {
			p = "";
		}
		pOutColTexts->Add(SystemUtils::UTF8ToUnicodeCString(p));
	}
	return 0;
}
int FindDataBase::GetTableColTexts(const char *tableName, const char *conditions, CArrayCString &outColTexts)
{
	return IterateTableRows(tableName, ItrTableRowsCallback_GetTableColTexts,
		conditions, &outColTexts);
}
CString FindDataBase::GetProperty(const CString &propName)
{
	CString retVal;
	CArrayCString arrString;
	CString condition;
	condition.Format(_T(" WHERE Name='%s'"), propName);
	GetTableColTexts("Property", SystemUtils::UnicodeToUTF8(condition).c_str(), arrString);
	if (arrString.GetCount() > 0)
		retVal = arrString.GetAt(1);
	return retVal;
}
bool FindDataBase::RemoveProperty(const CString &propName)
{
	CString query;
	query.Format(_T("DELETE FROM Property WHERE Name = '%s'"), propName);
	return QueryNonRows2(SystemUtils::UnicodeToUTF8(query).c_str()) == 0;
}
bool FindDataBase::SetProperty(const CString &propName, const CString &propValue)
{
	CString propVals(propValue);
	FindDataBase::MakeSQLString(propVals);
	propVals = _T("'") + propName + _T("','") + propVals + _T("'");
	return QueryNonRows("INSERT OR REPLACE INTO Property VALUES(%s)",
		SystemUtils::UnicodeToUTF8(propVals).c_str()) == 0;
}
CString FindDataBase::SGetProperty(const CString &propName, FDB_Database dataBaseType)
{
	CString propVal;
	FindDataBase fdb(dataBaseType, true);
	if (fdb.Open() == 0)
		propVal = fdb.GetProperty(propName);
	return propVal;
}
bool FindDataBase::SSetProperty(const CString &propName, const CString &propValue, FDB_Database dataBaseType)
{
	bool bSuccess(false);
	FindDataBase fdb(dataBaseType);
	if (fdb.Open() == 0) {
		bSuccess = fdb.SetProperty(propName, propValue);
		if (bSuccess)
			fdb.Commit();
	}
	return bSuccess;
}
bool FindDataBase::SRemoveProperty(const CString &propName, FDB_Database dataBaseType)
{
	bool bSuccess(false);
	FindDataBase fdb(dataBaseType);
	if (fdb.Open() == 0) {
		bSuccess = fdb.RemoveProperty(propName);
		if (bSuccess)
			fdb.Commit();
	}
	return bSuccess;
}
void FindDataBase::MakeSQLString(CString &inoutstring)
{
	SystemUtils::FindAndReplace(inoutstring, _T("'"), _T("''"));
//	SystemUtils::FindAndReplace(inoutstring, _T("%"), _T("%%"));
}

unsigned long long FindDataBase::GetTableRowCount( const CString &tableName, LPCTSTR condition /*= NULL*/ )
{
	std::string cCondition(SystemUtils::UnicodeToUTF8(condition));
	unsigned long long rowCount(0);
	__super::GetTableRowCount(SystemUtils::UnicodeToUTF8(tableName).c_str(), rowCount, cCondition.c_str());
	return rowCount;
}
