#include "StdAfx.h"
#include "FileMetaDataProvider.h"



CDiskFileMetaDataProvider::CDiskFileMetaDataProvider(LPCTSTR path)
    : mFilePath(path)
{

}

void CDiskFileMetaDataProvider::SetPath(LPCTSTR path /*= nullptr*/)
{
    mFilePath = path;
}

LONGLONG CDiskFileMetaDataProvider::GetFileSize() const
{
    if (!mFilePath.IsDir())
        return mFilePath.GetSize();
    return 0;
}

BOOL CDiskFileMetaDataProvider::GetLastWriteTime(CTime &time) const
{
    CFileTime ft;
    bool bSuccess = mFilePath.GetFileTime(nullptr, nullptr, &ft);
    time = ft;
    return bSuccess;
}

BOOL CDiskFileMetaDataProvider::GetCreationTime(CTime &time) const
{
    CFileTime ft;
    bool bSuccess = mFilePath.GetFileTime(&ft, nullptr, nullptr);
    time = ft;
    return bSuccess;
}

BOOL CDiskFileMetaDataProvider::GetLastAccessTime(CTime &time) const
{
    CFileTime ft;
    bool bSuccess = mFilePath.GetFileTime(nullptr, &ft, nullptr);
    time = ft;
    return bSuccess;
}


CFileFindExMetaDataProvider::CFileFindExMetaDataProvider(const CFileFindEx &fileFindEx)
	: mFileFindEx(fileFindEx)
{
}
LONGLONG CFileFindExMetaDataProvider::GetFileSize() const
{
	if (!mFileFindEx.HasSize())
		return -1;
	return mFileFindEx.GetFileSize();
}
BOOL CFileFindExMetaDataProvider::GetLastWriteTime(CTime &time) const
{
	return mFileFindEx.GetLastWriteTime(time);
}
BOOL CFileFindExMetaDataProvider::GetCreationTime(CTime &time) const
{
	return mFileFindEx.GetCreationTime(time);
}
BOOL CFileFindExMetaDataProvider::GetLastAccessTime(CTime &time) const
{
	return mFileFindEx.GetLastAccessTime(time);
}


CCacheDataFileMetaDataProvider::CCacheDataFileMetaDataProvider(sqlite3_stmt *instatement, int sizeCol, int createdTimeCol, int modifiedTimeCol, int accessedTimeCol, int updatedTimeCol)
	: statement(instatement), mSizeCol(sizeCol), mCreatedTimeCol(createdTimeCol), mModifiedTimeCol(modifiedTimeCol), mAccessedTimeCol(accessedTimeCol), mUpdatedTimeCol(updatedTimeCol)
{
}
LONGLONG CCacheDataFileMetaDataProvider::GetFileSize() const
{
	return SystemUtils::GetSizeFromString(SystemUtils::UTF8ToUnicodeCString((const char *)sqlite3_column_text(statement, mSizeCol)));
}
BOOL CCacheDataFileMetaDataProvider::GetLastWriteTime(CTime &time) const
{
	time = SystemUtils::IntToTime(sqlite3_column_int64(statement, mModifiedTimeCol));
	return TRUE;
}
BOOL CCacheDataFileMetaDataProvider::GetCreationTime(CTime &time) const
{
	time = SystemUtils::IntToTime(sqlite3_column_int64(statement, mCreatedTimeCol));
	return TRUE;
}
BOOL CCacheDataFileMetaDataProvider::GetLastAccessTime(CTime &time) const
{
	time = SystemUtils::IntToTime(sqlite3_column_int64(statement, mAccessedTimeCol));
	return TRUE;
}

BOOL CCacheDataFileMetaDataProvider::GetLastUpdateTime( CTime &time ) const
{
	if (mUpdatedTimeCol >= 0) {
		time = SystemUtils::IntToTime(sqlite3_column_int64(statement, mUpdatedTimeCol));
		return TRUE;
	}
	return FALSE;
}
