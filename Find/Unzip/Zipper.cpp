// Zipper.cpp: implementation of the CZipper class.
//
//////////////////////////////////////////////////////////////////////
#include "StdAfx.h"

#include "Zipper.h"

#include "minizip\zip.h"
#include "minizip\iowin32.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

const UINT BUFFERSIZE = 65536;

voidpf ZCALLBACK uwfopen_file_func (voidpf opaque, const wchar_t* filename, int mode)
{
	FILE* file = NULL;
	const wchar_t* mode_fopen = NULL;
	if ((mode & ZLIB_FILEFUNC_MODE_READWRITEFILTER)==ZLIB_FILEFUNC_MODE_READ)
		mode_fopen = L"rb";
	else
		if (mode & ZLIB_FILEFUNC_MODE_EXISTING)
			mode_fopen = L"r+b";
		else
			if (mode & ZLIB_FILEFUNC_MODE_CREATE)
				mode_fopen = L"wb";

	if ((filename!=NULL) && (mode_fopen != NULL))
		if (_wfopen_s(&file, filename, mode_fopen))
			file = NULL;
	return file;
}

bool GetLastModified(LPCTSTR szPath, SYSTEMTIME& sysTime, bool bLocalTime)
{
	ZeroMemory(&sysTime, sizeof(SYSTEMTIME));

	DWORD dwAttr = ::GetFileAttributes(szPath);

	// files only
	if (dwAttr == 0xFFFFFFFF)
		return false;

	WIN32_FIND_DATA findFileData;
	HANDLE hFind = FindFirstFile(szPath, &findFileData);

	if (hFind == INVALID_HANDLE_VALUE)
		return FALSE;

	FindClose(hFind);

	FILETIME ft = findFileData.ftLastWriteTime;

	if (bLocalTime)
		FileTimeToLocalFileTime(&findFileData.ftLastWriteTime, &ft);

	FileTimeToSystemTime(&ft, &sysTime);
	return true;
}
static int DefZipCallback(ZLibCBData *pData)
{
	return ZLIBCBFN_CONTINUE;
}

CZipper::CZipper(LPCTSTR szFilePath, LPCTSTR szRootFolder, bool bAppend) : m_uzFile(0), mCallbackFn(DefZipCallback)
{
	CloseZip();

	if (szFilePath)
		OpenZip(szFilePath, szRootFolder, bAppend);
}

CZipper::~CZipper()
{
	CloseZip();
}
void CZipper::SetUnzipCallbackFn(ZLibCallback unzipCallbackFn, void *pUserData)
{
	if (unzipCallbackFn)
		mCallbackFn = unzipCallbackFn;
	mZipData.pUserData = pUserData;
}

bool CZipper::CloseZip()
{
	int nRet = m_uzFile ? zipClose(m_uzFile, NULL) : ZIP_OK;

	m_uzFile = NULL;
	m_szRootFolder[0] = 0;
	ZeroMemory(&m_info, sizeof(m_info));

	return (nRet == ZIP_OK);
}

void CZipper::GetFileInfo(Z_FileInfo& info)
{
	info = m_info;
}

// simple interface
bool CZipper::ZipFile(LPCTSTR szFilePath)
{
	// make zip path
	wchar_t szDrive[_MAX_DRIVE], szFolder[MAX_PATH], szName[_MAX_FNAME];
	_wsplitpath_s(szFilePath, szDrive, _MAX_DRIVE, szFolder, MAX_PATH, szName, _MAX_FNAME, NULL, NULL);

	wchar_t szZipPath[MAX_PATH];
	_wmakepath_s(szZipPath, MAX_PATH, szDrive, szFolder, szName, L"zip");

	CZipper zip;

	if (zip.OpenZip(szZipPath, false))
		return zip.AddFileToZip(szFilePath, false);

	return FALSE;
}
	
bool CZipper::ZipFolder(LPCTSTR szFilePath, bool bIgnoreFilePath)
{
	// make zip path
	wchar_t szDrive[_MAX_DRIVE], szFolder[MAX_PATH], szName[_MAX_FNAME];
	_wsplitpath_s(szFilePath, szDrive, _MAX_DRIVE, szFolder, MAX_PATH, szName, _MAX_FNAME, NULL, NULL);

	wchar_t szZipPath[MAX_PATH];
	_wmakepath_s(szZipPath, MAX_PATH, szDrive, szFolder, szName, L"zip");

	
	if (OpenZip(szZipPath, FALSE))
		return AddFolderToZip(szFilePath, bIgnoreFilePath);

	return FALSE;
}
	
