// TreeCtrlDomain.cpp : implementation file
//

#include "stdafx.h"
#include "Find.h"
#include "TreeCtrlDomain.h"
#include "TreeCtrlIterator.h"
#include "NetWorkFinder.h"
#include "ThreadManager.h"
#include "Path.h"
#include "StringMatcher.h"

// CTreeCtrlDomain

#define LPNETRES_LOCALDISK_NOT_EXPANDED 1
#define LPNETRES_NETWORK_NOT_EXPANDED 2

IMPLEMENT_DYNAMIC(CTreeCtrlDomain, CTreeCtrl)

CTreeCtrlDomain::CTreeCtrlDomain()
: mbUseThread(true), mbSearchInZip(false)
{

}

CTreeCtrlDomain::~CTreeCtrlDomain()
{
	DeleteAllTreeItem();
}


BEGIN_MESSAGE_MAP(CTreeCtrlDomain, CTreeCtrl)
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()

CString CTreeCtrlDomain::GetFilePath(HTREEITEM hItem, bool bMakeUNCPath)
{
	CString str;
	while (hItem != NULL) {
		if (bMakeUNCPath) {
			LPNETRESOURCE lpnRes = (LPNETRESOURCE)GetItemData(hItem);
			if (lpnRes > (LPNETRESOURCE)2) {// Network Node
				str = CString(lpnRes->lpRemoteName) + str;
				break;
			}
		}
		str = GetItemText(hItem) + str;
		hItem = GetParentItem(hItem);
		if (hItem)
			str = CString(_T("\\")) + str;
	}
	return str;
}
static int CheckTreeIteratorCallBack(TreeIteratorCallBackData *pData, void *pUserParam)
{
	pData->pTree->SetCheck(pData->hItem, (BOOL)(pUserParam != NULL));
	return pData->hStartItem == pData->hItem ? TICB_DONTITERATE_SIBLING : TICB_CONTINUE;
}
static int UpdateCheckTreeIteratorCallBack(TreeIteratorCallBackData *pData, void *pUserParam)
{
	int retVal =  TICB_CONTINUE;
	BOOL bStartItem(pData->hStartItem == pData->hItem);
	if (!bStartItem && !pData->pTree->GetCheck(pData->hItem)) {
		retVal = TICB_ABORT;
		*(BOOL*)pUserParam = FALSE;
	}
	return bStartItem ? TICB_DONTITERATE_SIBLING : retVal;
}
BOOL CTreeCtrlDomain::UpdateCheckStatus(HTREEITEM hItem)
{
	BOOL bRet = FALSE;
	if (hItem != NULL) {
		if (GetChildItem(hItem) != NULL) {
			BOOL bAllChecked(TRUE);
			CTreeCtrlIterator cti(this, UpdateCheckTreeIteratorCallBack, &bAllChecked);
			cti.StartIterationEx(hItem);
			bRet = CTreeCtrl::SetCheck(hItem, bAllChecked);
		}
		UpdateCheckStatus(GetParentItem(hItem));
	}
	return bRet;
}
BOOL CTreeCtrlDomain::SetCheck(HTREEITEM hItem, BOOL fCheck, bool bNotify)
{
	BOOL bRet = CTreeCtrl::SetCheck(hItem, fCheck);
	CTreeCtrlIterator cti(this, CheckTreeIteratorCallBack, (void *)(__int64)fCheck);
	cti.StartIterationEx(hItem);
	UpdateCheckStatus(hItem);
	NMTVITEMCHECKSTATECHANGED nmItemCheckStateChanged;
	nmItemCheckStateChanged.hdr.code = TVN_ITEMCHECK_STATE_CHANGED;
	nmItemCheckStateChanged.hdr.hwndFrom = m_hWnd;
	nmItemCheckStateChanged.hdr.idFrom = GetDlgCtrlID();
	nmItemCheckStateChanged.item = hItem;
	nmItemCheckStateChanged.bChecked = fCheck;
	if (bNotify) {
		// Notify parent window
		GetParent()->SendMessage(WM_NOTIFY, nmItemCheckStateChanged.hdr.idFrom,
			(LPARAM)&nmItemCheckStateChanged);
	}
	return bRet;
}

