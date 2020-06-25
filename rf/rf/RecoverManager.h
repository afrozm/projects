#pragma once

#include "FileRecover.h"
#include <vector>

class CRecoverManager
{
public:
    CRecoverManager();
    ~CRecoverManager();
    void Initialize();
    void SetSavePath(const lstring &inSavePath) { mStrSavePath = inSavePath; }
    void SetInputFilePath(const lstring &inInputPath) { mStrInputFile = inInputPath; }
    void SetInputFileHandle(HANDLE hFile);
    bool BeginRecover();
    bool ProcessRecover();
    UINT64 GetTotal() const { return mTotal; }
    void SetTotal(UINT64 total) { mTotal = total; }
    UINT64 GetCurrentDone() const { return mCurrentDone; }
    void EndRecover();
    const lstring& GetSavePath() const { return mStrSavePath; }
    BinaryData GetData(size_t offset = 0, size_t len = -1, const BinaryData *pCurrentData = NULL);
    size_t GetData(BinaryData &outData, size_t offset = 0, size_t len = -1, const BinaryData *pCurrentData = NULL);
private:
    const BinaryData& GetDataRead(bool bCurrent = true);
    lstring mStrSavePath, mStrInputFile;
    typedef std::vector<CFileRecover*> VecFileRecover;
    CFileRecover *m_pActiveRecover;
    VecFileRecover mVecFileRecovers;
    FILE *m_pInputFile;
    HANDLE m_hInputFile; // either m_pInputFile or m_hInputFile
    UINT64 mCurrentDone, mTotal;
    BinaryData mDataRead[2];
    bool mbCurrent;
};

