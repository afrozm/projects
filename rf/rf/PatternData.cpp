#include "stdafx.h"
#include "PatternData.h"


PatternData::PatternData()
{
}


PatternData::~PatternData()
{
}

bool PatternData::SetupData(const Property & inProp, const lstring & name)
{
    mName = name;
    mFileExt = inProp.GetValue(_T("ext"));
    if (mFileExt.empty())
        mFileExt = mName;
    mStartPattern.BuildFromString(inProp.GetValue(_T("start")).c_str());
    m_iStartOffset = StringUtils::getLLfromStr(inProp.GetValue(_T("startOffset")).c_str());
    mEndPattern.BuildFromString(inProp.GetValue(_T("end")).c_str());
    m_iEndOffset = StringUtils::getLLfromStr(inProp.GetValue(_T("endOffset")).c_str());
    ResetState();
    return !mStartPattern.Size() && !mEndPattern.Size();
}

int PatternData::FindPattern(const void * buffer, size_t size)
{

    return -1;
}

void PatternData::ResetState()
{
    mFindState = FindStart;
    mBinaryFind.SetFindPattern(mStartPattern);
}

bool PatternData::operator<(const PatternData &data)
{
    return mStartPattern < data.mStartPattern;
}
