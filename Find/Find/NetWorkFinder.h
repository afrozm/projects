#pragma once

#include "Finder.h"

typedef int (*NetWorkFindCallBack)(LPNETRESOURCE, void *pUserParam);

LPNETRESOURCE CopyLPNetResource(LPNETRESOURCE);
const LPCTSTR kNETRESOURCEComment = _T("FreeThisResource");

void FreeLPNetResource(LPNETRESOURCE);

class CNetWorkFinder : public CFinder
{
public:
	CNetWorkFinder(NetWorkFindCallBack nfcb, bool bRecursive = true, void *pUserParam = NULL);
	int StartFind(LPNETRESOURCE netRes = NULL);
	~CNetWorkFinder(void);
private:
	NetWorkFindCallBack mNetWorkFindCallBack;
};

DWORD NetWorkOpenConnect(LPCTSTR name);
DWORD NetWorkCloseConnection(LPCTSTR name);