BOOL CTreeCtrlDomain::SetCheckChildMatchingPattern(HTREEITEM hItem, BOOL fCheck, const CString& pattern, bool bNotify)
{
	if (hItem == NULL)
		hItem = GetRootItem();
	if (hItem == NULL)
		return FALSE;
	if (pattern.IsEmpty()) {
		SetCheck(hItem, fCheck, bNotify);
	}
	else {
		Expand(hItem, TVE_EXPAND, false);
		CRegExpMatcher stringMatcher(pattern);
		hItem = GetChildItem(hItem);
		while (hItem) {
			if (stringMatcher.Match(GetItemText(hItem)))
				SetCheck(hItem, fCheck, bNotify);
			hItem = GetNextSiblingItem(hItem);
		}
	}
	return fCheck;
}

BOOL CTreeCtrlDomain::GetStartAndEnd(HTREEITEM &hStartItem, HTREEITEM &hEndItem)
{
	if (hStartItem == hEndItem)
		return FALSE;
	if (GetParentItem(hStartItem) != GetParentItem(hEndItem))
		return FALSE;
	HTREEITEM hItem = hStartItem;
	while ((hItem = GetNextSiblingItem(hItem)) != NULL) {
		if (hItem == hEndItem)
			return TRUE;
	}
	hItem = hEndItem;
	while ((hItem = GetNextSiblingItem(hItem)) != NULL) {
		if (hItem == hStartItem) {
			hStartItem = hEndItem;
			hEndItem = hItem;
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CTreeCtrlDomain::SetCheckInRange(HTREEITEM hStartItem, HTREEITEM hEndItem, BOOL fCheck, bool bNotify)
{
	if (!GetStartAndEnd(hStartItem, hEndItem))
		return FALSE;
	HTREEITEM hItem = hStartItem;
	while (hItem) {
		SetCheck(hItem, fCheck, bNotify);
		if (hItem == hEndItem)
			return TRUE;
		hItem = GetNextSiblingItem(hItem);
	}
	return FALSE;
}

typedef struct {
	LPCTSTR text;
	bool bRecursive;
	HTREEITEM hItemFound;
} FindTextICBData;

static int FindTextIteratorCallBack(TreeIteratorCallBackData *pData, void *pUserParam)
{
	FindTextICBData *pFindTextICBData = (FindTextICBData *)pUserParam;
	CString itemText(pData->pTree->GetItemText(pData->hItem));
	if (!lstrcmpi(pFindTextICBData->text, itemText)) {
		pFindTextICBData->hItemFound = pData->hItem;
		return TICB_ABORT;
	}
	if (pFindTextICBData->bRecursive) {
		return pData->hStartItem == pData->hItem ? TICB_DONTITERATE_SIBLING
			: TICB_CONTINUE;
	}
	return TICB_CONTINUE | TICB_DONTITERATE_CHILREN;
}

HTREEITEM CTreeCtrlDomain::FindText(LPCTSTR text, HTREEITEM hStartItem, bool bRecursive)
{
	FindTextICBData ficbData = {text, bRecursive, NULL};
	CTreeCtrlIterator cti(this, FindTextIteratorCallBack, (void *)&ficbData);
	if (hStartItem == TVI_ROOT)
		hStartItem = GetRootItem();
	cti.StartIterationEx(hStartItem);
	return ficbData.hItemFound;
}
HTREEITEM CTreeCtrlDomain::FindDomain(LPCTSTR text)
{
	CString path(_T("NetWork\\Microsoft Windows Network\\"));
	path += text;
	return Expand(path);
}
HTREEITEM CTreeCtrlDomain::FindPath(const CString &path, HTREEITEM hStartItem)
{
	Path fullPath(path);
	if (fullPath.IsUNC()) {
		fullPath.Delete(0, 2);
		if (hStartItem == TVI_ROOT) {
            hStartItem = FindText(fullPath.GetRoot(), hStartItem, true);
            if (NULL != hStartItem)
			    hStartItem = GetChildItem(hStartItem);
			fullPath = fullPath.RemoveRoot();
		}
	}
	while (!fullPath.IsEmpty()) {
		Path root(fullPath.GetRoot());
		hStartItem = FindText(root, hStartItem, false);
		if (hStartItem != NULL) {
			fullPath = fullPath.RemoveRoot();
			if (!fullPath.IsEmpty()) {
				Expand(hStartItem, TVE_EXPAND, false);
				hStartItem = GetChildItem(hStartItem);
			}
		}
		else break;
	}
	return hStartItem;
}
static int SearchForNetWorkPC_CallBackDomain(LPNETRESOURCE lpNetRes, void *pUserParam)
{
    UNREFERENCED_PARAMETER(lpNetRes);
    UNREFERENCED_PARAMETER(pUserParam);
	return FCB_ABORT;
}
HTREEITEM CTreeCtrlDomain::FindAddNetworkPath(const CString &networkPath)
{
	if (!PathIsNetworkPath(networkPath)) // only for network path
		return NULL;
	// Frist search in tree network node
	CString networkRoot(RootNameFromPath(networkPath));
	CString networkName = networkRoot.Right(networkRoot.GetLength()-2);
	HTREEITEM hItem = FindText(networkName, GetRootItem());
	if (!hItem) {
		CString statusText(_T("Searching "));
		statusText += networkPath + _T(" in Network");
		GetParent()->SendMessage(WM_SET_STATUS_MESSAGE, (WPARAM)(LPCTSTR)statusText);
		// Search in network
		NETRESOURCE netResourse = {0};
		LPNETRESOURCE lpnetResourse = NULL;
		netResourse.dwScope = RESOURCE_GLOBALNET;
		netResourse.dwType = RESOURCETYPE_ANY;
		netResourse.dwDisplayType = RESOURCEDISPLAYTYPE_SERVER;
		netResourse.dwUsage = RESOURCEUSAGE_CONTAINER;
		netResourse.lpRemoteName = (LPTSTR)(LPCTSTR)networkRoot;
		lpnetResourse = &netResourse;
		CNetWorkFinder cnf(SearchForNetWorkPC_CallBackDomain);
		int count = cnf.StartFind(lpnetResourse);
		if (count) {
			hItem = InsertNewItem(GetRootItem(), networkName, (DWORD_PTR)CopyLPNetResource(lpnetResourse));
		}
	}
	if (hItem != NULL) {
		Path fullPath(networkPath);
		Path networkDrivePath(fullPath.RemoveRoot().GetRoot());
		if (networkDrivePath.Find('$') == networkDrivePath.GetLength()-1) { // Network disk path e.g. \\computer\\c$
			networkDrivePath = fullPath.GetRoot().Append(networkDrivePath);
			if (networkDrivePath.Exists() && networkDrivePath.IsDir()) {
				Expand(hItem, TVE_EXPAND, false);
				if (FindText(networkDrivePath.FileName(), hItem) == NULL) {
					InsertNewItem(hItem, networkDrivePath.FileName(), LPNETRES_LOCALDISK_NOT_EXPANDED);
				}
			}
		}
		hItem = FindPath(networkPath, GetChildItem(GetParentItem(hItem)));
	}
	return hItem;
}

// CTreeCtrlDomain message handlers

LRESULT CTreeCtrlDomain::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT res = CTreeCtrl::WindowProc(message, wParam, lParam);
	switch (message) {
	case TVM_INSERTITEM:
		{
			LPTVINSERTSTRUCT lpInsertStruct = (LPTVINSERTSTRUCT)lParam;
			if (lpInsertStruct->hParent && lpInsertStruct->hParent != TVI_ROOT && res) {
				BOOL bChecked = GetCheck(lpInsertStruct->hParent);
				if (bChecked)
					SetCheck((HTREEITEM)res);
			}
		}
		break;
	}
	return res;
}
void CTreeCtrlDomain::OnToggleITemCheck(UINT nFlags, HTREEITEM hItem, bool bNotify)
{
	if (nFlags & MK_SHIFT) {
		HTREEITEM hSelectedItem = GetSelectedItem();
		SetCheckInRange(hItem, hSelectedItem, GetCheck(hItem), bNotify);
	}
	else {
		SetCheck(hItem, GetCheck(hItem), bNotify);
	}
}
void CTreeCtrlDomain::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	HTREEITEM hITem = HitTest(point);
	if (nFlags & MK_CONTROL) {
		if (hITem != NULL) {
			CString path = GetFilePath(hITem);
			if (PathIsDirectory(path) || PathIsNetworkPath(path))
				ShellExecute(NULL, _T("open"), path, NULL, NULL, SW_SHOWDEFAULT);
		}
	}
	BOOL bChecked = hITem ? GetCheck(hITem) : FALSE;
	CTreeCtrl::OnLButtonDblClk(nFlags, point);
	if (hITem && bChecked != GetCheck(hITem)) {
		OnToggleITemCheck(nFlags, hITem, true);
	}
}
void CTreeCtrlDomain::OnLButtonDown(UINT nFlags, CPoint point)
{
	HTREEITEM hITem = NULL;
	BOOL bChecked = FALSE;

	hITem = HitTest(point);
	bChecked = hITem ? GetCheck(hITem) : FALSE;
	CTreeCtrl::OnLButtonDown(nFlags, point);
	if (hITem && bChecked != GetCheck(hITem)) {
		OnToggleITemCheck(nFlags, hITem, true);
	}
}

void CTreeCtrlDomain::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	HTREEITEM hITem = GetSelectedItem();
	BOOL bChecked = hITem ? GetCheck(hITem) : FALSE;
	CTreeCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
	if (nChar == VK_SPACE && GetSelectedItem() == hITem && bChecked != GetCheck(hITem))
		SetCheck(hITem, GetCheck(hITem), true);
}

