#include "StdAfx.h"
#include "DataReader.h"

///////////////////////////// DataReader /////////////////////////////////////////////

///////////////////////////// BinaryDataReader /////////////////////////////////////////////

BinaryDataReader::BinaryDataReader()
    : m_uCurPos(0), m_pData(NULL)
{

}

void BinaryDataReader::SetData(const BinaryData * pData)
{
    m_pData = pData;
    m_uCurPos = 0;
}

size_t BinaryDataReader::Read(void *pBuffer, size_t bufferSize, size_t bytesToRead)
{
    if (pBuffer == NULL || bufferSize == 0 || bytesToRead == 0 || GetSize() == 0)
        return 0;
    if (bytesToRead > bufferSize || bytesToRead < 0)
        bytesToRead = bufferSize;
    const void *pSrcBuffer(*m_pData);
    if (bytesToRead + m_uCurPos > (size_t)GetSize())
        bytesToRead = (size_t)GetSize() - m_uCurPos;
    if (bytesToRead > 0)
        memcpy_s(pBuffer, bufferSize, pSrcBuffer, bytesToRead);
    m_uCurPos += bytesToRead;
    return bytesToRead;
}

long long BinaryDataReader::SetPointer(long long offset /* = 0 */, int seekType /* = SEEK_SET */)
{
    switch (seekType)
    {
    case SEEK_SET:
        m_uCurPos = (size_t)offset;
        break;
    case SEEK_CUR:
        m_uCurPos += (size_t)offset;
        break;
    case SEEK_END:
        m_uCurPos = (size_t)GetSize() - m_uCurPos;
        break;
    default:
        break;
    }
    if (m_uCurPos > (size_t)GetSize())
        m_uCurPos = (size_t)GetSize();
    return GetPointer();
}

long long BinaryDataReader::GetPointer() const
{
    return m_uCurPos;
}

long long BinaryDataReader::GetSize()
{
    if (m_pData == NULL)
        return 0;
    return m_pData->Size();
}

///////////////////////////// FileDataReader /////////////////////////////////////////////

FileDataReader::FileDataReader()
    : m_hFile(INVALID_HANDLE_VALUE)
{
}

FileDataReader::~FileDataReader()
{
    SetFile(NULL);
}

BOOL FileDataReader::SetFile(LPCTSTR pFilePath)
{
    if (m_hFile != INVALID_HANDLE_VALUE)
        CloseHandle(m_hFile);
    m_hFile = INVALID_HANDLE_VALUE;
    if (pFilePath != NULL)
        m_hFile = CreateFile(pFilePath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL,
            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    return pFilePath == NULL || m_hFile != INVALID_HANDLE_VALUE;
}

size_t FileDataReader::Read(void *pBuffer, size_t bufferSize, size_t bytesToRead)
{
    if (pBuffer == NULL || bufferSize == 0 || bytesToRead == 0 || m_hFile == INVALID_HANDLE_VALUE)
        return 0;
    if (bytesToRead > bufferSize || bytesToRead < 0)
        bytesToRead = bufferSize;
    DWORD nBytesRead(0);
    ReadFile(m_hFile, pBuffer, (DWORD)bytesToRead, &nBytesRead, NULL);
    bytesToRead = (size_t)nBytesRead;
    return bytesToRead;
}

long long FileDataReader::SetPointer(long long offset /*= 0*/, int seekType /*= SEEK_SET*/)
{
    LARGE_INTEGER liOffset, newOffset = { 0 };
    liOffset.QuadPart = offset;
    ::SetFilePointerEx(m_hFile, liOffset, &newOffset, seekType);
    return newOffset.QuadPart;
}

long long FileDataReader::GetPointer() const
{
    return ((FileDataReader*)this)->SetPointer(0, SEEK_CUR);
}

long long FileDataReader::GetSize()
{
    LARGE_INTEGER fileSize = { 0 };
    if (m_hFile != INVALID_HANDLE_VALUE) {
        GetFileSizeEx(m_hFile, &fileSize);
    }
    return fileSize.QuadPart;
}
