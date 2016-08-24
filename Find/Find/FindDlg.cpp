// FindDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Find.h"
#include "FindDlg.h"
#include "Finder.h"
#include "NetWorkFinder.h"
#include "TreeCtrlIterator.h"
#include "Percentage.h"
#include "FileDialogFind.h"
#include "Preferences.h"
#include "SystemUtils.h"
#include "FindDataBase.h"
#include "Path.h"
#include "ThreadManager.h"
#include "FileMetaDataProvider.h"

#define WM_ON_FIRST_SHOW WM_USER+3010
#define TIMER_TERMINATE 1
#define TIMER_UPDATESTATUS 2
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IContextMenu2  *g_pcm2;

struct ThreadData {
	CFindDlg *pFindDlg;
	ThreadOperation threadOp;
	LPVOID pThreadData;
	ThreadData(CFindDlg *inpFindDlg, ThreadOperation inthreadOp, LPVOID inpThreadData)
	{
		pFindDlg = inpFindDlg;
		threadOp = inthreadOp;
		pThreadData = inpThreadData;
	}
};

int FindCallBackFolders(CFileFindEx *pFindFile, bool bFileMatched, void *pUserParam)
{
	CFindDlg *findDlg = (CFindDlg *)pUserParam;
	return findDlg->FindFolderCallback(pFindFile, bFileMatched);
}

int NetWorkFindShared(LPNETRESOURCE lpNetRes, void *pUserParam)
{
	CFindDlg *findDlg = (CFindDlg *)pUserParam;
	if (RESOURCEUSAGE_CONTAINER != (lpNetRes->dwUsage & RESOURCEUSAGE_CONTAINER)) {
		return findDlg->SearchInFolder(lpNetRes->lpRemoteName);
	}
	else {
		lpNetRes = CopyLPNetResource(lpNetRes);
		lpNetRes->lpComment = (LPTSTR)kNETRESOURCEComment;
		if (!findDlg->StartThreadToSearchInNetwork(lpNetRes))
			FreeLPNetResource(lpNetRes); // Thread not started - free the data
	}
	return findDlg->IsSearchCancelled() ? FCB_ABORT : FCB_CONTINUE;
}

static int TreeNetWorkIteratorCallBack(TreeIteratorCallBackData *pData, void *pUserParam)
{
	HTREEITEM hItem = pData->hItem;
	CTreeCtrlDomain *pTree = (CTreeCtrlDomain *)pData->pTree;
	// Process and find
	LPNETRESOURCE lpnRes = (LPNETRESOURCE)pTree->GetItemData(hItem);
	if (lpnRes < (LPNETRESOURCE)3) {
		lpnRes = NULL;
	}
	if (lpnRes) {
		((CFindDlg*)pUserParam)->SetStatusMessage(_T("Searching in %s"), lpnRes->lpRemoteName);
		if (RESOURCEUSAGE_CONTAINER != (lpnRes->dwUsage & RESOURCEUSAGE_CONTAINER)) {
			((CFindDlg*)pUserParam)->SearchInFolder(lpnRes->lpRemoteName);
		}
		else {
			((CFindDlg*)pUserParam)->StartThreadToSearchInNetwork(lpnRes);
		}
	}

	int returnVal = ((CFindDlg*)pUserParam)->IsSearchCancelled() ? TICB_ABORT
		: pData->hStartItem == hItem ? TICB_DONTITERATE_SIBLING : TICB_CONTINUE;
	return returnVal | TICB_DONTITERATE_CHILREN;
}

DWORD WINAPI FindDlgThreadProc(LPVOID lpParameter)
{
	ThreadData *pThreadData = (ThreadData *)lpParameter;
	try {
		pThreadData->pFindDlg->DoThreadOperation(pThreadData->threadOp, pThreadData->pThreadData);
	}
	catch (...) {
		_tprintf(_T("Exception!!!"));
	}
	try {
		pThreadData->pFindDlg->DoPostThreadOperation();
		delete pThreadData;
	}
	catch (...) {
	}

	return 0;
}

static int TMFindDlgThreadProcFn(LPVOID pThreadData)
{
	return FindDlgThreadProc(pThreadData);
}

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CFindDlg dialog


TableItertatorClass(CFindDlg);

CFindDlg::CFindDlg(CWnd* pParent /*=NULL*/)
: CDialog(CFindDlg::IDD, pParent), mTreeCtrlDomain(NULL),
m_uFlags(0), mControlResizer(this),
m_uDrapDropOpCount(0), mResizeBar(NULL),
mFindOptionDlg(this),
m_pPreviewController(NULL)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_hAccel = LoadAccelerators(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR));
	mStatusUpdateTimer.SetTimeUpdateDuration(200);
}

CFindDlg::~CFindDlg()
{
	if (m_pPreviewController != NULL)
		delete m_pPreviewController;
	DestroyAcceleratorTable(m_hAccel);
}
void CFindDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CFindDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_SIZING()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
	ON_NOTIFY(TVN_ITEMEXPANDING, IDC_TREE_DOMAIN, &CFindDlg::OnTvnItemexpandingTreeDomain)
	ON_NOTIFY(TVN_KEYDOWN, IDC_TREE_DOMAIN, &CFindDlg::OnTvnKeydownTreeDomain)
	ON_NOTIFY(TVN_DELETEITEM, IDC_TREE_DOMAIN, &CFindDlg::OnTvnDeleteitemTreeDomain)
	ON_BN_CLICKED(IDOK, &CFindDlg::OnBnClickedOk)
	ON_WM_DESTROY()
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_RESULT, &CFindDlg::OnNMDblclkListResult)
	ON_CBN_SELCHANGE(IDC_COMBO_FIND, &CFindDlg::OnCbnSelchangeComboFind)
	ON_NOTIFY(HDN_ITEMCLICK, 0, &CFindDlg::OnHdnItemclickListResult)
	ON_NOTIFY(LVN_BEGINDRAG, IDC_LIST_RESULT, &CFindDlg::OnLvnBegindragListResult)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_RESULT, &CFindDlg::OnLvnItemchangedListResult)
	ON_NOTIFY(LVN_DELETEITEM, IDC_LIST_RESULT, &CFindDlg::OnLvnDeleteitemListResult)
//	ON_NOTIFY(LVN_DELETEALLITEMS, IDC_LIST_RESULT, &CFindDlg::OnLvnDeleteallitemsListResult)
	ON_COMMAND(ID_FILE_PREFERENCES, &CFindDlg::OnFilePreferences)
	ON_WM_DEVICECHANGE()
    ON_WM_DROPFILES()
END_MESSAGE_MAP()

// CFindDlg message handlers

