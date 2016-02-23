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
    const BinaryData& GetDataRead(bool bCurrent = true);
    const lstring& GetSavePath() const { return mStrSavePath; }
private:
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

