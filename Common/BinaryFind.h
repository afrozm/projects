#pragma once

#include "StringUtils.h"

typedef std::vector<unsigned char> VecChar;

class BinaryData
{
public:
    BinaryData(const void *pBuffer = NULL, size_t bufLen = 0, bool bStore = true);
    void SetData(const void *pBuffer = NULL, size_t bufLen = 0, bool bStore = true);
    ~BinaryData();
    operator const void*() const;
    operator void*() const;
    size_t Size() const;
    bool operator < (const BinaryData &inData) const;
    int Compare(const BinaryData &inData) const;
    unsigned char operator[](size_t index) const;
    size_t BuildFromString(const TCHAR *pStr, bool asString = false);
    size_t ReadFromFile(FILE *pFile, size_t nBytesToRead = 0, long long fileOffset = -1);
private:
    VecChar mBuffer;
    const void *m_pBuffer;
    size_t mSize;
};


////////////////////////////// BinaryFind //////////////////////////////

class BinaryFind
{
public:
    BinaryFind(const void *pBuffer = NULL, size_t bufLen = 0);
    ~BinaryFind();
    long long FindNext();
    void SetFindPattern(const void *pBuffer, size_t bufLen);
    bool HasFindPattern() const { return mFindPattern.Size() > 0; }
    void SetFindBuffer(const void *buffer, size_t size);
    void SetFindBuffer(const std::vector<char> &buffer);
private:
    char GetAt(size_t index);
    size_t GetTotalBufferSize() const { return mCurrentBufferSize; }
    void ComputeKMPTable();
    BinaryData mFindPattern;
    std::vector<int> mVecKMPTable;
    long long mOffSet;
    const char *m_pCurrentBuffer;
    size_t mCurrentBufferSize;
    size_t mCurrentBufferIndex;
    size_t mFindPatternIndex; // This represent start index in mFindPattern in FindeNext calls
};

namespace BinaryDataUtil {
    size_t StrToBuffer(const TCHAR *pStr, VecChar &outBuffer, bool asString = false);

    template <class T>
    T ToggleEndian(T t, unsigned nBits = 3 /* 1 << nBits - number if bits to swap*/)
    {
        unsigned sizeInBytes = sizeof(T);
        while (nBits) { // Sanity check - normalize the no. of bits if it greater than total number of bits
            if ((sizeInBytes << 3) <= ((unsigned)1 << nBits))
                --nBits;
            else break;
        }
        const unsigned mask = (1 << (1 << nBits)) - 1;	// Set all 1s with number of bits to swap
        const unsigned loopCount = (sizeInBytes << 3) / ((1 << nBits) << 1);
        sizeInBytes = (sizeInBytes << 3) / (1 << nBits); // Reset size in bytes multiple of bit length
        T retVal(0); // initialize output value to zero
        for (unsigned i = 0; i < loopCount; ++i) {
            T bitDiff = (sizeInBytes - 1 - (i << 1)) << nBits;
            T higherBits = (sizeInBytes - 1 - i) << nBits;
            retVal |= (t & (mask << (i << nBits))) << bitDiff; // Store lower bits of input value to higher bits of output
            retVal |= (t & (mask << higherBits)) >> bitDiff; // Store higher bits of input value to lower bits of output
        }
        return retVal;
    }

    template<class T>
    T GetValueType(const BinaryData & inData, size_t offset = 0)
    {
        T retVal(0);
        for (int i = 0; i < sizeof(T); ++i) {
            retVal <<= 8;
            retVal |= inData[i + offset];
        }
        return retVal;
    }
}