LRESULT CFindDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_USER+3000:
		switch (lParam) {
		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
			{
				ShowWindow(SW_RESTORE);
				SetForegroundWindow();
				NOTIFYICONDATA nicData = {sizeof (NOTIFYICONDATA)};
				nicData.hWnd = m_hWnd;
				nicData.uID = 1;
				Shell_NotifyIcon(NIM_DELETE, &nicData);
			}
			break;
		}
		break;
	case WM_SETCURSOR:
		if ((HWND)wParam == ::GetDlgItem(m_hWnd, IDC_STATIC_VERT_SEP))
		{
			SetCursor(m_hCurSorSizeWE);
			return 1;
		}
		break;
	case WM_INITMENUPOPUP:
		mListResult->InitExplorerContextMenu((HMENU)wParam);
	case WM_DRAWITEM:
	case WM_MENUCHAR:
	case WM_MEASUREITEM:
		if(g_pcm2)
		{
			g_pcm2->HandleMenuMsg(message, wParam, lParam);
		}
		break;
	case WM_ON_FIRST_SHOW:
		{
			if (!mPreferenceName.IsEmpty()) {
				CPreferences prefDlg;
				prefDlg.LoadPreference(mPreferenceName);
			}
            if (wParam == 0)
			    mResizeBar->HideControl();
			StartThreadOperation(THREAD_OP_LOAD_LAST_RESULT);
		}
		break;
	case WM_SET_STATUS_MESSAGE:
		{
			LPCTSTR stMSG((LPCTSTR)wParam);
			if (stMSG == NULL)
				stMSG = _T("");
			SetStatusMessage(_T("%s"), stMSG);
		}
		break;
	case WM_TOGGLE_PREVIEW:
		TogglePreview();
		break;
	case WM_CREATE_PREVIEW:
		return (LRESULT)((CDialogPreview*)(wParam))->GetDialogPreviewsEx((int)lParam);
		break;
	case WM_LOAD_PREVIEW:
		return (LRESULT)((CDialogPreviewBase*)(wParam))->Load((LPCTSTR)lParam);
		break;
	case WM_RESIZE_PREVIEW:
		if (m_pPreviewController != NULL)
			m_pPreviewController->PerformAction(CPreviewController::CA_ReSize);
		break;
	case WM_TIMER:
		switch (wParam) {
		case TIMER_TERMINATE:
			if (m_pPreviewController == NULL || m_pPreviewController->IsPreviewTerminated())
				PostMessage(WM_CLOSE);
            break;
        case TIMER_UPDATESTATUS:
            SetForceUpdateStatus();
            SetStatusMessage(_T("%s"), CString(mStatusMsg));
            KillTimer(TIMER_UPDATESTATUS);
            SetStatusTimerStarted(false);
            break;
        }
        break;
    case WM_FINDTREE_EXPAND_PATH:
        StartThreadOperation(THREAD_OP_EXPAND_TREE_NODE, (LPVOID)wParam);
        break;
	}
	return CDialog::WindowProc(message, wParam, lParam);
}
BOOL CFindDlg::PreTranslateMessage(MSG* pMsg)
{
	if (TranslateAccelerator(m_hWnd, m_hAccel, pMsg))
		return TRUE;
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) {
		CWnd *pWnd = GetCapture();
		if (pWnd) {
			pWnd->SendMessage(pMsg->message, pMsg->wParam, pMsg->lParam);
		}
		else {
			ShowWindow(SW_MINIMIZE);
		}
		return TRUE;
	}
	switch (pMsg->message) {
	case WM_SYSKEYDOWN:
		if (pMsg->wParam == VK_MENU && !IsSearchStarted()) {
			SetDiplayFindOption(!IsDiplayFindOption());
			CString buttonText;
			buttonText.LoadString(IDS_FIND);
			if (IsDiplayFindOption())
				buttonText += _T("...");
			SetDlgItemText(IDOK, buttonText);
		}
		break;
	case WM_KEYDOWN:
		if (pMsg->wParam == VK_F3) {
			mListResult->FindNext(GetAsyncKeyState(VK_SHIFT)!=0);
		}
		break;
	}
	return CDialog::PreTranslateMessage(pMsg);
}
void CFindDlg::OnSizing(UINT nSize, LPRECT lpRect)
{
	if (lpRect->right - lpRect->left < SystemUtils::GetTranslatedDPIPixelX(400)) {
		switch (nSize) {
		case WMSZ_BOTTOMRIGHT:
		case WMSZ_RIGHT:
		case WMSZ_TOPRIGHT:
			lpRect->right = lpRect->left + SystemUtils::GetTranslatedDPIPixelX(400);
			break;
		case WMSZ_BOTTOMLEFT:
		default:
			lpRect->left = lpRect->right - SystemUtils::GetTranslatedDPIPixelX(400);
		}
	}
	if (lpRect->bottom - lpRect->top < SystemUtils::GetTranslatedDPIPixelY(400)) {
		switch (nSize) {
		case WMSZ_BOTTOM:
		case WMSZ_BOTTOMLEFT:
		case WMSZ_BOTTOMRIGHT:
			lpRect->bottom = lpRect->top + SystemUtils::GetTranslatedDPIPixelY(400);
			break;
		default:
			lpRect->top = lpRect->bottom - SystemUtils::GetTranslatedDPIPixelY(400);
		}
	}
	CDialog::OnSizing(nSize, lpRect);
}
void CFindDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	switch (nType) {
	case SIZE_MINIMIZED:
		{
			NOTIFYICONDATA nicData = {sizeof (NOTIFYICONDATA)};
			nicData.hWnd = m_hWnd;
			nicData.uID = 1;
			nicData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
			nicData.uCallbackMessage = WM_USER+3000;
			nicData.hIcon = m_hIcon;
			lstrcpy(nicData.szTip, _T("Find"));
			if (Shell_NotifyIcon(NIM_ADD, &nicData))
				ShowWindow(SW_HIDE);
		}
		break;
	default:
		if (mResizeBar)
		{
			mResizeBar->OnParentResize();
		}
		mControlResizer.DoReSize();
		break;
	}
}
void CFindDlg::OnCancel()
{
	if (GetCapture())
		return;
	ShowWindow(SW_HIDE);
	bool bTerminate(true);
	if (m_pPreviewController != NULL) {
		m_pPreviewController->PerformAction(CPreviewController::CA_Terminate);
		if (!m_pPreviewController->IsPreviewTerminated()) {
			SetTimer(TIMER_TERMINATE, 50, NULL);
			bTerminate = false;
		}
	}
	if (IsSearchStarted()) {
		SetSearchCancelled(true);
		SetClosed(true);
	}
	else if (bTerminate) {
		SaveSearchKeyWords();
		CDialog::OnCancel();
	}
}
BOOL CFindDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
    DragAcceptFiles();
	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}
	m_hCurSorSizeWE = LoadCursor(NULL, IDC_SIZEWE);
	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	RECT rc;
	GetDlgItem(IDC_TREE_DOMAIN)->GetWindowRect(&rc);
	ScreenToClient(&rc);
	int bottomMargin(rc.bottom);
	GetDlgItem(IDC_STATIC_VERT_SEP)->GetWindowRect(&rc);
	ScreenToClient(&rc);
	rc.bottom = bottomMargin;
	GetDlgItem(IDC_STATIC_VERT_SEP)->SetWindowPos(NULL, 0, 0, SystemUtils::GetTranslatedDPIPixelX(5), rc.bottom-rc.top, SWP_NOMOVE|SWP_NOZORDER);
	mTreeCtrlDomain = new CTreeCtrlDomain();
	mTreeCtrlDomain->SubclassDlgItem(IDC_TREE_DOMAIN, this);
    mTreeCtrlDomain->DragAcceptFiles();
    mComboBoxFindText.SubclassDlgItem(IDC_COMBO_FIND, this);
    mComboBoxFindText.DragAcceptFiles();
	mListResult = new CSaveListResultCtrl(this);
	mListResult->SubclassDlgItem(IDC_LIST_RESULT, this);
	mCommitResultTimer.SetListCtrl(mListResult);
	mResizeBar = new CResizeBar(this);
	mResizeBar->SubclassDlgItem(IDC_STATIC_VERT_SEP, this);
	mResizeBar->OnSetCursor(NULL, 0, 0);
	mResizeBar->SetMinLeftRight(SystemUtils::GetTranslatedDPIPixelX(200), SystemUtils::GetTranslatedDPIPixelX(300));
	mTreeCtrlDomain->SetItemHeight((SHORT)SystemUtils::GetTranslatedDPIPixelX(18));
	mTreeCtrlDomain->RefreshDomainTree();
	// Add col to list result
	mListResult->InsertColumn(0, _T("Name"), 0, SystemUtils::GetTranslatedDPIPixelX(160));
	mListResult->InsertColumn(1, _T("Path"), 0, SystemUtils::GetTranslatedDPIPixelX(270));
	mListResult->AddOptionalColumn(ListColumns_Size);
	mListResult->SetExtendedStyle(mListResult->GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP);
	mControlResizer.AddControl(IDC_LIST_RESULT);
	mControlResizer.AddControl(IDC_TREE_DOMAIN, RSZF_BOTTOM_FIXED);
	mControlResizer.AddControl(IDC_STATIC_STATUS, RSZF_RIGHT_FIXED | RSZF_BOTTOM_FIXED | RSZF_SIZEY_FIXED);
	mControlResizer.AddControl(IDC_COMBO_FIND, RSZF_RIGHT_FIXED);
	mControlResizer.AddControl(IDOK, RSZF_RIGHT_FIXED|RSZF_SIZE_FIXED);
	mControlResizer.AddControl(IDC_STATIC_VERT_SEP, RSZF_BOTTOM_FIXED|RSZF_SIZEX_FIXED);
	CControlResizer &dragControlResizer(mResizeBar->GetControlResizer());
	dragControlResizer.AddControl(IDC_LIST_RESULT, RSZF_RIGHT_FIXED | RSZF_BOTTOM_FIXED | RSZF_RESIZE_OPPOSITE);
	dragControlResizer.AddControl(IDC_TREE_DOMAIN, RSZF_BOTTOM_FIXED | RSZF_LEFT_FIXED);
	dragControlResizer.AddControl(IDC_STATIC_SEARCH_DOMAIN, RSZF_NO_RESIZE);
	dragControlResizer.AddControl(IDC_COMBO_FIND, RSZF_RIGHT_FIXED | RSZF_RESIZE_OPPOSITE);
	dragControlResizer.AddControl(IDC_STATIC_VERT_SEP, RSZF_BOTTOM_FIXED|RSZF_SIZEX_FIXED);
	dragControlResizer.AddControl(IDC_STATIC_FIND, RSZF_SIZEX_FIXED);
	SystemUtils::SetWindowPos(GetSafeHwnd(), NULL, 0, 0, 800, 600, SWP_DRAWFRAME|SWP_FRAMECHANGED|SWP_NOMOVE|SWP_NOZORDER);
	{
		FindDataBase fdb;
		fdb.LoadSchema();
	}
	LoadSearchKeyWords();
	CComboBox *pComboBox = (CComboBox *)GetDlgItem(IDC_COMBO_FIND);
	COMBOBOXINFO cbInfo = {sizeof(COMBOBOXINFO)};
	pComboBox->GetComboBoxInfo(&cbInfo);
	SHAutoComplete(cbInfo.hwndItem, SHACF_DEFAULT);
	CString cueText;
	cueText.LoadString(IDS_STRING_EDIT_FIND_CUE_BANNER);
	pComboBox->SetCueBanner(cueText);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