HTREEITEM CTreeCtrlDomain::InsertNewItem(HTREEITEM hItem, LPCTSTR name, DWORD_PTR itemData)
{
	hItem = InsertItem(name, hItem, NULL);
	InsertItem(name, hItem, NULL);
	SetItemData(hItem, itemData);
	return hItem;
}
HTREEITEM CTreeCtrlDomain::Expand(const CString &inPath, HTREEITEM hPrentItem, UINT nCode)
{
	Path path(inPath);
	if (path.IsUNC())
		path.Delete(0, 2);
	while (path != _T("")) {
		hPrentItem = FindText(path.GetRoot(), hPrentItem, false);
		path = path.RemoveRoot();
		if (hPrentItem != NULL) {
			if (nCode)
				Expand(hPrentItem, nCode, false);
			if (!path.IsEmpty())
				hPrentItem = GetChildItem(hPrentItem);
		}
		else break;
	}
	return hPrentItem;
}
void CTreeCtrlDomain::RefreshDomainTree(void)
{
	DeleteAllTreeItem();
	InsertNewItem(NULL, _T("NetWork"), LPNETRES_NETWORK_NOT_EXPANDED); // Not filled Network
	// Enumerate all disks
	TCHAR disk[] = _T("C:");
	do {
		AddRootDrive(disk);
		disk[0]++;
	} while (disk[0] <= 'Z');
}
void CTreeCtrlDomain::AddRootDrive(LPCTSTR disk)
{
	if (PathIsDirectory(disk) && FindPath(disk) == NULL) {
		InsertNewItem(NULL, disk, LPNETRES_LOCALDISK_NOT_EXPANDED); // Not filled Disk
	}
}
void CTreeCtrlDomain::RemoveRootDrive(LPCTSTR disk) {
	HTREEITEM hItem = FindPath(disk);
	if (hItem != NULL)
		DeleteItem(hItem);
}

