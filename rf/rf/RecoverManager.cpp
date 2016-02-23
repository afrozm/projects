#include "stdafx.h"
#include "RecoverManager.h"
#include <algorithm>
#include "resource.h"

CRecoverManager::CRecoverManager()
    : m_pInputFile(NULL), mCurrentDone(0), m_pActiveRecover(0), m_hInputFile(INVALID_HANDLE_VALUE), mTotal(0), mbCurrent(false)
{
}


CRecoverManager::~CRecoverManager()
{
    EndRecover();
    for (auto &rec : mVecFileRecovers)
        CFileRecover::ReleaseFileRecover(rec);
    mVecFileRecovers.clear();
}

static bool FileRecoverComarator(CFileRecover *a, CFileRecover *b)
{
    return a->GetStartPattern().Compare(b->GetStartPattern()) > 0;
}

void CRecoverManager::Initialize()
{
    for (auto &rec : mVecFileRecovers)
        CFileRecover::ReleaseFileRecover(rec);
    mVecFileRecovers.clear();
    PropertySet propSet;
    PropertySetStreamer propertySetStreamer;
    propertySetStreamer.SetPropertySetStream(propSet);
    BinaryData bd;
    bd.ReadFromResource(MAKEINTRESOURCE(IDR_FTYP_DEFAULT), _T("FTYP"));
    propertySetStreamer.ReadFromString(StringUtils::UTF8ToUnicode(std::string((const char *)(const void *)bd, bd.Size()).c_str()));
    propertySetStreamer.ReadFromFile(Path::GetModuleFilePath().Parent().Append(_T("rf.ini")));
    for (auto &prop : propSet.GetMapProperty()) {
        prop.second.SetValue(_T("name"), prop.first, false);
        CFileRecover *pFileRecover(CFileRecover::GetFileRecover(prop.second));
        if (pFileRecover != NULL) {
            mVecFileRecovers.push_back(pFileRecover);
            pFileRecover->SetRecoverManager(this);
        }
    }
    std::sort(mVecFileRecovers.begin(), mVecFileRecovers.end(), FileRecoverComarator);
}

void CRecoverManager::SetInputFileHandle(HANDLE hFile)
{
    m_hInputFile = hFile;
}

bool CRecoverManager::BeginRecover()
{
    EndRecover();
    if (!Path(mStrInputFile).IsDir())
        _tfopen_s(&m_pInputFile, mStrInputFile.c_str(), _T("rb"));
    if (m_pInputFile != NULL)
        mTotal = Path(mStrInputFile).GetSize();
    if (mDataRead->Size() <= 0) {
        mDataRead[0].SetData(NULL, 64 * 1024); // 64k chunk
        mDataRead[1].SetData(NULL, mDataRead[0].Size());
    }
    return m_pInputFile != NULL || (m_hInputFile != NULL && m_hInputFile != INVALID_HANDLE_VALUE);
}

bool CRecoverManager::ProcessRecover()
{
    BinaryData &dataRead((BinaryData &)GetDataRead());
    size_t readSize(m_pInputFile ? dataRead.ReadFromFile(m_pInputFile) : dataRead.ReadFromFile(m_hInputFile));
    BinaryData currentData(dataRead, readSize, false);
    while (currentData.Size() > 0) {
        if (m_pActiveRecover != NULL)
            if (!m_pActiveRecover->ParseBuffer(currentData))
                m_pActiveRecover = NULL;
        if (m_pActiveRecover == NULL) {
            for (auto &rec : mVecFileRecovers) {
                if (rec->ParseBuffer(currentData)) {
                    m_pActiveRecover = rec;
                    break;
                }
            }
            if (m_pActiveRecover == NULL) // No recover find in this current buffer
                currentData.SetData();      // reset buffer to exit the loop to process next buffer
        }
    }
    mbCurrent = !mbCurrent;
    mCurrentDone += readSize;
    return readSize > 0;
}

void CRecoverManager::EndRecover()
{
    for (auto &rec : mVecFileRecovers)
        rec->ResetState();
    if (m_pInputFile != NULL)
        fclose(m_pInputFile);
    m_pInputFile = NULL;
    mCurrentDone = 0;
    mDataRead[0].SetData();
    mDataRead[1].SetData();
    mbCurrent = false;
}

const BinaryData& CRecoverManager::GetDataRead(bool bCurrent /*= true*/)
{
    return mDataRead[bCurrent ? mbCurrent : !mbCurrent];
}
