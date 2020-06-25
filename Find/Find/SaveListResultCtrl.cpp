// SaveListResultCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "Find.h"
#include "SaveListResultCtrl.h"
#include "Finder.h"
#include "DupFileFilter.h"
#include "FileIconMgr.h"

extern IContextMenu2  *g_pcm2;

// CSaveListResultCtrl

IMPLEMENT_DYNAMIC(CSaveListResultCtrl, CListCtrlUtil)

CSaveListResultCtrl::CSaveListResultCtrl(CFindDlg *pFindDlg) : m_iColSorted(-1),
m_pFindDlg(pFindDlg), m_hExplorerContextMenu(NULL),
mContextMenu(NULL), pExplorerContextMenu(NULL), mbDrawingDisabled(false)
{
	for(int i = 0; i < sizeof(m_iOptionalColumns)/sizeof(m_iOptionalColumns[0]); ++i)
		m_iOptionalColumns[i] = -1;
}

CSaveListResultCtrl::~CSaveListResultCtrl()
{
}

UINT CSaveListResultCtrl::WM_FINDREPLACE = ::RegisterWindowMessage(FINDMSGSTRING);

BEGIN_MESSAGE_MAP(CSaveListResultCtrl, CListCtrlUtil)
	ON_WM_CONTEXTMENU()
	ON_WM_KEYDOWN()
	ON_WM_CHAR()
	ON_REGISTERED_MESSAGE(CSaveListResultCtrl::WM_FINDREPLACE, OnFindReplace)
	ON_WM_LBUTTONDOWN()
	ON_WM_PAINT()
END_MESSAGE_MAP()

struct SortListData {
	CListCtrlUtil* pListResult;
	bool bReverse;
};
// Sort items by associated lParam
static int CALLBACK 
MyCompareProcFileName(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	CListCtrlUtil* pListResult = ((SortListData* )lParamSort)->pListResult;
	int retVal = lstrcmp(pListResult->GetItemText((int)lParam1, 0),
		pListResult->GetItemText((int)lParam2, 0));
	if (((SortListData* )lParamSort)->bReverse)
		retVal = -retVal;
	return retVal;
}

