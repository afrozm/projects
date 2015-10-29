// Find.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "Find.h"
#include "FindDlg.h"
#include "FindServerDlg.h"
#include "NamedLock.h"
#include "FindDataBase.h"
#include <tlhelp32.h>
#include <Psapi.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CFindApp

BEGIN_MESSAGE_MAP(CFindApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CFindApp construction
static void DefaultLogMessageHandler(const wchar_t *msg)
{
	// Add different log messages handling here.
#ifdef _DEBUG
	OutputDebugString(msg);
#else
    UNREFERENCED_PARAMETER(msg);
#endif
}

CFindApp::CFindApp()
: mbIsServer(false)
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
	SystemUtils::SetLogMessageHandler(DefaultLogMessageHandler);
}


// The one and only CFindApp object

CFindApp theApp;

static CString FindArg(LPCTSTR argName) {
	CString arg;
	int nArgs(0);
	LPWSTR *szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
	if (szArglist != NULL) {
		int len(lstrlen(argName));
		for(int i=0; i<nArgs; i++) {
			if (!_tcsncicmp(szArglist[i], argName, len)) {
				arg = szArglist[i];
				break;
			}
		}
		LocalFree(szArglist);
	}
	return arg;
}

static bool IsServer()
{
	CString serverArg(FindArg(_T("-s")));
	if (serverArg.IsEmpty())
		serverArg = FindArg(_T("--server"));
	return !serverArg.IsEmpty();
}
struct EnumWndData {
	HWND hWnd;
	DWORD processID;
};
BOOL CALLBACK EnumTopLEvelWindows(HWND hWnd, LPARAM lParam)
{
	EnumWndData *ewd((EnumWndData *)lParam);
	DWORD procID(0);
	GetWindowThreadProcessId(hWnd, &procID);
	if (procID == ewd->processID) {
		hWnd = GetWindow(hWnd, GW_OWNER);
		if (hWnd != NULL) {
			ewd->hWnd = hWnd;
			return FALSE;
		}
	}
	return TRUE;
}
static HWND GetPtocessWindow( DWORD dwOwnerPID ) 
{
	EnumWndData ewd = {0, dwOwnerPID};
	EnumWindows((WNDENUMPROC)EnumTopLEvelWindows, (LPARAM)&ewd);
	return ewd.hWnd;
}

BOOL IsWindowServer(HWND hWnd)
{
	return hWnd && SendMessage(hWnd, WM_IS_SERVER, 0, 0);
}
#include <list>
typedef std::list<DWORD> ListDWORD;
static ListDWORD GetProcessIDFromModule(LPCTSTR processPath)
{
	HANDLE hProcessSnap;
	PROCESSENTRY32 pe32;
	bool bPathIsRelative(PathIsRelative(processPath) == TRUE);
	ListDWORD retProcessID;

	// Take a snapshot of all processes in the system.
	hProcessSnap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
	if( hProcessSnap == INVALID_HANDLE_VALUE )
	{
		return retProcessID;
	}

	// Set the size of the structure before using it.
	pe32.dwSize = sizeof( PROCESSENTRY32 );

	// Retrieve information about the first process,
	// and exit if unsuccessful
	if( !Process32First( hProcessSnap, &pe32 ) )
	{
		CloseHandle( hProcessSnap );    // Must clean up the
		//   snapshot object!
		return retProcessID;
	}

	// Now walk the snapshot of processes, and
	// display information about each process in turn
	do
	{
		HANDLE hProcess = OpenProcess(
			PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, FALSE, pe32.th32ProcessID );
		if (hProcess) {
			TCHAR processName[MAX_PATH] = {0};
			LPCTSTR procName(processName);
			//GetProcessImageFileName(hProcess, processName, MAX_PATH);
			GetModuleFileNameEx(hProcess, NULL, processName, MAX_PATH);
			if (bPathIsRelative)
				procName = PathFindFileName(processName);
			if (!lstrcmpi(procName, processPath)) {
				retProcessID.push_back(pe32.th32ProcessID);
			}
			CloseHandle(hProcess);
		}
	} while( Process32Next( hProcessSnap, &pe32 ) );

	CloseHandle( hProcessSnap );
	return retProcessID;
}
bool CFindApp::IsServer()
{
	return theApp.mbIsServer;
}
// CFindApp initialization

BOOL CFindApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	AfxEnableControlContainer();
	OleInitialize(NULL);
	AfxInitRichEdit2();
	//CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
//	AfxOleInit();
//	AfxOleGetMessageFilter()->EnableBusyDialog(FALSE);
//	AfxOleGetMessageFilter()->EnableNotRespondingDialog(FALSE);
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));
	if (::IsServer()) {
		mbIsServer = true;
		CNamedLock namedLock(_T("Global\\FindServer"));
		if (namedLock.IsLocked()) {
			{
				FindDataBase fdbServer(FDB_CacheDatabase);
				fdbServer.LoadSchema();
			}
			// Set ServerPath to current exe
			{
				TCHAR moduleName[1024];
				GetModuleFileName(NULL, moduleName, sizeof(moduleName)/sizeof(moduleName[0]));
				FindDataBase::SSetProperty(_T("ServerPath"), moduleName);
			}
			// Invoke server dialog
			CFindServerDlg dlg;
			m_pMainWnd = &dlg;
			dlg.DoModal();
		}
		else { // Already locked
			// Popup already running server program
			CString serverPath(FindDataBase::SGetProperty(_T("ServerPath")));
			ListDWORD serverProcessID(GetProcessIDFromModule(serverPath));
			for (ListDWORD::const_iterator citPID = serverProcessID.begin();
				citPID != serverProcessID.end(); ++citPID) {
				HWND hWnd(GetPtocessWindow(*citPID));
				if (IsWindowServer(hWnd)) {
					AllowSetForegroundWindow(*citPID);
					SendMessage(hWnd, WM_USER+3000, 0, WM_LBUTTONDOWN);
				}
			}
		}
	}
	else {
		try {
			CFindDlg dlg;
			m_pMainWnd = &dlg;
			INT_PTR nResponse = dlg.DoModal();
			if (nResponse == IDOK)
			{
				// TODO: Place code here to handle when the dialog is
				//  dismissed with OK
			}
			else if (nResponse == IDCANCEL)
			{
				// TODO: Place code here to handle when the dialog is
				//  dismissed with Cancel
			}
		}
		catch(...) {
			OutputDebugString(_T("Exception!!!\r\n"));
		}
	}
	OleUninitialize();
	//CoUninitialize();
	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
