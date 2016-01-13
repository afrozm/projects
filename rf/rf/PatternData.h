#pragma once

#include "Property.h"
#include "BinaryFind.h"
#include "FileSaver.h"

class PatternData
{
public:
    PatternData();
    ~PatternData();
    bool SetupData(const Property &inProp, const lstring &name);
    int FindPattern(const void *buffer, size_t size);
    void ResetState();
    bool operator < (const PatternData &data);
private:
    BinaryData mStartPattern;
    int m_iStartOffset;
    BinaryData mEndPattern;
    int m_iSkipCount;
    int m_iEndOffset;
    enum State {
        FindStart,
        FindEnd
    } mFindState;
    lstring mName, mFileExt;
    BinaryFind mBinaryFind;
    CFileSaver mFileSaver;
};