static int CALLBACK 
MyCompareProcPathName(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	CListCtrlUtil* pListResult = ((SortListData* )lParamSort)->pListResult;
	int retVal =  lstrcmp(pListResult->GetItemText((int)lParam1, 1),
		pListResult->GetItemText((int)lParam2, 1));
	if (((SortListData* )lParamSort)->bReverse)
		retVal = -retVal;
	return retVal;
}
static int CALLBACK 
MyCompareProcSize(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	if (!lParam1 || !lParam2)
		return (int)(lParam1-lParam2);
	LONGLONG size1 = ((CListResItemData*)lParam1)->m_ullFileSize;
	LONGLONG size2 = ((CListResItemData*)lParam2)->m_ullFileSize;
	int retVal = 0;
	if (size1 < size2)
		retVal = -1;
	else if (size1 > size2)
		retVal =  1;
	if (((SortListData* )lParamSort)->bReverse)
		retVal = -retVal;
	return retVal;
}
static int CALLBACK 
MyCompareProcCTime(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	if (!lParam1 || !lParam2)
		return (int)(lParam1-lParam2);
	int retVal = (int)(((CListResItemData*)lParam1)->mCreatedTime.GetTime() -
		((CListResItemData*)lParam2)->mCreatedTime.GetTime());
	if (((SortListData* )lParamSort)->bReverse)
		retVal = -retVal;
	return retVal;
}
static int CALLBACK 
MyCompareProcMTime(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	if (!lParam1 || !lParam2)
		return (int)(lParam1-lParam2);
	int retVal = (int)(((CListResItemData*)lParam1)->mModifedTime.GetTime() -
		((CListResItemData*)lParam2)->mModifedTime.GetTime());
	if (((SortListData* )lParamSort)->bReverse)
		retVal = -retVal;
	return retVal;
}
static int CALLBACK 
MyCompareProcATime(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	if (!lParam1 || !lParam2)
		return (int)(lParam1-lParam2);
	int retVal = (int)(((CListResItemData*)lParam1)->mAccessedTime.GetTime() -
		((CListResItemData*)lParam2)->mAccessedTime.GetTime());
	if (((SortListData* )lParamSort)->bReverse)
		retVal = -retVal;
	return retVal;
}
HMENU CSaveListResultCtrl::GetExplorerContextMenu(IContextMenu **pInExplorerContextMenu)
{
	POSITION pos = GetFirstSelectedItemPosition();
	*pInExplorerContextMenu = NULL;
	if (pos == NULL)
		return NULL;
	HMENU hMenu = NULL;
	LPITEMIDLIST *pidl = new LPITEMIDLIST[GetSelectedCount()+1];
	IShellFolder *psf = NULL;
	HRESULT hr = E_FAIL;
	int i = 0;
	while (pos)
	{
		CString path = GetItemText(GetNextSelectedItem(pos), 1);
		SFGAOF sfgao;
		if (!psf) {
			if (SUCCEEDED(hr = SHParseDisplayName(path, NULL, pidl+i, 0, &sfgao))) {
				hr = SHBindToParent(pidl[i], IID_IShellFolder,
											(void**)&psf, NULL);
			}
			CoTaskMemFree(pidl[i]);
		}
		if (psf) {
			TCHAR pathChild[MAX_PATH];
			ULONG cch;
			lstrcpyS(pathChild, FileNameFromPath(path));
			if (SUCCEEDED(hr = psf->ParseDisplayName(NULL, NULL, pathChild, &cch, pidl+i, NULL)))
				i++;
			cch = 0;
		}
	}
	if (i > 0) {
		IContextMenu *pcm;
		hr = psf->GetUIObjectOf(NULL, i, (LPCITEMIDLIST *)pidl, IID_IContextMenu, NULL, (void **)&pcm);
		psf->Release();
		if (SUCCEEDED(hr))
			hMenu = CreatePopupMenu();
		hr=pcm->QueryContextMenu(hMenu, 0, 0, (UINT)0x7fff, CMF_EXPLORE | CMF_NORMAL);
		if (SUCCEEDED(hr)) {
			IContextMenu2* pCM2;
			if (SUCCEEDED (pcm->QueryInterface (IID_IContextMenu2, (LPVOID*)&pCM2)))
			{
				g_pcm2 = pCM2;
			}
		}
		*pInExplorerContextMenu = pcm;
		while (i-- > 0)
			CoTaskMemFree(pidl[i]);
	}
	delete []pidl;
	return hMenu;
}
void CSaveListResultCtrl::InitExplorerContextMenu(HMENU hMenu)
{
	if (m_hExplorerContextMenu != NULL && m_hExplorerContextMenu == hMenu) {
		MENUITEMINFO mi = {sizeof(MENUITEMINFO), MIIM_SUBMENU};
		mi.hSubMenu = GetExplorerContextMenu(&pExplorerContextMenu);
		if (mi.hSubMenu)
			mContextMenu->SetMenuItemInfo(ID_OPEN_EXPLORERMENU, &mi);
		DestroyMenu(m_hExplorerContextMenu);
		m_hExplorerContextMenu = NULL;
	}
}
bool CSaveListResultCtrl::DisableNotification(bool bDisable)
{
	bool bIsDisabled(IsNotificationDisabled());
	if (__super::DisableNotification(bDisable)) {
		if (!IsNotificationDisabled()) {
			CFindDlg* pDlg(GetFindDlg());
			LRESULT lResult(0);
			pDlg->OnLvnItemchangedListResult(NULL, &lResult);
		}
	}
	return bIsDisabled;
}
// CSaveListResultCtrl message handlers

