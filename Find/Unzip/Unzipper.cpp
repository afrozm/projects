// Unzipper.cpp: implementation of the CUnzipper class.
//
//////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "SystemUtils.h"

#include "Unzipper.h"

#include "minizip\unzip.h"
#include "minizip\iowin32.h"

static int DefUnzipCallback(ZLibCBData *pData)
{
	return ZLIBCBFN_CONTINUE;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

/* 
 * Buffer size increased from 16384. With the change unzip takes an 
 * average of 212 s over 10 runs for unzipping the Ps Staging Area of 
 * size 1.3 G. It takes average over 10 runs of 255 s without the change.
 * See Watson bug 2334175
 */
const UINT BUFFERSIZE = 65536;

CUnzipper::CUnzipper(LPCTSTR szFileName) : m_uzFile(0),
	mCallbackFn(DefUnzipCallback), muUncompressedSize(0)
{
	m_szOutputFolder[0] = 0;

	OpenZip(szFileName);
}

CUnzipper::~CUnzipper()
{
	CloseZip();
}

BOOL CUnzipper::CloseZip()
{
	unzCloseCurrentFile(m_uzFile);

	int nRet = unzClose(m_uzFile);
	m_uzFile = NULL;
	return (nRet == UNZ_OK);
}

// simple interface
BOOL CUnzipper::Unzip(BOOL bIgnoreFilePath)
{
	if (!m_uzFile)
		return FALSE;

	return UnzipTo(m_szOutputFolder, bIgnoreFilePath);
}

BOOL CUnzipper::UnzipTo(LPCTSTR szFolder, BOOL bIgnoreFilePath)
{
	if (!m_uzFile)
		return FALSE;

	if (!szFolder || !CreateFolder(szFolder))
		return FALSE;

	if (GetFileCount() == 0)
		return FALSE;

	if (!GotoFirstFile())
		return FALSE;

	// else
	do
	{
		if (!UnzipFile(szFolder, bIgnoreFilePath))
			return FALSE;
	}
	while (GotoNextFile());
	
	return TRUE;
}

BOOL CUnzipper::Unzip(LPCTSTR szFileName, LPCTSTR szFolder, BOOL bIgnoreFilePath)
{
	CloseZip();

	if (!OpenZip(szFileName))
		return FALSE;
	if (szFolder == NULL)
		szFolder = m_szOutputFolder;

	BOOL bRet = UnzipTo(szFolder, bIgnoreFilePath);

	CloseZip();

	return bRet;
}
// extended interface
voidpf ZCALLBACK wfopen_file_func (voidpf opaque, const wchar_t* filename, int mode)
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
ZPOS64_T ZCALLBACK ftell64_file_func(voidpf opaque, voidpf stream)
{
	return _ftelli64((FILE*)stream);
}
long     ZCALLBACK fseek64_file_func(voidpf opaque, voidpf stream, ZPOS64_T offset, int origin)
{
	return _fseeki64((FILE*)stream, offset, origin);
}
BOOL CUnzipper::OpenZip(LPCTSTR szFileName)
{
	CloseZip();

	if (szFileName)
	{
		zlib_filefunc64_def filefunc_def;
		fill_fopen64_filefunc(&filefunc_def);
		filefunc_def.zopen64_file = (open64_file_func)wfopen_file_func;
		filefunc_def.ztell64_file = (tell64_file_func)ftell64_file_func;
		filefunc_def.zseek64_file = (seek64_file_func)fseek64_file_func;
		m_uzFile = unzOpen2_64((const char *)szFileName, &filefunc_def);

		if (m_uzFile)
		{
			// set the default output folder
			wchar_t* szPath = _wcsdup(szFileName);

			// strip off extension
			wchar_t* p = wcsrchr(szPath, '.');

			if (p)
				*p = 0;
			wcscpy_s(m_szOutputFolder, MAX_PATH + 1, szPath);
			free(szPath);
		}
	}

	return (m_uzFile != NULL);
}
BOOL CUnzipper::SetOutputFolder(LPCTSTR szFolder)
{
	DWORD dwAttrib = GetFileAttributes(szFolder);

	if (dwAttrib != 0xffffffff && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY))
		return FALSE;

	wcscpy_s(m_szOutputFolder, MAX_PATH + 1, szFolder);

	return CreateFolder(szFolder);
}

