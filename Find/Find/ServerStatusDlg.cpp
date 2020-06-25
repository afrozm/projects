// ServerStatusDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Find.h"
#include "ServerStatusDlg.h"
#include "afxdialogex.h"
#include "CountTimer.h"
#include "AutoLock.h"
#include "ListCtrlUtil.h"

LPCTSTR ServerThreadOperationGetThreadName(ServerThreadOperation threadOp)
{
    switch (threadOp)
    {
    case SERVER_THREAD_OP_START_SEARCH:
        return _T("MainSearch");
    case SERVER_THREAD_OP_START_DBCOMMITER:
        return _T("DBComitter");
    case SERVER_THREAD_OP_SEARCH_IN_NETWORK:
        return _T("WorkerSearch");
    case SERVER_THREAD_OP_VERIFY:
        return _T("Verify");
    case SERVER_THREAD_OP_IPENUM:
        return _T("IPEnum");
    case SERVER_THREAD_OP_LOAD_DEFAULT:
        return _T("LoadDefault");
    case SERVER_THREAD_OP_EXECUTE:
        return _T("Execute");
    case SERVER_THREAD_OP_STATUS:
        return _T("Status");
    }
    return NULL;
}

static lstring GetThreadName(int iThreadId) {
    LPCTSTR threadName(ThreadManager::GetInstance().GetThreadName(iThreadId));
    if (threadName == NULL)
        threadName = _T("Unknown: ");
	return threadName;
}

CServerStatusThreadNotifier::CServerStatusThreadNotifier(CServerStatusDlg *pDlg)
	: m_pDlg(pDlg)
{

}

void CServerStatusThreadNotifier::Notify( ThreadNotification notifiation, DWORD dwThreadID, LPVOID notifyData /*= NULL */ )
{
	m_pDlg->Notify(notifiation, dwThreadID, notifyData);
}

CServerStatusContext::CServerStatusContext(CServerStatusDlg *pDlg)
	: m_pDlg(pDlg)
{

}
BOOL CServerStatusContext::IsWindowVisible()
{
	return m_pDlg->IsWindowVisible();
}
void CServerStatusContext::ShowWindow(int cmdShow /* = SW_SHOW */)
{
	m_pDlg->ShowWindow(cmdShow);
}
LogTargetServerStatus::LogTargetServerStatus(CServerStatusDlg *pDlg)
	: m_pDlg(pDlg)
{

}
void LogTargetServerStatus::LogSimple(LPCTSTR logMessage)
{
	m_pDlg->SetStatus(logMessage);
}
void LogTargetServerStatus::Log(LPCTSTR logMessage)
{
	m_pDlg->Log(logMessage);
}
// CServerStatusDlg dialog

IMPLEMENT_DYNAMIC(CServerStatusDlg, CDialogEx)

CServerStatusDlg::CServerStatusDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CServerStatusDlg::IDD, pParent), mResizeBar(NULL), m_dwStatusThread((DWORD)-1),
	mControlResizer(this), mThreadNotifier(this), mContext(this), mServerStatus(this), mbStatusThreadTerminate(false)
{
	Create(CServerStatusDlg::IDD, m_pParentWnd);
	m_hAccel = LoadAccelerators(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR_SS));
	GetServerLogger().AddTarget(&mServerStatus);
}
void CServerStatusDlg::Init()
{
	// Resize as per parent
	RECT pcr;
	GetParent()->GetClientRect(&pcr);
	RECT br;
	GetParent()->GetDlgItem(IDOK)->GetWindowRect(&br);
	::MapWindowPoints(NULL, GetParent()->m_hWnd, (LPPOINT)&br, 2);
	pcr.bottom = br.top - SystemUtils::GetTranslatedDPIPixelY(5);
	pcr.top += br.bottom - br.top;
	SetWindowPos(NULL, 0, pcr.top, pcr.right-pcr.left, pcr.bottom-pcr.top, SWP_NOZORDER);
}