void CTreeCtrlDomain::OnTvnDeleteitem(HTREEITEM hItem)
{
	if (GetParentItem(hItem) == NULL) // We add static for root items
		return;
	LPNETRESOURCE lpnRes = (LPNETRESOURCE)GetItemData(hItem);
	if (lpnRes > (LPNETRESOURCE)3)
		FreeLPNetResource(lpnRes);
	SetItemData(hItem, NULL);
}
static int FreeTreeNodeIteratorCallBack(TreeIteratorCallBackData *pData, void *pUserParam)
{
	CTreeCtrlDomain *treeCtrlDomain = (CTreeCtrlDomain *)pData->pTree;
	treeCtrlDomain->OnTvnDeleteitem(pData->hItem);
	return pUserParam && pData->hStartItem == pData->hItem ? TICB_DONTITERATE_SIBLING : TICB_CONTINUE;
}

void CTreeCtrlDomain::DeleteAllTreeItem(void)
{
	CTreeCtrlIterator cti(this, FreeTreeNodeIteratorCallBack);
	cti.StartIteration();
	CTreeCtrlDomain::DeleteAllItems();
}
BOOL CTreeCtrlDomain::DeleteItem(HTREEITEM hItem)
{
	CTreeCtrlIterator cti(this, FreeTreeNodeIteratorCallBack, (void *)1);
	cti.StartIterationEx(hItem);
	return CTreeCtrl::DeleteItem(hItem);
}
typedef struct {
	CTreeCtrlDomain *pTreeCtrl;
	HTREEITEM hItem;
	int folderOrNetWork;
	DWORD threadID;
} TVExpandData;