BOOL CFindDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	switch (LOWORD(wParam)) {
	case ID_OPEN_OPEN:
	case ID_OPEN_OPENCONTAININGFOLDER:
	case ID_OPEN_PROPERTIES:
	{
		POSITION pos = mListResult->GetFirstSelectedItemPosition();
		if (pos == NULL)
			break;
		int nItem = mListResult->GetNextSelectedItem(pos);
		CString path = mListResult->GetItemText(nItem, 1);
		CString cParams;
		SHELLEXECUTEINFO shExInfo = {sizeof(SHELLEXECUTEINFO)};
		if (LOWORD(wParam) == ID_OPEN_OPENCONTAININGFOLDER) {
			cParams = _T("/select,") + path;
			path = _T("explorer.exe");
			shExInfo.lpParameters = cParams;
		}
		if (LOWORD(wParam) == ID_OPEN_PROPERTIES) {
			shExInfo.lpVerb = _T("properties");
			shExInfo.fMask = SEE_MASK_INVOKEIDLIST;
		}
		else
			shExInfo.lpVerb = _T("open");
		shExInfo.nShow = SW_SHOWDEFAULT;
		shExInfo.lpFile = path;
		BOOL l = ShellExecuteEx(&shExInfo);
		l = l;
	}
	break;
	case ID_OPEN_SIZECOLUMN:
	case ID_SELECTCOLUMNS_DATECREATED:
	case ID_SELECTCOLUMNS_DATEMODIFIED:
	case ID_SELECTCOLUMNS_DATEACCESSED:
	case ID_SELECTCOLUMNS_SHOWICONS:
		{
			ListColumns optionalColumn(ListColumns_Size);
			bool bUpdateCol(false);
			switch (LOWORD(wParam)) {
				case ID_SELECTCOLUMNS_DATECREATED:
					optionalColumn = ListColumns_CreatedTime;
					break;
				case ID_SELECTCOLUMNS_DATEMODIFIED:
					optionalColumn = ListColumns_ModifiedTime;
					break;
				case ID_SELECTCOLUMNS_DATEACCESSED:
					optionalColumn = ListColumns_AccessedTime;
					break;
				case ID_SELECTCOLUMNS_SHOWICONS:
					optionalColumn = ListColumns_FileIcon;
					bUpdateCol = true;
					break;
			}
			if (mListResult->IsOptionalColumnPresent(optionalColumn)) {
				mListResult->RemoveOptionalColumn(optionalColumn);
			}
			else {
				mListResult->AddOptionalColumn(optionalColumn);
				bUpdateCol = true;
			}
			if (bUpdateCol)
				StartThreadOperation(THREAD_OP_FILL_COL_OPTIONAL, (LPVOID)optionalColumn);
		}
		break;
	case ID_SAVESEARCHRESULT_PARTIAL:
	case ID_SAVESEARCHRESULT_FULL:
		StartThreadOperation(THREAD_OP_SAVE_SEARCH_RESULT, (LPVOID)(LOWORD(wParam) == ID_SAVESEARCHRESULT_FULL));
		break;
	case ID_OPEN_SELECTALL:
		mListResult->SelectAllItems();
		break;
	case ID_OPEN_INVERTSELECTION:
		mListResult->InvertSelection();
		break;
	case ID_OPEN_COPYFILES:
		mListResult->CopyFilesToClipBoard();
		break;
	case ID_OPEN_DELETEFILES:
	case ID_OPEN_REMOVE:
		mListResult->RemoveSelectedFiles(LOWORD(wParam) == ID_OPEN_DELETEFILES);
		break;
	case ID_OPEN_COPYPATH:
		mListResult->CopyPath();
		break;
	case ID_OPEN_FILTERDUPLICATES:
		StartThreadOperation(THREAD_OP_FILTER_DUPLICATES);
		break;
	case ID_OPEN_FIND:
		mListResult->Find();
		break;
	}
	return CDialog::OnCommand(wParam, lParam);
}

void CFindDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CFindDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CFindDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CFindDlg::OnDropFiles(HDROP hDropInfo)
{
    mTreeCtrlDomain->OnDropFiles(hDropInfo);
}

void CFindDlg::OnTvnItemexpandingTreeDomain(NMHDR *pNMHDR, LRESULT *pResult)
{
	mTreeCtrlDomain->OnTvnItemexpanding(pNMHDR, pResult);
}