CServerStatusDlg::~CServerStatusDlg()
{
	DestroyAcceleratorTable(m_hAccel);
}

void CServerStatusDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

void CServerStatusDlg::Notify(ThreadNotifier::ThreadNotification notifiation, DWORD threadID, LPVOID notifyData /* = NULL */)
{
	CListCtrl *pListStatus((CListCtrl*)GetDlgItem(IDC_LIST_THREADS));
	int index(0x7fffffff);
	switch (notifiation) {
	case ThreadNotifier::ThreadCreated:
		{
			CAutoLock autoLock(mLock);
			index = pListStatus->InsertItem(index, GetThreadName(threadID).c_str());
			pListStatus->SetItemData(index, (DWORD_PTR)threadID);
			SetStatusMessage(_T("Thread: Number of threads %d"), pListStatus->GetItemCount());
		}
		break;
	case ThreadNotifier::ThreadBeingRemove:
		{
			CAutoLock autoLock(mLock);
			index = GetIndex(threadID);
			if (index >= 0)
				pListStatus->DeleteItem(index);
			SetStatusMessage(_T("Thread: Number of threads %d"), pListStatus->GetItemCount());
		}
		break;
	case ThreadNotifier::ThreadStatus:
		SetText(threadID, 1, (LPCTSTR)notifyData);
		break;
	}
}
int CServerStatusDlg::GetIndex(DWORD threadID)
{
	CListCtrl *pListStatus((CListCtrl*)GetDlgItem(IDC_LIST_THREADS));
	for (int i = 0; i < pListStatus->GetItemCount(); ++i)
		if (pListStatus->GetItemData(i) == (DWORD_PTR)threadID)
			return i;
	return -1;
}
struct ThreadData {
	CServerStatusDlg *pFindDlg;
	ServerThreadOperation threadOp;
	LPVOID pThreadData;
	ThreadData(CServerStatusDlg *inpFindDlg, ServerThreadOperation inthreadOp, LPVOID inpThreadData)
	{
		pFindDlg = inpFindDlg;
		threadOp = inthreadOp;
		pThreadData = inpThreadData;
	}
};

static int TMServerStatusThreadProcFn(LPVOID pInThreadData)
{
	ThreadData *pThreadData((ThreadData *)pInThreadData);
	pThreadData->pFindDlg->DoThreadOperation(pInThreadData);
	delete pThreadData;
	return 0;
}

DWORD CServerStatusDlg::StartThreadOperation(ServerThreadOperation op, LPVOID threadData)
{
	ThreadData *td = new ThreadData(this, op, threadData);
	DWORD threadID((DWORD)-1);
	ThreadManager::GetInstance().CreateThread(TMServerStatusThreadProcFn, td, op, &threadID, ServerThreadOperationGetThreadName(op));
	return threadID;
}
int CServerStatusDlg::DoThreadOperation(LPVOID pInThreadData)
{
	ThreadData *pThreadData((ThreadData *)pInThreadData);
	switch (pThreadData->threadOp) {
	case SERVER_THREAD_OP_STATUS:
		UpdateTime();
		break;
	}
	return 0;
}
void CServerStatusDlg::WaitForFinish()
{
	ThreadManager::GetInstance().SetNotifier(NULL);
	GetServerLogger().RemoveTarget(&mServerStatus);
	mbStatusThreadTerminate = true;
	ThreadManager::GetInstance().WaitForThread(m_dwStatusThread);
	m_dwStatusThread = (DWORD)-1;
}
BEGIN_MESSAGE_MAP(CServerStatusDlg, CDialogEx)
	ON_WM_CONTEXTMENU()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_TIMER()
END_MESSAGE_MAP()