UINT64 CUnzipper::GetFileCount()
{
	if (!m_uzFile)
		return 0;

	unz_global_info64 info;

	if (unzGetGlobalInfo64(m_uzFile, &info) == UNZ_OK)
	{
		return info.number_entry;
	}

	return 0;
}

BOOL CUnzipper::GetFileInfo(int nFile, UZ_FileInfo& info)
{
	if (!m_uzFile)
		return FALSE;

	if (!GotoFile(nFile))
		return FALSE;

	return GetFileInfo(info);
}

BOOL CUnzipper::UnzipFile(int nFile, LPCTSTR szFolder, BOOL bIgnoreFilePath)
{
	if (!m_uzFile)
		return FALSE;

	if (!szFolder)
		szFolder = m_szOutputFolder;

	if (!GotoFile(nFile))
		return FALSE;

	return UnzipFile(szFolder, bIgnoreFilePath);
}

BOOL CUnzipper::GotoFirstFile(LPCTSTR szChildPath)
{
	if (!m_uzFile)
		return FALSE;

	if (!szChildPath || !lstrlen(szChildPath))
		return (unzGoToFirstFile(m_uzFile) == UNZ_OK);

	// else
	if (unzGoToFirstFile(m_uzFile) == UNZ_OK)
	{
		UZ_FileInfo info;

		if (!GetFileInfo(info))
			return FALSE;

		if (_tcsicmp(info.szFileName, szChildPath) == 0)
			return TRUE;

		return GotoNextFile(szChildPath);
	}

	return FALSE;
}

BOOL CUnzipper::GotoNextFile(LPCTSTR szChildPath)
{
	if (!m_uzFile)
		return FALSE;

	if (!szChildPath || !lstrlen(szChildPath))
		return (unzGoToNextFile(m_uzFile) == UNZ_OK);

	// else
	UZ_FileInfo info;

	while (unzGoToNextFile(m_uzFile) == UNZ_OK)
	{
		if (!GetFileInfo(info))
			return FALSE;

		if (_tcsicmp(info.szFileName, szChildPath) == 0)
			return TRUE;
	}

	return FALSE;

}

static int UTF8ToUniCodeString(const char *inStr, wchar_t *outStr, int inOutStrLen)
{
    int len = MultiByteToWideChar(CP_UTF8, 0, inStr, -1, outStr,
        inOutStrLen);
    return len;
}

BOOL CUnzipper::GetFileInfo(UZ_FileInfo& info)
{
	if (!m_uzFile)
		return FALSE;

	unz_file_info64 uzfi;

	ZeroMemory(&info, sizeof(info));
	ZeroMemory(&uzfi, sizeof(uzfi));
	char szFileName[MAX_PATH+1];
	char szComment[MAX_COMMENT+1];
	if (UNZ_OK != unzGetCurrentFileInfo64(m_uzFile, &uzfi, szFileName, MAX_PATH, NULL, 0, szComment, MAX_COMMENT))
		return FALSE;
    UTF8ToUniCodeString(szFileName, info.szFileName,
		sizeof(info.szFileName)/sizeof(TCHAR));
    UTF8ToUniCodeString(szComment, info.szComment,
		sizeof(info.szComment)/sizeof(TCHAR));

	// copy across
	info.dwVersion = uzfi.version;	
	info.dwVersionNeeded = uzfi.version_needed;
	info.dwFlags = uzfi.flag;	
	info.dwCompressionMethod = uzfi.compression_method; 
	info.dwDosDate = uzfi.dosDate;  
	info.dwCRC = uzfi.crc;	 
	info.dwCompressedSize = uzfi.compressed_size; 
	info.dwUncompressedSize = uzfi.uncompressed_size;
	info.dwInternalAttrib = uzfi.internal_fa; 
	info.dwExternalAttrib = uzfi.external_fa; 

	// replace filename forward slashes with backslashes
	int nLen = (int) lstrlen(info.szFileName);
	const int sLen = nLen;

	while (nLen--)
	{
		if (info.szFileName[nLen] == '/')
			info.szFileName[nLen] = '\\';
	}

	// is it a folder?
	info.bFolder = (info.szFileName[sLen - 1] == '\\');
	if (info.bFolder)
		info.szFileName[sLen - 1] = 0;

	return TRUE;
}

