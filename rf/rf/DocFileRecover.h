#pragma once
#include "FileRecover.h"

class CDocFileRecover :
    public CFileRecoverSE
{
public:
    CDocFileRecover();
    ~CDocFileRecover();


protected:
    virtual bool SetupData() override;

    virtual bool ParseBuffer(BinaryData &inData) override;

    virtual void ResetState() override;

    typedef std::map<lstring, BinaryFind> MapBinaryFind;
    MapBinaryFind mExtList;
    bool mbExtFound;
};

