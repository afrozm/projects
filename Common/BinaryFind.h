#pragma once

#include "StringUtils.h"
#ifdef _WIN32
#include <windows.h>
#endif // _WIN32


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
    size_t DataSize() const;
    void SetDataSize(size_t dataSize);
    bool operator < (const BinaryData &inData) const;
    int Compare(const BinaryData &inData) const;
    unsigned char operator[](size_t index) const;
    const void* operator + (size_t inOffset) const;
    size_t BuildFromString(const TCHAR *pStr, bool asString = false);
    size_t ReadFromFile(FILE *pFile, size_t nBytesToRead = 0, long long fileOffset = -1);
#ifdef _WIN32
    size_t ReadFromFile(HANDLE hFile, size_t nBytesToRead = 0, long long fileOffset = -1);
    size_t ReadFromResource(LPCTSTR lpName, LPCTSTR lpType, HMODULE hModule = NULL);
#endif // _WIN32
    BinaryData GetDataRef(size_t offset = 0, size_t inSize = -1);
    size_t Append(const BinaryData &inData);
    void Clear(); // Clears data, does not erase memory
    lstring HexDump(long long startAddress = 0, unsigned hexWidth = 16) const;
private:
    VecChar mBuffer;
    const void *m_pBuffer;
    size_t mBufferSize, mDataSize;
};


////////////////////////////// BinaryFind //////////////////////////////

class BinaryFind
{
public:
    BinaryFind(const void *pBuffer = NULL, size_t bufLen = 0);
    ~BinaryFind();
    long long FindNext();
    void SetFindPattern(const BinaryData &inFindPatter);
    void SetFindPattern(const void *pBuffer, size_t bufLen);
    void SetFindPattern(const char *str);
    bool HasFindPattern() const { return mFindPattern.Size() > 0; }
    void SetFindBuffer(const void *buffer = NULL, size_t size = 0, bool bResetOffset = false);
    void SetFindBuffer(const std::vector<char> &buffer, bool bResetOffset = false);
    void SetFindBuffer(const BinaryData &inFindBuffer, bool bResetOffset = false);
    size_t GetCurrentBufferIndex() const; // get position index to find pattern
    void SetCurrentBufferIndex(size_t currentIndex); // set start position to start find pattern
    size_t GetTotalBufferSize() const { return mCurrentBufferSize; }
private:
    unsigned char GetAt(size_t index);
    void ComputeKMPTable();
    BinaryData mFindPattern;
    std::vector<int> mVecKMPTable;
    long long mOffSet;
    const unsigned char *m_pCurrentBuffer;
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
