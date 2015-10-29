#pragma once
#include "SystemUtils.h"
#include "Finder.h"
#include "FindDataBase.h"

class CFileMetaDataProvider {
public:
	virtual LONGLONG GetFileSize() const = 0;
	virtual BOOL GetLastWriteTime(CTime &time) const = 0;
	virtual BOOL GetCreationTime(CTime &time) const = 0;
	virtual BOOL GetLastAccessTime(CTime &time) const = 0;
	virtual BOOL GetLastUpdateTime(CTime& /*time*/) const {return FALSE;}
};

class CFileFindExMetaDataProvider : public CFileMetaDataProvider {
	const CFileFindEx &mFileFindEx;
public:
	CFileFindExMetaDataProvider(const CFileFindEx &fileFindEx);
	LONGLONG GetFileSize() const;
	BOOL GetLastWriteTime(CTime &time) const;
	BOOL GetCreationTime(CTime &time) const;
	BOOL GetLastAccessTime(CTime &time) const;
};

class CCacheDataFileMetaDataProvider : public CFileMetaDataProvider {
	sqlite3_stmt *statement;
	int mSizeCol;
	int mCreatedTimeCol;
	int mModifiedTimeCol;
	int mAccessedTimeCol;
	int mUpdatedTimeCol;
public:
	CCacheDataFileMetaDataProvider(sqlite3_stmt *instatement, int sizeCol, int createdTimeCol, int modifiedTimeCol, int accessedTimeCol, int updatedTimeCol);
	LONGLONG GetFileSize() const;
	BOOL GetLastWriteTime(CTime &time) const;
	BOOL GetCreationTime(CTime &time) const;
	BOOL GetLastAccessTime(CTime &time) const;
	BOOL GetLastUpdateTime(CTime &time) const;
};
