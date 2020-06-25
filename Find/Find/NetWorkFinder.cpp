#include "StdAfx.h"
#include "NetWorkFinder.h"

CNetWorkFinder::CNetWorkFinder(NetWorkFindCallBack nfcb, bool bRecursive, void *pUserParam)
: CFinder(_T("*"), NULL, bRecursive, pUserParam), mNetWorkFindCallBack(nfcb)
{
}

CNetWorkFinder::~CNetWorkFinder(void)
{
}
static int CompareNetResource(LPNETRESOURCE lpnr1, LPNETRESOURCE lpnr2)
{
	return lstrcmpi(lpnr1->lpRemoteName, lpnr2->lpRemoteName);
}
int CNetWorkFinder::StartFind(LPNETRESOURCE lpnr)
{
	DWORD dwResult, dwResultEnum;
	HANDLE hEnum;
	DWORD cbBuffer = 16384;      // 16K is a good size
	DWORD cEntries = (DWORD)-1;         // enumerate all possible entries
	LPNETRESOURCE lpnrLocal;     // pointer to enumerated structures
	DWORD i;
	int count = 0;
	//
	// Call the WNetOpenEnum function to begin the enumeration.
	//
	dwResult = WNetOpenEnum(RESOURCE_GLOBALNET, // all network resources
						  RESOURCETYPE_ANY,   // all resources
						  0,        // enumerate all resources
						  lpnr,     // NULL first time the function is called
						  &hEnum);  // handle to the resource

    TCHAR szDescription[256];
    TCHAR szProvider[256];
	DWORD dwWNetResult, dwLastError; 

	if (dwResult != NO_ERROR)
	{  
        dwWNetResult = WNetGetLastError(&dwLastError, // error code
            szDescription,  // buffer for error description 
            sizeof(szDescription),  // size of error buffer
            szProvider,     // buffer for provider name 
            sizeof(szProvider));    // size of name buffer
	//
	// Process errors with an application-defined error handler.
	//
	return count;
	}
	//
	// Call the GlobalAlloc function to allocate resources.
	//
	lpnrLocal = (LPNETRESOURCE) GlobalAlloc(GPTR, cbBuffer);
	if (lpnrLocal == NULL) 
		return count;

	do
	{  
	//
	// Initialize the buffer.
	//
	ZeroMemory(lpnrLocal, cbBuffer);
	//
	// Call the WNetEnumResource function to continue
	//  the enumeration.
	//
	dwResultEnum = WNetEnumResource(hEnum,      // resource handle
									&cEntries,  // defined locally as -1
									lpnrLocal,  // LPNETRESOURCE
									&cbBuffer); // buffer size
	//
	// If the call succeeds, loop through the structures.
	//
	bool bRecursive = m_bRecursive;
	if (dwResultEnum == NO_ERROR)
	{
		count += cEntries;
			// Sort as per thier name as 
		qsort(lpnrLocal, cEntries, sizeof(NETRESOURCE),
			(GenericCompareFn)CompareNetResource);
		for(i = 0; i < cEntries && !m_bAborted; i++)
		{
			// Call callback
			if (mNetWorkFindCallBack != NULL) {
				switch (mNetWorkFindCallBack(lpnrLocal+i,m_pUserParam)) {
				case FCB_ABORT:
					m_bAborted = true;
					break;
				case FCB_DORECURSIVE:
					bRecursive = true;
					break;
				case FCB_NORECURSIVE:
					bRecursive = false;
					break;
				}
			}
			if (!m_bAborted && cEntries > 0 && bRecursive) {
				if(RESOURCEUSAGE_CONTAINER == (lpnrLocal[i].dwUsage
											   & RESOURCEUSAGE_CONTAINER))
					count += StartFind(&lpnrLocal[i]);
			}
		}
	}
	// Process errors.
	//
	else if (dwResultEnum != ERROR_NO_MORE_ITEMS)
	{
	  break;
	}
	}
	//
	// End do.
	//
	while(dwResultEnum != ERROR_NO_MORE_ITEMS && !m_bAborted);
	//
	// Call the GlobalFree function to free the memory.
	//
	GlobalFree((HGLOBAL)lpnrLocal);
	//
	// Call WNetCloseEnum to end the enumeration.
	//
	dwResult = WNetCloseEnum(hEnum);


	return count;
}

LPNETRESOURCE CopyLPNetResource(LPNETRESOURCE lpnr)
{
	LPNETRESOURCE retVal = NULL;
	unsigned size = sizeof(NETRESOURCE);
	LPTSTR lpDesc[] = {lpnr->lpLocalName, lpnr->lpRemoteName, lpnr->lpComment, lpnr->lpProvider};
	const unsigned lenArray = sizeof(lpDesc) / sizeof(LPTSTR);
	unsigned i;
	for (i = 0; i < lenArray; i++) {
		if (lpDesc[i]) {
			size += (lstrlen(lpDesc[i])+1) * sizeof (TCHAR);
		}
	}
	retVal = (LPNETRESOURCE)malloc(size);
	size = sizeof(NETRESOURCE);
	memcpy(retVal, lpnr, size);
	LPTSTR *lpretValDesc[] = {&retVal->lpLocalName, &retVal->lpRemoteName, &retVal->lpComment, &retVal->lpProvider};
	for (i = 0; i < lenArray; i++) {
		if (lpDesc[i]) {
			*lpretValDesc[i] = (LPTSTR)((unsigned char *)retVal + size);
			lstrcpy(*lpretValDesc[i], lpDesc[i]);
			size += (lstrlen(lpDesc[i])+1) * sizeof (TCHAR);
		}
	}
	return retVal;
}

void FreeLPNetResource(LPNETRESOURCE lpnr)
{
	free(lpnr);
}
DWORD NetWorkOpenConnect(LPCTSTR networkPath)
{
	if (!PathIsNetworkPath(networkPath))
		return 0;
	LPTSTR path = new TCHAR[lstrlen(networkPath)+1];
	lstrcpy(path, networkPath);
	NETRESOURCE nr;
	ZeroMemory(&nr, sizeof(nr));
	nr.dwType = RESOURCETYPE_ANY;
	nr.dwScope = RESOURCE_GLOBALNET;
	nr.dwDisplayType = RESOURCEDISPLAYTYPE_GENERIC;
	nr.dwUsage = RESOURCEUSAGE_CONNECTABLE;
	nr.lpRemoteName = path; 

	DWORD dwRet = ::WNetAddConnection2(&nr, NULL, NULL, 0);
	delete []path;
	return dwRet;
}
DWORD NetWorkCloseConnection(LPCTSTR path)
{
	if (!PathIsNetworkPath(path))
		return 0;
	return WNetCancelConnection2(path, CONNECT_UPDATE_PROFILE, FALSE);
}
