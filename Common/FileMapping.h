#pragma once

#include "Common.h"

class CFileMapping
{
public:
	CFileMapping();
	~CFileMapping(void);
	LPVOID GetFileMapping(LPCTSTR filePath, UINT64 uMapSize = 0);
	LPVOID GetFileMapping(HANDLE hFile, UINT64 uMapSize = 0);
	void CloseFileMapping();
	LPVOID GetStartAddress() const
	{
		return m_pStart;
	}
	LPVOID GetEndAddress() const
	{
		return m_pEnd;
	}
	UINT Write(LPCVOID pBuf, UINT uSize);
	UINT Read(LPVOID pBuf, UINT uSize);
	UINT64 GetRemainingSize() const
	{
		return (char*)GetEndAddress()-(char*)m_pCurrentStart;
	}
	UINT64 GetFileSize() const;
	void SetFileHandle(HANDLE hFile);
private:
	LPVOID MapView();
	BOOL Alloc(UINT64 size);
	UINT m_uFlags;
	HANDLE m_hFile;
	HANDLE m_hFileMapping;
	LPVOID m_pStart;
	LPVOID m_pEnd;
	LPVOID m_pCurrentStart;
	UINT64 m_uFileOffset;
	UINT64 m_uMapSize;
	DWORD m_dwMapViewDesiredAccess;
	lstring mFilePath;
};
