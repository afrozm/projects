#pragma once

#include "Database.h"

// Find databases types
typedef enum {
	FDB_PrefDatabase,
	FDB_CacheDatabase
} FDB_Database;


enum CachedDataTableIndex {
	CachedData_Path,
	CachedData_Size,
	CachedData_LastUpdated,
	CachedData_MissCount,
	CachedData_CatagoryNumber,
	CachedData_Root,
	CachedData_CreatedTime,
	CachedData_ModifiedTime,
	CachedData_AccessedTime,
	CachedData_AddedTime
};

enum RecentSearchTableIndex {
	RecentSearch_Path,
	RecentSearch_Size,
	RecentSearch_CreatedTime,
	RecentSearch_ModifiedTime,
	RecentSearch_AccessedTime,
};

enum SearchHistoryTableIndex {
	SearchHistory_SearchKeys,
	SearchHistory_LastUpdated,
	SearchHistory_MissCount,
	SearchHistory_LastSearchPath,
	SearchHistory_flags
};
enum PreferencesTableIndex {
	Preferences_PreferenceName,
	Preferences_PreferenceKey,
	Preferences_SizeMin,
	Preferences_SizeMax,
	Preferences_SizeCondition,
	Preferences_DateFrom,
	Preferences_DateTo,
	Preferences_DateCondition,
	Preferences_DateType,
};
enum PrefSearchLocationsTableIndex {
	PrefSearchLocations_PreferenceName,
	PrefSearchLocations_SearchLocation
};
enum CatagoryTableIndex {
	Catagory_CatagoryNumber,
	Catagory_Name,
	Catagory_SearchKeys,
	Catagory_ExceptKeys,
	Catagory_SizeMin,
	Catagory_SizeMax,
	Catagory_SizeCondition,
	Catagory_Flags,
};

class FindDataBase : public Database 
{
public:
	int Open();
	static CString GetPreferencesFolderPahth();
	CString GetDBPath();
	void LoadSchema();
	FindDataBase(FDB_Database dataBaseType = FDB_PrefDatabase, bool bReadOnly = false);
	int GetTableColTexts(const char *tableName, const char *conditions, CArrayCString &outColTexts);
	unsigned long long GetTableRowCount(const CString &tableName, LPCTSTR condition = NULL);
	~FindDataBase(void);
	CString GetProperty(const CString &propName);
	bool RemoveProperty(const CString &propName);
	bool SetProperty(const CString &propName, const CString &propValue);
	static CString SGetProperty(const CString &propName, FDB_Database dataBaseType = FDB_PrefDatabase);
	static bool SSetProperty(const CString &propName, const CString &propValue, FDB_Database dataBaseType = FDB_PrefDatabase);
	static bool SRemoveProperty(const CString &propName, FDB_Database dataBaseType = FDB_PrefDatabase);
	static void MakeSQLString(CString &inoutstring);
	static bool SetCacheDBPath(LPCTSTR path = NULL);
	static const CString& GetCacheDBPath();
protected:
	FDB_Database mFDB_DatabaseType;
};