void CSaveListResultCtrl::OnContextMenu(CWnd *pWnd, CPoint pos)
{
    UNREFERENCED_PARAMETER(pWnd);
	CMenu menu;
	menu.LoadMenu(IDR_MENU_CONTEXT);
	CMenu *contextMenu = menu.GetSubMenu(0);
	mContextMenu = contextMenu;
	HMENU hExpSubMenu = NULL;
	UINT toRemove[] = {ID_OPEN_OPEN, ID_OPEN_OPENCONTAININGFOLDER, ID_OPEN_SEARCHINFILE, ID_OPEN_PROPERTIES,
		ID_OPEN_COPYFILES, ID_OPEN_EXPLORERMENU, ID_OPEN_REMOVE,
		ID_OPEN_COPYPATH, ID_OPEN_DELETEFILES};
	UINT toRemoveIfCountZero[] = {ID_OPEN_FILTERDUPLICATES, ID_OPEN_FIND,
		ID_OPEN_SELECTALL, ID_OPEN_INVERTSELECTION};
	UINT toRemoveIfSearchStarted[] = {ID_OPEN_FILTERDUPLICATES};
	if (GetItemCount() == 0) {
		for (int i = 0; i < sizeof (toRemoveIfCountZero) / sizeof (UINT); i++)
			contextMenu->RemoveMenu(toRemoveIfCountZero[i], MF_BYCOMMAND);
	}
	if (((CFindDlg*)GetParent())->IsSearchStarted())
		for (int i = 0; i < sizeof (toRemoveIfSearchStarted) / sizeof (UINT); i++)
			contextMenu->RemoveMenu(toRemoveIfSearchStarted[i], MF_BYCOMMAND);
	int sel = GetSelectionMark();
	if (GetSelectedCount() == 0)
		for (int i = 0; i < sizeof (toRemove) / sizeof (UINT); i++)
			contextMenu->RemoveMenu(toRemove[i], MF_BYCOMMAND);
	else {
		contextMenu->SetDefaultItem(ID_OPEN_OPEN);
		MENUITEMINFO mi = {sizeof(MENUITEMINFO), MIIM_SUBMENU};
		m_hExplorerContextMenu = CreatePopupMenu();
		hExpSubMenu = mi.hSubMenu = m_hExplorerContextMenu;
		if (hExpSubMenu)
			contextMenu->SetMenuItemInfo(ID_OPEN_EXPLORERMENU, &mi);
        bool bRemoveContentSearch(true);
        POSITION selpos = GetFirstSelectedItemPosition();
        while (bRemoveContentSearch)
        {
            if (selpos == nullptr)
                break;
            int item(GetNextSelectedItem(selpos));
            if (GetItemData(item))  // is a path/file name row - null means content search result
                continue;
            CString itemText(GetItemText(item, 0)); // get string and compare is first char is space - already done content search
            bRemoveContentSearch = SystemUtils::StringGetAt(itemText, 0) == ' ';
        }
        if (bRemoveContentSearch)
            contextMenu->RemoveMenu(ID_OPEN_SEARCHINFILE, MF_BYCOMMAND);
	}
	UINT nCommandID[] = {ID_OPEN_SIZECOLUMN, ID_SELECTCOLUMNS_DATECREATED, ID_SELECTCOLUMNS_DATEMODIFIED, ID_SELECTCOLUMNS_DATEACCESSED, ID_SELECTCOLUMNS_SHOWICONS};
	for (ListColumns lc = ListColumns_Size; lc < ListColumns_OptianlCount; lc = (ListColumns)(lc + 1)) {
		UINT flag = MF_BYCOMMAND;
		if (IsOptionalColumnPresent(lc))
			flag |= MF_CHECKED;
		contextMenu->CheckMenuItem(nCommandID[lc], flag);
	}
	if (pos.x < 0 || pos.y < 0) {
		RECT rc;
		if (sel < 0) {
			GetClientRect(&rc);
		}
		else {
			GetItemRect(sel, &rc, LVIR_BOUNDS);
		}
		pos.x = (rc.right+rc.left) >> 1;
		pos.y = (rc.bottom+rc.top) >> 1;
		ClientToScreen(&pos);
	}
	int idCmd = contextMenu->TrackPopupMenu(TPM_RETURNCMD, pos.x, pos.y, GetParent());
	mContextMenu = NULL;
	contextMenu->DestroyMenu();
	//prepare to invoke properties command
	CMINVOKECOMMANDINFO invoke;
	memset (&invoke, 0, sizeof(invoke));
	invoke.cbSize = sizeof(invoke);
	invoke.fMask = 0;
	invoke.hwnd = NULL;
	invoke.nShow = SW_SHOWNORMAL;
	invoke.lpVerb = (LPCSTR)MAKEINTRESOURCE(idCmd);
	if (pExplorerContextMenu == NULL || !SUCCEEDED(pExplorerContextMenu->InvokeCommand(&invoke)))
		GetParent()->PostMessage(WM_COMMAND, MAKEWPARAM(idCmd, 0));
	if (g_pcm2) {
		g_pcm2->Release();
		g_pcm2 = NULL;
	}
	if (hExpSubMenu)
		DestroyMenu(hExpSubMenu);
	if (pExplorerContextMenu)
		pExplorerContextMenu->Release();
	pExplorerContextMenu = NULL;
	menu.DestroyMenu();
}

void CSaveListResultCtrl::SortItemsEx(int nCol, bool bForce)
{
	CAutoDisableNotificaltion autoDisableNotificaltion(this);
	int count = GetItemCount();
	if (count == 0) {
		m_iColSorted = -1;
		return;
	}
	SortListData sortListData = {this, bForce ? false : m_iColSorted == nCol};
	PFNLVCOMPARE procs[] = {MyCompareProcFileName, MyCompareProcPathName, MyCompareProcSize, MyCompareProcCTime, MyCompareProcMTime, MyCompareProcATime};
	//CLockerTemplate<CSingleLock, CSaveListResultCtrl> lock(*this);
	if (nCol >= 2) {
		CListCtrlUtil::SortItems(procs[nCol], (DWORD_PTR)&sortListData);
	}
	else {
		CListCtrlUtil::SortItemsEx(procs[nCol], (DWORD_PTR)&sortListData);
	}
	if (m_iColSorted == nCol)
		m_iColSorted = -1;
	else
		m_iColSorted = nCol;
}

