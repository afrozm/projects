#include "StdAfx.h"
#include "FileRecover.h"
#include <set>
#include "RecoverManager.h"
#include "JpgFileRecover.h"

//////////////////////////////////////////////////////////////////////////
/// CFileRecover

CFileRecover::CFileRecover()
    : m_pRecoverManager(NULL)
{

}


bool CFileRecover::SetupData(const Property & inProp)
{
    mPropData = inProp;
    return SetupData();
}


#define DEFINE_FN_FRGET_OBJECT(cn) static CFileRecover* get##cn() {return new cn;}

DEFINE_FN_FRGET_OBJECT(CFileRecoverSE)
DEFINE_FN_FRGET_OBJECT(CFileRecoverSS)
DEFINE_FN_FRGET_OBJECT(CJpgFileRecover)

CFileRecover* CFileRecover::GetFileRecover(const Property &inProp)
{
    typedef CFileRecover* (*GetCFileRecoverFn)();
    static std::map<lstring, GetCFileRecoverFn>mapStrFn;
    if (mapStrFn.empty()) {
        mapStrFn[_T("jpg")] = getCJpgFileRecover;
        mapStrFn[_T("png")] = getCFileRecoverSE;
        mapStrFn[_T("mp3")] = getCFileRecoverSE;
        mapStrFn[_T("mpg")] = getCFileRecoverSE;
        mapStrFn[_T("avi")] = getCFileRecoverSS;
        mapStrFn[_T("mp4")] = getCFileRecoverSS;
        mapStrFn[_T("mov")] = getCFileRecoverSS;
    }
    CFileRecover *pFileRecover(NULL);
    lstring name = StringUtils::ToLower(inProp.GetValue(_T("name")));
    auto cit = mapStrFn.find(name);
    if (cit != mapStrFn.end()) {
        pFileRecover = cit->second();
        if (!pFileRecover->SetupData(inProp))
            ReleaseFileRecover(pFileRecover); // Fail to setup data
    }
    return pFileRecover;
}

void CFileRecover::ReleaseFileRecover(CFileRecover *&pFileRecover)
{
    if (pFileRecover != NULL)
        delete pFileRecover;
    pFileRecover = NULL;
}

const BinaryData& CFileRecover::GetStartPattern()
{
    return mStartPattern;
}

void CFileRecover::SetRecoverManager(CRecoverManager *pRecoverManager)
{
    m_pRecoverManager = pRecoverManager;
    if (m_pRecoverManager != NULL)
        mFileSaver.SetSavePath(m_pRecoverManager->GetSavePath().c_str());
}

bool CFileRecover::SetupData()
{
    bool bSuccess(mStartPattern.BuildFromString(mPropData.GetValue(_T("start")).c_str()) > 3);

    mBinaryFind.SetFindPattern(mStartPattern);

    return bSuccess;
}

//////////////////////////////////////////////////////////////////////////
/// CFileRecoverSE
CFileRecoverSE::CFileRecoverSE()
    : m_iEndPatternSkipCount(0), m_iEndOffset(0), mFindState(FindStart), m_iCurrrentEndPatternSkipCount(0),
    m_iCurrentEndOffset(0)
{

}

bool CFileRecoverSE::ParseBuffer(BinaryData &inData)
{
    mBinaryFind.SetFindBuffer(); // reset
    mBinaryFind.SetFindBuffer(inData);
    long long findPos(0);
    if (mFindState != ReadTillEndOffset)
        findPos = mBinaryFind.FindNext();
    switch (mFindState)
    {
    case CFileRecoverSE::FindStart:
        if (findPos < 0)
            break;
        mFileSaver.OpenNew(mPropData.GetValue(_T("name")).c_str(), mPropData.GetValue(_T("fileNamePrefix")).c_str());
        mFileSaver.Write(mStartPattern, mStartPattern.Size());
        mBinaryFind.SetFindPattern(mEndPattern);
        findPos += mStartPattern.Size();
        mFindState = FindEnd;
        break;
    case CFileRecoverSE::FindEnd:
        if (findPos >= 0) {
            ++m_iCurrrentEndPatternSkipCount;
            findPos += mEndPattern.Size();
            if (m_iCurrrentEndPatternSkipCount > m_iEndPatternSkipCount)
                mFindState = ReadTillEndOffset;
        }
        else
            findPos = inData.Size();
        mFileSaver.Write(inData, findPos);
        break;
    case ReadTillEndOffset:
    {
        BinaryData remainigData(inData.GetDataRef(findPos, m_iCurrentEndOffset));
        findPos += remainigData.Size();
        m_iCurrentEndOffset -= (int)remainigData.Size();
        mFileSaver.Write(remainigData, remainigData.Size());
        if (m_iCurrentEndOffset <= 0) // Finished the file - save it
            ResetState();
        break;
    }
    default:
        break;
    }
    if (findPos > 0)
        inData = inData.GetDataRef(findPos);
    return mFindState != FindStart;
}

bool CFileRecoverSE::SetupData()
{
    bool bSuccess = mEndPattern.BuildFromString(mPropData.GetValue(_T("end")).c_str()) > 0;
    m_iEndPatternSkipCount = (int)StringUtils::getLLfromStr(mPropData.GetValue(_T("skipStart")).c_str());
    m_iEndOffset = (int)StringUtils::getLLfromStr(mPropData.GetValue(_T("endOffset")).c_str());
    ResetState();
    return bSuccess && __super::SetupData();
}

void CFileRecoverSE::ResetState()
{
    mFileSaver.Close();
    mFindState = FindStart;
    m_iCurrrentEndPatternSkipCount = 0;
    m_iCurrentEndOffset = m_iEndOffset;
    mBinaryFind.SetFindBuffer();
    mBinaryFind.SetFindPattern(mStartPattern);
}

//////////////////////////////////////////////////////////////////////////
/// CFileRecoverSS
CFileRecoverSS::CFileRecoverSS()
    : m_ullSectionSize(0)
{

}

bool CFileRecoverSS::ParseBuffer(BinaryData &inData)
{
    UNREFERENCED_PARAMETER(inData);
    return false;
}

bool CFileRecoverSS::SetupData()
{
    return __super::SetupData();
}

void CFileRecoverSS::ResetState()
{

}
