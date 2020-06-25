#include "stdafx.h"
#include "JpgFileRecover.h"


CJpgFileRecover::CJpgFileRecover()
    : mExifFindLimit(0)
{
    mDataExif.BuildFromString(_T("Exif"), true);
    mBinaryFindExif.SetFindPattern(mDataExif, mDataExif.Size());
}


CJpgFileRecover::~CJpgFileRecover()
{
}

bool CJpgFileRecover::ParseBuffer(BinaryData &inData)
{
    BinaryData data(inData, inData.Size(), false);
    State oldState(GetState());
    bool bParseBuffer(__super::ParseBuffer(inData));
    if (m_iEndPatternSkipCount <= 0 && mExifFindLimit < 256) {
        if (GetState() == FindEnd) {
            mBinaryFindExif.SetFindBuffer(oldState == FindStart ? inData : data, true);
            mExifFindLimit += mBinaryFindExif.GetTotalBufferSize();
            if (mBinaryFindExif.FindNext() >= 0)
                m_iEndPatternSkipCount = 1;
        }
    }
    return bParseBuffer;
}

void CJpgFileRecover::ResetState()
{
    __super::ResetState();
    mBinaryFindExif.SetFindBuffer();
    mExifFindLimit = 0;
}