void CSaveListResultCtrl::MoveItem(int nSrcItem, int nDstIndex)
{
	int newItem = InsertItem(nDstIndex, GetItemText(nSrcItem, 0));
	int nCol = GetHeaderCtrl()->GetItemCount();
	for (int i = 1; i < nCol; i++) {
		SetItemText(newItem, i, GetItemText(nSrcItem, i));
	}
	SetItemData(newItem, GetItemData(nSrcItem));
	DeleteItem(nDstIndex);
}

HANDLE CSaveListResultCtrl::GetHDrop()
{
	POSITION pos = GetFirstSelectedItemPosition();
	int fileLenght = 0;
	if (pos == NULL)
		return NULL;
	while (pos) {
		fileLenght += (GetItemText(GetNextSelectedItem(pos), 1).GetLength() + 1)*sizeof(TCHAR);
	}
	fileLenght += sizeof(TCHAR);
	HANDLE hDrop = GlobalAlloc(GHND, sizeof(DROPFILES)+fileLenght);
	if (hDrop == NULL)
		return NULL;
	LPDROPFILES pDropFiles = (LPDROPFILES)GlobalLock(hDrop);
	pDropFiles->pFiles = sizeof(DROPFILES);
	pDropFiles->fWide = sizeof(TCHAR) > 1 ? TRUE : FALSE;
	LPTSTR pFiles = (LPTSTR)((char*)pDropFiles + pDropFiles->pFiles);
	pos = GetFirstSelectedItemPosition();
	while (pos) {
		CString file = GetItemText(GetNextSelectedItem(pos), 1);
		lstrcpy(pFiles, file);
		pFiles += file.GetLength() + 1;
	}
	GlobalUnlock(hDrop);
	return hDrop;
}
void CSaveListResultCtrl::DoDragDrop()
{
/*	HWND hWnd = CreateWindow(_T("edit"), _T("dummy"), WS_BORDER | WS_CAPTION | WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 200, 50, NULL, NULL, NULL, NULL);
	OleInitialize(NULL);
	CWnd cwnd;
	RECT rc = {200, 200, 300, 300};
	cwnd.Create(_T("edit"), _T("dummy"), WS_BORDER | WS_CAPTION | WS_OVERLAPPEDWINDOW | WS_VISIBLE, rc, GetDesktopWindow(), 0);
	CWnd *pWnd = AfxGetThread()->m_pMainWnd;
	AfxGetThread()->m_pMainWnd = &cwnd;
	cwnd.SetForegroundWindow();*/
	HANDLE hDrop = GetHDrop();
	COleDataSource DropData;
	DropData.CacheGlobalData( CF_HDROP, hDrop );
//	COleFindDropSource DropSource;
#if 1
	DROPEFFECT de = DropData.DoDragDrop(DROPEFFECT_COPY|DROPEFFECT_LINK|DROPEFFECT_MOVE/*,NULL, &DropSource*/);

	if(de == DROPEFFECT_COPY){
		// Copy files if CTRL btn is hold;
	}
	else{
		// Move files, as default;
	}
#else
	COleDropSource dropSource;
	LPDATAOBJECT lpDataObject = (LPDATAOBJECT)DropData.GetInterface(&IID_IDataObject);
	LPDROPSOURCE lpDropSource =
		(LPDROPSOURCE)dropSource.GetInterface(&IID_IDropSource);
	DWORD dwResultEffect = DROPEFFECT_NONE;
	::DoDragDrop(lpDataObject, lpDropSource, DROPEFFECT_COPY, &dwResultEffect);
#endif
/*	AfxGetThread()->m_pMainWnd = pWnd;
	cwnd.DestroyWindow();
	OleUninitialize();*/
	//::DestroyWindow(hWnd);
}
void CSaveListResultCtrl::CopyFilesToClipBoard()
{
	HANDLE hDrop = GetHDrop();
	if (hDrop) {
		BOOL bSuccess = FALSE;
		if (OpenClipboard()) {
			EmptyClipboard();
			bSuccess = SetClipboardData(CF_HDROP, hDrop) != NULL;
			CloseClipboard();
		}
		if (!bSuccess)
			GlobalFree(hDrop);
	}
}
void CSaveListResultCtrl::RemoveSelectedFiles(bool bDelete)
{
	CAutoDisableNotificaltion autoDisableNotificaltion(this);
	if (bDelete) {
		if (MessageBox(_T("Are you sure you want to delete selected files/folders?"),
			_T("Confirm Delete"), MB_YESNO) == IDNO)
			return;
	}
	CArray<CString> stringArray;
	POSITION pos = NULL;
	int fileSize = 0;
	while ((pos = GetFirstSelectedItemPosition()) != NULL) {
		int nItem = GetNextSelectedItem(pos);
		if (bDelete) {
			CString filePath = GetItemText(nItem, 1);
			stringArray.Add(filePath);
			fileSize += filePath.GetLength()+1;
		}
		DeleteItem(nItem);
	}
	if (fileSize > 0) {
		SHFILEOPSTRUCT shFileOp = {GetParent()->m_hWnd, FO_DELETE, NULL};
		shFileOp.fFlags = FOF_NOCONFIRMATION|FOF_NOCONFIRMMKDIR/*|FOF_SILENT*/;
		shFileOp.pFrom = (LPCTSTR)malloc(sizeof(TCHAR)*(fileSize+1));
		LPTSTR pFrom = (LPTSTR)shFileOp.pFrom;
		if (pFrom) {
			fileSize = (int)stringArray.GetCount();
			for (int i = 0; i < fileSize; i++) {
				lstrcpy(pFrom, stringArray[i]);
				pFrom += lstrlen(pFrom)+1;
			}
			*pFrom = 0;
			stringArray.RemoveAll();
			SHFileOperation(&shFileOp);
			free((LPVOID)shFileOp.pFrom);
		}
	}
}
void CSaveListResultCtrl::OnKeyDown(UINT nChar, UINT nReptCnt, UINT nFlags)
{
	bool bCtrlPressed = GetAsyncKeyState(VK_CONTROL) != 0;
	switch (nChar) {
	case 'C':
		if (bCtrlPressed)
			CopyFilesToClipBoard();
	break;
	case VK_DELETE:
		CAutoDisableNotificaltion autoDisableNotificaltion(this);
		RemoveSelectedFiles(GetAsyncKeyState(VK_SHIFT) != 0);
	break;
	}
	__super::OnKeyDown(nChar, nReptCnt, nFlags);
}
void CSaveListResultCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (!IsNotificationDisabled()) {
		CAutoDisableNotificaltion autoDisableNotificaltion(this);
		CListCtrlUtil::OnLButtonDown(nFlags, point);
	}
	else {
		CListCtrlUtil::OnLButtonDown(nFlags, point);
	}
}
void CSaveListResultCtrl::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CListCtrlUtil::OnChar(nChar, nRepCnt, nFlags);
	if (nChar == VK_SPACE) {
		GetParent()->PostMessage(WM_TOGGLE_PREVIEW);
	}
}
void CSaveListResultCtrl::CopyPath()
{	POSITION pos = GetFirstSelectedItemPosition();

	if (pos != NULL && OpenClipboard()) {
		EmptyClipboard();
		CString path = GetItemText(GetNextSelectedItem(pos), 1);
		HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE, 
			(path.GetLength()+1) * sizeof(TCHAR)); 
		if (hglbCopy != NULL) {
			// Lock the handle and copy the text to the buffer.
			LPTSTR lptstrCopy = (LPTSTR)GlobalLock(hglbCopy);
			lstrcpy(lptstrCopy, path);
			GlobalUnlock(hglbCopy); 
			// Place the handle on the clipboard.
#if defined(_UNICODE) || defined(UNICODE)
				SetClipboardData(CF_UNICODETEXT, hglbCopy);
#else
			SetClipboardData(CF_TEXT, hglbCopy);
#endif
		}
		CloseClipboard();
	}
}

