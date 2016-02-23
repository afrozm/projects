#pragma once
#include "BinaryFind.h"
#include "FileSaver.h"
#include "Property.h"

class CRecoverManager;

class CFileRecover
{
public:
    CFileRecover();
    // inData to process data
    // return true if data processed
    // inData - modified if processed some data and returns remaining data
    virtual bool ParseBuffer(BinaryData &inData) = 0;
    virtual void ResetState() = 0;
    bool SetupData(const Property &inProp);
    static CFileRecover* GetFileRecover(const Property &inProp);
    static void ReleaseFileRecover(CFileRecover *&pFileRecover);
    const BinaryData& GetStartPattern();
    void SetRecoverManager(CRecoverManager *pRecoverManager);
protected:
    virtual bool SetupData();
    BinaryData mStartPattern;
    BinaryFind mBinaryFind;
    CFileSaver mFileSaver;
    Property mPropData;
    CRecoverManager *m_pRecoverManager;
};

//////////////////////////////////////////////////////////////////////////
/// CFileRecoverSE
// Start and end pattern based recover
// covers: jpg, mp3, mpg
class CFileRecoverSE : public CFileRecover
{
public:
    CFileRecoverSE();

    virtual bool ParseBuffer(BinaryData &inData) override;
    virtual void ResetState() override;

protected:
    virtual bool SetupData() override;


    BinaryData mEndPattern;
    int m_iEndPatternSkipCount, m_iCurrrentEndPatternSkipCount;
    int m_iEndOffset, m_iCurrentEndOffset;
    enum State {
        FindStart,
        FindEnd,
        ReadTillEndOffset,
    } mFindState;

    State GetState() const { return mFindState; }
};

//////////////////////////////////////////////////////////////////////////
/// CFileRecoverSS
// Start and size pattern based recover
// covers: avi, mp4, mov
class CFileRecoverSS : public CFileRecover
{
public:
    CFileRecoverSS();
    virtual bool ParseBuffer(BinaryData &inData) override;
    virtual void ResetState() override;

protected:
    virtual bool SetupData() override;


    unsigned long long m_ullSectionSize;
};
