// FindServerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Find.h"
#include "FindServerDlg.h"
#include "SystemUtils.h"
#include "Path.h"
#include <activeds.h>
#include "AutoLock.h"
#include "TreeCtrlIterator.h"
#include "LoggerFactory.h"
#include "IPEnumerator.h"
#include "SocketUitl.h"
#include "DBCommiter.h"
#include "Property.h"

FinderClass(CFindServerDlg)

CServerCombinedButtonContext::CServerCombinedButtonContext(CFindServerDlg *pDlg)
	: mDlg(pDlg)
{
}
BOOL CServerCombinedButtonContext::IsWindowVisible()
{
	return mDlg->GetDlgItem(IDC_LIST_CATAGORIES)->IsWindowVisible();
}
void CServerCombinedButtonContext::ShowWindow(int cmdShow /* = SW_SHOW */)
{
	mDlg->ShowServerMainControls(cmdShow);
}
// CFindServerDlg dialog
// CFindServerDlg::FindInSearchHistory return values
IMPLEMENT_DYNAMIC(CFindServerDlg, CDialog)

CFindServerDlg::CFindServerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFindServerDlg::IDD, pParent),
	mTreeCtrlDomain(NULL), mResizeBar(NULL),
	mControlResizer(this), mServerMainControls(this),
	m_hIcon(NULL), mCatagoryEmbedListCtrl(NULL),
	mCmdEditCtrl(NULL), m_uSearchFlags(0),
	mServerSearchStatus(SST_NewSearch),
	mDataBase(FDB_CacheDatabase), m_pStatusDlg(NULL),
	mSearchHistory(this), mCombinedBotton(this, IDC_BUTTON_LEFT, IDC_BUTTON_RIGHT), mFindServerDlgContext(this), m_iMaxThreadCount(FIND_MAX_THREAD_COUNT),
    m_pDBCommitter(NULL)
{
    SetDialogShowMinized();
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CFindServerDlg::~CFindServerDlg()
{
}

void CFindServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}
void CFindServerDlg::DisableControls(bool bDisable)
{
	DWORD controls[] = {
		IDC_BUTTON_LOADDEFAULT,
		IDC_BUTTON_EXECUTE
	};
	for (int i = 0; i < sizeof(controls)/sizeof(controls[0]); ++i)
		GetDlgItem(controls[i])->EnableWindow(!bDisable);
}

BEGIN_MESSAGE_MAP(CFindServerDlg, CDialog)
	ON_WM_SIZING()
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_NOTIFY(TVN_ITEMEXPANDING, IDC_TREE_DOMAIN, &CFindServerDlg::OnTvnItemexpandingTreeDomain)
	ON_BN_CLICKED(IDC_BUTTON_ADD, &CFindServerDlg::OnBnClickedButtonAdd)
	ON_BN_CLICKED(IDC_BUTTON_EXECUTE, &CFindServerDlg::OnBnClickedButtonExecute)
	ON_BN_CLICKED(IDC_BUTTON_LOADDEFAULT, &CFindServerDlg::OnBnClickedButtonLoaddefault)
	ON_BN_CLICKED(IDOK, &CFindServerDlg::OnBnClickedOk)
	ON_NOTIFY(TVN_ITEMCHECK_STATE_CHANGED, IDC_TREE_DOMAIN, &CFindServerDlg::OnTvnItemCheckStateChangedTreeDomain)
	ON_BN_CLICKED(IDC_BUTTON_DELETE, &CFindServerDlg::OnBnClickedButtonDelete)
	ON_WM_DEVICECHANGE()
	ON_BN_CLICKED(IDC_BUTTON_LEFT, &CFindServerDlg::OnBnClickedButtonLeft)
	ON_BN_CLICKED(IDC_BUTTON_RIGHT, &CFindServerDlg::OnBnClickedButtonRight)
	ON_MESSAGE(WM_SERVER_ADDTSPANEL, &CFindServerDlg::OnAddThreadStatusPanel)
END_MESSAGE_MAP()

static struct Condtion
{
    enum CondtionType {
        NoCondition,
        LessThan,
        LessThanOrEqual,
        Equal,
        GreaterThan,
        GreaterThanOrEqual,
        Between
    } conditionType;
    LPCTSTR name;
    static CondtionType ConditionFromString(const CString &cond);
} scondition[] = {
    Condtion::NoCondition, _T(""),
    Condtion::LessThan,_T("<"),
    Condtion::LessThanOrEqual,_T("<="),
    Condtion::Equal,_T("="),
    Condtion::GreaterThan,_T(">"),
    Condtion::GreaterThanOrEqual,_T(">="),
    Condtion::Between,_T("in")
};

static int ListEmbedControlCallBackFn(CEmbedListCtrl *pList, int message, WPARAM /*wParam*/, LPARAM lParam, LPVOID pUserData)
{
    UNREFERENCED_PARAMETER(pList);
    UNREFERENCED_PARAMETER(pUserData);
	int retVal(0);
	switch (message) {
	case EMT_AddControl:
		if (lParam < 3) // Col 0-2
			retVal = ECT_Edit;
		else
			retVal = ECT_NO_Control;
		break;
	}
	return retVal;
}
// CFindServerDlg message handlers
const CString kLoggerName(_T("Server"));
static Logger& GetLogger()
{
	return LoggerFacory::GetInstance().GetLogger(kLoggerName);
}
Logger& GetServerLogger()
{
	return GetLogger();
}
static void LogMessageHandler_Server(const wchar_t *msg)
{
	GetLogger().Log(msg);
}
BOOL CFindServerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	SystemUtils::SetLogMessageHandler(LogMessageHandler_Server);
	SystemUtils::LogMessage(_T("Init"));
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	mResizeBar = new CResizeBar(this);
	mResizeBar->SubclassDlgItem(IDC_STATIC_VERT_SEP, this);
	mResizeBar->OnSetCursor(NULL, 0, 0);
	mResizeBar->SetMinLeftRight(SystemUtils::GetTranslatedDPIPixelX(200), SystemUtils::GetTranslatedDPIPixelX(300));
	mControlResizer.AddControl(IDC_LIST_CATAGORIES, RSZF_RIGHT_FIXED | RSZF_SIZEY_FIXED);
	mControlResizer.AddControl(IDC_EDIT_COMMAND);
	mControlResizer.AddControl(IDC_TREE_DOMAIN, RSZF_BOTTOM_FIXED);
	mControlResizer.AddControl(IDC_BUTTON_EXECUTE, RSZF_RIGHT_FIXED|RSZF_SIZE_FIXED|RSZF_TOP_FIXED);
	mControlResizer.AddControl(IDC_BUTTON_LOADDEFAULT, RSZF_LEFT_FIXED|RSZF_SIZE_FIXED|RSZF_BOTTOM_FIXED);
	mControlResizer.AddControl(IDC_STATIC_VERT_SEP, RSZF_BOTTOM_FIXED|RSZF_SIZEX_FIXED);
	mServerMainControls = mControlResizer;
	mControlResizer.AddControl(IDOK, RSZF_RIGHT_FIXED|RSZF_SIZE_FIXED|RSZF_BOTTOM_FIXED);
	mControlResizer.AddControl(IDC_BUTTON_LEFT, RSZF_SIZE_FIXED|RSZF_RIGHT_FIXED);
	mControlResizer.AddControl(IDC_BUTTON_RIGHT, RSZF_SIZE_FIXED|RSZF_RIGHT_FIXED);
	CControlResizer &dragControlResizer(mResizeBar->GetControlResizer());
	dragControlResizer.AddControl(IDC_LIST_CATAGORIES, RSZF_RIGHT_FIXED | RSZF_BOTTOM_FIXED | RSZF_RESIZE_OPPOSITE);
	dragControlResizer.AddControl(IDC_TREE_DOMAIN, RSZF_BOTTOM_FIXED | RSZF_LEFT_FIXED);
	dragControlResizer.AddControl(IDC_STATIC_SEARCH_DOMAIN, RSZF_NO_RESIZE);
	dragControlResizer.AddControl(IDC_STATIC_VERT_SEP, RSZF_BOTTOM_FIXED|RSZF_SIZEX_FIXED);
	dragControlResizer.AddControl(IDC_STATIC_CATAGORY, RSZF_SIZEX_FIXED);
	dragControlResizer.AddControl(IDC_STATIC_STARTUPCOMMAND, RSZF_SIZEX_FIXED);
	dragControlResizer.AddControl(IDC_BUTTON_ADD, RSZF_SIZEX_FIXED);
	dragControlResizer.AddControl(IDC_BUTTON_DELETE, RSZF_SIZEX_FIXED|RSZF_RESIZE_UPDATE);
	dragControlResizer.AddControl(IDC_EDIT_COMMAND, RSZF_RIGHT_FIXED | RSZF_BOTTOM_FIXED | RSZF_RESIZE_OPPOSITE);
	dragControlResizer.AddControl(IDC_BUTTON_LOADDEFAULT, RSZF_SIZEX_FIXED);
	mServerMainControls.Append(dragControlResizer);
	SystemUtils::SetWindowPos(GetSafeHwnd(), NULL, 0, 0, 800, 600, SWP_DRAWFRAME|SWP_FRAMECHANGED|SWP_NOMOVE|SWP_NOZORDER);
	mTreeCtrlDomain = new CTreeCtrlDomain();
	mTreeCtrlDomain->SubclassDlgItem(IDC_TREE_DOMAIN, this);
	mTreeCtrlDomain->RefreshDomainTree();
	mTreeCtrlDomain->SetItemHeight((SHORT)SystemUtils::GetTranslatedDPIPixelX(18));
	CEmbedListCtrl *pEmbedListCtrl(new CEmbedListCtrl());
	mCatagoryEmbedListCtrl = pEmbedListCtrl;
	pEmbedListCtrl->SubclassDlgItem(IDC_LIST_CATAGORIES, this);
	mCatagoryEmbedListCtrl->SetExtendedStyle(mCatagoryEmbedListCtrl->GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP);
	LPCTSTR columns[] = {
		_T("Name"),
		_T("Search Keys"),
		_T("Options"),
	};
    int colWidths[] = { 100, 200, 180 };
	for (int i = 0; i < sizeof(columns)/sizeof(LPCTSTR); ++i) {
		pEmbedListCtrl->InsertColumn(i, columns[i]);
        pEmbedListCtrl->SetColumnWidth(i, SystemUtils::GetTranslatedDPIPixelX(colWidths[i]));
	}
	int colWidth(0);
	for (int i = 0; i < pEmbedListCtrl->GetColumnCount(); ++i) {
        if (i!=1)
		    colWidth += pEmbedListCtrl->GetColumnWidth(i);
	}
	RECT rc;
	pEmbedListCtrl->GetClientRect(&rc);
	colWidth = rc.right-rc.left-colWidth;
	if (colWidth > 0)
		pEmbedListCtrl->SetColumnWidth(1, colWidth);
	pEmbedListCtrl->CheckRowColCheckBox(0);
	pEmbedListCtrl->SetEmbedControlCallBackFn(ListEmbedControlCallBackFn, (LPVOID)this);
	mCmdEditCtrl = new CCmdEditCtrl();
	mCmdEditCtrl->SubclassDlgItem(IDC_EDIT_COMMAND, this);
	mCombinedBotton.Init();
	mCombinedBotton.AddPage(&mFindServerDlgContext);
    if (FindDataBase::SGetProperty(_T("Command"), FDB_CacheDatabase).IsEmpty())
        OnBnClickedButtonLoaddefault();
    else
        LoadFromDB();
	if (FindDataBase::SGetProperty(_T("StartedByUser"), FDB_CacheDatabase).IsEmpty()) {
        SetDialogShowMinized(false);
	}
	else {
        SetDialogShowMinized();
		StartFind();
	}
	return TRUE;
}
bool CFindServerDlg::IsSearchCancelled(bool bCheckThread)
{
	bool bIsCancelled(IsSearchCancel());
	if (bIsCancelled)
		return bIsCancelled;
	if (bCheckThread) {
		if (ThreadManager::GetInstance().IsThreadTerminated())
			return true;
	}
	return false;
}
struct Catagory {
	LPCTSTR name;
	LPCTSTR searchKeys;
	LPCTSTR options;
	bool bEnabled;
};