void CSaveListResultCtrl::FilterDuplicates(CDialog *pParentDialog)
{
	if (GetItemCount() <= 0)
		return;
	CDupFileFilter duplicateFilter(this, pParentDialog);
	duplicateFilter.ApplyFilter();
}

void CSaveListResultCtrl::OnLVNDeleteItem(int nItem)
{
	CListResItemData* pItemData = (CListResItemData*)GetItemData(nItem);
	if (pItemData)
		delete pItemData;
//	SetItemData(nItem, 0);
}

void CSaveListResultCtrl::OnLVNDeleteAllItem()
{
	m_iColSorted = -1;
	//int count = GetItemCount();
	//while (count-- > 0)
	//	OnLVNDeleteItem(count);
}

LONGLONG CSaveListResultCtrl::GetFileSize(int nItem)
{
	LONGLONG size = 0;
	int itemCount(GetItemCount());
	if (nItem < 0 || nItem >= itemCount) {
		if (nItem == -1) { // Count size of all items
			for (nItem = 0; nItem < itemCount; ++nItem) {
				CListResItemData* pItemData = (CListResItemData*)GetItemData(nItem);
				if (pItemData)
					size += pItemData->m_ullFileSize;
			}
		}
		else { // count size of all selected items
			POSITION pos(GetFirstSelectedItemPosition());
			while (pos) {
				nItem = GetNextSelectedItem(pos);
				CListResItemData* pItemData = (CListResItemData*)GetItemData(nItem);
				if (pItemData)
					size += pItemData->m_ullFileSize;
			}
		}
	}
	else {
		CListResItemData* pItemData = (CListResItemData*)GetItemData(nItem);
		if (pItemData)
			size = pItemData->m_ullFileSize;
	}
	return size;
}

