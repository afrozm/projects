#include "stdafx.h"
#include "FileMapping.h"

#define FMF_CLOSEFILEHANDLE 0x1
#define FMF_READONLY 0x2

CFileMapping::CFileMapping()
: m_hFile(INVALID_HANDLE_VALUE), m_hFileMapping(NULL), m_pStart(NULL), m_pEnd(NULL),
m_uFlags(0), m_uMapSize(0), m_pCurrentStart(NULL), m_uFileOffset(0), m_dwMapViewDesiredAccess(FILE_MAP_READ)
{
}

CFileMapping::~CFileMapping(void)
{
	CloseFileMapping();
}
void CFileMapping::CloseFileMapping()
{
	if (m_pStart != NULL)
		UnmapViewOfFile(m_pStart);
	m_pEnd = m_pCurrentStart = m_pStart = NULL;
	if (m_hFileMapping != NULL)
		CloseHandle(m_hFileMapping);
	m_hFileMapping = NULL;
	if ((m_uFlags & FMF_CLOSEFILEHANDLE) && m_hFile != INVALID_HANDLE_VALUE)
		CloseHandle(m_hFile);
	m_hFile = INVALID_HANDLE_VALUE;
	m_uFlags = 0;
	mFilePath.clear();
}
LPVOID CFileMapping::GetFileMapping(LPCTSTR filePath, UINT64 uMapSize)
{
	bool bReadOnly(uMapSize == 0UL);
	if (bReadOnly)
		m_uFlags |= FMF_READONLY;
	DWORD dwDesiredAccess(bReadOnly ? GENERIC_READ : GENERIC_ALL);
	DWORD sharedMode(bReadOnly ? FILE_SHARE_WRITE : 0);
	HANDLE hTargetFile = CreateFile(filePath, dwDesiredAccess, sharedMode | FILE_SHARE_READ, NULL, 
		bReadOnly ? OPEN_EXISTING : CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	GetFileMapping(hTargetFile, uMapSize);
	m_uFlags |= FMF_CLOSEFILEHANDLE;
	mFilePath = filePath;
	return GetStartAddress();
}
LPVOID CFileMapping::GetFileMapping(HANDLE hFile, UINT64 uMapSize)
{
	CloseFileMapping();
	if (hFile != INVALID_HANDLE_VALUE) {
		bool bReadOnly(uMapSize == 0UL);
		DWORD flProtect(bReadOnly ? PAGE_READONLY : PAGE_READWRITE);
		if (!bReadOnly)
			m_dwMapViewDesiredAccess = FILE_MAP_ALL_ACCESS;
		m_hFile = hFile;
		m_hFileMapping = CreateFileMapping(hFile, NULL, flProtect | SEC_COMMIT, uMapSize >> 32, (DWORD)uMapSize, NULL);
		MapView();
	}
	return GetStartAddress();
}
LPVOID CFileMapping::MapView()
{
	if (m_hFileMapping != NULL) {
		if (m_pStart != NULL) {
			//if (!(m_uFlags & FMF_READONLY))
			//	FlushViewOfFile(m_pStart, 0);
			UnmapViewOfFile(m_pStart);
		}
		m_pEnd = m_pCurrentStart = m_pStart = NULL;
		UINT64 fileSize(GetFileSize());
		if (m_uFileOffset < fileSize) {
			SIZE_T sizeToMap(0);
			// Frist try to map entire remaining file
			m_pStart = MapViewOfFile(m_hFileMapping, m_dwMapViewDesiredAccess, m_uFileOffset >> 32, (DWORD)m_uFileOffset, sizeToMap);
			if (m_pStart == NULL) {
				sizeToMap = 64*1024*1024; // 64 MB
				while (sizeToMap >= 1024*1024) {
					m_pStart = MapViewOfFile(m_hFileMapping, m_dwMapViewDesiredAccess, m_uFileOffset >> 32, (DWORD)m_uFileOffset, sizeToMap);
					DWORD err = GetLastError();
					if (m_pStart == NULL)
						sizeToMap >>= 1;
					else break;
				}
			}
			else sizeToMap = (SIZE_T)(fileSize - m_uFileOffset);
			if (m_pStart != NULL) {
				m_pEnd = m_pCurrentStart = m_pStart;
				m_pEnd = (char*)m_pEnd + sizeToMap;
			}
		}
		if (m_pStart == NULL) {
			CloseHandle(m_hFileMapping);
			m_hFileMapping = NULL;
			LARGE_INTEGER li;
			li.QuadPart = m_uFileOffset;
			SetFilePointerEx(m_hFile, li, NULL, FILE_BEGIN);
		}
	}
	return GetStartAddress();
}
UINT64 CFileMapping::GetFileSize() const
{
	UINT64 fileSize(0);
	if (m_hFile != NULL) {
		LARGE_INTEGER li = {0};
		GetFileSizeEx(m_hFile, &li);
		fileSize = li.QuadPart;
		if (fileSize == 0 && mFilePath.size() > 4) {
			ULARGE_INTEGER li = {0};
			GetDiskFreeSpaceEx(mFilePath.c_str()+4, NULL, &li, NULL);
			fileSize = li.QuadPart;
		}
	}
	return fileSize;
}
UINT CFileMapping::Write(LPCVOID pBuf, UINT uSize)
{
	UINT nBytesWritten(0);
	while (uSize > 0) {
		unsigned int uBytesWritten(uSize);
		UINT64 remainingSize(GetRemainingSize());
		if (uSize > remainingSize)
			uBytesWritten = (unsigned int)remainingSize;
		if (uBytesWritten > 0UL) {
			memcpy(m_pCurrentStart, pBuf, uBytesWritten);
			m_pCurrentStart = (char *)m_pCurrentStart + uBytesWritten;
			pBuf = (const char *)m_pCurrentStart + uBytesWritten;
			uSize -= uBytesWritten;
			m_uFileOffset += uBytesWritten;
			nBytesWritten += uBytesWritten;
		}
		else if (MapView() == NULL)
			break;
	}
	if (uSize > 0) {
		DWORD nWritten(0);
		WriteFile(m_hFile, pBuf, uSize, &nWritten, NULL);
		nBytesWritten += nWritten;
	}
	return nBytesWritten;
}
UINT CFileMapping::Read(LPVOID pBuf, UINT uSize)
{
	UINT nBytesRead(0);
	while (uSize > 0) {
		unsigned int uBytesRead(uSize);
		UINT64 remainingSize(GetRemainingSize());
		if (uSize > remainingSize)
			uBytesRead = (unsigned int)remainingSize;
		if (uBytesRead > 0UL) {
			memcpy(pBuf, m_pCurrentStart, uBytesRead);
			m_pCurrentStart = (char *)m_pCurrentStart + uBytesRead;
			pBuf = (char *)m_pCurrentStart + uBytesRead;
			uSize -= uBytesRead;
			m_uFileOffset += uBytesRead;
			nBytesRead += uBytesRead;
		}
		else if (MapView() == NULL)
			break;
	}
	if (uSize > 0) {
		DWORD nRead(0);
		ReadFile(m_hFile, pBuf, uSize, &nRead, NULL);
		nBytesRead += nRead;
	}
	return nBytesRead;
}
void CFileMapping::SetFileHandle(HANDLE hFile)
{
	CloseFileMapping();
	m_hFile = hFile;
}