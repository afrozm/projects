#include "StdAfx.h"
#include "FileSaver.h"
#include "BinaryFind.h"

CFileSaver::CFileSaver(void)
	: mid(1), m_hFile(INVALID_HANDLE_VALUE), m_sNewExt(NULL)
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
    if (m_sNewExt != NULL && m_FilePath.Exists() && m_FilePath.CompareExtension(m_sNewExt)) {
        Path newFileName(m_SavePath.GetUniqueFileName(mid, m_sNewExt, m_FilePath.FileNameWithoutExt().c_str()));
        m_FilePath.Move(newFileName);
    }
    m_sNewExt = NULL;
	m_hFile = INVALID_HANDLE_VALUE;
    m_FilePath.clear();
}

void CFileSaver::SetExt(LPCTSTR newExt)
{
    m_sNewExt = newExt;
}

BOOL CFileSaver::OpenNew(LPCTSTR ext, LPCTSTR prefix)
{
	Close();

    m_FilePath = m_SavePath.GetUniqueFileName(mid, ext, prefix);

	m_hFile = CreateFile(m_FilePath.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL);

	return m_hFile != INVALID_HANDLE_VALUE;
}

BOOL CFileSaver::WriteFrom(LPCTSTR inFromFile, long long startPos /*= 0*/, long long endPos /*= -1*/)
{
    if (inFromFile == NULL || Path(inFromFile).IsDir())
        return FALSE;
    if (endPos < 0)
        endPos = Path(inFromFile).GetSize();
    if (endPos < startPos)
        std::swap(startPos, endPos);
    long long size(endPos - startPos);
    if (size < 0)
        return FALSE;
    const long long kBufSize(4 * 1024 * 1024);
    long long bufSize(size > kBufSize ? kBufSize : size);
    BinaryData data(NULL, (size_t)bufSize);
    FILE *pFile = NULL;
    _tfopen_s(&pFile, inFromFile, _T("rb"));
    if (pFile == NULL)
        return FALSE;

    while (size > 0)
    {
        size_t sizeRead(data.ReadFromFile(pFile, (size_t)size, startPos));
        if (sizeRead == 0)
            break;
        Write(data, sizeRead);
        size -= sizeRead;
    }

    fclose(pFile);

    return size <= 0;
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
BOOL CFileSaver::Write(const void *buffer, size_t size)
{
    if (buffer == NULL || size == 0)
        return FALSE;
	DWORD nByteWrite(0);

	WriteFile(m_hFile, buffer, (DWORD)size, &nByteWrite, NULL);

	return nByteWrite == size;
}