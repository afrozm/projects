#pragma once

#include <vector>

class BinaryFind
{
public:
    BinaryFind(const void *pBuffer = NULL, size_t bufLen = 0);
    ~BinaryFind();
    long long FindNext();
    void SetFindPattern(const void *pBuffer, size_t bufLen);
    bool HasFindPattern() const { return !mFindPattern.empty(); }
    void SetFindBuffer(const void *buffer, size_t size);
private:
    char GetAt(size_t index);
    size_t GetTotalBufferSize() const { return mCurrentBufferSize; }
    void ComputeKMPTable();
    typedef std::vector<char> Buffer;
    Buffer mFindPattern;
    std::vector<int> mVecKMPTable;
    long long mOffSet;
    const char *m_pCurrentBuffer;
    size_t mCurrentBufferSize;
    size_t mCurrentBufferIndex;
    size_t mFindPatternIndex; // This represent start index in mFindPattern in FindeNext calls
};