Condtion::CondtionType Condtion::ConditionFromString(const CString &cond)
{
	for (int i = 0; i < sizeof(scondition)/sizeof(LPCTSTR); ++i)
		if (cond == scondition[i].name) {
			return scondition[i].conditionType;
		}
	return NoCondition;
}
//#define DISTRIBUTE_BUILD
void CFindServerDlg::LoadDefault()
{
	Catagory catagories[] = {
#ifdef DISTRIBUTE_BUILD
		_T("Documents"), _T(".doc;.xsl;.pdf;.txt"), _T(""), true,
		_T("Code"), _T(".c;.h;.vcproj;.sln;.xcode"), _T(""), true,
		_T("Images"), _T(".jpg;.jpeg;.bmp;.gif;.tif;.png;.dib"), true,
#else
        _T("Movies"), _T(".mp4;.mov;.mpg;.mpeg;.div;.avi;.vlc;.vob;.wmv;.flv"), _T("Condition.Size=Size>=300M"), true,
		_T("Music"), _T(".mp3"), _T("Condition.Size=Size>=2M"), true,
#endif
		_T("Softwares"), _T(".msi;.dmg;.exe;.zip;.rar"), _T(""), false,
	};
	mCatagoryEmbedListCtrl->DeleteAllItems();
	for (int i = 0 ; i < sizeof(catagories) / sizeof(Catagory); ++i) {
		int iTem = mCatagoryEmbedListCtrl->InsertItem(i, catagories[i].name);
		mCatagoryEmbedListCtrl->SetItemText(iTem, 1, catagories[i].searchKeys);
		mCatagoryEmbedListCtrl->SetItemText(iTem, 2, catagories[i].options);
		mCatagoryEmbedListCtrl->CheckRowColCheckBox(0, iTem, catagories[i].bEnabled);
	}
	mCmdEditCtrl->Clear();
#ifdef DISTRIBUTE_BUILD
	mCmdEditCtrl->AddCommand(_T("check:[mydocuments]"), true);
	mCmdEditCtrl->AddCommand(_T("check:[mypictures]"), true);
	mCmdEditCtrl->AddCommand(_T("check:[programfiles]"), true);
#else
	mCmdEditCtrl->SetWindowText(_T("check:[domain]"));
	mCmdEditCtrl->AddCommand(_T("ip:"), true);
#endif
	//Execute();
}
TableItertatorClass(CFindServerDlg);

int CFindServerDlg::ItrCatagoryTableRowsCallbackFn(sqlite3_stmt *statement, void *pUserData)
{
    UNREFERENCED_PARAMETER(pUserData);
	CString line(
		SystemUtils::UTF8ToUnicodeCString((const char *)sqlite3_column_text(statement, Catagory_Name)));
	int iTem = mCatagoryEmbedListCtrl->InsertItem(0x7fffffff, line);
	int colCount(mCatagoryEmbedListCtrl->GetColumnCount());
	for (int iCol = 1; iCol < colCount; ++iCol) {
		line = SystemUtils::UTF8ToUnicodeCString((const char *)sqlite3_column_text(statement, iCol+1));
		mCatagoryEmbedListCtrl->SetItemText(iTem, iCol, line);
	}
	int flag = sqlite3_column_int(statement, Catagory_Flags);
	mCatagoryEmbedListCtrl->CheckRowColCheckBox(0, iTem, flag & 1);
	return 0;
}

void CFindServerDlg::LoadFromDB()
{
	ItrTableRowsCallbackData_CFindServerDlg itSHTable(this,
		&CFindServerDlg::ItrCatagoryTableRowsCallbackFn);
	FindDataBase fdb(FDB_CacheDatabase, true);
	mCatagoryEmbedListCtrl->DeleteAllItems();
	if (fdb.Open() == 0) {
		itSHTable.IterateTableRows(fdb, "Category");
		mCmdEditCtrl->SetWindowText(fdb.GetProperty(_T("Command")));
		mServerSearchStatus =
			(ServerSearchStatus)SystemUtils::StringToInt(fdb.GetProperty(_T("ServerSearchStatus")));
	}
}
void CFindServerDlg::CommitToDB()
{
	bool bDatabaseOpen(mDataBase.IsOpen());
	int ret(0);
	if (!bDatabaseOpen)
		ret = mDataBase.Open();
	if (ret == 0) {
		mDataBase.QueryNonRows2("DELETE FROM Category");
		int rowCount(mCatagoryEmbedListCtrl->GetItemCount());
		int colCount(mCatagoryEmbedListCtrl->GetColumnCount());
		for (int iRow = 0; iRow < rowCount; ++iRow) {
			CString text;
			text += SystemUtils::IntToString(iRow) + _T(",");
			for (int iCol = 0; iCol < colCount; ++iCol) {
				text += _T("'") + mCatagoryEmbedListCtrl->GetItemText(iRow, iCol) + _T("',");
			}
			text += SystemUtils::IntToString(mCatagoryEmbedListCtrl->IsChecked(0, iRow));
			mDataBase.QueryNonRows("INSERT OR REPLACE INTO Category VALUES (%s)", SystemUtils::UnicodeToUTF8(text).c_str());
		}
		CString cmd;
		mCmdEditCtrl->GetWindowText(cmd);
		mDataBase.SetProperty(_T("Command"), cmd);
		mDataBase.SetProperty(_T("ServerSearchStatus"), SystemUtils::IntToString(mServerSearchStatus));
		mDataBase.Commit();
	}
	if (!bDatabaseOpen)
		mDataBase.Close();
}
static CString GetDomainName()
{
	CString domainName;
	HRESULT hr;

	hr = CoInitialize(NULL);

	IADsADSystemInfo *pSys(NULL);
	hr = CoCreateInstance(CLSID_ADSystemInfo,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_IADsADSystemInfo,
		(void**)&pSys);

	if (pSys) {
		BSTR bstr;
		hr = pSys->get_DomainShortName(&bstr);
		if (SUCCEEDED(hr)) {
			domainName = bstr;
			SysFreeString(bstr);
		}
		pSys->Release();
	}

	CoUninitialize();
	return domainName;
}

static CString GetPropertyValue(const CString &propName)
{
	CString propVal(propName);
	propVal = propVal.MakeLower();
	static std::map<CString, int> sTokenMap;
	if (sTokenMap.empty()) {
		sTokenMap[_T("mydocuments")] = CSIDL_MYDOCUMENTS;
		sTokenMap[_T("mypictures")] = CSIDL_MYPICTURES;
		sTokenMap[_T("programfiles")] = CSIDL_PROGRAM_FILES;
		sTokenMap[_T("windows")] = CSIDL_WINDOWS;
	}
	std::map<CString, int>::const_iterator cit(sTokenMap.find(propVal));
	if (cit != sTokenMap.end()) {
		TCHAR path[MAX_PATH];
		SHGetFolderPath(NULL, cit->second, NULL, 0, path);
		propVal = path;
	}
	else {
		propVal.Empty();
	}
	return propVal;
}