// works with prior opened zip
bool CZipper::AddFileToZip(LPCTSTR szFilePath, bool bIgnoreFilePath)
{
	if (!m_uzFile)
		return FALSE;

	// we don't allow paths beginning with '..\' because this would be outside
	// the root folder
	if (!bIgnoreFilePath && wcsstr(szFilePath, L"..\\") == szFilePath)
		return false;

//	TRACE ("CZipper::AddFileToZip(%s)\n", szFilePath);

	bool bFullPath = (wcschr(szFilePath, L':') != NULL);

	// if the file is relative then we need to append the root before opening
	wchar_t szFullFilePath[MAX_PATH];
	
	lstrcpy(szFullFilePath, szFilePath);
	PrepareSourcePath(szFullFilePath);

	// if the file is a fullpath then remove the root path bit
	wchar_t szFileName[MAX_PATH] = L"";

	if (bIgnoreFilePath)
	{
		wchar_t szName[_MAX_FNAME], szExt[_MAX_EXT];
		_wsplitpath_s(szFilePath, NULL, NULL, NULL, NULL, szName, _MAX_FNAME, szExt, _MAX_EXT);

		_wmakepath_s(szFileName, MAX_PATH, NULL, NULL, szName, szExt);
	}
	else if (bFullPath)
	{
		// check the root can be found
		if (0 != _wcsnicmp(szFilePath, m_szRootFolder, lstrlen(m_szRootFolder)))
			return false;

		// else
		lstrcpy(szFileName, szFilePath + lstrlen(m_szRootFolder));
	}
	else // relative path
	{
		// if the path begins with '.\' then remove it
		if (wcsstr(szFilePath, L".\\") == szFilePath)
			lstrcpy(szFileName, szFilePath + 2);
		else
			lstrcpy(szFileName, szFilePath);
	}

	// save file attributes
	zip_fileinfo zfi;

	zfi.internal_fa = 0;
	zfi.external_fa = ::GetFileAttributes(szFilePath);
	
	// save file time
	SYSTEMTIME st;

	GetLastModified(szFullFilePath, st, TRUE);

	zfi.dosDate = 0;
	zfi.tmz_date.tm_year = st.wYear;
	zfi.tmz_date.tm_mon = st.wMonth - 1;
	zfi.tmz_date.tm_mday = st.wDay;
	zfi.tmz_date.tm_hour = st.wHour;
	zfi.tmz_date.tm_min = st.wMinute;
	zfi.tmz_date.tm_sec = st.wSecond;
	
	// load input file
	HANDLE hInputFile = ::CreateFile(szFullFilePath, 
									GENERIC_READ,
									0,
									NULL,
									OPEN_EXISTING,
									FILE_ATTRIBUTE_READONLY,
									NULL);

	if (hInputFile == INVALID_HANDLE_VALUE)
		return FALSE;

	char chFileName[MAX_PATH];
	mZipData.message = ZLIBCBFN_FILE;
	WideCharToMultiByte(CP_UTF8, 0, szFileName, -1, chFileName, MAX_PATH, NULL, NULL);
	int nRet = zipOpenNewFileInZip(m_uzFile, 
									chFileName,
									&zfi, 
									NULL, 
									0,
									NULL,
									0, 
									NULL,
									Z_DEFLATED,
									Z_DEFAULT_COMPRESSION);

	if (nRet == ZIP_OK)
	{
		m_info.nFileCount++;

		// read the file and output to zip
		char pBuffer[BUFFERSIZE];
		DWORD dwBytesRead = 0, dwFileSize = 0;

		while (nRet == ZIP_OK && ::ReadFile(hInputFile, pBuffer, BUFFERSIZE, &dwBytesRead, NULL))
		{
			dwFileSize += dwBytesRead;

			if (dwBytesRead) {
				nRet = zipWriteInFileInZip(m_uzFile, pBuffer, dwBytesRead);
				mZipData.szBytesProcessed = dwBytesRead;
				if (mCallbackFn(&mZipData) != ZLIBCBFN_CONTINUE) {
					nRet = ZIP_ERRNO; // User says abort
					break;
				}
			}
			else
				break;
		}

		m_info.dwUncompressedSize += dwFileSize;
	}

	zipCloseFileInZip(m_uzFile);
	::CloseHandle(hInputFile);

	return (nRet == ZIP_OK);
}

