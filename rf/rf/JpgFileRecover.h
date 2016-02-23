#pragma once
#include "FileRecover.h"
class CJpgFileRecover :
    public CFileRecoverSE
{
public:
    CJpgFileRecover();
    ~CJpgFileRecover();

    virtual bool ParseBuffer(BinaryData &inData) override;

    virtual void ResetState() override;
protected:
    BinaryFind mBinaryFindExif;
    BinaryData mDataExif;
    size_t mExifFindLimit;
};