static CString ExpandProperty(const CString &propName)
{
	CString propVal(propName);
	if (propName == _T("[domain]"))
		return _T("NetWork\\Microsoft Windows Network\\") + GetDomainName();
	else {
		int pos = propName.Find(_T("[domain="));
		if (pos == 0) {
			CString domainName(propName);
			domainName.Delete(0, 8);
			domainName.Remove(']');
			return _T("NetWork\\Microsoft Windows Network\\") + domainName;
		}
	}
	// check for other properties
	int pos = 0;
	while (1) {
		pos = propVal.Find('[', pos);
		if (pos < 0)
			break;
		int startPos=pos;
		pos = propVal.Find(']', pos);
		if (pos < 0)
			break;
		CString propNameLocal(propVal.Mid(startPos+1, pos-1-startPos));
		propNameLocal = GetPropertyValue(propNameLocal);
		if (!propNameLocal.IsEmpty()) {
			propVal.Delete(startPos, pos+1-startPos);
			propVal.Insert(startPos, propNameLocal);
		}
	}
	return propVal;
}

void CFindServerDlg::Execute(LPCTSTR commandsToExecute /* = NULL */)
{
	mTreeCtrlDomain->RefreshDomainTree();
	mArrMirrorInfo.RemoveAll();
	mHostIPsToEnumerate.RemoveAll();
//	mTreeCtrlDomain->FindDomain(GetDomainName());
	CSortedArrayCString arrCommandToExecute;
	if (commandsToExecute != NULL) {
		CArrayCString commands;
		SystemUtils::SplitString(commandsToExecute, commands);
		for (INT_PTR i = 0; i < commands.GetCount(); ++i)
			arrCommandToExecute.InsertUnique(commands.GetAt(i));
	}
	mCmdEditCtrl->GetLines();
    SetEnumIP(false);
	if (SST_QuickSearch != mServerSearchStatus) {
		const CArrayCString &commands(mCmdEditCtrl->GetCommands());
		for (int i = 0; i < commands.GetCount(); ++i) {
			const CString &command(commands[i]);
			if (command.Find(_T("#"))==0) // comment
				continue;
			CString commandName(command);
			{
				int pos(commandName.Find(':'));
				if (pos > 0)
					commandName = commandName.Left(pos);
			}
			if (!arrCommandToExecute.IsEmpty() && arrCommandToExecute.Find(commandName) < 0)
				continue;
			if (command.Find(_T("ip:"))==0) {
                SetEnumIP();
				CString text = command;
				text.Delete(0, text.Find(':')+1);
				mHostIPsToEnumerate.Add(text);
				continue;
			}
			if (command.Find(_T("mirror:")) == 0 || command.Find(_T("skip:")) == 0) { // Mirror servers info
				INT_PTR index(-1);
				{
					MirrorInfo mi;
					index = mArrMirrorInfo.Add(mi);
				}
				CString text = command;
				text.Delete(0, text.Find(':')+1);
				CArrayCString outStrs;
				SystemUtils::SplitString(text, outStrs, _T("="));
				if (outStrs.GetCount() > 0) {
					mArrMirrorInfo[index].mStringMatcher.SetExpression(outStrs[0]);
					if (outStrs.GetCount() > 1) {
						mArrMirrorInfo[index].mMirrorMachine = outStrs[1];
						PushIpHostname(CString(Path(outStrs[1]).MakeUNCPath()));
					}
				}
				else
					mArrMirrorInfo.RemoveAt(index, 1);
				continue;
			}
			int check = -1;
			if (command.Find(_T("uncheck:"))==0)
				check = 0;
			else if (command.Find(_T("check:")) == 0)
				check = 1;
			if (check > -1) {
				CString text = command;
				text.Delete(0, text.Find(':')+1);
				CString path(text);
				int pos(text.Find(':'));
				if (pos == 1) { // may be a local drive c:
					Path localDrive(Path(path).GetRoot());
					if (localDrive.Exists())
						pos = text.Find(':', 2);
				}
				if (pos < 0) {
					text.Empty();
				}
				else {
					text.Delete(0, pos+1);
					path.Delete(pos, path.GetLength());
				}
				path = ExpandProperty(path);
				HTREEITEM hItem(mTreeCtrlDomain->FindPath(path));
				if (hItem == NULL)
					hItem = mTreeCtrlDomain->FindAddNetworkPath(path);
				if (hItem == NULL) {
					// may be it is "Network\<server name>"
					CString netowrkName(mTreeCtrlDomain->GetItemText(mTreeCtrlDomain->GetRootItem()));
					if (path.Find(netowrkName) == 0) {
						path.Delete(0, netowrkName.GetLength());
						path.Insert(0, '\\');
						hItem = mTreeCtrlDomain->FindAddNetworkPath(path);
					}
				}
				if (hItem != NULL) {
					mTreeCtrlDomain->SetCheckChildMatchingPattern(
						hItem,
						check, text);
					mTreeCtrlDomain->SelectSetFirstVisible(hItem);
				}
			}
		}
	}
	CommitToDB();
}
void CFindServerDlg::OnSizing(UINT nSize, LPRECT lpRect)
{
	if (lpRect->right - lpRect->left < SystemUtils::GetTranslatedDPIPixelX(500)) {
		switch (nSize) {
		case WMSZ_BOTTOMRIGHT:
		case WMSZ_RIGHT:
		case WMSZ_TOPRIGHT:
			lpRect->right = lpRect->left + SystemUtils::GetTranslatedDPIPixelX(500);
			break;
		case WMSZ_BOTTOMLEFT:
		default:
			lpRect->left = lpRect->right - SystemUtils::GetTranslatedDPIPixelX(500);
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
void CFindServerDlg::OnSize(UINT nType, int cx, int cy)
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
			GetWindowText(nicData.szTip, sizeof(nicData.szTip)/sizeof(nicData.szTip[0]));
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
#define TIMER_NEXT_SEARCH 1
#define TIMER_QUIT_APP 2
LRESULT CFindServerDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_USER+3000:
		switch (lParam) {
		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
			{
				ShowWindow(SW_RESTORE);
				SetForegroundWindow();
				BringWindowToTop();
				NOTIFYICONDATA nicData = {sizeof (NOTIFYICONDATA)};
				nicData.hWnd = m_hWnd;
				nicData.uID = 1;
				Shell_NotifyIcon(NIM_DELETE, &nicData);
			}
			break;
		}
		break;
	case WM_TIMER:
		switch (wParam) {
		case TIMER_NEXT_SEARCH:
			if (!IsSearching()) {
				KillTimer(wParam);
				StartFind();
			}
			break;
        case TIMER_QUIT_APP:
            KillTimer(wParam);
            PostMessage(WM_CLOSE);
            break;
		}
		break;
	case WM_IS_SERVER:
		return 1;
		break;
	}
	return CDialog::WindowProc(message, wParam, lParam);
}
void CFindServerDlg::OnDestroy()
{
	delete mTreeCtrlDomain;
	delete mResizeBar;
	delete mCatagoryEmbedListCtrl;
	delete mCmdEditCtrl;
}

void CFindServerDlg::OnTvnItemexpandingTreeDomain(NMHDR *pNMHDR, LRESULT *pResult)
{
	mTreeCtrlDomain->OnTvnItemexpanding(pNMHDR, pResult);
}

void CFindServerDlg::OnBnClickedButtonAdd()
{
	int row = mCatagoryEmbedListCtrl->InsertItem(0x7fffffff, _T("New Catagory"));
	mCatagoryEmbedListCtrl->EnableControl(row, 0);
}
void CFindServerDlg::OnBnClickedButtonDelete()
{
	while (1) {
		POSITION selectedItem = mCatagoryEmbedListCtrl->GetFirstSelectedItemPosition();
		if (selectedItem) {
			int nItem = mCatagoryEmbedListCtrl->GetNextSelectedItem(selectedItem);
			mCatagoryEmbedListCtrl->DeleteItem(nItem);
		}
		else break;
	}
}

void CFindServerDlg::OnBnClickedButtonExecute()
{
    StartThreadOperation(SERVER_THREAD_OP_EXECUTE);
}
void CFindServerDlg::SetSearchStarted(bool bSearchStarted)
{
    SetSearching(bSearchStarted);
	CString buttonText;
	buttonText.LoadString(bSearchStarted ? IDS_STOP : IDS_START);
	SetDlgItemText(IDOK, buttonText);
	DisableControls(bSearchStarted);
	FindDataBase *pDataBase(&mDataBase);
	if (bSearchStarted == false) {
        SetSearchCancel(false);
        SetSearchForce(false);
		pDataBase = NULL;
	}
	mSearchHistory.Load(pDataBase);
	mIPEnumHostNames.RemoveAll();
}
CFindServerDlg::FindCatagory::~FindCatagory()
{
}

void CFindServerDlg::FindCatagory::Init(int num, CEmbedListCtrl *pList)
{
	catagoryNum = num;
	CString text(pList->GetItemText(num, 1));
	mStringMatcher.SetExpression(text);
	text = pList->GetItemText(num, 2);
    text.MakeLower();
    CSortedArrayCString arrOptions;
    Property prop;
    {
        PropertySet ps;
        PropertySetStreamer pss;
        pss.SetPropertySetStream(ps);
        pss.ReadFromString((LPCTSTR)text);
        prop = ps.GetProperty(_T(""));
    }
    text = prop.GetValue(_T("condition")).c_str();
    mSizeMin = mSizeMax = 0;
    sizeCond = 0;
    if (!text.IsEmpty()) {
        text.Replace(_T(" "), _T(""));
        text.Replace(_T("and"), _T("&"));
        text.Replace(_T("&&"), _T("&"));
        text.Trim();
        CArrayCString arrCond;
        SystemUtils::SplitString(text, arrCond, _T("&"));
        if (arrCond.GetCount() > 0) {
            sizeCond = Condtion::ConditionFromString(SystemUtils::StringFindOneOf(arrCond[0], _T("<>=")));
            if (arrCond.GetCount() > 1) {
                sizeCond = Condtion::Between; // in
                mSizeMax = SystemUtils::GetSizeFromString(SystemUtils::StringFindOneOf(arrCond[1], _T("0123456789")));
            }
            else
                mSizeMax = -1;
            mSizeMin = SystemUtils::GetSizeFromString(SystemUtils::StringFindOneOf(arrCond[0], _T("0123456789")));
            
            if (sizeCond == Condtion::Between // in
                && mSizeMin > mSizeMax)
                std::swap(mSizeMin, mSizeMax);
        }
	}
    text = prop.GetValue(_T("content")).c_str();
    if (!text.IsEmpty()) {
        SetFindContent(text==_T("true") || text==_T("1"));
    }
}
bool CFindServerDlg::FindCatagory::Match(const CFileFindEx *pFileFile)
{
	CString fileName(pFileFile->GetFileName());
	bool bMatched = mStringMatcher.Match(fileName);
	if (bMatched && sizeCond) {
        Condtion::CondtionType sizeCondType((Condtion::CondtionType)sizeCond);
		LONGLONG fileSize(pFileFile->GetFileSize());
		switch (sizeCondType) {
		case Condtion::LessThan: // <
			bMatched = fileSize < mSizeMin;
			break;
		case Condtion::LessThanOrEqual: // <=
			bMatched = fileSize <= mSizeMin;
			break;
		case Condtion::Equal: // =
			bMatched = fileSize == mSizeMin;
			break;
		case Condtion::GreaterThan: // >
			bMatched = fileSize > mSizeMin;
			break;
		case Condtion::GreaterThanOrEqual: // >=
			bMatched = fileSize >= mSizeMin;
			break;
		case Condtion::Between: // in (range) >= && <=
			bMatched = fileSize >= mSizeMin && fileSize <= mSizeMax;
			break;
		}
	}
	return bMatched;
}
bool CFindServerDlg::RemoveObsoleteSearchHistory(const CSearchHistory &searchHistory)
{
	bool bRemove(false);
	//CTimeSpan timeDiff = CTime::GetCurrentTime() - searchHistory.GetLastupdated();
	if (!searchHistory.IsAvailable()) {
		CArrayCString queryArray;
		CString searchHistoryKey(searchHistory.GetRootKey());
		UINT missCount(searchHistory.GetMissCount()+1);
		if (missCount > 3) {
			GetQueryRemoveFromSearchHistory(searchHistoryKey, queryArray);
			SystemUtils::LogMessage(_T("Removing host %s"), searchHistoryKey);
			bRemove = true;
		}
		else {
			FindDataBase::MakeSQLString(searchHistoryKey);
			CString query;
			query.Format(_T("UPDATE SearchHistory SET LastUpdated=%I64d, MissCount=%d WHERE SearchKeys='%s'"),
			SystemUtils::TimeToInt(CTime::GetCurrentTime()),
			missCount, searchHistoryKey);
			queryArray.Add(query);
		}
		AddDBQueryStrings(queryArray);
	}
	return bRemove;
}
void CFindServerDlg::InitCatagotyList(bool bFill)
{
	for (int i = 0; i < mArrayFindCatagory.GetCount(); i++) {
		FindCatagory *fc(mArrayFindCatagory.GetAt(i));
		delete fc;
	}
	mArrayFindCatagory.RemoveAll();
	if (bFill) {
		for (int iRow = 0 ; iRow < mCatagoryEmbedListCtrl->GetItemCount(); ++iRow) {
			if (!mCatagoryEmbedListCtrl->IsChecked(0, iRow))
				continue;
			FindCatagory *fc(new FindCatagory());
			fc->Init(iRow, mCatagoryEmbedListCtrl);
			mArrayFindCatagory.Add(fc);
		}
	}
}
bool CFindServerDlg::StartLimitedThreadOperation(ServerThreadOperation op, LPVOID threadData)
{
	// Wait till one on the thread is freed.
	while (ThreadManager::GetInstance().GetAllThreadCount() > GetMaxThreadCount() && !IsSearchCancelled())
		Sleep(1000);
	if (!IsSearchCancelled()) {
		StartThreadOperation(op, threadData);
		return true;
	}
	return false;
}
static int TreeNetWorkIteratorCallBack(TreeIteratorCallBackData *pData, void *pUserParam)
{
	return ((CFindServerDlg*)pUserParam)->TreeNetWorkIteratorCallBack(pData, pUserParam);
}
static int NetWorkFindShared(LPNETRESOURCE lpNetRes, void *pUserParam)
{
	CFindServerDlg *findDlg = (CFindServerDlg *)pUserParam;
	return findDlg->NetWorkFindShared(lpNetRes);
}
int CFindServerDlg::NetWorkFindShared(LPNETRESOURCE lpNetRes)
{
	if (RESOURCEUSAGE_CONTAINER != (lpNetRes->dwUsage & RESOURCEUSAGE_CONTAINER)) {
		return SearchInFolder(lpNetRes->lpRemoteName);
	}
	else {
		int retVal(FSH_CONTINUE_SEARCH);
		PreSearchCheck(lpNetRes->lpRemoteName, retVal);
		if (retVal == FSH_CONTINUE_SEARCH) {
			lpNetRes = CopyLPNetResource(lpNetRes);
			lpNetRes->lpComment = (LPTSTR)kNETRESOURCEComment;
			if (!StartLimitedThreadOperation(SERVER_THREAD_OP_SEARCH_IN_NETWORK, lpNetRes))
				FreeLPNetResource(lpNetRes); // Thread not started - free the data
		}
	}
	return IsSearchCancelled(true) ? FCB_ABORT : FCB_CONTINUE;
}
void CFindServerDlg::StartSearchInNetworkFoder(LPNETRESOURCE lpnRes)
{
	Path remoteName;
	CountTimer ct;
	if (lpnRes->lpRemoteName != NULL) {
		SystemUtils::LogMessage(_T("Searching in %s"), lpnRes->lpRemoteName);
		remoteName = Path(lpnRes->lpRemoteName);
	}
	int count(0);
	if (mServerSearchStatus == SST_QuickSearch)
		count = QuickSearch(remoteName);
	else {
		CNetWorkFinder cnf(::NetWorkFindShared, false, this);
		count = cnf.StartFind(lpnRes);
	}
	// We are finished with this node - add entry in db
	if (remoteName.IsUNC()) {
		// Note SHF_ALREADY_SEARCHED is added to mark it already processed node.
		CString rootName(remoteName.GetRoot());
		CSearchHistory *pSrchHistory(mSearchHistory.Find(rootName));
		if (!IsSearchCancelled(true)) {
			if (pSrchHistory != NULL)
				pSrchHistory->SetAlreadySearched();
			if (count <= 0 ) // Not found - update database miss count/remove obsolete
				Verify(pSrchHistory, true);
			else {
				CArrayCString queryArray;
				AddDBQueryStrings(GetQueryAddToSearchHistory(rootName, queryArray,SHF_ALREADY_SEARCHED));
			}
		}
		if (ct.GetTimeDuration() >= 1000*60)
			SystemUtils::LogMessage(_T("End: Searching in %s: Time taken: %s "), lpnRes->lpRemoteName, ct.GetString());
	}
	if (lpnRes->lpComment == kNETRESOURCEComment)
		FreeLPNetResource(lpnRes);
}
int CFindServerDlg::TreeNetWorkIteratorCallBack(void *pIterData, void *pUserParam)
{
    UNREFERENCED_PARAMETER(pUserParam);
	TreeIteratorCallBackData *pData((TreeIteratorCallBackData *)pIterData);
	HTREEITEM hItem = pData->hItem;
	CTreeCtrlDomain *pTree = (CTreeCtrlDomain *)pData->pTree;
	// Process and find
	LPNETRESOURCE lpnRes = (LPNETRESOURCE)pTree->GetItemData(hItem);
	if (lpnRes < (LPNETRESOURCE)3) {
		lpnRes = NULL;
	}
	// Search in network/folder.
	if (lpnRes)
		NetWorkFindShared(lpnRes);
	int returnVal = IsSearchCancelled() ? TICB_ABORT
		: pData->hStartItem == hItem
		? TICB_DONTITERATE_SIBLING
		: TICB_CONTINUE;
	return returnVal | TICB_DONTITERATE_CHILREN;
}
int CFindServerDlg::SearchInNetwork(HTREEITEM hItem)
{
	LPNETRESOURCE lpnRes = (LPNETRESOURCE)mTreeCtrlDomain->GetItemData(hItem);
	if (lpnRes < (LPNETRESOURCE)2) // Resolved Folder
		return FIND_CONTINUE_SEARCH;
	CTreeCtrlIterator ti(mTreeCtrlDomain, ::TreeNetWorkIteratorCallBack, this);
	ti.StartIterationEx(hItem);
	return ti.IsAborted() ? FIND_ABORT_SEARCH : FIND_NOTNETWORK_NODE;
}
int CFindServerDlg::SearchInNetwork(const CString &networkPath)
{
	NETRESOURCE netRes = {0};
	netRes.lpRemoteName = (LPWSTR)(LPCTSTR)networkPath;
	netRes.dwUsage = RESOURCEUSAGE_CONTAINER;
	return NetWorkFindShared(&netRes);
}
struct FindFolderCallback_Data {
	FindFolderCallback_Data(CFindServerDlg *pSrvDlg = NULL, const CSearchHistory *pSearchHistory = NULL)
		: pDlg(pSrvDlg), searchHistory(pSearchHistory), mTimeElapsed(10*1000), // 10 sec refresh time
			m_uMatchCount(0), pExtraData(NULL)
	{}
	CFindServerDlg *pDlg;
	const CSearchHistory *searchHistory;
	CArrayCString dbQueryString;
	CountTimer mTimeElapsed;
	UINT_PTR m_uMatchCount;
	CFindServerDlg::SearchInFolderData *pExtraData;
};
int CFindServerDlg::FindFolderCallback(CFileFindEx *pFindFile, bool bMatched, void *pUserData)
{
	FindFolderCallback_Data *pFFCD = (FindFolderCallback_Data *)pUserData;
	CString path(pFindFile->GetFilePath());
	bool bIsDir(pFindFile->IsDirectory()==TRUE);
	bool bProcess(true);
	int retVal(FCB_CONTINUE);
	if (IsSearchCancelled(true)) {
		GetQueryAddToSearchHistory(path, pFFCD->dbQueryString);
		retVal = FCB_ABORT;
		bProcess = false;
	}
	// Skip till search history
	else if (pFFCD->searchHistory != NULL && bIsDir) {
		if (ComparePath(path, pFFCD->searchHistory->GetSearchKey(), true) < 0)
			retVal = FCB_NORECURSIVE;
		else {
			// Remove existing from search history
			GetQueryAddToSearchHistory(pFFCD->searchHistory->GetRootKey(), pFFCD->dbQueryString);
			pFFCD->searchHistory = NULL;
		}
		bProcess = false;
	}
	if (pFFCD->pExtraData && pFFCD->pExtraData->callbackFn)
		retVal = (this->*pFFCD->pExtraData->callbackFn)(pFindFile, bMatched, pFFCD->pExtraData->pUserData);
	if (bProcess) { // Now do actual stuff
		// Match category
		FindCatagory *pCatagotyMatched(NULL);
		for (INT_PTR i = 0; i < mArrayFindCatagory.GetCount(); ++i) {
			if (mArrayFindCatagory.GetAt(i)->Match(pFindFile)) {
				pCatagotyMatched = mArrayFindCatagory.GetAt(i);
				break;
			}
		}
		if (pCatagotyMatched != NULL) { // Got match
			pFFCD->m_uMatchCount++;
			// Update table
			FindDataBase::MakeSQLString(path);
			CTime ct, mt, at;
			try {
			pFindFile->GetCreationTime(ct);
			pFindFile->GetLastWriteTime(mt);
			pFindFile->GetLastAccessTime(at);
			}
			catch(...) {}
			CString query, rootPath(Path(path).GetRoot());
			INT_PTR curTime(SystemUtils::TimeToInt(CTime::GetCurrentTime()));
			query.Format(_T("INSERT OR REPLACE INTO CachedData VALUES ('%s', '%s', %I64d, 0, '%d', '%s', '%I64d', '%I64d', '%I64d', %I64d)"),
				path,
				bIsDir ? _T("") : SystemUtils::LongLongToString(pFindFile->GetFileSize()),
				curTime,
				pCatagotyMatched->catagoryNum,
                rootPath,
				SystemUtils::TimeToInt(ct),
				SystemUtils::TimeToInt(mt),
				SystemUtils::TimeToInt(at),
				curTime);
			CString query2;
			CString condition;
			condition.Format(_T("WHERE Path='%s'"), path);
			query2.Format(_T("UPDATE CachedData SET Path='%s', Size='%s', LastUpdated=%I64d, MissCount=0, CatagoryNumber=%d, Root='%s', CreatedTime=%I64d, ModifiedTime=%I64d, AccessedTime=%I64d WHERE Path='%s'"),
				path,
				bIsDir ? _T("") : SystemUtils::LongLongToString(pFindFile->GetFileSize()),
				curTime,
				pCatagotyMatched->catagoryNum,
                rootPath,
				SystemUtils::TimeToInt(ct),
				SystemUtils::TimeToInt(mt),
				SystemUtils::TimeToInt(at),
				path);
			query = _T("|")+query2+_T("|CachedData|")+condition+_T("|")+query;
			pFFCD->dbQueryString.Add(query);
            if (!bIsDir) {
                if (pCatagotyMatched->IsFindContent()) {
                    FileTableEntry fte;
                    fte.path = path;
                    fte.fileModTime = mt;
                    // Add entry to content db
                    mContentSearchManager.AddFileEntry(fte);
                }
                else
                    mContentSearchManager.RemoveFileEntry(path, true);
            }
		}
	}
	if (pFFCD->dbQueryString.GetCount() > 99) {
		AddDBQueryStrings(pFFCD->dbQueryString);
		pFFCD->dbQueryString.RemoveAll();
	}
	if (pFFCD->mTimeElapsed.UpdateTimeDuration()) {
		ThreadManager::GetInstance().SetThreadStatusStr(_T("Searching %s"), path);
	}
	return retVal;
}
int CFindServerDlg::SearchInFolder(const CString &pathToSearch, SearchInFolderData *pExtraData /* = NULL */)
{
	int retVal(0);
	FindFolderCallback_Data ffcd(this, PreSearchCheck(pathToSearch, retVal, 0));
	if (retVal == FSH_SKIP_SEARCH)
		return FIND_SKIP_SEARCH;
	ffcd.pExtraData = pExtraData;
	FinderCallbackData_CFindServerDlg fcbData(this, &CFindServerDlg::FindFolderCallback, &ffcd);
	CFinder cf(NULL, NULL, pExtraData ? pExtraData->bRecursive : true);
	fcbData.Find(cf, pathToSearch);
	retVal = cf.IsAborted() ? FIND_ABORT_SEARCH : FIND_CONTINUE_SEARCH;
	AddDBQueryStrings(ffcd.dbQueryString);
	return retVal;
}
struct VerifyData {
	CArrayCString queryStrings;
	CountTimer timer;
};
int CFindServerDlg::ItrVerifyCacheDataTableRowsCallbackFn(sqlite3_stmt *statement, void *pUserData)
{
	if (IsSearchCancelled(true))
		return 1;
	CTime cuurentTime(CTime::GetCurrentTime());
    Path path(SystemUtils::UTF8ToUnicodeCString((const char *)sqlite3_column_text(statement, CachedData_Path)));
    LONGLONG nDays(0);
	{
		CTime lastUpdated(SystemUtils::IntToTime(sqlite3_column_int64(statement, CachedData_LastUpdated)));
		CTimeSpan timeDiff = cuurentTime -lastUpdated;
		nDays = timeDiff.GetDays();
        const int delayDays(path.IsUNC() ? 3 : IsSearchForce() ? 0 : 1);
		if (nDays < delayDays) // More than 3 days
			return 0;
	}
	CString query;
	CString condition;
    CString qpath(path);
    {
		FindDataBase::MakeSQLString(qpath);
		condition.Format(_T(" WHERE Path='%s'"), qpath);
	}
	VerifyData *pVerifyData((VerifyData*)pUserData);
	if (pVerifyData->timer.UpdateTimeDuration())
		ThreadManager::GetInstance().SetThreadStatusStr(_T("Verifying %s"), (LPCTSTR)path);
	if (path.Exists()) {
		query.Format(_T("UPDATE CachedData SET LastUpdated=%I64d, MissCount=0"),
			SystemUtils::TimeToInt(cuurentTime));
        mContentSearchManager.AddFileEntry(FileTableEntry(qpath));
	}
	else {
		UINT missCount(sqlite3_column_int(statement, CachedData_MissCount)+1);
		if (!path.IsUNC() || missCount > 3 || nDays > 9) {
			query = _T("DELETE FROM CachedData");
            mContentSearchManager.RemoveFileEntry(path);
		}
		else {
			query.Format(_T("UPDATE CachedData SET LastUpdated=%I64d, MissCount=%d"),
				SystemUtils::TimeToInt(cuurentTime), missCount);
		}
	}
	query += condition;
	pVerifyData->queryStrings.Add(query);
	if (pVerifyData->queryStrings.GetCount() > 99) {
		AddDBQueryStrings(pVerifyData->queryStrings);
		pVerifyData->queryStrings.RemoveAll();
	}
	return 0;
}
void CFindServerDlg::Verify(CSearchHistory *rootPath, bool bRemove /* = false */)
{
	if (rootPath == NULL)
		return;
	const CString &rootKey(rootPath->GetRootKey());
	SystemUtils::LogMessage(_T("Verifying %s"), rootKey);
	if (IsMirrorServer(rootKey)) {
		bRemove = true;
		rootPath->ResetMissCount(~(1<<((sizeof(UINT)<<3)-1)));
	}
	if (!bRemove && NetWorkOpenConnect(rootKey) == 0) {
		bRemove = false;
		rootPath->SetAvailable(true);
		VerifyData verifyData;
		verifyData.timer.Reset(1000*60);
		ItrTableRowsCallbackData_CFindServerDlg itSHTable(this,
			&CFindServerDlg::ItrVerifyCacheDataTableRowsCallbackFn,
			&verifyData);
		char condition[1024];
		sprintf_s(condition, sizeof(condition)/sizeof(condition[0]),
			"WHERE Root='%s'",
			SystemUtils::UnicodeToUTF8(rootKey).c_str());
		itSHTable.IterateTableRows(mDataBase, "CachedData", condition);
		NetWorkCloseConnection(rootKey);
		AddDBQueryStrings(verifyData.queryStrings);
	}
	else
		bRemove = true;
	if (bRemove) {
		rootPath->SetAvailable(false);
		bRemove = RemoveObsoleteSearchHistory(*rootPath);
	}
	if (rootPath->IsAvailable() && !IsSearchCancelled(true))
		AddDBQueryString(_T("UPDATE SearchHistory SET LastUpdated=%I64d, MissCount=0 WHERE SearchKeys='%s'"),
			SystemUtils::TimeToInt(CTime::GetCurrentTime()), rootPath->GetRootKey());
}
void CFindServerDlg::StartIpEnum()
{
	if (IsEnumIP()) {
		bool bStartIPEnum(true);
		CString lastUpdatedStr(mDataBase.GetProperty(_T("EnumIP_LastUpdateTime")));
		if (!lastUpdatedStr.IsEmpty()) {
			CArrayCString ipes;
			mDataBase.GetTableColTexts("Property", "WHERE Name LIKE 'IPE_%'", ipes);
			if (ipes.IsEmpty() && mDataBase.GetProperty(_T("Main_IPE")).IsEmpty()) { // start only if all ip enum threads were finished.
				CTime lastUpdated(SystemUtils::StringToDate(lastUpdatedStr));
				CTimeSpan timeDiff = CTime::GetCurrentTime() - lastUpdated;
				CTimeSpan enumIP_Time(SystemUtils::StringToLongLong(mDataBase.GetProperty(_T("EnumIP_Time")))/1000);
				LONGLONG nDays(enumIP_Time.GetDays()+1);
				nDays *= 3;
				if (timeDiff.GetDays() < nDays) { // check for days passed.
					bStartIPEnum = false;
				}
			}
		}
		if (bStartIPEnum)
			StartThreadOperation(SERVER_THREAD_OP_IPENUM);
	}
}
void CFindServerDlg::Find(bool bForce) // Start Find
{
	if (IsSearching()) // Already started - exit
		return;
	// Check for at least 1 day have been passed since last cache update
	if (!bForce)
	{
		CString lastUpdatedStr(
			FindDataBase::SGetProperty(_T("LastUpdateTime"), FDB_CacheDatabase));
		if (!lastUpdatedStr.IsEmpty()) {
			CTime lastUpdated(SystemUtils::StringToDate(lastUpdatedStr));
			CTimeSpan timeDiff = CTime::GetCurrentTime() - lastUpdated;
			if (timeDiff.GetDays() < 1) {
				return;
			}
		}
	}
	{
		int retVal(mDataBase.Open());
		if (retVal != 0) {
			GetLogger().Log(Logger::kLogLevelFatal, _T("Cannot open database %s. Error: %d"), mDataBase.GetDBPath(), retVal);
			return;
		}
        CString started(mDataBase.GetProperty(_T("StartedByUser")));
        if (!bForce && started.IsEmpty())
            return;
        mDataBase.SetProperty(_T("StartedByUser"), _T("1"));
		CString lastStartTime(mDataBase.GetProperty(_T("StartTime")));
		if (lastStartTime.IsEmpty()) {
			// Set start time
			m_LastStartTime = CTime::GetCurrentTime();
			mDataBase.SetProperty(_T("StartTime"),
				SystemUtils::DateToString(m_LastStartTime));
		}
		else {
			m_LastStartTime = SystemUtils::StringToDate(lastStartTime);
		}
	}
    SetSearchForce(bForce);
	AddThreadStatusPanel();
	SystemUtils::LogMessage(_T("Server: Start"));
    // Start db commiter thread
    m_pDBCommitter = DBCommiterManager::GetInstance().AddDbCommiter(&mDataBase);
    SetSearchStarted();
    mContentSearchManager.StartContentSearch(ContentSearchManager::CheckIfContentSearchRequired|
        (bForce ? ContentSearchManager::ForceSearch : 0));
	if (mServerSearchStatus != SST_Verify) { // Search mode
		SystemUtils::LogMessage(_T("Server: Search started"));
		{
			CString title(_T("Searching"));
			if (mServerSearchStatus == SST_QuickSearch)
				title += _T(" Quick");
			SetTitle(title);
		}
		Execute();
		HTREEITEMVec searchList;
		mTreeCtrlDomain->GetCheckList(searchList);
		InitCatagotyList();
		StartIpEnum();
		// Start find here
		int retVal = FIND_CONTINUE_SEARCH;
		for (HTREEITEMVec::const_iterator it = searchList.begin();
			it != searchList.end() && retVal != FIND_ABORT_SEARCH;
			++it) {
			retVal = SearchInNetwork(*it);
			if (retVal == FIND_CONTINUE_SEARCH) {
				CString localPath(mTreeCtrlDomain->GetFilePath(*it));
				retVal = SearchInFolder(localPath);
				if (retVal != FIND_ABORT_SEARCH) {
					CStringArray queryList;
					AddDBQueryStrings(GetQueryAddToSearchHistory(localPath, queryList, SHF_ALREADY_SEARCHED));
				}
			}
		}
	}
	else
	{ // Verification work flow
		SystemUtils::LogMessage(_T("Server: Verify started"));
		SetTitle(_T("Verifying"));
		Execute(_T("mirror,skip"));
    }
	// Also search in history machines
	INT_PTR searchHistoryCount(mSearchHistory.GetCount());
	for (INT_PTR i=0 ; i < searchHistoryCount
		&& !IsSearchCancelled(); ++i) {
		CSearchHistory *rootPath(mSearchHistory.GetAt(i));
		if (rootPath->GetLastupdated() >= m_LastStartTime)
			continue; // Skip this node as it was already verified.
		
		if (mServerSearchStatus == SST_Verify)
			StartLimitedThreadOperation(SERVER_THREAD_OP_VERIFY, (LPVOID)rootPath);
		else
			SearchInNetwork(rootPath->GetRootKey());
	}
	// Search for ip enum threads
	while(!IsSearchCancelled()) {
		CString hostName(PopHostName());
		if (hostName.IsEmpty()) { // No host found
			if (ThreadManager::GetInstance().GetThreadCount(SERVER_THREAD_OP_IPENUM) > 0) // If ip enum thread is still running then wait for more host to come
				Sleep(1000);
			else break; // else no more hosts left - we are done
		}
		else
			SearchInNetwork(hostName); // search the host
	}
	Sleep(5000);
	// Wait for other worker threads to finish up
	SystemUtils::LogMessage(_T("Server: Wait for other worker threads to finish up"));
    // Wait for content search to finish up
    SystemUtils::LogMessage(_T("Server: Wait for content search finish up"));
    mContentSearchManager.StopContentSearch(IsSearchCancelled());
    // Wait for db commiter thread to finish up
	SystemUtils::LogMessage(_T("Server: Wait for db finish up"));
    DBCommiterManager::GetInstance().RemoveDBCommitter((FindDataBase*)-1); // Remove all and wait
    InitCatagotyList(false);
    // toggle search type 
	// Restart search again 
	if (!IsSearchCancelled()) {
		mServerSearchStatus = (ServerSearchStatus)((mServerSearchStatus + 1) % SST_Total);
		mDataBase.SetProperty(_T("ServerSearchStatus"),
			SystemUtils::IntToString(mServerSearchStatus));
		mDataBase.SetProperty(_T("LastUpdateTime"),
			SystemUtils::DateToString(CTime::GetCurrentTime()));
		mDataBase.RemoveProperty(_T("StartTime"));
	}
    else if (!IsDialogClosed()) // Explicitly stopped by user
        mDataBase.RemoveProperty(_T("StartedByUser"));
    mDataBase.Commit();
	mDataBase.Close();
	AddThreadStatusPanel(false);
    bool bSearchCancelled(IsSearchCancelled());
	SetSearchStarted(false);
	SetTitle(NULL);
	SystemUtils::LogMessage(_T("Server: Finished%s"), bSearchCancelled ? _T(" - Operation cancelled") : _T(""));
}
struct ThreadData {
	CFindServerDlg *pFindDlg;
	ServerThreadOperation threadOp;
	LPVOID pThreadData;
	ThreadData(CFindServerDlg *inpFindDlg, ServerThreadOperation inthreadOp, LPVOID inpThreadData)
	{
		pFindDlg = inpFindDlg;
		threadOp = inthreadOp;
		pThreadData = inpThreadData;
	}
};

static int TMFindServerDlgThreadProcFn(LPVOID pInThreadData)
{
	ThreadData *pThreadData((ThreadData *)pInThreadData);
	pThreadData->pFindDlg->DoThreadOperation(pThreadData);
	delete pThreadData;
	return 0;
}

int CFindServerDlg::StartThreadOperation(ServerThreadOperation op, LPVOID threadData)
{
    DWORD threadID(0);
	ThreadData *td = new ThreadData(this, op, threadData);
	ThreadManager::GetInstance().CreateThread(TMFindServerDlgThreadProcFn, td, op, &threadID, ServerThreadOperationGetThreadName(op));
	return threadID;
}
int CFindServerDlg::DoThreadOperation(LPVOID pInThreadData)
{
	ThreadData *pThreadData((ThreadData *)pInThreadData);
	switch (pThreadData->threadOp) {
    case SERVER_THREAD_OP_LOAD_DEFAULT:
        LoadDefault();
        break;
    case SERVER_THREAD_OP_EXECUTE:
        Execute();
        break;
	case SERVER_THREAD_OP_START_SEARCH:
		Find(pThreadData->pThreadData != NULL);
        if (IsDialogClosed())
            SetTimer(TIMER_QUIT_APP, 100, NULL);
        SetTimer(TIMER_NEXT_SEARCH, 1000 * 60 * 60 * 3, NULL); // Wait for other 3 hours
		break;
	case SERVER_THREAD_OP_SEARCH_IN_NETWORK:
		StartSearchInNetworkFoder((LPNETRESOURCE)pThreadData->pThreadData);
		break;
	case SERVER_THREAD_OP_VERIFY:
		Verify((CSearchHistory*)pThreadData->pThreadData);
		break;
	case SERVER_THREAD_OP_IPENUM:
		EnumerateIps((DWORD_PTR)pThreadData->pThreadData);
		break;
	}
	return 0;
}

void CFindServerDlg::OnBnClickedButtonLoaddefault()
{
    StartThreadOperation(SERVER_THREAD_OP_LOAD_DEFAULT);
}

static bool StrRemoveLastCharFromString(CString &inOutStr, TCHAR lastChar)
{
    bool bHas(false);
    if (inOutStr.GetLength() > 0 && inOutStr[inOutStr.GetLength() - 1] == lastChar) {
        inOutStr = inOutStr.Left(inOutStr.GetLength() - 1);
        bHas = true;
    }
    return bHas;
}


void CFindServerDlg::AddDBQueryString(LPCTSTR inStr, ...)
{
	va_list args;
	// retrieve the variable arguments
	va_start( args, inStr );
	CString str;
	str.FormatV(inStr, args);
	va_end(args);
	AddDBQueryString(str);
}
void CFindServerDlg::AddDBQueryString(const CString &queryString)
{
    m_pDBCommitter->AddDBQueryString(queryString);
}
void CFindServerDlg::AddDBQueryStrings(const CArrayCString &queryStrings)
{
	if (queryStrings.GetCount() <= 0)
		return;
    m_pDBCommitter->AddDBQueryStrings(queryStrings);
}
void CFindServerDlg::StartFind(bool bForce)
{
	ThreadManager &tm(ThreadManager::GetInstance());
	if (!IsSearching() && tm.GetAllThreadCount() == 0) {
		StartThreadOperation(SERVER_THREAD_OP_START_SEARCH, (LPVOID)bForce);
	}
	else {
		CString buttonText;
		buttonText.LoadString(IDS_STOPPING);
		SetDlgItemText(IDOK, buttonText);
        SetSearchCancel();
		tm.TerminateThreads();
	}
}
void CFindServerDlg::OnBnClickedOk()
{
	StartFind(true);
}
void CFindServerDlg::OnCancel()
{
	if (GetCapture())
		return;
	if (IsSearching()) {
        SetSearchCancel();
        mContentSearchManager.StopContentSearch(true, false);
        SetDialogClosed();
		ShowWindow(SW_HIDE);
	}
	else {
		CDialog::OnCancel();
	}
}
BOOL CFindServerDlg::PreTranslateMessage(MSG* pMsg)
{
	switch (pMsg->message) {
	case WM_KEYDOWN:
		switch (pMsg->wParam) {
		case VK_ESCAPE:
			{
				CWnd *pWnd = GetCapture();
				if (pWnd) {
					pWnd->SendMessage(pMsg->message, pMsg->wParam, pMsg->lParam);
				}
				else {
					ShowWindow(SW_MINIMIZE);
				}
				return TRUE;
			}
			break;
		}
		break;
	}
	return CDialog::PreTranslateMessage(pMsg);
}
void CFindServerDlg::SetTitle(LPCTSTR newTtile)
{
	CString title(_T("Find - Server"));
	if (newTtile != NULL) {
		title += _T(" - ");
		title += newTtile;
	}
	SetWindowText(title);
	NOTIFYICONDATA nicData = {sizeof (NOTIFYICONDATA)};
	nicData.hWnd = m_hWnd;
	nicData.uID = 1;
	nicData.uFlags = NIF_TIP;
	GetWindowText(nicData.szTip, sizeof(nicData.szTip)/sizeof(nicData.szTip[0]));
	Shell_NotifyIcon(NIM_MODIFY, &nicData);
}
void CFindServerDlg::OnTvnItemCheckStateChangedTreeDomain(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 0;
	LPNMTVITEMCHECKSTATECHANGED pTVItemCheckStateChanged = reinterpret_cast<LPNMTVITEMCHECKSTATECHANGED>(pNMHDR);
	if (pTVItemCheckStateChanged->item == NULL)
		return;
	CString command(_T("check:"));
	command += mTreeCtrlDomain->GetFilePath(pTVItemCheckStateChanged->item, false);
	if (pTVItemCheckStateChanged->bChecked)
		mCmdEditCtrl->AddCommand(command, true);
	else {
		mCmdEditCtrl->RemoveCommand(command, true);
		command = _T("check:") + mTreeCtrlDomain->GetFilePath(pTVItemCheckStateChanged->item, true);
		mCmdEditCtrl->RemoveCommand(command, true);
	}
}
BOOL CFindServerDlg::OnDeviceChange(UINT nEventType, DWORD_PTR dwData)
{
	mTreeCtrlDomain->OnDeviceChange(nEventType, dwData);
	return __super::OnDeviceChange(nEventType, dwData);
}
bool CFindServerDlg::PushIpHostname(const CString& hostName)
{
	CAutoLock autoLock(mLockerIPEnumHostNames);
	bool bPushed(false);
	if (mSearchHistory.Find(hostName) == NULL) {
		mSearchHistory.Insert(hostName);
		mIPEnumHostNames.Add(hostName);
		bPushed = true;
	}
	return bPushed;
}
CString CFindServerDlg::PopHostName()
{
	CAutoLock autoLock(mLockerIPEnumHostNames);
	CString hostName;
	if (mIPEnumHostNames.GetCount() > 0) {
		hostName = mIPEnumHostNames.GetAt(0);
		mIPEnumHostNames.RemoveAt(0, 1);
	}
	return hostName;
}
struct IPEnumerator_Callback_data {
	IPEnumerator_Callback_data() : pDlg(NULL), ipData(NULL), timeElapsed(1000*60) // 1 min to save into db
	{}
	CFindServerDlg *pDlg;
	IPEnumCBData* ipData;
	CString startIP;
	CString endIp;
	CountTimer timeElapsed;
};
static int Srv_IPEnumerator_Callback(IPEnumCBData *ipData, void *pUSerData)
{
	IPEnumerator_Callback_data *pData((IPEnumerator_Callback_data *)pUSerData);
	pData->ipData = ipData;
	return pData->pDlg->IPEnumerator_Callback(pData);
}
int CFindServerDlg::IPEnumerator_Callback(void *ipData)
{
	bool  bCancelled(IsSearchCancelled(true));
	IPEnumerator_Callback_data *pIECData((IPEnumerator_Callback_data *)ipData);
	IPEnumCBData* pData(pIECData->ipData);
	if (!bCancelled) {
		if (pData->hostname[0] && !IsMirrorServer(pData->hostname)) {
			CString networkPath(pData->hostname);
			networkPath = _T("\\\\") + networkPath;
			NETRESOURCE netRes = {0};
			netRes.lpRemoteName = (LPWSTR)(LPCTSTR)networkPath;
			netRes.dwUsage = RESOURCEUSAGE_CONTAINER;
			CNetWorkFinder cnf(NULL, false);
			if (cnf.StartFind(&netRes) > 0) {
				if (PushIpHostname(networkPath)) {
					SystemUtils::LogMessage(_T("Server: ipenumerator: ip=%s, host=%s"), pData->ip, pData->hostname);
					CArrayCString queryList;
					AddDBQueryStrings(GetQueryAddToSearchHistory(networkPath, queryList));
				}
			}
		}
	}
	if (pIECData->timeElapsed.UpdateTimeDuration(bCancelled)) { // more than one minutes -  save state to db
		ThreadManager::GetInstance().SetThreadStatusStr(_T("ipenumerator  %s"), pData->ip);
		AddDBPropertyKeyValue( _T("IPE_") + pIECData->endIp, pData->ip);
	}
	return bCancelled ? 1 : 0;
}
bool CFindServerDlg::StartEnumerateIps(DWORD startIp, DWORD endIp)
{
	DWORD_PTR ipRange(startIp);
	ipRange <<= 32;
	ipRange |= endIp;
	return StartLimitedThreadOperation(SERVER_THREAD_OP_IPENUM, (LPVOID)ipRange);
}
static void AddIpToArray(CArrayCString &inArray, DWORD inIP)
{
	DWORD inIPToMatch(inIP &= 0xffff0000);
	for (INT_PTR i=0; i < inArray.GetCount(); ++i) {
		DWORD ip(SocketUtil::GetIpV4AddrFromString(inArray.GetAt(i)));
		if ((ip & 0xffff0000) == inIPToMatch)
			return;
	}
	inArray.Add(SocketUtil::GetIpV4StrFromAddr(inIP));
}
void CFindServerDlg::EnumerateIps(DWORD_PTR ipRange)
{
	if (ipRange == 0) {
		SystemUtils::LogMessage(_T("Server: ipenumerator: Start"));
		CArrayCString startIPs;
		bool bStartMainIPEnum(true);
		SystemUtils::SplitString(mDataBase.GetProperty(_T("Main_IPE")), startIPs);
		mIPEnumTime.Reset(1000*60);
		mIPEnumTime.SetTimeDuration(SystemUtils::StringToLongLong(mDataBase.GetProperty(_T("EnumIP_Time"))));
		StartThreadOperation(SERVER_THREAD_OP_IPENUM, (LPVOID)~0);
		{
			CArrayCString ipes;
			mDataBase.GetTableColTexts("Property", "WHERE Name LIKE 'IPE_%'", ipes);
			if (!ipes.IsEmpty()) {
				if (startIPs.IsEmpty())
					bStartMainIPEnum = false; // Main ip enumerator thread was finished - so do not start it again
				for (int i = 0; i < ipes.GetCount(); i+=2) {
					DWORD eip(SocketUtil::GetIpV4AddrFromString(((LPCTSTR)ipes.GetAt(i))+4));
					DWORD sip(SocketUtil::GetIpV4AddrFromString(ipes.GetAt(i+1)));
					if (!StartEnumerateIps(sip, eip))
						break;
				}
			}
		}
		if (bStartMainIPEnum) {
			if (startIPs.IsEmpty()) {
				for (INT_PTR i=0; i < mHostIPsToEnumerate.GetCount() && !IsSearchCancelled(); ++i) {
					if (mHostIPsToEnumerate[i].IsEmpty()) {
						mHostIPsToEnumerate[i] = SocketUtil::GetHostName();
						if (!SocketUtil::IsConnectedToLAN()) {
							GetLogger().Log(Logger::kLogLevelWarning, _T("Not connected to LAN - skipping ip enum"));
							continue;
						}
					}
					SocketUtil::VecWString ips;
					SocketUtil::GetIPs(mHostIPsToEnumerate[i], ips);
					if (ips.size() > 0) {
						DWORD startIp(SocketUtil::GetIpV4AddrFromString(ips.begin()->c_str()));
						startIp &= 0xffff0000; // Start from zero e.g. ip x.y.0.0
						AddIpToArray(startIPs, startIp);
					}
				}
				mIPEnumTime.Reset(1000*60);
			}
			for (INT_PTR i=0; i < startIPs.GetCount() && !IsSearchCancelled(); ++i) {
				DWORD startIp(SocketUtil::GetIpV4AddrFromString(startIPs[i]));
				DWORD endIp = (startIp & 0xffff0000) + 0xffff;
				CountTimer timeElapsed(1000*60); // 1 min
				while((startIp < endIp) && !IsSearchCancelled()) {
					DWORD sip(startIp);
					startIp += 256;
					if (startIp > endIp)
						startIp = endIp;
					if (!StartEnumerateIps(sip, startIp))
						break;
					if (timeElapsed.UpdateTimeDuration()) { // more than one minutes -  save state to db
						startIPs[i] = SocketUtil::GetIpV4StrFromAddr(startIp);
						AddDBPropertyKeyValue(_T("Main_IPE"), SystemUtils::CombineString(startIPs));
					}
				}
				if (startIp < endIp) {
					startIPs[i] = SocketUtil::GetIpV4StrFromAddr(startIp);
					AddDBPropertyKeyValue(_T("Main_IPE"), SystemUtils::CombineString(startIPs));
				}
			}
			UpdateIpEnumStatusInDB(_T("Main_IPE"));
		}
	}
	if (ipRange == ~0) {
		// update timer
		ThreadManager::GetInstance().SetThreadStatusStr(_T("ipenumerator: time  %s"), mIPEnumTime.GetString());
		while (!IsSearchCancelled(true) && ThreadManager::GetInstance().GetThreadCount(SERVER_THREAD_OP_IPENUM) > 1) {
			Sleep(10000);
			if (mIPEnumTime.UpdateTimeDuration()) {
				ThreadManager::GetInstance().SetThreadStatusStr(_T("ipenumerator: time  %s"), mIPEnumTime.GetString());
				AddDBPropertyKeyValue(_T("EnumIP_Time"), SystemUtils::LongLongToString(mIPEnumTime.GetTimeDuration()));
			}
		}
		SystemUtils::LogMessage(_T("Server: ipenumerator: End. Time taken %s"), mIPEnumTime.GetString());
	}
	else if (ipRange != 0) {
		DWORD startIp(ipRange >> 32);
		DWORD endIp(ipRange & 0xffffffff);
		IPEnumerator_Callback_data ipe_cb_data;
		ipe_cb_data.pDlg = this;
		ipe_cb_data.startIP = SocketUtil::GetIpV4StrFromAddr(startIp);
		ipe_cb_data.endIp = SocketUtil::GetIpV4StrFromAddr(endIp);
		ThreadManager::GetInstance().SetThreadStatusStr(_T("ipenumerator: %s to %s"), ipe_cb_data.startIP, ipe_cb_data.endIp);
		IPEnumerator ipEnumerator(Srv_IPEnumerator_Callback, &ipe_cb_data);
		ipEnumerator.Enumerate(startIp, endIp);
		if (!ipe_cb_data.endIp.IsEmpty())
			UpdateIpEnumStatusInDB(_T("IPE_") + ipe_cb_data.endIp);
	}
}
void CFindServerDlg::UpdateIpEnumStatusInDB(LPCTSTR propertyName)
{
	if (!IsSearchCancelled(true))
		AddDBQueryString(_T("DELETE FROM Property WHERE Name = '%s'"), propertyName);
	AddDBPropertyKeyValue(_T("EnumIP_LastUpdateTime"), SystemUtils::DateToString(CTime::GetCurrentTime()));
}
void CFindServerDlg::AddDBPropertyKeyValue(const CString& key, const CString &value)
{
	AddDBQueryString(_T("INSERT OR REPLACE INTO Property VALUES ('%s', '%s') "), key, value);
}


void CFindServerDlg::OnBnClickedButtonLeft()
{
	mCombinedBotton.OnButtonClickEvent(IDC_BUTTON_LEFT);
}
void CFindServerDlg::OnBnClickedButtonRight()
{
	mCombinedBotton.OnButtonClickEvent(IDC_BUTTON_RIGHT);
}
void CFindServerDlg::ShowServerMainControls(int cmdShow /* = SW_SHOW */)
{
	mServerMainControls.ShowControls(cmdShow);
}
void CFindServerDlg::AddThreadStatusPanel(bool bAdd)
{
	if (!bAdd) {
		if (m_pStatusDlg != NULL)
			m_pStatusDlg->WaitForFinish();
	}
	SendMessage(WM_SERVER_ADDTSPANEL, bAdd);
}
LRESULT CFindServerDlg::OnAddThreadStatusPanel(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
	if (wParam) {
		if (m_pStatusDlg == NULL) {
			m_pStatusDlg = new CServerStatusDlg(this);
			m_pStatusDlg->Init();
			m_pStatusDlg->SetDlgCtrlID(CServerStatusDlg::IDD);
			mCombinedBotton.AddPage(m_pStatusDlg->GetContext());
			mControlResizer.AddControl(CServerStatusDlg::IDD);
		}
	}
	else if (m_pStatusDlg != NULL) {
		mControlResizer.RemoveControl(CServerStatusDlg::IDD);
		mCombinedBotton.RemovePage(m_pStatusDlg->GetContext());
		m_pStatusDlg->DestroyWindow();
		delete m_pStatusDlg;
		m_pStatusDlg = NULL;
	}
	return 0;
}

const CSearchHistory* CFindServerDlg::PreSearchCheck(const CString &inSearchHistory, int &retVal, UINT uFlags /* = SDF_CHECK_INPROGRESS */)
{
	CSearchHistoryArray::SearchData sd = {uFlags};
	const CSearchHistory* outRetVal(mSearchHistory.FindInSearchHistory(inSearchHistory, retVal, &sd));
	if (retVal == FSH_CONTINUE_SEARCH && IsMirrorServer(inSearchHistory))
		retVal =  FSH_SKIP_SEARCH;
	return outRetVal;
}

bool CFindServerDlg::IsMirrorServer( const CString &inServerName )
{
	bool bIsMirror(false);
	CString rootKey(Path(inServerName).GetMachineNameFromUNCPath());
	for (INT_PTR i = 0; i < mArrMirrorInfo.GetCount(); ++i) {
		// if inServerName is not equal to mirror machine and it matches mirror machine reg exp, then inServerName is mirror machine
		if (mArrMirrorInfo.GetAt(i).mMirrorMachine.CompareNoCase(rootKey) && mArrMirrorInfo.GetAt(i).mStringMatcher.Match(rootKey)) {
			bIsMirror = true;
			break;
		}
	}
	return bIsMirror;
}

int CFindServerDlg::QuickSearch( const Path& remoteHostPath )
{
	int count(0);
	int retVal(0);
	const CSearchHistory* outRetVal(PreSearchCheck(CString(remoteHostPath), retVal));
	if (outRetVal != NULL && NetWorkOpenConnect(outRetVal->GetRootKey()) == 0) {
		CSortedArrayCString allPaths;
		ItrTableRowsCallbackData_CFindServerDlg itSHTable(this,
			&CFindServerDlg::ItrGetFolderListCacheDataTableRowsCallbackFn,
			&allPaths);
		char condition[1024];
		sprintf_s(condition, sizeof(condition)/sizeof(condition[0]),
			"WHERE Root='%s'",
			SystemUtils::UnicodeToUTF8(outRetVal->GetRootKey()).c_str());
		itSHTable.IterateTableRows(mDataBase, "CachedData", condition);
		count = 1;
		NetWorkCloseConnection(outRetVal->GetRootKey());
	}
	return count;
}

int CFindServerDlg::ItrGetFolderListCacheDataTableRowsCallbackFn( sqlite3_stmt *statement, void *pUserData )
{
	int retVal(FIND_CONTINUE_SEARCH);
	if (IsSearchCancelled(true))
		return FIND_ABORT_SEARCH;
	CString path(SystemUtils::UTF8ToUnicodeCString((const char *)sqlite3_column_text(statement, CachedData_Path)));
	path = Path(path).Parent();
	CSortedArrayCString *pDirList((CSortedArrayCString *)pUserData);
	if (pDirList->Find(path) < 0) {
		pDirList->InsertUnique(path);
		SearchInFolderData extraData(false, &CFindServerDlg::FindFolderCallbackForQuickSearch, pUserData);
		retVal = SearchInFolder(path, &extraData);
	}
	return retVal;
}

int CFindServerDlg::FindFolderCallbackForQuickSearch( CFileFindEx *pFindFile, bool, void *pUserData )
{
	int retVal(FCB_CONTINUE);
	if (IsSearchCancelled(true))
		retVal = FCB_ABORT;
	else if (pFindFile->IsDirectory()) {
		CSortedArrayCString *pDirList((CSortedArrayCString *)pUserData);
		CString filePath(pFindFile->GetFilePath());
		if (pDirList->Find(filePath) < 0) {
			pDirList->InsertUnique(filePath);
			retVal = FCB_DORECURSIVE;
		}
	}
	return retVal;
}

