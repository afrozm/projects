#pragma once

#include "ThreadManager.h"
#include "CombinedButton.h"
#include "ControlResizer.h"
#include "ResizeBar.h"
#include "LoggerFactory.h"
#include "ListCtrlUtil.h"
#include "LogEditCtrl.h"

Logger& GetServerLogger();

// Thread op
typedef enum {
    SERVER_THREAD_OP_START_SEARCH,
	SERVER_THREAD_OP_START_DBCOMMITER,
	SERVER_THREAD_OP_SEARCH_IN_NETWORK,
	SERVER_THREAD_OP_VERIFY,
	SERVER_THREAD_OP_IPENUM,
    SERVER_THREAD_OP_LOAD_DEFAULT,
    SERVER_THREAD_OP_EXECUTE,
    SERVER_THREAD_OP_STATUS // status of all thread in status window
} ServerThreadOperation;

// CServerStatusDlg dialog
class CServerStatusDlg;

class CServerStatusThreadNotifier : public ThreadNotifier {
public:
	CServerStatusThreadNotifier(CServerStatusDlg *pDlg);

	virtual void Notify( ThreadNotification notifiation, DWORD dwThreadID, LPVOID notifyData = NULL );

private:
	CServerStatusDlg *m_pDlg;
};
class CServerStatusContext : public CCombinedButtonContext {
public:
	CServerStatusContext(CServerStatusDlg *pDlg);
	BOOL IsWindowVisible();
	void ShowWindow(int cmdShow = SW_SHOW);
private:
	CServerStatusDlg *m_pDlg;
};

class LogTargetServerStatus : public LogTarget {
public:
	LogTargetServerStatus(CServerStatusDlg *pDlg);
	void Log(LPCTSTR logMessage);
	void LogSimple(LPCTSTR logMessage);
private:
	CServerStatusDlg *m_pDlg;
};

class CServerStatusDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CServerStatusDlg)

public:
	CServerStatusDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CServerStatusDlg();
	void Init();
// Dialog Data
	enum { IDD = IDD_DIALOG_SERVER_STATUS };
	void Notify(ThreadNotifier::ThreadNotification notifiation, DWORD threadID, LPVOID notifyData = NULL);
	void SetStatus(LPCTSTR statusMsg);
	void Log(LPCTSTR logMessage);
	CCombinedButtonContext* GetContext() {return &mContext;}
	void UpdateTime();
	int DoThreadOperation(LPVOID pThreadData);
	void WaitForFinish();
	void SetStatusMessage(LPCTSTR fmt = NULL, ...);
	BOOL PreTranslateMessage(MSG* pMsg);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	int GetIndex(DWORD threadID);
	void SetText(DWORD threadID, int col, LPCTSTR text);
	DWORD StartThreadOperation(ServerThreadOperation op, LPVOID threadData);
	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint pos);
	BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	CServerStatusThreadNotifier mThreadNotifier;
	CServerStatusContext mContext;
	CControlResizer mControlResizer;
	CResizeBar *mResizeBar;
	CMutex mLock;
	LogTargetServerStatus mServerStatus;
	CListCtrlUtil mListStatus;
	DWORD m_dwStatusThread;
	bool mbStatusThreadTerminate;
	HACCEL m_hAccel;
	CLogEditCtrl mLogEditCtrl;
};