BOOL CUnzipper::UnzipFile(LPCTSTR szFolder, BOOL bIgnoreFilePath)
{
	if (!m_uzFile)
		return FALSE;

	if (!szFolder)
		szFolder = m_szOutputFolder;

	if (!CreateFolder(szFolder))
		return FALSE;

	UZ_FileInfo info;
	GetFileInfo(info);

	// if the item is a folder then simply return 'TRUE'
	if (info.szFileName[lstrlen(info.szFileName) - 1] == '\\')
		return TRUE;

	// build the output filename
	TCHAR szFilePath[MAX_PATH];
	wcscpy_s(szFilePath, MAX_PATH, szFolder);

	// append backslash
	if (szFilePath[lstrlen(szFilePath) - 1] != '\\')
		lstrcat(szFilePath, _T("\\"));

	if (bIgnoreFilePath)
	{
		TCHAR* p = _tcsrchr(info.szFileName, '\\');

		if (p)
			_tcscpy_s(info.szFileName, MAX_PATH+1,p + 1);
	}

	TCHAR szFileName[MAX_PATH+1];

	_tcscat_s(szFilePath, MAX_PATH, szFileName);
		
	// open the input and output files
	if (!CreateFilePath(szFilePath))
		return FALSE;

	HANDLE hOutputFile = ::CreateFile(szFilePath, 
										GENERIC_WRITE,
										0,
										NULL,
										CREATE_ALWAYS,
										FILE_ATTRIBUTE_NORMAL,
										NULL);

	if (INVALID_HANDLE_VALUE == hOutputFile)
		return FALSE;

	// read the file and output
	int nRet = UNZ_OK; 
	TCHAR pBuffer[BUFFERSIZE];
	mUnzipData.message = ZLIBCBFN_FILE;
	do
	{
		nRet = unzReadCurrentFile(m_uzFile, pBuffer, BUFFERSIZE);

		if (nRet > 0)
		{
			// output
			DWORD dwBytesWritten = 0;
			if (!::WriteFile(hOutputFile, pBuffer, nRet, &dwBytesWritten, NULL) ||
				dwBytesWritten != (DWORD)nRet)
			{
				nRet = UNZ_ERRNO;
				break;
			}
			mUnzipData.szBytesProcessed = dwBytesWritten;
			if (mCallbackFn(&mUnzipData) != ZLIBCBFN_CONTINUE) {
				nRet = UNZ_ERRNO; // User says abort
				break;
			}
		}
	}
	while (nRet > 0);

	CloseHandle(hOutputFile);
	unzCloseCurrentFile(m_uzFile);

	if (nRet == UNZ_OK)
		SetFileModTime(szFilePath, info.dwDosDate);

	return (nRet == UNZ_OK);
}

BOOL CUnzipper::GotoFile(int nFile)
{
	if (!m_uzFile)
		return FALSE;

	if (nFile < 0 || nFile >= GetFileCount())
		return FALSE;

	GotoFirstFile();

	while (nFile--)
	{
		if (!GotoNextFile())
			return FALSE;
	}

	return TRUE;
}

