#pragma once
#include "BinaryFind.h"
///////////////////////////// DataReader /////////////////////////////////////////////

class DataReader
{
public:
    // return number of bytes read or -1 for no more data
    virtual size_t Read(void *pBuffer, size_t bufferSize, size_t bytesToRead = -1) = 0;
    size_t ReadData(BinaryData &outData, size_t bytesToRead = -1);
    virtual long long SetPointer(long long offset = 0, int seekType = SEEK_SET) = 0;
    virtual long long GetPointer() const = 0;
    virtual long long GetSize() = 0;
};

///////////////////////////// BinaryDataReader /////////////////////////////////////////////

class BinaryDataReader : public DataReader
{
public:
    BinaryDataReader();
    void SetData(const BinaryData *pData);
    virtual size_t Read(void *pBuffer, size_t bufferSize, size_t bytesToRead = -1) override;

    virtual long long SetPointer(long long offset = 0, int seekType = SEEK_SET) override;

    virtual long long GetPointer() const override;

    virtual long long GetSize() override;

private:
    size_t m_uCurPos;
    const BinaryData *m_pData;
};

///////////////////////////// FileDataReader /////////////////////////////////////////////

class FileDataReader : public DataReader
{
public:
    FileDataReader();
    ~FileDataReader();
    BOOL SetFile(LPCTSTR pFilePath);
    virtual size_t Read(void *pBuffer, size_t bufferSize, size_t bytesToRead = -1) override;

    virtual long long SetPointer(long long offset = 0, int seekType = SEEK_SET) override;

    virtual long long GetPointer() const override;

    virtual long long GetSize() override;
private:
    HANDLE m_hFile;
};
