#include "stdafx.h"
#include "BinaryFind.h"


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
    const size_t m = mFindPattern.size();
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
                findPos = mOffSet - size + mCurrentBufferIndex - mFindPattern.size();
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

void BinaryFind::SetFindPattern(const void *pBuffer, size_t bufLen)
{
    mFindPatternIndex = 0;
    mFindPattern.clear();
    if (pBuffer != NULL && bufLen > 0) {
        mFindPattern.resize(bufLen);
        memcpy_s(&mFindPattern[0], bufLen, pBuffer, bufLen);
    }
    ComputeKMPTable();
}

void BinaryFind::SetFindBuffer(const void * buffer, size_t size)
{
    const char *target((const char *)buffer);
    m_pCurrentBuffer = target;
    mCurrentBufferIndex = 0;
    if (m_pCurrentBuffer == NULL || size <= 0) {
        m_pCurrentBuffer = NULL;
        size = 0;
        mOffSet = 0;
    }
    mCurrentBufferSize = size;
    mOffSet += mCurrentBufferSize;
}

char BinaryFind::GetAt(size_t index)
{
    return m_pCurrentBuffer[index];
}

void BinaryFind::ComputeKMPTable()
{
    mVecKMPTable.clear();
    if (mFindPattern.size() > 0) {
        int m((int)mFindPattern.size()), k(0);
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
