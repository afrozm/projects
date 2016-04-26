#include "stdafx.h"
#include "BinaryFind.h"



BinaryData::BinaryData(const void * pBuffer, size_t bufLen, bool bStore)
    : m_pBuffer(pBuffer), mBufferSize(bufLen), mDataSize(bufLen)
{
    SetData(pBuffer, bufLen, bStore);
}

void BinaryData::SetData(const void *pBuffer /*= NULL*/, size_t bufLen /*= 0*/, bool bStore /*= true*/)
{
    mBuffer.clear();
    mBufferSize = bufLen;
    mDataSize = 0;
    bool bSetDataSize(true);
    if (!bStore) {
        m_pBuffer = pBuffer;
        if (mBufferSize <= 0)
            m_pBuffer = NULL;
        if (m_pBuffer == NULL)
            mBufferSize = 0;
    }
    else
    {
        m_pBuffer = NULL;
        if (mBufferSize > 0) {
            mBuffer.resize(mBufferSize);
            if (pBuffer != NULL)
                memcpy_s(&mBuffer[0], mBuffer.size(), pBuffer, bufLen);
            else
                bSetDataSize = false;
        }
    }
    if (bSetDataSize)
        mDataSize = Size();
}

BinaryData::~BinaryData()
{
    SetData();
}

BinaryData::operator const void*() const
{
    if (mBuffer.size() > 0)
        return &mBuffer[0];
    return m_pBuffer;
}

BinaryData::operator void*() const
{
    return (void *)(const void *)*this;
}

size_t BinaryData::Size() const
{
    return mBufferSize;
}

size_t BinaryData::DataSize() const
{
    return mDataSize;
}

bool BinaryData::operator<(const BinaryData & inData) const
{
    return Compare(inData) < 0;
}

int BinaryData::Compare(const BinaryData & inData) const
{
    int diff((int)DataSize() - (int)inData.DataSize());
    if (!diff && DataSize() > 0)
        diff = memcmp((const void *)*this, (const void *)inData, DataSize());
    return diff;
}

unsigned char BinaryData::operator[](size_t index) const
{
    unsigned char ch = 0;
    if (index < DataSize())
        ch = ((const unsigned char *)((const void *)*this))[index];
    return ch;
}

const void* BinaryData::operator+(size_t inOffset) const
{
    if (inOffset < Size())
        return (const unsigned char *)((const void *)*this) + inOffset;
    return NULL;
}

size_t BinaryData::BuildFromString(const TCHAR * pStr, bool asString)
{
    SetData();
    mBufferSize = BinaryDataUtil::StrToBuffer(pStr, mBuffer, asString);
    mDataSize = mBufferSize;
    return DataSize();
}

size_t BinaryData::ReadFromFile(FILE *pFile, size_t nBytesRead /* = 0 */, long long fileOffset /* = -1 */)
{
    mDataSize = 0;
    if (pFile == NULL)
        return 0;
    if (nBytesRead <= 0 || nBytesRead > Size())
        nBytesRead = Size();
    if (nBytesRead == 0)
        return 0;
    if (fileOffset >= 0)
        _fseeki64(pFile, fileOffset, SEEK_SET);
    nBytesRead = fread_s(*this, Size(), 1, nBytesRead, pFile);
    mDataSize = nBytesRead;
    return nBytesRead;
}

size_t BinaryData::ReadFromFile(HANDLE hFile, size_t nBytesRead, long long fileOffset)
{
    mDataSize = 0;
    if (hFile == NULL || hFile == INVALID_HANDLE_VALUE)
        return 0;
    if (nBytesRead <= 0 || nBytesRead > Size())
        nBytesRead = Size();
    if (nBytesRead == 0)
        return 0;
    if (fileOffset >= 0) {
        LARGE_INTEGER li;
        li.QuadPart = fileOffset;
        SetFilePointerEx(hFile, li, NULL, FILE_BEGIN);
    }
    DWORD dwRead(0);
    ReadFile(hFile, *this, (DWORD)Size(), &dwRead, NULL);
    nBytesRead = dwRead;
    mDataSize = nBytesRead;
    return nBytesRead;
}

size_t BinaryData::ReadFromResource(LPCTSTR lpName, LPCTSTR lpType, HMODULE hModule /* = NULL */)
{
    SetData();
    HRSRC hRSRC(FindResource(hModule, lpName, lpType));
    if (hRSRC != NULL) {
        HGLOBAL hRes(LoadResource(hModule, hRSRC));
        if (hRes != NULL) {
            LPVOID pData(LockResource(hRes));
            if (pData != NULL) {
                SetData(pData, SizeofResource(hModule, hRSRC), true);
            }
        }
    }
    return Size();
}

BinaryData BinaryData::GetDataRef(size_t offset, size_t inSize)
{
    if (inSize < 0 || inSize > Size())
        inSize = Size();
    if (offset > Size())
        offset = Size();
    if (offset + inSize > Size())
        inSize = Size() - offset;
    return BinaryData(*this + offset, inSize, false);
}

size_t BinaryData::Append(const BinaryData &inData)
{
    if (inData.DataSize() == 0)
        return 0;

    if (m_pBuffer != NULL)
        SetData(m_pBuffer, DataSize());
    if (Size() - DataSize() < inData.DataSize()) {
        mBuffer.resize(Size() + inData.DataSize(), 0);
        mBufferSize = mBuffer.size();
    }
    memcpy_s((void *)(*this + DataSize()), Size() - DataSize(), inData, inData.DataSize());
    mDataSize += inData.DataSize();

    return DataSize();
}