static int ThreadIteratorCallbackInit(DWORD threadID, LPVOID pUserData)
{
	((CServerStatusDlg *)pUserData)->Notify(ThreadNotifier::ThreadCreated, threadID);
	return 0;
}
// CServerStatusDlg message handlers
BOOL CServerStatusDlg::OnInitDialog()
{
	__super::OnInitDialog();
	mResizeBar = new CResizeBar(this);
	mResizeBar->SubclassDlgItem(IDC_STATIC_HORZ_SEP, this);
	mResizeBar->OnSetCursor(NULL, 0, 0);
	mResizeBar->SetMinLeftRight(SystemUtils::GetTranslatedDPIPixelX(100), SystemUtils::GetTranslatedDPIPixelX(100));
	mControlResizer.AddControl(IDC_STATIC_THREAD_STATUS, RSZF_RIGHT_FIXED);
	mControlResizer.AddControl(IDC_LIST_THREADS, RSZF_RIGHT_FIXED);
	mControlResizer.AddControl(IDC_STATIC_HORZ_SEP, RSZF_SIZEY_FIXED|RSZF_RIGHT_FIXED);
	mControlResizer.AddControl(IDC_STATIC_LOGS, RSZF_RIGHT_FIXED);
	mControlResizer.AddControl(IDC_EDIT_LOGS);
	CControlResizer &dragControlResizer(mResizeBar->GetControlResizer());
	dragControlResizer.AddControl(IDC_STATIC_HORZ_SEP, RSZF_RIGHT_FIXED|RSZF_SIZEY_FIXED);
	dragControlResizer.AddControl(IDC_LIST_THREADS, RSZF_TOP_FIXED);
	dragControlResizer.AddControl(IDC_STATIC_LOGS, RSZF_SIZEY_FIXED);
	dragControlResizer.AddControl(IDC_EDIT_LOGS, RSZF_BOTTOM_FIXED|RSZF_RESIZE_OPPOSITE);
	mListStatus.SubclassDlgItem(IDC_LIST_THREADS, this);
	CListCtrl *pListStatus((CListCtrl*)GetDlgItem(IDC_LIST_THREADS));
	pListStatus->SetExtendedStyle(pListStatus->GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP);
	LPCTSTR columns[] = {
		_T("Name"),
		_T("Status"),
		_T("Time"),
	};
	for (int i = 0; i < sizeof(columns)/sizeof(LPCTSTR); ++i) {
		pListStatus->InsertColumn(i, columns[i], 0, SystemUtils::GetTranslatedDPIPixelX(150));
	}
	int colWidth(0);
	for (int i = 1; i < pListStatus->GetHeaderCtrl()->GetItemCount(); ++i) {
		colWidth += pListStatus->GetColumnWidth(i);
	}
	RECT rc;
	pListStatus->GetClientRect(&rc);
	colWidth = rc.right-rc.left-colWidth;
	if (colWidth > 0)
		pListStatus->SetColumnWidth(1, colWidth);
	ThreadManager::GetInstance().IterateThread(ThreadIteratorCallbackInit, this);
	mLogEditCtrl.SubclassDlgItem(IDC_EDIT_LOGS, this);
	mbStatusThreadTerminate = false;
	m_dwStatusThread = StartThreadOperation(SERVER_THREAD_OP_STATUS, NULL);
	ThreadManager::GetInstance().SetNotifier(&mThreadNotifier);
	return TRUE;
}
void CServerStatusDlg::OnDestroy()
{
	mbStatusThreadTerminate = true;
	m_dwStatusThread = (DWORD)-1;
	delete mResizeBar;
}
void CServerStatusDlg::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);
	switch (nType) {
	case SIZE_MINIMIZED:
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
void CServerStatusDlg::SetStatus(LPCTSTR statusMsg)
{
	SetText(ThreadManager::GetInstance().GetCurrentThread(), 1, statusMsg);
}
struct ThreadIterData {
	ThreadIterData(DWORD inThreadID = 0) : threadID(inThreadID) {}
	DWORD threadID;
	CString strThreadTime;
};
static int ThreadIteratorCallback(DWORD threadID, LPVOID pUserData)
{
	CArray<ThreadIterData> *pThreadTimes((CArray<ThreadIterData> *)pUserData);
	INT_PTR index(pThreadTimes->Add(ThreadIterData(threadID)));
	pThreadTimes->GetAt(index).strThreadTime = CountTimer::GetString(ThreadManager::GetInstance().GetThreadTime(threadID));
	return 0;
}
void CServerStatusDlg::UpdateTime()
{
	while (!mbStatusThreadTerminate) {
		DWORD sleepTime(100);
		if (IsWindowVisible()) {
			CArray<ThreadIterData> threadTimes;
			ThreadManager::GetInstance().IterateThread(ThreadIteratorCallback, &threadTimes);
			for (INT_PTR i = 0; i < threadTimes.GetCount(); ++i)
				SetText(threadTimes[i].threadID, 2, threadTimes[i].strThreadTime);
			sleepTime = 1000;
		}
		Sleep(sleepTime);
	}
	m_dwStatusThread = (DWORD)-1;
}
void CServerStatusDlg::SetText(DWORD threadID, int col, LPCTSTR text)
{
	CAutoLock autoLock(mLock);
	int index(GetIndex(threadID));
	CListCtrlUtil *pListStatus((CListCtrlUtil*)GetDlgItem(IDC_LIST_THREADS));
	if (index >= 0)
		pListStatus->SetItemTextIfModified(index, col, text);
}
void CServerStatusDlg::SetStatusMessage(LPCTSTR fmt, ...)
{
	if (fmt == NULL)
		fmt = _T("");
	va_list arg;
	va_start(arg, fmt);
	TCHAR buf[4096];
	_vstprintf_s(buf, sizeof(buf)/sizeof(TCHAR), fmt, arg);
	SetDlgItemText(IDC_STATIC_THREAD_STATUS, buf);
}
void CServerStatusDlg::OnContextMenu(CWnd* pWnd, CPoint pos)
{
	CListCtrl *pListStatus((CListCtrl*)GetDlgItem(IDC_LIST_THREADS));
	if (pWnd == pListStatus) {
		CMenu menu;
		menu.LoadMenu(IDR_MENU_CONTEXT_SS);
		CMenu *contextMenu = menu.GetSubMenu(0);
		UINT toRemove[] = {ID_SERVERSTATUS_TERMINATE};
		if (mListStatus.GetSelectedCount() == 0)
			for (int i = 0; i < sizeof (toRemove) / sizeof (UINT); i++)
				contextMenu->RemoveMenu(toRemove[i], MF_BYCOMMAND);
		int idCmd = contextMenu->TrackPopupMenu(TPM_RETURNCMD, pos.x, pos.y, GetParent());
		contextMenu->DestroyMenu();
		PostMessage(WM_COMMAND, MAKEWPARAM(idCmd, 0));
	}
}
BOOL CServerStatusDlg::PreTranslateMessage(MSG* pMsg)
{
	if (TranslateAccelerator(m_hWnd, m_hAccel, pMsg))
		return TRUE;
	return m_pParentWnd->PreTranslateMessage(pMsg);
}
BOOL CServerStatusDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	switch (LOWORD(wParam)) {
	case ID_SERVERSTATUS_SELECTALL:
		mListStatus.SelectAllItems();
		break;
	case ID_SERVERSTATUS_INVERTSELECTION:
		mListStatus.InvertSelection();
		break;
	case ID_SERVERSTATUS_TERMINATE:
		{
			POSITION pos(mListStatus.GetFirstSelectedItemPosition());
			while (pos) {
				int index(mListStatus.GetNextSelectedItem(pos));
				DWORD threadID((DWORD)mListStatus.GetItemData(index));
				if ((int)threadID > 0)
					ThreadManager::GetInstance().TerminateThread(threadID);
			}
		}
		break;
	}
	return __super::OnCommand(wParam, lParam);
}
void CServerStatusDlg::Log(LPCTSTR logMessage)
{
	mLogEditCtrl.Append(logMessage);
}