void CFindDlg::RefreshDomainTree(void)
{
	if (IsSearchStarted())
		return;
	mTreeCtrlDomain->RefreshDomainTree();
}

void CFindDlg::OnTvnKeydownTreeDomain(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTVKEYDOWN pTVKeyDown = reinterpret_cast<LPNMTVKEYDOWN>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;
	if (pTVKeyDown->wVKey == VK_F5) // Refresh Tree
		RefreshDomainTree();
	else if (pTVKeyDown->wVKey == VK_CANCEL && IsSearchStarted()) // Cancel
		SetSearchCancelled(true);
}
void CFindDlg::OnTvnDeleteitemTreeDomain(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	*pResult = 0;
	mTreeCtrlDomain->OnTvnDeleteitem(pNMTreeView->itemOld.hItem);
}

void CFindDlg::GetFindList()
{
	SetStatusMessage(_T("Preparing directory list..."));
	mTreeCtrlDomain->GetCheckList(mSearchList);
	SetStatusMessage(_T(""));
}
HTREEITEM CFindDlg::SearchForNetWorkPC(const CString &inFullPath)
{
	CAutoDisableThreadOp autoDisableThreadOp(this);
	HTREEITEM hItem(NULL);
	CString fullPath(inFullPath);
	CString statusText;
	int pos(fullPath.Find(':'));
	if (pos > 1)
		fullPath = fullPath.Mid(pos+1);
	CString networkPath(RootNameFromPath(fullPath));
	if (PathIsNetworkPath((networkPath))) {
		CString networkName = networkPath.Right(networkPath.GetLength()-2);
		statusText = (_T("Searching "));
		statusText += networkPath + _T(" in tree control");
		SetStatusMessage(_T("%s"), statusText);
		hItem = mTreeCtrlDomain->FindAddNetworkPath(fullPath);
	}
	else {
		hItem = mTreeCtrlDomain->Expand(fullPath);
	}
	statusText = _T(" ");
	if (hItem) {
		mTreeCtrlDomain->SetFocus();
		mTreeCtrlDomain->SelectItem(hItem);
		mTreeCtrlDomain->SelectSetFirstVisible(hItem);
		mResizeBar->HideControl(false); // Show tree view
	}
	else {
		statusText += networkPath + _T(" not found.");
	}
	SetStatusMessage(_T("%s"), statusText);
	return hItem;
}
bool CFindDlg::CheckUncheckSearchNodes(const CString &pattern, HTREEITEM hItem /* = NULL */)
{
	int check = -1;
	if (pattern.Find(_T("uncheck:"))==0)
		check = 0;
	else if (pattern.Find(_T("check:")) == 0)
		check = 1;
	if (check > -1) {
		CString text = pattern;
		text.Delete(0, text.Find(':')+1);
		if (hItem == NULL)
			hItem = mTreeCtrlDomain->FindPath(pattern);
		if (hItem != NULL)
			mTreeCtrlDomain->SetCheck(hItem, check);
		return true;
	}
	return false;
}
void CFindDlg::Find() // Start Find
{
	mListResult->UpdateImageList();
	SetSearchStartedImpl();
	mFindText.Empty();
	mFileContentSearchText.Empty();
	SetPhoneticSearch(false);
	GetDlgItemText(IDC_COMBO_FIND, mFindText);
	CArrayCString outStr;
	if (SystemUtils::SplitString(mFindText, outStr) > 1) {
		CPhoneticStringMatcher pm(outStr[0]);
		pm.Match(outStr[1]);
		return;
	}
	bool bDone(false);
	CComboBox *pComboBox = (CComboBox *)GetDlgItem(IDC_COMBO_FIND);
	if (!mFindText.IsEmpty()) {
		int i = pComboBox->FindStringExact(-1, mFindText);
		DWORD editSel = pComboBox->GetEditSel();
		if (i >= 0)
			pComboBox->DeleteString(i);
		pComboBox->InsertString(0, mFindText);
		if (pComboBox->GetCount() == 1) {
			pComboBox->InsertString(-1, _T("Clear List..."));
		}
		pComboBox->SetCurSel(0);
		pComboBox->SetEditSel(HIWORD(editSel), LOWORD(editSel));
		HTREEITEM hItem(SearchForNetWorkPC(mFindText));
		if (hItem)
			bDone = true;
		if (CheckUncheckSearchNodes(mFindText, hItem))
			bDone = true;
	}
	bool bNetWorkSearchSelected(false);
	if (!bDone) {
		if (IsDiplayFindOption()) {
			mFindOptionDlg.DoModal();
			SetDiplayFindOption(false);
			mTreeCtrlDomain->SetSearchInZip(mFindOptionDlg.IsSearchZipEnabled());
		}
		GetFindList();
		mListResult->DeleteAllItems();
		mCommitResultTimer.DoCommit(CRTCF_REMOVE | CRTCF_FORCE);
		if (mFindText[0] == '?') {
			mFindText.Remove('?');
			SetPhoneticSearch(true);
		}
		if (mSearchList.empty()) {
			FindInCache(mFindText);
		}
		else {
			if (!mFindText.IsEmpty()) {
				CArrayCString outStrArr;
				SystemUtils::SplitString(mFindText, outStrArr, _T(":"));
				if (outStrArr.GetCount() > 2) {
					mFileContentSearchText = SystemUtils::CombineString(outStrArr, _T(":"), 2);
					mFindText = SystemUtils::CombineString(outStrArr, _T(":"), 0, 2);
				}
			}
			if (mFindText.IsEmpty()) {
				mFindText.SetString(_T("*")); // Search everything
			}
			HTREEITEMVec::const_iterator it = mSearchList.begin();
			int retVal = FIND_CONTINUE_SEARCH;
			while (it != mSearchList.end() && retVal != FIND_ABORT_SEARCH) {
				retVal = SearchInNetwork(*it);
				if (retVal == FIND_CONTINUE_SEARCH)
					retVal = SearchInFolder(mTreeCtrlDomain->GetFilePath(*it));
				else {
					bNetWorkSearchSelected = true;
					mCommitResultTimer.DoCommit();
				}
				++it;
			}
		}
	}
	// Wait for others to finish up
	while (ThreadManager::GetInstance().GetThreadCount(-1) > GetIdleThreadCount())
		Sleep(1000);
	SetStatusMessage(_T("%d files found."), mListResult->GetItemCount());
	if (bNetWorkSearchSelected)
		mCommitResultTimer.DoCommit(CRTCF_FORCE);
}
int CFindDlg::ItrSearchCachedDataTableRowsCallbackFn(sqlite3_stmt *statement, void *pUserData)
{
	if (IsSearchCancelled())
		return 1; // Abort
	Path path(SystemUtils::UTF8ToUnicodeCString((const char *)sqlite3_column_text(statement, CachedData_Path)));
	Path fileName(path.FileName());
	CString fileSize;
	LONGLONG llFileSize;
	bool bAdd(true);
	int matchWeight(0);
	StringMatcher *pStringMatcher(NULL);
	if (pUserData)
	{
		pStringMatcher = (StringMatcher *)pUserData;
		bAdd = pStringMatcher->Match(fileName);
		if (bAdd)
			matchWeight += 5;
		else
			bAdd = pStringMatcher->Match(path.Parent());
		if (bAdd)
			matchWeight += pStringMatcher->GetMatchWeight();
	}
	CCacheDataFileMetaDataProvider fileMetaDataProvider(statement, CachedData_Size, CachedData_CreatedTime, CachedData_ModifiedTime, CachedData_AccessedTime, CachedData_AddedTime);
	if (bAdd) {
		llFileSize = fileMetaDataProvider.GetFileSize();
		fileSize = SystemUtils::GetReadableSize(llFileSize);
		bAdd = mFindOptionDlg.CheckFileSatisfies(fileMetaDataProvider);
	}
	if (bAdd)
	{
		int item(mListResult->GetItemMatchingWeight(matchWeight));
		if (item < 0)
			item = 0x7fffffff;
		int maxCountLimit(pStringMatcher && pStringMatcher->GetMatchWeight() > 0 ? 200 : 0);
		if (maxCountLimit ==0  || item < maxCountLimit) {
			item = mListResult->InsertItem(item , fileName);
			CListResItemData *pListItemData = new CListResItemData(fileMetaDataProvider, matchWeight);
			mListResult->SetItemData(item, (DWORD_PTR)pListItemData);
			mListResult->SetItemText(item, 1, path);
			mListResult->SetOptionalColumnItemText(item, ListColumns_Size, fileSize);
			mListResult->UpdateOptionalDateColumns(item);
			if (maxCountLimit) {
				int totoalItemCount(mListResult->GetItemCount());
				if (totoalItemCount > maxCountLimit)
					mListResult->DeleteItem(totoalItemCount-1);
			}
		}
	}
	return 0;
}
void CFindDlg::FindInCache(CString findText)
{
	FindDataBase fdbCacheDB(FDB_CacheDatabase, true);
	if (!findText.IsEmpty() && findText.Find('\\', 3) < 0
		&& fdbCacheDB.Open() == 0) {
		SetStatusMessage(_T("Searching in cache..."));
		CRegExpMatcher stringMatcher;
		CStringMatcherList stringMatcherList(findText);
		CPhoneticStringMatcherList phoneticStringMatcherList(findText);
		StringMatcher *pStringMatcher(NULL);
		CString condition(mFindOptionDlg.GetCatogoryQueryCondition());
		if (IsWildCardExp(findText)) {
			pStringMatcher = &stringMatcher;
			stringMatcher.SetExpression(findText);
		}
		else {
			if (IsPhoneticSearch()) {
				pStringMatcher = &phoneticStringMatcherList;
			}
			else if (stringMatcherList.GetWordCount() < 2) {
				if (!condition.IsEmpty())
					condition += _T(" AND ");
				CString pathCond;
				pathCond.Format(_T("Path LIKE '%%%s%%'"), findText);
				condition += pathCond;
			}
			else
				pStringMatcher = &stringMatcherList;
		}
		if (!condition.IsEmpty())
			condition = _T(" WHERE ") + condition;
		ItrTableRowsCallbackData_CFindDlg itSHTable(this,
			&CFindDlg::ItrSearchCachedDataTableRowsCallbackFn,
			pStringMatcher);

		itSHTable.IterateTableRows(fdbCacheDB, "CachedData",
			SystemUtils::UnicodeToUTF8(condition).c_str());
		fdbCacheDB.Close();
		SetStatusMessage(_T("%d files found."), mListResult->GetItemCount());
	}
	else {
		SetStatusMessage(_T("Nothing checked. Please check the desired location to search."));
	}
}
void CFindDlg::OnBnClickedOk() // Start/Stop Find
{
	if (!IsSearchStarted()) {
		StartThreadOperation(THREAD_OP_START_SEARCH);
	}
	else {
		CString buttonText;
		buttonText.LoadString(IDS_STOPPING);
		SetDlgItemText(IDOK, buttonText);
		SetSearchCancelled(true);
	}
}

