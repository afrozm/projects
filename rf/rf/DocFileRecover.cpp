#include "stdafx.h"
#include "DocFileRecover.h"
#include <set>

CDocFileRecover::CDocFileRecover()
    : mbExtFound(false)
{
}


CDocFileRecover::~CDocFileRecover()
{
}

bool CDocFileRecover::SetupData()
{
    StringUtils::VecString extList;
    bool bSuccess(StringUtils::SplitString(extList, mPropData.GetValue(_T("extList"))) > 0);
    std::set<lstring> setExtList(extList.begin(), extList.end());
    for (const auto &ext : setExtList) {
        BinaryData bd;
        bSuccess = bd.BuildFromString(mPropData.GetValue(ext).c_str(), true) > 0;
        if (!bSuccess)
            break;
        mExtList[ext].SetFindPattern(bd);
    }
    return bSuccess && __super::SetupData();
}

bool CDocFileRecover::ParseBuffer(BinaryData &inData)
{
    BinaryData data(inData);
    State curState(GetState());
    bool bRet(__super::ParseBuffer(inData));
    if (!mbExtFound && (FindEnd == GetState()|| curState==FindEnd)) {
        for (auto &ext : mExtList) {
            BinaryFind &bf(ext.second);
            bf.SetFindBuffer(curState == FindEnd ? data : inData);
            mbExtFound = bf.FindNext() > 0;
            if (mbExtFound) {
                mFileSaver.SetExt(ext.first.c_str());
                break;
            }
        }
    }
    return bRet;
}

void CDocFileRecover::ResetState()
{
    __super::ResetState();
    mbExtFound = false;
    for (auto &ext : mExtList)
        ext.second.SetFindBuffer();
}
