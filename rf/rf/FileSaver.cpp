#include "StdAfx.h"
#include "FileSaver.h"


CFileSaver::CFileSaver(void)
	: mid(1), m_hFile(INVALID_HANDLE_VALUE)
{
}


CFileSaver::~CFileSaver(void)
{
	Close();
}
void CFileSaver::Close()
{
	if (m_hFile != INVALID_HANDLE_VALUE)
		CloseHandle(m_hFile);
	m_hFile = INVALID_HANDLE_VALUE;
}
BOOL CFileSaver::OpenNew(LPCTSTR ext, LPCTSTR prefix)
{
	Close();

	Path fileName(m_SavePath.GetUniqueFileName(mid, ext, prefix));

	m_hFile = CreateFile(fileName.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL);

	return m_hFile != INVALID_HANDLE_VALUE;
}
BOOL CFileSaver::SetSavePath(LPCTSTR saveDir)
{
	BOOL bSucess(FALSE);

	if (saveDir) {
		Path pSaveDir(saveDir);
		if (pSaveDir.IsDir()) {
			m_SavePath = pSaveDir;
			bSucess = TRUE;
		}
	}
	return bSucess;
}
BOOL CFileSaver::Write(const void *buffer, int size)
{
	DWORD nByteWrite(0);

	WriteFile(m_hFile, buffer, size, &nByteWrite, NULL);

	return nByteWrite == size;
}