bool CZipper::AddFileToZip(LPCTSTR szFilePath, LPCTSTR szRelFolderPath)
{
	if (!m_uzFile)
		return FALSE;

	// szRelFolderPath cannot contain drive info
	if (szRelFolderPath && wcschr(szRelFolderPath, L':'))
		return FALSE;

	// if the file is relative then we need to append the root before opening
	wchar_t szFullFilePath[MAX_PATH];
	
	lstrcpy(szFullFilePath, szFilePath);
	PrepareSourcePath(szFullFilePath);

	// save file attributes and time
	zip_fileinfo zfi;

	zfi.internal_fa = 0;
	zfi.external_fa = ::GetFileAttributes(szFilePath);
	
	// save file time
	SYSTEMTIME st;

	GetLastModified(szFullFilePath, st, TRUE);

	zfi.dosDate = 0;
	zfi.tmz_date.tm_year = st.wYear;
	zfi.tmz_date.tm_mon = st.wMonth - 1;
	zfi.tmz_date.tm_mday = st.wDay;
	zfi.tmz_date.tm_hour = st.wHour;
	zfi.tmz_date.tm_min = st.wMinute;
	zfi.tmz_date.tm_sec = st.wSecond;

	// load input file
	HANDLE hInputFile = ::CreateFile(szFullFilePath, 
									GENERIC_READ,
									0,
									NULL,
									OPEN_EXISTING,
									FILE_ATTRIBUTE_READONLY,
									NULL);

	if (hInputFile == INVALID_HANDLE_VALUE)
		return FALSE;

	// strip drive info off filepath
	wchar_t szName[_MAX_FNAME], szExt[_MAX_EXT];
	_wsplitpath_s(szFilePath, NULL, NULL, NULL, NULL, szName, _MAX_FNAME, szExt, _MAX_EXT);

	// prepend new folder path 
	wchar_t szFileName[MAX_PATH];
	_wmakepath_s(szFileName, MAX_PATH, NULL, szRelFolderPath, szName, szExt);

	// open the file in the zip making sure we remove any leading '\'

	char chFileName[MAX_PATH];
	WideCharToMultiByte(CP_UTF8, 0, szFileName, -1, chFileName, MAX_PATH, NULL, NULL);

	int nRet = zipOpenNewFileInZip(m_uzFile, 
									chFileName + (chFileName[0] == '\\' ? 1 : 0),
									&zfi, 
									NULL, 
									0,
									NULL,
									0, 
									NULL,
									Z_DEFLATED,
									Z_DEFAULT_COMPRESSION);

	if (nRet == ZIP_OK)
	{
		m_info.nFileCount++;

		// read the file and output to zip
		char pBuffer[BUFFERSIZE];
		DWORD dwBytesRead = 0, dwFileSize = 0;

		while (nRet == ZIP_OK && ::ReadFile(hInputFile, pBuffer, BUFFERSIZE, &dwBytesRead, NULL))
		{
			dwFileSize += dwBytesRead;

			if (dwBytesRead)
				nRet = zipWriteInFileInZip(m_uzFile, pBuffer, dwBytesRead);
			else
				break;
		}

		m_info.dwUncompressedSize += dwFileSize;
	}

	zipCloseFileInZip(m_uzFile);
	::CloseHandle(hInputFile);

	return (nRet == ZIP_OK);
}
bool CZipper::AddFolderContentsToZip(LPCTSTR szFolderPath, bool bIgnoreFilePath)
{
	// if the path is relative then we need to append the root before opening
	wchar_t szFullPath[MAX_PATH];
	
	lstrcpy(szFullPath, szFolderPath);
	PrepareSourcePath(szFullPath);

	// if the folder is a fullpath then remove the root path bit
	wchar_t szFolderName[MAX_PATH] = L"";
	
	if (bIgnoreFilePath)
	{
		_wsplitpath_s(szFullPath, NULL, NULL,NULL, NULL, szFolderName, MAX_PATH, NULL, NULL);
	}
	else
	{
		// check the root can be found
		if (0 != _wcsnicmp(szFullPath, m_szRootFolder, lstrlen(m_szRootFolder)))
			return false;
		
		// else
		lstrcpy(szFolderName, szFullPath + lstrlen(m_szRootFolder));
	}

	// build searchspec
	wchar_t szDrive[_MAX_DRIVE], szFolder[MAX_PATH], szName[_MAX_FNAME];
	_wsplitpath_s(szFullPath, szDrive, _MAX_DRIVE, szFolder, MAX_PATH, szName, _MAX_FNAME, NULL, NULL);
	lstrcat(szFolder, szName);

	wchar_t szSearchSpec[MAX_PATH];
	_wmakepath_s(szSearchSpec, szDrive, szFolder, L"*", L"*");

	WIN32_FIND_DATA finfo;
	HANDLE hSearch = FindFirstFile(szSearchSpec, &finfo);

	if (hSearch != INVALID_HANDLE_VALUE) 
	{
		do 
		{
			if (finfo.cFileName[0] != '.') 
			{
				wchar_t szItem[MAX_PATH];
				_wmakepath_s(szItem, MAX_PATH, szDrive, szFolder, finfo.cFileName, NULL);
				
				if (finfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					AddFolderToZip(szItem, bIgnoreFilePath);
				}
				else 
					AddFileToZip(szItem, bIgnoreFilePath);
			}
		} 
		while (FindNextFile(hSearch, &finfo));
		
		FindClose(hSearch);
	}

	return TRUE;
}

