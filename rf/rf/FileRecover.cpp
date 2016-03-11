#include "StdAfx.h"
#include "FileRecover.h"
#include <set>
#include "RecoverManager.h"
#include "JpgFileRecover.h"
#include "DocFileRecover.h"
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
DEFINE_FN_FRGET_OBJECT(CFileRecoverMov)
DEFINE_FN_FRGET_OBJECT(CJpgFileRecover)
DEFINE_FN_FRGET_OBJECT(CDocFileRecover)

CFileRecover* CFileRecover::GetFileRecover(const Property &inProp)
{
    typedef CFileRecover* (*GetCFileRecoverFn)();
    static std::map<lstring, GetCFileRecoverFn>mapStrFn;
    if (mapStrFn.empty()) {
        mapStrFn[_T("jpg")] = getCJpgFileRecover;
        mapStrFn[_T("png")] = getCFileRecoverSE;
        mapStrFn[_T("mp3")] = getCFileRecoverSE;
        mapStrFn[_T("mpg")] = getCFileRecoverSE;
        mapStrFn[_T("avi")] = getCFileRecoverMov;
        mapStrFn[_T("mp4")] = getCFileRecoverMov;
        mapStrFn[_T("mov")] = getCFileRecoverMov;
        mapStrFn[_T("docx")] = getCDocFileRecover;
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

CRecoverManager* CFileRecover::GetRecoverManager() const
{
    return m_pRecoverManager;
}

bool CFileRecover::SetupData()
{
    bool bSuccess(mStartPattern.BuildFromString(mPropData.GetValue(_T("start")).c_str()) > 3);

    mBinaryFind.SetFindPattern(mStartPattern);
    ResetState();

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
        mFileSaver.Write(inData, (size_t)findPos);
        break;
    case ReadTillEndOffset:
    {
        BinaryData remainigData(inData.GetDataRef((size_t)findPos, m_iCurrentEndOffset));
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
        inData = inData.GetDataRef((size_t)findPos);
    return mFindState != FindStart;
}

bool CFileRecoverSE::SetupData()
{
    bool bSuccess = mEndPattern.BuildFromString(mPropData.GetValue(_T("end")).c_str()) > 0;
    m_iEndPatternSkipCount = (int)StringUtils::getLLfromStr(mPropData.GetValue(_T("skipStart")).c_str());
    m_iEndOffset = (int)StringUtils::getLLfromStr(mPropData.GetValue(_T("endOffset")).c_str());
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

#define MEDIA_SECTION_SIZE 8

CFileRecoverMov::CFileRecoverMov()
    : m_ullSectionSize(0), mbIsAVI(false), mSectionData(NULL, (size_t)MEDIA_SECTION_SIZE)
{

}

static bool isValidFOURCC(const BinaryData &inData, size_t offset = 0)
{
    size_t i = 0;
    for (; i < 4; ++i) {
        if (!isprint(inData[i + offset]))
            break;
    }
    return i == 4;
}

bool CFileRecoverMov::ParseBuffer(BinaryData &inData)
{
    mBinaryFind.SetFindBuffer(); // reset
    mBinaryFind.SetFindBuffer(inData);
    long long findPos(-1);
    switch (mFindState)
    {
    case CFileRecoverMov::FindStart:
    {
        findPos = mBinaryFind.FindNext();
        if (findPos < 0)
            break;
        if (!mbIsAVI) // for non-avi read size - 4 bytes before pattern matched
            GetRecoverManager()->GetData(mSectionData, (size_t)findPos - 4, 4, &inData);
        mFileSaver.OpenNew(mPropData.GetValue(_T("name")).c_str(), mPropData.GetValue(_T("fileNamePrefix")).c_str());
        mFindState = SectionSize;
        break;
    }
    case SectionSize:
        findPos = ReadSectionSize(&inData);
        if (mSectionData.DataSize() == MEDIA_SECTION_SIZE) {
            if (m_ullSectionSize == 0)// Not valid section
                ResetState();
            else {
                // Write section
                mFileSaver.Write(mSectionData, mSectionData.DataSize());
                if (!mbIsAVI && m_ullSectionSize != 1) // non-avi: size is includes section data
                    m_ullSectionSize -= mSectionData.DataSize();
                mSectionData.Clear();
                if (m_ullSectionSize != 1)
                    mFindState = Save;
            }
        }
        break;
    case Save:
    {
        size_t szWrite(m_ullSectionSize > inData.DataSize() ? inData.DataSize() : (size_t)m_ullSectionSize);
        mFileSaver.Write(inData, szWrite);
        m_ullSectionSize -= szWrite;
        findPos = szWrite;
        if (m_ullSectionSize == 0)
            mFindState = SectionSize;
    }
        break;
    }
    if (findPos > 0)
        inData = inData.GetDataRef((size_t)findPos);
    return mFindState != FindStart;
}

void CFileRecoverMov::ResetState()
{
    mFileSaver.Close();
    mFindState = FindStart;
    m_ullSectionSize = 0;
    mBinaryFind.SetFindBuffer();
}

bool CFileRecoverMov::SetupData()
{
    bool bSuccess(__super::SetupData());
    BinaryData riff;
    riff.BuildFromString(_T("RIFF"), true);
    mbIsAVI = mStartPattern.Compare(riff) == 0;
    mSectionData.SetData(NULL, MEDIA_SECTION_SIZE);
    mBinaryFind.SetFindPattern(mStartPattern);
    return bSuccess;
}

size_t CFileRecoverMov::ReadSectionSize(const BinaryData *pCurrentData)
{
    size_t szRead(0);
    if (mSectionData.DataSize() < MEDIA_SECTION_SIZE) {
        BinaryData data(GetRecoverManager()->GetData(0, MEDIA_SECTION_SIZE-mSectionData.DataSize(), pCurrentData));
        szRead += data.DataSize();
        mSectionData.Append(data);
        if (mSectionData.DataSize() == MEDIA_SECTION_SIZE) {
            if (m_ullSectionSize == 0) {
                size_t offsets[2] = { 0,4 };
                m_ullSectionSize = BinaryDataUtil::GetValueType<unsigned int>(mSectionData, offsets[mbIsAVI]);
                if (mbIsAVI)
                    m_ullSectionSize = BinaryDataUtil::ToggleEndian((unsigned int)m_ullSectionSize);
                if (!isValidFOURCC(mSectionData, offsets[!mbIsAVI]))
                    m_ullSectionSize = 0; // EOF
            }
            else {
                m_ullSectionSize = BinaryDataUtil::GetValueType<unsigned long long>(mSectionData);
                if (mbIsAVI)
                    m_ullSectionSize = BinaryDataUtil::ToggleEndian((unsigned int)m_ullSectionSize);
            }
        }
    }
    return szRead;
}