void BinaryData::Clear()
{
    if (m_pBuffer != NULL)
        m_pBuffer = NULL;
    mDataSize = 0;
}

////////////////////////////// BinaryFind //////////////////////////////

BinaryFind::BinaryFind(const void *pBuffer /* = NULL */, size_t bufLen /* = 0 */)
    : mOffSet(0), m_pCurrentBuffer(NULL), mCurrentBufferSize(0), mFindPatternIndex(0), mCurrentBufferIndex(0)
{
    SetFindPattern(pBuffer, bufLen);
}


BinaryFind::~BinaryFind()
{
}

long long BinaryFind::FindNext()
{
    if (m_pCurrentBuffer == NULL || mCurrentBufferSize <= 0)
        return -1;
    const size_t m = mFindPattern.Size();
    size_t i = mCurrentBufferIndex;
    size_t k = mFindPatternIndex;
    size_t size = GetTotalBufferSize();
    long long findPos(-1);
    while (i < size)
    {
        if (k == -1)
        {
            i++;
            k = 0;
        }
        else if (GetAt(i) == mFindPattern[k])
        {
            i++;
            k++;
            if (k == m) {
                mCurrentBufferIndex = i;
                findPos = mOffSet - size + mCurrentBufferIndex - mFindPattern.Size();
                mFindPatternIndex = 0; // Reset to start nex search from start
                break;
            }
        }
        else
            k = mVecKMPTable[k];
    }
    if (findPos < 0) // Not found
        mFindPatternIndex = k; // Save current index so that next to start from k
    return findPos;
}

void BinaryFind::SetFindPattern(const BinaryData & inFindPatter)
{
    SetFindPattern(inFindPatter, inFindPatter.Size());
}

void BinaryFind::SetFindPattern(const void *pBuffer, size_t bufLen)
{
    mFindPatternIndex = 0;
    mFindPattern.SetData(pBuffer, bufLen);
    ComputeKMPTable();
}

void BinaryFind::SetFindBuffer(const void * buffer, size_t size)
{
    m_pCurrentBuffer = (const unsigned char *)buffer;
    mCurrentBufferIndex = 0;
    if (m_pCurrentBuffer == NULL || size <= 0) {
        m_pCurrentBuffer = NULL;
        size = 0;
        mOffSet = 0;
    }
    mCurrentBufferSize = size;
    mOffSet += mCurrentBufferSize;
}

void BinaryFind::SetFindBuffer(const std::vector<char> &buffer)
{
    SetFindBuffer(buffer.size() > 0 ? &buffer[0] : NULL, buffer.size());
}

void BinaryFind::SetFindBuffer(const BinaryData &inFindBuffer)
{
    SetFindBuffer(inFindBuffer, inFindBuffer.Size());
}

size_t BinaryFind::GetCurrentBufferIndex() const
{
    return mCurrentBufferIndex;
}

void BinaryFind::SetCurrentBufferIndex(size_t currentIndex)
{
    mCurrentBufferIndex = currentIndex;
    if (mCurrentBufferIndex > mCurrentBufferSize)
        mCurrentBufferIndex = mCurrentBufferSize;
}

unsigned char BinaryFind::GetAt(size_t index)
{
    return m_pCurrentBuffer[index];
}

void BinaryFind::ComputeKMPTable()
{
    mVecKMPTable.clear();
    if (mFindPattern.Size() > 0) {
        int m((int)mFindPattern.Size()), k(0);
        mVecKMPTable.resize(m);
        mVecKMPTable[0] = -1;
        for (int i = 1; i < m; i++)
        {
            k = mVecKMPTable[i - 1];
            while (k >= 0)
            {
                if (mFindPattern[k] == mFindPattern[i - 1])
                    break;
                else
                    k = mVecKMPTable[k];
            }
            mVecKMPTable[i] = k + 1;
        }
    }
}


////////////////////////////// BinaryDataUtil //////////////////////////////

static char getHexByte(wchar_t ch)
{
    if (ch >= '0' && ch <= '9')
        return ch - '0';
    if (ch >= 'A' && ch <= 'F')
        return 0xA + ch - 'A';
    if (ch >= 'a' && ch <= 'f')
        return 0xA + ch - 'a';
    return (char)ch;
}
static char getHexByte(const wchar_t *inStr)
{
    char ch = 0;
    if (inStr) {
        for (int i = 0; i < 2 && inStr[i]; ++i) {
            ch <<= 4;
            ch |= getHexByte(inStr[i]);
        }
    }
    return ch;
}

size_t BinaryDataUtil::StrToBuffer(const TCHAR * pStr, VecChar & outBuffer, bool asString)
{
    if (pStr != NULL)
    {
        while (*pStr)
        {
            if (!asString) {
                STR_SKIP_SPACE(pStr);
                if (!STR_IS_VALID_PTR(pStr))
                    continue;
                outBuffer.push_back(getHexByte(pStr));
                STR_INR_PTR(pStr);
            }
            else
                outBuffer.push_back((char)*pStr);
            STR_INR_PTR(pStr);
        }
    }
    return outBuffer.size();
}
