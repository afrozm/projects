#pragma once

#include "WindowIterator.h"

// CRefreshWindowThread

class CRefreshWindowThread : public CWinThread
{
	DECLARE_DYNCREATE(CRefreshWindowThread)

protected:
	CRefreshWindowThread();           // protected constructor used by dynamic creation
	virtual ~CRefreshWindowThread();
    static CRefreshWindowThread *m_pThreadInstance;

    DWORD m_iLastRefreshTime;
    CWindowIterator m_TopWindowIterator, m_WindowIterator;

public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

    void RefreshWindows(HWND hWnd = NULL);
    static CRefreshWindowThread* GetInstance();
    static void Shutdown();
    void CancelRefresh();
    const CArray<CWindowEntry>& GetWindowList(bool bTopLevel) const;

protected:
    afx_msg void OnRefreshWindows(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};