static LPCTSTR GetFileFromPath(LPCTSTR path)
{
	int len = lstrlen(path);
	while (len-- > 0) {
		if (path[len] == '\\' || path[len] == '/' || path[len] == ':')
			break;
	}
	return path+len+1;
}

static int NetWorkFindCallBackDomain(LPNETRESOURCE lpNetRes, void *pUserParam)
{
	TVExpandData *fcbData = (TVExpandData *)pUserParam;
	CTreeCtrlDomain *treeCtrl = (CTreeCtrlDomain*)fcbData->pTreeCtrl;
	LPCTSTR remoteName = GetFileFromPath(lpNetRes->lpRemoteName);
	if (*remoteName) {
		treeCtrl->InsertNewItem(fcbData->hItem, remoteName, (DWORD_PTR)CopyLPNetResource(lpNetRes));
	}
	return ThreadManager::GetInstance().IsThreadTerminated(fcbData->threadID) ? FCB_ABORT : FCB_CONTINUE;
}

static int FindCallBackDomain(CFileFindEx *pFindFile, bool bFileMatched, void *pUserParam)
{
	TVExpandData *fcbData = (TVExpandData *)pUserParam;
	if (pFindFile->IsDirectory() && bFileMatched) {
		CTreeCtrlDomain *treeCtrl = (CTreeCtrlDomain*)fcbData->pTreeCtrl;
		CString filename = pFindFile->GetFileName();
		treeCtrl->InsertNewItem(fcbData->hItem, filename, (DWORD_PTR)fcbData->folderOrNetWork);
	}
	return ThreadManager::GetInstance().IsThreadTerminated(fcbData->threadID) ? FCB_ABORT : FCB_CONTINUE;
}