void CSaveListResultCtrl::CFindOption::CreateFindDialog(CSaveListResultCtrl *pList)
{
	if (m_pFindDialog == NULL) {
		m_pFindDialog = new CFindReplaceDialogEx(pList, mCurrentStringType);
		m_pFindDialog->m_fr.lStructSize = sizeof(FINDREPLACE);
		m_pFindDialog->m_fr.hwndOwner = pList->m_hWnd;
		m_pFindDialog->m_fr.lpstrReplaceWith = NULL;
		m_pFindDialog->m_fr.wReplaceWithLen = 0;
		m_pFindDialog->m_fr.Flags |= FR_FINDNEXT;
		if (!m_bMatchCaseInSen)
			m_pFindDialog->m_fr.Flags |= FR_MATCHCASE;
		if (m_bMatchFullPath)
			m_pFindDialog->m_fr.Flags |= FR_WHOLEWORD;
		m_pFindDialog->Create(TRUE, m_StrFind, NULL, FR_DOWN, pList);
	}
	m_pFindDialog->BringWindowToTop();
}

CFindReplaceDialogEx::FindStringType CSaveListResultCtrl::CFindOption::GetFindStringType()
{
	if (m_pFindDialog)
		mCurrentStringType = m_pFindDialog->GetFindStringType();
	return mCurrentStringType;
}
CString& CSaveListResultCtrl::CFindOption::GetFindString()
{
	if (m_pFindDialog) {
		m_StrFind = m_pFindDialog->GetFindString();
		m_bMatchCaseInSen = !m_pFindDialog->MatchCase();
		m_bMatchFullPath = !!m_pFindDialog->MatchWholeWord();
	}
	return m_StrFind;
}
void CSaveListResultCtrl::Find()
{
	m_FindOption.CreateFindDialog(this);
}
void CSaveListResultCtrl::FindNext(bool bUp, bool bAll)
{
	CString findText = m_FindOption.GetFindString();
	if (findText.IsEmpty())
		return;
	CFindReplaceDialogEx::FindStringType strType(m_FindOption.GetFindStringType());
	bool isRegularExp(false);
	bool isTrueRegExp(false);
	switch (strType) {
	case CFindReplaceDialogEx::PlainText:
		break;
	case CFindReplaceDialogEx::RegularExpression:
		isTrueRegExp = true;
	case CFindReplaceDialogEx::WildCard:
		isRegularExp = true;
		break;
	default:
		isRegularExp = IsWildCardExp(findText);
	}
	
	CRegExpMatcher stringMatcher;
	if (isRegularExp)
		stringMatcher.SetExpression(findText, isTrueRegExp, !m_FindOption.MatchCaseInSensitive());
	if (m_FindOption.MatchCaseInSensitive())
		findText.MakeLower();
	int itemCount = GetItemCount();
	if (itemCount == 0)
		return;
	int i = GetSelectionMark();
	if (i < 0)
		i = bUp ? itemCount : -1;
	int curSel = i;
	RemoveAllSelection(); // Remove all selection
	int incr = bUp ? itemCount-1 : 1;
	int iCount = itemCount;
	bool bFound = false;
	int iCol = m_FindOption.MatchFullPath() ? 1 : 0;
	GetFindDlg()->SetStatusMessage(_T(""));
	while (iCount-- > 0) {
		i = (i+incr)%itemCount;
		CString str = GetItemText(i, iCol);
		bool bMatched(false);
		if (isRegularExp) {
			bMatched = stringMatcher.Match(str);
		}
		else {
			if (m_FindOption.MatchCaseInSensitive())
				str.MakeLower();
			if (str.Find(findText) >= 0)
				bMatched = true;
		}
		if (bMatched)
		{
			SetSelectionMark(i);
			SetItemState(i, LVIS_SELECTED|LVIS_FOCUSED , LVIS_SELECTED|LVIS_FOCUSED ); // Select it
			if (!bAll && (!bFound && (bUp && i >= curSel || !bUp && i <= curSel)))
				GetFindDlg()->SetStatusMessage(_T("Passed the end of the list"));
			bFound = true;
			if (!bAll) {
				EnsureVisible(i, FALSE);
				break;
			}
		}
	}
	if (!bFound) {
		GetFindDlg()->SetStatusMessage(_T("The specified text was not found: %s"), (LPCTSTR)findText);
	}
}
LRESULT CSaveListResultCtrl::OnFindReplace(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(wParam);
	CFindReplaceDialog *pDlg = CFindReplaceDialog::GetNotifier(lParam);
	if (!pDlg)
		return 0;
	if (pDlg->FindNext()) {
		FindNext(!pDlg->SearchDown());
	}
	if (pDlg->IsTerminating()) {
		m_FindOption.Delete();
	}
	return 0;
}
void CSaveListResultCtrl::OnPaint()
{
	if (!mbDrawingDisabled)
		CListCtrlUtil::OnPaint();
}
static LPCTSTR optionalColumnText[] = {
	 _T("Size"),
	 _T("Date created"),
	 _T("Date modified"),
	 _T("Date accessed")
};
static int optionalColumnSize[] = {
	 SystemUtils::GetTranslatedDPIPixelX(80),
	 SystemUtils::GetTranslatedDPIPixelX(120),
	 SystemUtils::GetTranslatedDPIPixelX(120),
	 SystemUtils::GetTranslatedDPIPixelX(120)
};
bool CSaveListResultCtrl::AddOptionalColumn(ListColumns optionalComun)
{
	bool bSuccess(optionalComun < ListColumns_OptianlCount);

	if (bSuccess) {
		if (optionalComun == ListColumns_FileIcon)
			m_iOptionalColumns[ListColumns_FileIcon] = 0;
		else if (!IsOptionalColumnPresent(optionalComun)) {
			int iCol = InsertColumn(optionalComun+2, optionalColumnText[optionalComun], 0, optionalColumnSize[optionalComun]);
			for (int i = 0; i < ListColumns_OptianlCount; ++i) {
				if (m_iOptionalColumns[i] >= iCol)
					++m_iOptionalColumns[i];
			}
			m_iOptionalColumns[optionalComun] = iCol;
			AdjustColumnWidth();
		}
	}
	return bSuccess;
}
bool CSaveListResultCtrl::RemoveOptionalColumn(ListColumns optionalComun)
{
	bool bSuccess(optionalComun < ListColumns_OptianlCount);
	if (bSuccess) {
		if (optionalComun == ListColumns_FileIcon)
			m_iOptionalColumns[ListColumns_FileIcon] = -1;
		if (IsOptionalColumnPresent(optionalComun)) {
			bSuccess = DeleteColumn(m_iOptionalColumns[optionalComun]) == TRUE;
			for (int i = 0; i < ListColumns_OptianlCount; ++i) {
				if (m_iOptionalColumns[i] > m_iOptionalColumns[optionalComun])
					--m_iOptionalColumns[i];
			}
			m_iOptionalColumns[optionalComun] = -1;
			AdjustColumnWidth();
		}
	}
	return bSuccess;
}
bool CSaveListResultCtrl::IsOptionalColumnPresent(ListColumns optionalComun) const
{
	return optionalComun < ListColumns_OptianlCount && m_iOptionalColumns[optionalComun] >= 0;
}
int CSaveListResultCtrl::GetOptionalColumnsIndex(ListColumns optionalComun) const
{
	return optionalComun < ListColumns_OptianlCount && optionalComun != ListColumns_FileIcon ?  m_iOptionalColumns[optionalComun] : -1;
}
BOOL CSaveListResultCtrl::SetOptionalColumnItemText(int item, ListColumns optionalComun, LPCTSTR text)
{
	int iCol = GetOptionalColumnsIndex(optionalComun);
	if (iCol > 0)
		return SetItemText(item, iCol, text);
	return FALSE;
}
BOOL CSaveListResultCtrl::UpdateOptionalDateColumns(int item)
{
	BOOL bUpdateResult(FALSE);
	CListResItemData *pListItemData((CListResItemData *)GetItemData(item));
	if (pListItemData != NULL) {
		if (IsOptionalColumnPresent(ListColumns_CreatedTime)) {
			bUpdateResult = SetOptionalColumnItemText(item, ListColumns_CreatedTime, SystemUtils::DateToRString(pListItemData->mCreatedTime));
		}
		if (IsOptionalColumnPresent(ListColumns_ModifiedTime)) {
			bUpdateResult = SetOptionalColumnItemText(item, ListColumns_CreatedTime, SystemUtils::DateToRString(pListItemData->mModifedTime));
		}
		if (IsOptionalColumnPresent(ListColumns_AccessedTime)) {
			bUpdateResult = SetOptionalColumnItemText(item, ListColumns_CreatedTime, SystemUtils::DateToRString(pListItemData->mAccessedTime));
		}
	}
	return bUpdateResult;
}
int CSaveListResultCtrl::InsertItem(int nItem, LPCTSTR lpszItem, bool isFolder)
{
	int iconIndex = GetIconIndex(isFolder ? NULL : lpszItem);
	return __super::InsertItem(nItem, lpszItem, iconIndex);
}
int CSaveListResultCtrl::GetIconIndex(LPCTSTR itemText)
{
	int iconIndex = -1;
	if (IsOptionalColumnPresent(ListColumns_FileIcon))
		iconIndex = itemText == NULL ? FileIconMgr::GetInstance().GetFolderIconIndex() : FileIconMgr::GetInstance().GetIconIndex(itemText);
	return iconIndex;
}
BOOL CSaveListResultCtrl::UpdateOptionalColumn(int item, ListColumns optionalComun)
{
	CListResItemData *pListItemData((CListResItemData *)GetItemData(item));
	CString text;
	if (pListItemData != NULL) {
		switch (optionalComun) {
		case ListColumns_Size:
		{
			LONGLONG size = pListItemData->m_ullFileSize;
			if (size >= 0) {
				text = SystemUtils::GetReadableSize(size);
			}
		}
		break;
		case ListColumns_CreatedTime:
			text = SystemUtils::DateToRString(pListItemData->mCreatedTime);
			break;
		case ListColumns_ModifiedTime:
			text = SystemUtils::DateToRString(pListItemData->mModifedTime);
			break;
		case ListColumns_AccessedTime:
			text = SystemUtils::DateToRString(pListItemData->mAccessedTime);
			break;
		case ListColumns_FileIcon:
			{
				int iconIndex = GetIconIndex(pListItemData->m_ullFileSize < 0 ? NULL : GetItemText(item, 0));
				SetItem(item, 0, LVIF_IMAGE, NULL, iconIndex, 0, 0, NULL);
			}
			break;
		}
	}
	return SetOptionalColumnItemText(item, optionalComun, text);
}
int CSaveListResultCtrl::GetColumnDataIndex(int iCol)
{
	if (iCol > 1) {
		for (int i = 0; i < ListColumns_OptianlCount; ++i) {
			if (m_iOptionalColumns[i] == iCol) {
				iCol = 2 + i;
				break;
			}
		}
	}
	return iCol;
}
bool CSaveListResultCtrl::UpdateImageList()
{
	bool bSuccss(FileIconMgr::GetInstance().GetFolderIconIndex()>=0);
	CImageList *pImageList(IsOptionalColumnPresent(ListColumns_FileIcon) ? FileIconMgr::GetInstance().GetImageList() : NULL);
	SetImageList(pImageList, LVSIL_SMALL);
	return bSuccss;
}
void CSaveListResultCtrl::SaveDefault(FindDataBase &findDb)
{
	for (int i = ListColumns_Size+1; i < ListColumns_OptianlCount; ++i) {
		CString propertyName;
		propertyName.Format(_T("col_present_%d"), i);
		if (IsOptionalColumnPresent((ListColumns)i))
			findDb.SetProperty(propertyName, _T("1"));
		else
			findDb.RemoveProperty(propertyName);
	}
}
void CSaveListResultCtrl::LoadDefault(FindDataBase &findDb)
{
	for (int i = ListColumns_Size+1; i < ListColumns_OptianlCount; ++i) {
		CString propertyName;
		propertyName.Format(_T("col_present_%d"), i);
		if (findDb.GetProperty(propertyName) == _T("1"))
			AddOptionalColumn((ListColumns)i);
		else
			RemoveOptionalColumn((ListColumns)i);
	}
}

