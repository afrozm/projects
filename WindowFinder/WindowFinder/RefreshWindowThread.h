#pragma once

#include "WindowIterator.h"
// CRefreshWindowThread

class CRefreshWindowThread : public CWinThread
{
	DECLARE_DYNCREATE(CRefreshWindowThread)

protected:
	CRefreshWindowThread();           // protected constructor used by dynamic creation
	virtual ~CRefreshWindowThread();
    static int Callback_Wi(const CWindowEntry& /*we*/);
    int Callback_WinIter(const CWindowEntry& /*we*/);


    static CRefreshWindowThread *m_pThreadInstance;

    DWORD m_iLastRefreshTime;
    CWindowIterator m_TopWindowIterator, m_WindowIterator;

public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

    void RefreshWindows(HWND hWnd = NULL);
    typedef int(*ThreadedFn)(void*); // this should check IsCancelled periodically and return if true
    void DoThreadedOp(ThreadedFn fn, void *pData);
    static CRefreshWindowThread* GetInstance(bool bCreateIfNotExists = true);
    static void Shutdown();
    static void Cancel();
    static bool IsCancelled();
    void CancelRefresh();
    const CArray<CWindowEntry>& GetWindowList(bool bTopLevel) const;

protected:
    afx_msg void OnRefreshWindows(WPARAM wParam, LPARAM lParam);
    afx_msg void OnThreadOp(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};