static int ExpandTreeThreadProcFn(LPVOID pThread)
{
	TVExpandData *fcbData = (TVExpandData *)pThread;
	fcbData->threadID = GetCurrentThreadId();
	CTreeCtrlDomain *mTreeCtrlDomain(fcbData->pTreeCtrl);
	CString statusText;
	statusText.Format(_T("Searching %s..."), mTreeCtrlDomain->GetItemText(fcbData->hItem));
	if (fcbData->folderOrNetWork == 1) { // Local Disk
		mTreeCtrlDomain->SetItemData(fcbData->hItem, 0);
		mTreeCtrlDomain->DeleteItem(mTreeCtrlDomain->GetChildItem(fcbData->hItem));
		CFinder cf(NULL, FindCallBackDomain, false, fcbData);
		mTreeCtrlDomain->GetParent()->SendMessage(WM_SET_STATUS_MESSAGE, (WPARAM)(LPCTSTR)statusText);
		cf.Find(mTreeCtrlDomain->GetFilePath(fcbData->hItem), mTreeCtrlDomain->SearchInZip());
		mTreeCtrlDomain->GetParent()->SendMessage(WM_SET_STATUS_MESSAGE);
	}
	else {
		LPNETRESOURCE lpnRes = NULL;
		bool bAlreadyProcessed(false);
		if (fcbData->folderOrNetWork == 2) { // Network
			mTreeCtrlDomain->SetItemData(fcbData->hItem, 0);
			lpnRes = NULL;
		}
		else {
			lpnRes = (LPNETRESOURCE)mTreeCtrlDomain->GetItemData(fcbData->hItem);
			HTREEITEM hITem = mTreeCtrlDomain->GetChildItem(fcbData->hItem);
			// Already processed
			if (hITem == NULL || hITem && mTreeCtrlDomain->GetItemText(hITem) != mTreeCtrlDomain->GetItemText(fcbData->hItem))
				bAlreadyProcessed = true;
		}
		if (!bAlreadyProcessed) {
			mTreeCtrlDomain->DeleteItem(mTreeCtrlDomain->GetChildItem(fcbData->hItem));
			CNetWorkFinder cnf(NetWorkFindCallBackDomain, false, fcbData);
			mTreeCtrlDomain->GetParent()->SendMessage(WM_SET_STATUS_MESSAGE, (WPARAM)(LPCTSTR)statusText);
			int count = cnf.StartFind(lpnRes);
			if (count == 0) {
				CString filePath = mTreeCtrlDomain->GetFilePath(fcbData->hItem);
				if (PathIsNetworkPath(filePath)) {
					fcbData->folderOrNetWork = 1;
					CFinder cf(NULL, FindCallBackDomain, false, fcbData);
					cf.Find(filePath, true);
				}
			}
			mTreeCtrlDomain->GetParent()->SendMessage(WM_SET_STATUS_MESSAGE);
		}
	}
	delete fcbData;
	return 0;
}
void CTreeCtrlDomain::OnTvnItemexpanding(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	*pResult = 0;
	if (pNMTreeView->action != 2) // Not opening
		return;
	TVExpandData fcbData = {this, pNMTreeView->itemNew.hItem};
	fcbData.folderOrNetWork = (int)GetItemData(fcbData.hItem);
	if (fcbData.folderOrNetWork == 0)
		return;
	LPVOID pThreaData(new TVExpandData(fcbData));
	if (mbUseThread)
		ThreadManager::GetInstance().CreateThread(ExpandTreeThreadProcFn, pThreaData, 1);
	else
		ExpandTreeThreadProcFn(pThreaData);
}
BOOL CTreeCtrlDomain::Expand(HTREEITEM hItem, UINT nCode, bool bThreaded)
{
	mbUseThread = bThreaded;
	if (!bThreaded) {
		NMTREEVIEWW nmTV = {0};
		nmTV.action = nCode;
		nmTV.itemNew.hItem = hItem;
		LRESULT lResult(0);
		OnTvnItemexpanding((NMHDR*)&nmTV, &lResult);
	}
	BOOL bSuccess(CTreeCtrl::Expand(hItem, nCode));
	mbUseThread = true;
	return bSuccess;
}
static int GetCheckTreeIteratorCallBack(TreeIteratorCallBackData *pData, void *pUserParam)
{
	HTREEITEMVec *outCheckList = (HTREEITEMVec *)pUserParam;
	if (pData->pTree->GetCheck(pData->hItem)) {
		outCheckList->push_back(pData->hItem);
		return TICB_DONTITERATE_CHILREN;
	}
	return TICB_CONTINUE;
}
void CTreeCtrlDomain::GetCheckList(HTREEITEMVec &outCheckList)
{
	outCheckList.clear();
	CTreeCtrlIterator ti(this, GetCheckTreeIteratorCallBack, &outCheckList);
	ti.StartIteration();
}
BOOL CTreeCtrlDomain::OnDeviceChange(UINT nEventType, DWORD_PTR dwData)
{
	PDEV_BROADCAST_HDR lpdb = (PDEV_BROADCAST_HDR)dwData;

	switch(nEventType )
	{
	case DBT_DEVICEARRIVAL:
		// Check whether a CD or DVD was inserted into a drive.
		if (lpdb -> dbch_devicetype == DBT_DEVTYP_VOLUME)
		{
			PDEV_BROADCAST_VOLUME lpdbv = (PDEV_BROADCAST_VOLUME)lpdb;
			AddRootDrive(SystemUtils::FirstDriveFromMask(lpdbv ->dbcv_unitmask));
		}
		break;

	case DBT_DEVICEREMOVECOMPLETE:
		// Check whether a CD or DVD was removed from a drive.
		if (lpdb -> dbch_devicetype == DBT_DEVTYP_VOLUME)
		{
			PDEV_BROADCAST_VOLUME lpdbv = (PDEV_BROADCAST_VOLUME)lpdb;
			RemoveRootDrive(SystemUtils::FirstDriveFromMask(lpdbv ->dbcv_unitmask));
		}
		break;

	default:
		/*
		Process other WM_DEVICECHANGE notifications for other 
		devices or reasons.
		*/ 
		;
	}
	return TRUE;
}
