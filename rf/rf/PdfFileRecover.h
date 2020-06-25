#pragma once
#include "FileRecover.h"
class CPdfFileRecover :
    public CFileRecoverSE
{
public:
    CPdfFileRecover();
    ~CPdfFileRecover();

    virtual bool ParseBuffer(BinaryData &inData) override;
private:
    BinaryFind mObjFind;
};