bool CZipper::AddFolderToZip(LPCTSTR szFolderPath, bool bIgnoreFilePath)
{
	if (!m_uzFile)
		return FALSE;

	m_info.nFolderCount++;

	// if the path is relative then we need to append the root before opening
	wchar_t szFullPath[MAX_PATH];
	
	lstrcpy(szFullPath, szFolderPath);
	PrepareSourcePath(szFullPath);

	// always add folder first
	// save file attributes
	zip_fileinfo zfi;
	
	zfi.internal_fa = 0;
	zfi.external_fa = ::GetFileAttributes(szFullPath);
	
	SYSTEMTIME st;
	
	GetLastModified(szFullPath, st, TRUE);
	
	zfi.dosDate = 0;
	zfi.tmz_date.tm_year = st.wYear;
	zfi.tmz_date.tm_mon = st.wMonth - 1;
	zfi.tmz_date.tm_mday = st.wDay;
	zfi.tmz_date.tm_hour = st.wHour;
	zfi.tmz_date.tm_min = st.wMinute;
	zfi.tmz_date.tm_sec = st.wSecond;
	
	// if the folder is a fullpath then remove the root path bit
	wchar_t szFolderName[MAX_PATH] = L"";
	
	if (bIgnoreFilePath)
	{
		_wsplitpath_s(szFullPath, NULL, NULL,NULL, NULL, szFolderName, MAX_PATH, NULL, NULL);
	}
	else
	{
		// check the root can be found
		if (0 != _wcsnicmp(szFullPath, m_szRootFolder, lstrlen(m_szRootFolder)))
			return false;
		
		// else
		lstrcpy(szFolderName, szFullPath + lstrlen(m_szRootFolder));
	}
	
	// folders are denoted by a trailing '\\'
	lstrcat(szFolderName, L"\\");
	
	// open the file in the zip making sure we remove any leading '\'

	char chFolderName[MAX_PATH];
	WideCharToMultiByte(CP_UTF8, 0, szFolderName, -1, chFolderName, MAX_PATH, NULL, NULL);

	int nRet = zipOpenNewFileInZip(m_uzFile, 
		chFolderName,
		&zfi, 
		NULL, 
		0,
		NULL,
		0, 
		NULL,
		Z_DEFLATED,
		Z_DEFAULT_COMPRESSION);
	
	zipCloseFileInZip(m_uzFile);

	// build searchspec
	wchar_t szDrive[_MAX_DRIVE], szFolder[MAX_PATH], szName[_MAX_FNAME];
	_wsplitpath_s(szFullPath, szDrive, _MAX_DRIVE, szFolder, MAX_PATH, szName, _MAX_FNAME, NULL, NULL);
	lstrcat(szFolder, szName);

	wchar_t szSearchSpec[MAX_PATH];
	_wmakepath_s(szSearchSpec, szDrive, szFolder, L"*", L"*");

	WIN32_FIND_DATA finfo;
	HANDLE hSearch = FindFirstFile(szSearchSpec, &finfo);

	if (hSearch != INVALID_HANDLE_VALUE) 
	{
		do 
		{
			if (finfo.cFileName[0] != '.') 
			{
				wchar_t szItem[MAX_PATH];
				_wmakepath_s(szItem, MAX_PATH, szDrive, szFolder, finfo.cFileName, NULL);
				
				if (finfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					AddFolderToZip(szItem, bIgnoreFilePath);
				}
				else 
					AddFileToZip(szItem, bIgnoreFilePath);
			}
		} 
		while (FindNextFile(hSearch, &finfo));
		
		FindClose(hSearch);
	}

	return TRUE;
}
#include <string>
bool CZipper::GetFolderInfo(LPCTSTR folderPath, Z_FileInfo& info)
{
	bool bSuccess(false);
	WIN32_FIND_DATA finfo;
	std::wstring strSearchFolder(folderPath);
	if (strSearchFolder.length() > 0) {
		if (strSearchFolder[strSearchFolder.length()-1] != '\\')
			strSearchFolder += _T("\\");
	}
	strSearchFolder += _T("*");
	HANDLE hSearch = FindFirstFile(strSearchFolder.c_str(), &finfo);

	if (hSearch != INVALID_HANDLE_VALUE) 
	{
		bSuccess = true;
		do 
		{
			if (finfo.cFileName[0] != '.') 
			{
				std::wstring strChildFilePath(folderPath);
				strChildFilePath += std::wstring(_T("\\")) + finfo.cFileName;

				if (finfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					info.nFolderCount++;
					GetFolderInfo(strChildFilePath.c_str(), info);
				}
				else {
					info.nFileCount++;
					LARGE_INTEGER li;
					li.LowPart = finfo.nFileSizeLow;
					li.HighPart = finfo.nFileSizeHigh;
					info.dwUncompressedSize += li.QuadPart;
				}
			}
		} 
		while (FindNextFile(hSearch, &finfo));

		FindClose(hSearch);
	}
	return bSuccess;
}
// extended interface
bool CZipper::OpenZip(LPCTSTR szFilePath, LPCTSTR szRootFolder, bool bAppend)
{
	CloseZip();

	if (!szFilePath || !lstrlen(szFilePath))
		return false;

	// convert szFilePath to fully qualified path 
	wchar_t szFullPath[MAX_PATH];

	if (!GetFullPathName(szFilePath, MAX_PATH, szFullPath, NULL))
		return false;

	// zipOpen will fail if bAppend is TRUE and zip does not exist
	if (bAppend && ::GetFileAttributes(szFullPath) == 0xffffffff)
		bAppend = false;
	
	zlib_filefunc_def filefunc_def;
	fill_fopen_filefunc(&filefunc_def);
	filefunc_def.zopen_file = (open_file_func)uwfopen_file_func;

	m_uzFile = zipOpen2((char *)szFullPath, bAppend ? 1 : 0, NULL, &filefunc_def);

	if (m_uzFile)
	{
		if (!szRootFolder)
		{
			wchar_t szDrive[_MAX_DRIVE], szFolder[MAX_PATH];
			_wsplitpath_s(szFullPath, szDrive,_MAX_DRIVE, szFolder, MAX_PATH, NULL, NULL, NULL, NULL);

			_wmakepath_s(m_szRootFolder, szDrive, szFolder, NULL, NULL);
		}
		else if (lstrlen(szRootFolder))
		{
			_wmakepath_s(m_szRootFolder, MAX_PATH, NULL, szRootFolder, L"", NULL);
		}
	}

	return (m_uzFile != NULL);
}

void CZipper::PrepareSourcePath(LPTSTR szPath)
{
	bool bFullPath = (wcschr(szPath, L':') != NULL);

	// if the file is relative then we need to append the root before opening
	if (!bFullPath)
	{
		wchar_t szTemp[MAX_PATH];
		lstrcpy(szTemp, szPath);

		_wmakepath_s(szPath, MAX_PATH, NULL, m_szRootFolder, szTemp, NULL);
	}
}
