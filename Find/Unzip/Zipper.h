// Zipper.h: interface for the CZipper class.
//
//////////////////////////////////////////////////////////////////////
#include "windows.h"
#include <tchar.h>

#if !defined(AFX_ZIPPER_H__4249275D_B50B_4AAE_8715_B706D1CA0F2F__INCLUDED_)
#define AFX_ZIPPER_H__4249275D_B50B_4AAE_8715_B706D1CA0F2F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ZLibCommon.h"

struct Z_FileInfo
{
	int nFileCount;
	int nFolderCount;
	LONGLONG dwUncompressedSize;
	Z_FileInfo() : nFileCount(0), nFolderCount(0), dwUncompressedSize(0)
	{}
};

class CZipper  
{
public:
	CZipper(LPCTSTR szFilePath = NULL, LPCTSTR szRootFolder = NULL, bool bAppend = FALSE);
	virtual ~CZipper();

	// simple interface
	static bool ZipFile(LPCTSTR szFilePath); // saves as same name with .zip
	bool ZipFolder(LPCTSTR szFilePath, bool bIgnoreFilePath = FALSE); // saves as same name with .zip
	
	// works with prior opened zip
	bool AddFileToZip(LPCTSTR szFilePath, bool bIgnoreFilePath = FALSE);
	bool AddFileToZip(LPCTSTR szFilePath, LPCTSTR szRelFolderPath); // replaces path info from szFilePath with szFolder
	bool AddFolderContentsToZip(LPCTSTR szFolderPath, bool bIgnoreFilePath = FALSE);
	bool AddFolderToZip(LPCTSTR szFolderPath, bool bIgnoreFilePath = FALSE);

	// extended interface
	bool OpenZip(LPCTSTR szFilePath, LPCTSTR szRootFolder = NULL, bool bAppend = FALSE);
	bool CloseZip(); // for multiple reuse
	void GetFileInfo(Z_FileInfo& info);
	static bool GetFolderInfo(LPCTSTR folderPath, Z_FileInfo& info);
	void SetUnzipCallbackFn(ZLibCallback unzipCallbackFn, void *pUserData = NULL);
protected:
	void* m_uzFile;
	wchar_t m_szRootFolder[MAX_PATH + 1];
	Z_FileInfo m_info;
	ZLibCBData mZipData;
	ZLibCallback mCallbackFn;

protected:
	void PrepareSourcePath(LPTSTR szPath);
};

#endif // !defined(AFX_ZIPPER_H__4249275D_B50B_4AAE_8715_B706D1CA0F2F__INCLUDED_)