class CSaveListResultCtrlBinarySOnWeight {
public:
	CSaveListResultCtrlBinarySOnWeight(CSaveListResultCtrl *pList);
	INT_PTR GetCount() const;
	int GetAt(INT_PTR i) const; 
private:
	CSaveListResultCtrl *m_pList;
	int mITemCount;
};
CSaveListResultCtrlBinarySOnWeight::CSaveListResultCtrlBinarySOnWeight(CSaveListResultCtrl *pList) : m_pList(pList)
{
	mITemCount = m_pList->GetItemCount();
}
INT_PTR CSaveListResultCtrlBinarySOnWeight::GetCount() const
{
	return mITemCount;
}
int CSaveListResultCtrlBinarySOnWeight::GetAt(INT_PTR i) const
{
	int index((int)i);
	index = mITemCount-1 - index;
    CListResItemData *pItemData((CListResItemData*)m_pList->GetItemData(index));
	return pItemData ? pItemData->mMatchWeight : 0;
}
int CSaveListResultCtrl::GetItemMatchingWeight(int matchWeight)
{
	int index(-1);
	if (matchWeight > 2) {
		CSaveListResultCtrlBinarySOnWeight binSrchOnWt(this);
		CBinarySearch<int, CSaveListResultCtrlBinarySOnWeight> bs(binSrchOnWt);
		index = ((int)bs.GetInsertIndex(matchWeight));
		index = (int)binSrchOnWt.GetCount() - index;
	}
	return index;
}