int CFindDlg::SearchInNetwork(HTREEITEM hItem)
{
	LPNETRESOURCE lpnRes = (LPNETRESOURCE)mTreeCtrlDomain->GetItemData(hItem);
	if (lpnRes < (LPNETRESOURCE)2) // Resolved Folder
		return FIND_CONTINUE_SEARCH;
	CTreeCtrlIterator ti(mTreeCtrlDomain, TreeNetWorkIteratorCallBack, this);
	ti.StartIterationEx(hItem);

	return ti.IsAborted() ? FIND_ABORT_SEARCH : FIND_NOTNETWORK_NODE;
}

int CFindDlg::SearchInFolder(CString pathToSearch)
{
	CString statusText(_T("Searching in "));
	CFinder cf(mFindText, FindCallBackFolders, true, this,
		IsPhoneticSearch() ? CFinder::Phonetic : CFinder::WildCard);
	SetStatusMessage(_T("%s"), statusText + pathToSearch);
	bool bSearchZip(mFindOptionDlg.IsSearchZipEnabled());
	cf.Find(pathToSearch, bSearchZip);
	SetStatusMessage(_T(""));
	return cf.IsAborted() ? FIND_ABORT_SEARCH : FIND_CONTINUE_SEARCH;
}
void CFindDlg::ProcessMessageDuringDragDrop()
{
	MSG msg;
	while (m_uDrapDropOpCount > 0) {
		if (PeekMessage(&msg, m_hWnd, 0, 0, PM_REMOVE)) {
			if (!IsDialogMessage(&msg)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else Sleep(10);
	}
}
int CFindDlg::StartThreadOperation(ThreadOperation op, LPVOID threadData /* = NULL */, int iThreadClass /* = 0 */)
{
	ThreadData *td = new ThreadData(this, op, threadData);
	if (IsDisableThreadedoperation()) {
		FindDlgThreadProc(td);
	}
	else {
		ThreadManager::GetInstance().CreateThread(TMFindDlgThreadProcFn, td, iThreadClass);
	}
	return 0;
}
int CFindDlg::DoThreadOperation(ThreadOperation threadOp, LPVOID pThreadData)
{
	switch (threadOp) {
	case THREAD_OP_START_SEARCH:
		Find();
		break;
	case THREAD_OP_FILL_COL_OPTIONAL:
		FillColOptional((ListColumns)(long long)pThreadData);
		break;
	case THREAD_OP_SAVE_SEARCH_RESULT:
		SaveSearchResult(pThreadData != NULL);
		break;
	case THREAD_OP_SEARCH_IN_NETWORK:
		StartSearchInNetworkFoder((LPNETRESOURCE)pThreadData);
		break;
	case THREAD_OP_DODRAG_DROP:
		ProcessMessageDuringDragDrop();
		break;
	case THREAD_OP_FILTER_DUPLICATES:
		mListResult->FilterDuplicates(this);
		break;
	case THREAD_OP_LOAD_LAST_RESULT:
		mCommitResultTimer.Load();
		break;
	case THREAD_OP_FIND_FILE_CONTENT:
		SearchFileContent((CListResItemData *)pThreadData);
		break;
    case THREAD_OP_EXPAND_TREE_NODE:
    {
        CString *pPath((CString*)pThreadData);
        SearchForNetWorkPC(*pPath);
        delete pPath;
    }
    break;
	}
	return 0;
}
void CFindDlg::DoPostThreadOperation()
{
	if (ThreadManager::GetInstance().GetThreadCount(-1) <= GetIdleThreadCount() ) {
		if (IsSearchStarted()) {
			CString buttonText;
			buttonText.LoadString(IDS_FIND);
			if (IsDiplayFindOption())
				buttonText += _T("...");
			SetDlgItemText(IDOK, buttonText);
			SetStatusMessage(_T("%d files found."), mListResult->GetItemCount());
			SetSearchStarted(false);
		}
		if (IsClosed())
			PostMessage(WM_CLOSE);
		SetSearchCancelled(false);
	}
}

void CFindDlg::OnDestroy()
{
	delete mTreeCtrlDomain;
	delete mListResult;
	delete mResizeBar;
}
void CFindDlg::FillColOptional(ListColumns optionalColumn)
{
	int count = mListResult->GetItemCount();
	if (optionalColumn == ListColumns_FileIcon) {
		mListResult->UpdateImageList();
	}
	for (int i =0; i < count && !IsClosed(); i++) {
		mListResult->UpdateOptionalColumn(i, optionalColumn);
	}
}
void CFindDlg::OnNMDblclkListResult(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	*pResult = 0;
	LPNMITEMACTIVATE lp = (LPNMITEMACTIVATE)pNMHDR;
	POINT pt = lp->ptAction;
	if (lp->iItem == -1) {
		LVHITTESTINFO ht = {pt};
		mListResult->SubItemHitTest(&ht);
		lp->iItem = ht.iItem;
		if (ht.iItem == -1)
			return;
	}
	CString path = mListResult->GetItemText(lp->iItem, 1);
	ShellExecute(NULL, _T("open"), path, NULL, NULL, SW_SHOWDEFAULT);
}

void CFindDlg::OnCbnSelchangeComboFind()
{
	CComboBox *pComboBox = (CComboBox *)GetDlgItem(IDC_COMBO_FIND);
	int curSel = pComboBox->GetCurSel();
	CString str;
	pComboBox->GetLBText(curSel, str);
	if (str == _T("Clear List...") && curSel == pComboBox->GetCount()-1) {
		pComboBox->ResetContent();
	}
}

void GetFileNameUserAppDir(LPTSTR fileName, int size, int fileType)
{
	std::wstring appPath;
	SystemUtils::GetSpecialFolderPath(CSIDL_APPDATA, true, appPath);
	appPath.append(_T("\\Find"));
	::SHCreateDirectoryEx(NULL, appPath.c_str(), NULL);
	LPCTSTR fileTypes[] = {_T("search.lst"), _T("result.lst")};
    StringCchCopy(fileName, size, appPath.c_str());
	PathAppend(fileName, fileTypes[fileType]);
}

void CFindDlg::SaveSearchKeyWords()
{
	CComboBox *pComboBox = (CComboBox *)GetDlgItem(IDC_COMBO_FIND);
	int count = pComboBox->GetCount() - 1;
	FindDataBase prefDataBase;
	if (prefDataBase.Open() == 0) {
		CString str;
		prefDataBase.QueryNonRows("DELETE FROM SearchHistory");
		for (int  i = 0; i < count; i++) {
			pComboBox->GetLBText(i, str);
			FindDataBase::MakeSQLString(str);
			std::string utf8Str(SystemUtils::UnicodeToUTF8(str));
			prefDataBase.QueryNonRows("insert into SearchHistory values ('%s')",
				utf8Str.c_str());
		}
		if (!mPreferenceName.IsEmpty()) {
			std::string utf8Str(SystemUtils::UnicodeToUTF8(mPreferenceName));
			prefDataBase.QueryNonRows("insert into Property values ('Preferences','%s')",
				utf8Str.c_str());
		}
		else {
			prefDataBase.QueryNonRows("DELETE FROM Property WHERE Name='Preferences'");
		}
		mFindOptionDlg.SaveDefault(prefDataBase);
		mListResult->SaveDefault(prefDataBase);
        CString isTreeDomainVisible(mTreeCtrlDomain->GetStyle() & WS_VISIBLE ? L"1" : L"0");
        prefDataBase.SetProperty(L"isTreeDomainVisible", isTreeDomainVisible);
		prefDataBase.Commit();
	}
}

int CFindDlg::ItrSearchhistoryTableRowsCallbackFn(sqlite3_stmt *statement, void* /*pUserData*/)
{
	CComboBox *pComboBox = (CComboBox *)GetDlgItem(IDC_COMBO_FIND);
	CString line(
		SystemUtils::UTF8ToUnicode((const char *)sqlite3_column_text(statement, SearchHistory_SearchKeys)).c_str());
		pComboBox->InsertString(-1, line);
	return 0;
}

void CFindDlg::LoadSearchKeyWords()
{
	CComboBox *pComboBox = (CComboBox *)GetDlgItem(IDC_COMBO_FIND);
	ItrTableRowsCallbackData_CFindDlg itSHTable(this,
		&CFindDlg::ItrSearchhistoryTableRowsCallbackFn);
	FindDataBase prefDataBase(FDB_PrefDatabase, true);
	if (prefDataBase.Open() == 0) {
		itSHTable.IterateTableRows(prefDataBase, "SearchHistory");
		if (pComboBox->GetCount() > 0)
			pComboBox->InsertString(-1, _T("Clear List..."));
		CArrayCString prefName;
		prefDataBase.GetTableColTexts("Property", " WHERE Name='Preferences'", prefName);
		if (prefName.GetCount() >= 2)
			mPreferenceName = prefName.GetAt(1);
		mFindOptionDlg.LoadDefault(prefDataBase);
		mTreeCtrlDomain->SetSearchInZip(mFindOptionDlg.IsSearchZipEnabled());
		mListResult->LoadDefault(prefDataBase);
	}
    bool isTreeVisible = prefDataBase.GetProperty(L"isTreeDomainVisible") == L"1";
	prefDataBase.Close();
	PostMessage(WM_ON_FIRST_SHOW, isTreeVisible);
}
bool FileContainsLine(FILE *fp, LPCTSTR path)
{
	bool bContains = false;
	_fseeki64(fp, 0, SEEK_SET);
	TCHAR fileName[MAX_PATH];
	while (_fgetts(fileName, MAX_PATH, fp)) {
		int len = lstrlen(fileName);
		while (len-- > 0)
			if (fileName[len] == '\n' || fileName[len] == '\r')
				fileName[len] = 0;
			else break;
		if (lstrcmp(fileName, path) == 0) {
			bContains = true;
			break;
		}
	}
	_fseeki64(fp, 0, SEEK_END);
	return bContains;
}
void ListSavePercentUpdateCallback(double curPercentage, void *pUserData)
{
	((CFindDlg*)pUserData)->SetStatusMessage(_T("Saving search list %.2f%% done"), curPercentage);
}

void CFindDlg::SaveSearchResult(bool bFull)
{
	int count = mListResult->GetItemCount();
	if (count <= 0)
		return;
	CFileDialogFind cfd(FALSE, _T("txt"), NULL, 4|2, NULL, this);
	if (cfd.DoModal() != IDOK)
		return;
	CString fileName = cfd.GetPathName();
	LPCTSTR mode = _T("w+, ccs=UTF-8");
	if (cfd.IsAppendChecked()) {
		mode = _T("a+, ccs=UTF-8");
	}
	FILE *fp = NULL;
	_tfopen_s(&fp, fileName, mode);
	if (fp == NULL)
		return;
	CPercentage percentage(count, ListSavePercentUpdateCallback, this);
	if (bFull) {
		for (int i = 0; i < count; i++) {
			fileName = mListResult->GetItemText(i, 1);
			_ftprintf(fp, _T("%s\n"), (LPCTSTR)fileName);
			percentage.Update(i);
		}
	}
	else {
		for (int i = 0; i < count; i++) {
			fileName = ParentDirectoryFromPath(mListResult->GetItemText(i, 1));
			if (!FileContainsLine(fp, fileName))
				_ftprintf(fp, _T("%s\n"), (LPCTSTR)fileName);
			percentage.Update(i);
		}
	}
	fclose(fp);
	SetStatusMessage(_T(""));
	mCommitResultTimer.DoCommit(CRTCF_REMOVE|CRTCF_FORCE);
}

void CFindDlg::OnHdnItemclickListResult(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;
	if (phdr->iButton == 0) { // Sort

		mListResult->SortItemsEx(mListResult->GetColumnDataIndex(phdr->iItem));
	}
	else { // Right button
	}
}

bool CFindDlg::StartLimitedThreadOperation(ThreadOperation op, LPVOID threadData /* = NULL */, int iThreadClass /* = 0 */, int iMaxThreadCount /* = FIND_MAX_THREAD_COUNT */)
{
	// Wait till one on the thread is freed.
	while (ThreadManager::GetInstance().GetThreadCount(iThreadClass) > iMaxThreadCount && !IsSearchCancelled())
		Sleep(1000);
	if (!IsSearchCancelled()) {
		StartThreadOperation(op, threadData, iThreadClass);
		return true;
	}
	return false;
}

bool CFindDlg::StartThreadToSearchInNetwork(LPNETRESOURCE lpnRes)
{
	return StartLimitedThreadOperation(THREAD_OP_SEARCH_IN_NETWORK, lpnRes);
}

void CFindDlg::StartSearchInNetworkFoder(LPNETRESOURCE lpnRes)
{
	CString statusText(_T("Searching in "));
	SetStatusMessage(_T("%s"), statusText + lpnRes->lpRemoteName);
	CNetWorkFinder cnf(NetWorkFindShared, false, this);
	cnf.StartFind(lpnRes);
	if (lpnRes->lpComment == kNETRESOURCEComment)
		FreeLPNetResource(lpnRes);
}
void CFindDlg::OnLvnBegindragListResult(NMHDR *pNMHDR, LRESULT *pResult)
{
    UNREFERENCED_PARAMETER(pNMHDR);
	m_uDrapDropOpCount++;
//	StartThreadOperation(THREAD_OP_DODRAG_DROP);
	mListResult->DoDragDrop();
	m_uDrapDropOpCount--;
	//StartThreadOperation(THREAD_OP_DODRAG_DROP);
	*pResult = 0;
}

void CFindDlg::OnLvnItemchangedListResult(NMHDR *pNMHDR, LRESULT *pResult)
{
    UNREFERENCED_PARAMETER(pNMHDR);
	if (!mListResult->IsNotificationDisabled()) {
		int selCount = mListResult->GetSelectedCount();
		if (selCount <= 1) {
			SetStatusMessage(_T("%d files found."), mListResult->GetItemCount());
			ShowPreview();
		}
		else {
			CString totalSize(SystemUtils::GetReadableSize(mListResult->GetFileSize(-2)));
			SetStatusMessage(_T("%d files selected. Size: %s"), selCount, totalSize);
		}
	}
	*pResult = 0;
}

void CFindDlg::OnLvnDeleteitemListResult(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	mListResult->OnLVNDeleteItem(pNMLV->iItem);
	*pResult = 0;
}

void CFindDlg::OnLvnDeleteallitemsListResult(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
    UNREFERENCED_PARAMETER(pNMLV);
	mListResult->OnLVNDeleteAllItem();
	*pResult = 0;
}

void CFindDlg::SetStatusMessage(LPCTSTR fmt, ...)
{
	if (fmt == NULL)
		fmt = _T("");
	va_list arg;
	va_start(arg, fmt);
	TCHAR buf[4096];
	_vstprintf_s(buf, sizeof(buf)/sizeof(TCHAR), fmt, arg);
	{
		CAutoLock al(mStatusLock);
		mStatusMsg = buf;
	}
	if (mStatusUpdateTimer.UpdateTimeDuration(IsForceUpdateStatus()))
		SetDlgItemText(IDC_STATIC_STATUS, buf);
	else if (!IsStatusTimerStarted()) {
		SetStatusTimerStarted(true);
		SetTimer(TIMER_UPDATESTATUS, (UINT)mStatusUpdateTimer.GetTimeUpdateDuration(), NULL);
	}
	SetForceUpdateStatus(false);
}

void CFindDlg::SetSearchStartedImpl()
{
	SetSearchStarted(true);
	CString buttonText;
	buttonText.LoadString(IDS_STOP);
	SetDlgItemText(IDOK, buttonText);
}

int CFindDlg::UpdateResultItem( CListResItemData *pListItemData )
{
	CAutoLock al(mListUpdateLock);
	int item(mListResult->GetItemMatchingWeight(pListItemData->mMatchWeight));
	if (item < 0)
		item = 0x7fffffff;
	item = mListResult->InsertItem(item, Path(pListItemData->mPath).FileName(), !pListItemData->HasSize());
	mListResult->SetItemData(item, (DWORD_PTR)pListItemData);
	mListResult->SetItemText(item, 1, pListItemData->mPath);
	pListItemData->mPath.Empty();
	mListResult->UpdateOptionalColumn(item, ListColumns_Size);
	mListResult->UpdateOptionalDateColumns(item);
	return item;
}

int CFindDlg::FindFolderCallback(CFileFindEx *pFindFile, bool bFileMatched)
{
	CFileFindExMetaDataProvider fileMetaDataProvider(*pFindFile);
	bFileMatched = bFileMatched && mFindOptionDlg.CheckFileSatisfies(fileMetaDataProvider);
	if (bFileMatched) { // Files & Folders
		CListResItemData *pListItemData(new CListResItemData(fileMetaDataProvider, pFindFile->GetMatchWeight()));
		pListItemData->mPath = pFindFile->GetFilePath();
		if (mFileContentSearchText.IsEmpty())
			UpdateResultItem(pListItemData);
		if (!mFileContentSearchText.IsEmpty()) {
			if (!pFindFile->IsDirectory())
				StartLimitedThreadOperation(THREAD_OP_FIND_FILE_CONTENT, pListItemData, THREAD_OP_FIND_FILE_CONTENT, FIND_MAX_THREAD_COUNT >> 1);
			else
				delete pListItemData;
		}
	}
	if (pFindFile->GetDirectory()) {
		SetStatusMessage(_T("Searching in %s"), pFindFile->GetDirectory());
	}
	return IsSearchCancelled() ? FCB_ABORT : FCB_CONTINUE;
}
void CFindDlg::OnFilePreferences()
{
	CPreferences prefDialog(mPreferenceName);
	prefDialog.DoModal();
	mPreferenceName = prefDialog.mLoadedPreferenceName;
}

int CFindDlg::SavePrefToFile(FindDataBase &findDb, const CString &preferenceName)
{
	GetDlgItemText(IDC_COMBO_FIND, mFindText);
	CString findText(mFindText);
	FindDataBase::MakeSQLString(findText);
	std::string prefName(SystemUtils::UnicodeToUTF8(preferenceName));
	findDb.QueryNonRows("insert or replace into Preferences values ('%s', '%s', '', '', '', '', '', '', '');",
		prefName.c_str(),
		SystemUtils::UnicodeToUTF8(findText).c_str());
	if (!IsSearchStarted())
		GetFindList();
	for (HTREEITEMVec::iterator it = mSearchList.begin();
		it != mSearchList.end(); ++it) {
		findText = mTreeCtrlDomain->GetFilePath(*it, false);
		FindDataBase::MakeSQLString(findText);
		findDb.QueryNonRows("insert or replace into PrefSearchLocations values ('%s', '%s');",
			prefName.c_str(),
			SystemUtils::UnicodeToUTF8(findText).c_str());
	}
	return mFindOptionDlg.SavePrefToFile(findDb, preferenceName);
}
int CFindDlg::ItrPreferencesTableRowsCallbackFn(sqlite3_stmt *statement, void *pUserData)
{
    UNREFERENCED_PARAMETER(pUserData);
	mFindText =
		SystemUtils::UTF8ToUnicode((const char *)sqlite3_column_text(statement, Preferences_PreferenceKey)).c_str();
	SetDlgItemText(IDC_COMBO_FIND, mFindText);
	return 0;
}
int CFindDlg::ItrPrefSearchLocTableRowsCallbackFn(sqlite3_stmt *statement, void *pUserData)
{
    UNREFERENCED_PARAMETER(pUserData);
	CString line(SystemUtils::UTF8ToUnicode((const char *)sqlite3_column_text(statement, PrefSearchLocations_SearchLocation)).c_str());
	HTREEITEM hItem(mTreeCtrlDomain->FindPath(line));
	if (hItem == NULL) {
		// may be it is "Enite Network\<server name>"
		CString netowrkName(mTreeCtrlDomain->GetItemText(mTreeCtrlDomain->GetRootItem()));
		if (line.Find(netowrkName) == 0) {
			line.Delete(0, netowrkName.GetLength());
			line.Insert(0, '\\');
			hItem = SearchForNetWorkPC(line);
		}
	}
	if (hItem != NULL)
		mTreeCtrlDomain->SetCheck(hItem);
	return 0;
}
int CFindDlg::LoadPrefFromFile(FindDataBase &findDb, const CString &preferenceName)
{
	CAutoDisableThreadOp autoDisableThreadOp(this);
	ItrTableRowsCallbackData_CFindDlg itSHTable(this,
		&CFindDlg::ItrPreferencesTableRowsCallbackFn);
	std::string prefName(SystemUtils::UnicodeToUTF8(preferenceName));
	char condition[256];
	sprintf_s(condition, 256, " where PreferenceName = '%s'", prefName.c_str());
	itSHTable.IterateTableRows(findDb, "Preferences", condition);
	itSHTable.SetCallbackFn(&CFindDlg::ItrPrefSearchLocTableRowsCallbackFn);
	itSHTable.IterateTableRows(findDb, "PrefSearchLocations", condition);
	return mFindOptionDlg.LoadPrefFromFile(findDb, preferenceName);
}

bool CFindDlg::DisableThreadOp(bool bDisable)
{
	bool bOldVal(IsDisableThreadedoperation());

	SetDisableThreadedoperation(bDisable);

	return bOldVal;
}
void CFindDlg::TogglePreview()
{
	if (m_pPreviewController == NULL)
		m_pPreviewController = new CPreviewController;

	if (m_pPreviewController->IsPreviewVisible()) {
		m_pPreviewController->HidePreview();
	}
	else { // window is not visible - show it if any item is selected
		POSITION pos = mListResult->GetFirstSelectedItemPosition();
		if (pos != NULL) {
			CDialogPreview *m_pDialogPreview(m_pPreviewController->GetPreviewDialog(this));
			RECT cr;
			mListResult->GetWindowRect(&cr);
			RECT pcr;
			m_pDialogPreview->GetWindowRect(&pcr);
			pcr.top = cr.bottom - (pcr.bottom - pcr.top);
			pcr.left = cr.right - (pcr.right - pcr.left);
			m_pDialogPreview->SetWindowPos(NULL, pcr.left-SystemUtils::GetTranslatedDPIPixelX(20), pcr.top-SystemUtils::GetTranslatedDPIPixelX(10), 0, 0, SWP_NOZORDER | SWP_NOSIZE);
			m_pDialogPreview->ShowWindow(SW_SHOW);
			ShowPreview();
		}
	}
}
void CFindDlg::ShowPreview()
{
	if (m_pPreviewController != NULL) {
		POSITION pos = mListResult->GetFirstSelectedItemPosition();
		if (pos != NULL) {
			int nSelItem = mListResult->GetNextSelectedItem(pos);
			int nItem = mListResult->GetItemIndex(NULL, nSelItem, false, false);
			CString path = mListResult->GetItemText(nItem, 1);
			CRefCountObj *pExtraData(NULL);
			if (!mFileContentSearchText.IsEmpty() && nItem != nSelItem) {
				CPreviewExtraDataFileContentSearch *pExtraSelData(new CPreviewExtraDataFileContentSearch);
				pExtraSelData->nLineToScroll = SystemUtils::StringToLongLong(mListResult->GetItemText(nSelItem, 2));
				pExtraSelData->textToSel = mListResult->GetItemText(nSelItem, 1);
				pExtraData = (CRefCountObj *)pExtraSelData;
			}
			m_pPreviewController->SetExtraData(pExtraData);
			m_pPreviewController->ShowPreview(path);
			SetActiveWindow();
		}
	}
}
BOOL CFindDlg::ShowWindow(int nCmdShow)
{
	if (nCmdShow == SW_HIDE) {
		if (m_pPreviewController && m_pPreviewController->IsPreviewVisible())
			m_pPreviewController->HidePreview();
	}
	return CDialog::ShowWindow(nCmdShow);
}
BOOL CFindDlg::OnDeviceChange(UINT nEventType, DWORD_PTR dwData)
{
	mTreeCtrlDomain->OnDeviceChange(nEventType, dwData);
	return __super::OnDeviceChange(nEventType, dwData);
}

#include "FileContentFinder.h"

class CFileContentFinderCallbackFindDlg : public CFileContentFinderCallback {
public:
	CFileContentFinderCallbackFindDlg(CFindDlg *pDlg, CListResItemData *pItemData);
	int MatchCallback(const MatchData &inMatchData);
private:
	CFindDlg *m_pDlg;
	CListResItemData *m_pItemData;
};

int CFileContentFinderCallbackFindDlg::MatchCallback( const MatchData &inMatchData )
{
	m_pDlg->FileContentSearchCallback(m_pItemData, &inMatchData);
	return m_pDlg->IsSearchCancelled();
}

CFileContentFinderCallbackFindDlg::CFileContentFinderCallbackFindDlg( CFindDlg *pDlg, CListResItemData *pItemData )
	: m_pDlg(pDlg), m_pItemData(pItemData)
{

}

void CFindDlg::SearchFileContent( CListResItemData * pItemData)
{
	CFileContentFinderCallbackFindDlg fDlg(this, pItemData);
	CFileContentFinder cf(&fDlg);
	cf.Find(pItemData->mPath, mFileContentSearchText);
	if (mListResult->GetItemIndex(pItemData) < 0)
		delete (CListResItemData *)pItemData;
}

void CFindDlg::FileContentSearchCallback( CListResItemData *pItemData, LPCVOID pSrchData )
{
	const CFileContentFinderCallback::MatchData *p_inMatchData((CFileContentFinderCallback::MatchData *)pSrchData);
	if (p_inMatchData->bMatched) {
		CAutoLock al(mListUpdateLock);
		int index(mListResult->GetItemIndex(pItemData));
		if (index < 0)
			index = UpdateResultItem(pItemData);
		CString fileName(mListResult->GetItemText(index, 0));
		index += p_inMatchData->iMatchCount;
		mListResult->InsertItem(index, _T("   ")+fileName);
		mListResult->SetItemText(index, 1, p_inMatchData->strLine);
		mListResult->SetItemText(index, 2, SystemUtils::IntToString(p_inMatchData->iLineNo));
	}
}

int CFindDlg::GetIdleThreadCount()
{
	int idleThreadCount(1);
	if (m_pPreviewController && !m_pPreviewController->IsPreviewTerminated())
		++idleThreadCount;
	return idleThreadCount;
}