BOOL CUnzipper::GotoFile(LPCTSTR szFileName, BOOL bIgnoreFilePath)
{
	if (!m_uzFile)
		return FALSE;

	// try the simple approach
	if (unzLocateFile(m_uzFile, SystemUtils::UnicodeToUTF8(szFileName).c_str(), 2) == UNZ_OK)
		return TRUE;

	else if (bIgnoreFilePath)
	{ 
		// brute force way
		if (unzGoToFirstFile(m_uzFile) != UNZ_OK)
			return FALSE;

		UZ_FileInfo info;

		do
		{
			if (!GetFileInfo(info))
				return FALSE;

			// test name
			TCHAR* pName = _tcsrchr(info.szFileName, '\\');

			if (pName)
			{
				pName++;

				if (_tcsicmp(szFileName, pName) == 0)
					return TRUE;
			}
		}
		while (unzGoToNextFile(m_uzFile) == UNZ_OK);
	}

	// else
	return FALSE;
}

BOOL CUnzipper::CreateFolder(LPCTSTR szFolder)
{
	if (!szFolder || !lstrlen(szFolder))
		return FALSE;

	DWORD dwAttrib = GetFileAttributes(szFolder);

	// already exists ?
	if (dwAttrib != 0xffffffff)
		return ((dwAttrib & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY);

	// recursively create from the top down
	TCHAR *szPath = _tcsdup(szFolder);
	TCHAR *p = _tcsrchr(szPath, '\\');

	if (p) 
	{
		// The parent is a dir, not a drive
		*p = '\0';
			
		// if can't create parent
		if (!CreateFolder(szPath))
		{
			free(szPath);
			return FALSE;
		}
		free(szPath);

		if (!::CreateDirectory(szFolder, NULL)) 
			return FALSE;
	}
	
	return TRUE;
}

BOOL CUnzipper::CreateFilePath(LPCTSTR szFilePath)
{
	TCHAR *szPath = _tcsdup(szFilePath);
	TCHAR *p = _tcsrchr(szPath,'\\');

	BOOL bRes = FALSE;

	if (p)
	{
		*p = '\0';

		bRes = CreateFolder(szPath);
	}

	free(szPath);

	return bRes;
}

BOOL CUnzipper::SetFileModTime(LPCTSTR szFilePath, DWORD dwDosDate)
{
	HANDLE hFile = CreateFile(szFilePath, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

	if (!hFile)
		return FALSE;
	
	FILETIME ftm, ftLocal, ftCreate, ftLastAcc, ftLastWrite;

	BOOL bRes = (GetFileTime(hFile, &ftCreate, &ftLastAcc, &ftLastWrite) == TRUE);

	if (bRes)
		bRes = DosDateTimeToFileTime((WORD)(dwDosDate >> 16), (WORD)dwDosDate, &ftLocal);

	if (bRes)
		bRes = LocalFileTimeToFileTime(&ftLocal, &ftm);

	if (bRes)
		bRes = SetFileTime(hFile, &ftm, &ftLastAcc, &ftm);

	CloseHandle(hFile);

	return bRes;
}

void CUnzipper::SetUnzipCallbackFn(ZLibCallback unzipCallbackFn, void *pUserData)
{
	if (unzipCallbackFn)
		mCallbackFn = unzipCallbackFn;
	mUnzipData.pUserData = pUserData;
}

UINT64 CUnzipper::GetUnzipSize()
{
	if (muUncompressedSize == 0) {
		UINT64 nFile = GetFileCount();

		GotoFirstFile();

		while (nFile--)
		{
			UZ_FileInfo info;
			GetFileInfo(info);
			muUncompressedSize += info.dwUncompressedSize;
			if (!GotoNextFile())
				break;
		}
	}
	return muUncompressedSize;
}
LONGLONG CUnzipper::GetUnzipSize(LPCTSTR zipFilePath)
{
	CUnzipper unzipper(zipFilePath);

	return unzipper.GetUnzipSize();
}
BOOL CUnzipper::SetUnzipPassword(const char *password)
{
	if (m_uzFile == NULL)
		return FALSE;
	if (unzOpenCurrentFilePassword(m_uzFile, password) != UNZ_OK)
		return FALSE;
	return TRUE;
}
