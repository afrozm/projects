// RefreshWindowThread.cpp : implementation file
//

#include "stdafx.h"
#include "WindowFinder.h"
#include "RefreshWindowThread.h"

int CRefreshWindowThread::Callback_Wi(const CWindowEntry& we)
{
    if (GetInstance(false))
        return GetInstance(false)->Callback_WinIter(we);
    return 1;
}

int CRefreshWindowThread::Callback_WinIter(const CWindowEntry& /*we*/)
{
    // return 0 to continue otherwise 1 to cancel
    // Cancel iteration if time is zero
    return m_iLastRefreshTime > 0 ? 0 : 1;
}

// CRefreshWindowThread
#define WM_RWT_REFRESH_WINDOWS WM_USER+0x348
#define WM_RWT_THREAD_OP WM_USER+0x349

CRefreshWindowThread* CRefreshWindowThread::m_pThreadInstance = NULL;

IMPLEMENT_DYNCREATE(CRefreshWindowThread, CWinThread)

CRefreshWindowThread::CRefreshWindowThread()
    : m_iLastRefreshTime(0), m_TopWindowIterator(Callback_Wi), m_WindowIterator(Callback_Wi)
{
}

CRefreshWindowThread::~CRefreshWindowThread()
{
}

BOOL CRefreshWindowThread::InitInstance()
{
    AfxOleInit();
	return TRUE;
}

int CRefreshWindowThread::ExitInstance()
{
    m_pThreadInstance = NULL;
    AfxOleTerm();
	return CWinThread::ExitInstance();
}

CRefreshWindowThread* CRefreshWindowThread::GetInstance(bool bCreateIfNotExists /* = true */)
{
    if (m_pThreadInstance == NULL && bCreateIfNotExists)
        m_pThreadInstance = (CRefreshWindowThread*)AfxBeginThread(RUNTIME_CLASS(CRefreshWindowThread));
    return m_pThreadInstance;
}

void CRefreshWindowThread::Shutdown()
{
    if (m_pThreadInstance) {
        Cancel();
        HANDLE hThread(m_pThreadInstance->m_hThread);
        m_pThreadInstance->PostThreadMessage(WM_QUIT, 0, 0);
        WaitForSingleObject(hThread, 5000);
    }
}

void CRefreshWindowThread::Cancel()
{
    if (m_pThreadInstance)
        m_pThreadInstance->CancelRefresh();
}

bool CRefreshWindowThread::IsCancelled()
{
    return m_pThreadInstance == NULL ? true : m_pThreadInstance->m_iLastRefreshTime == 0;
}

void CRefreshWindowThread::CancelRefresh()
{
    // set time to zero to cancel the window iteration
    m_iLastRefreshTime = 0;
}

const CArray<CWindowEntry>& CRefreshWindowThread::GetWindowList(bool bTopLevel) const
{
    return bTopLevel ? m_TopWindowIterator.GetWindowList() : m_WindowIterator.GetWindowList();
}

void CRefreshWindowThread::OnRefreshWindows(WPARAM wParam, LPARAM /*lParam*/)
{
    if (!wParam && (GetTickCount() - m_iLastRefreshTime < 5000))
        return;
    m_iLastRefreshTime = GetTickCount();
    CWindowIterator *pIter(wParam ? &m_WindowIterator : &m_TopWindowIterator);
    pIter->Iterate((HWND)wParam);
    if (!IsCancelled())
        AfxGetMainWnd()->PostMessage(WM_WINDOW_ITER_OP, 0, wParam);
    m_iLastRefreshTime = GetTickCount();
}

void CRefreshWindowThread::OnThreadOp(WPARAM wParam, LPARAM lParam)
{
    m_iLastRefreshTime = GetTickCount();
    ThreadedFn fn((ThreadedFn)wParam);
    fn((void*)lParam);
    m_iLastRefreshTime = GetTickCount();
}


BEGIN_MESSAGE_MAP(CRefreshWindowThread, CWinThread)
    ON_THREAD_MESSAGE(WM_RWT_REFRESH_WINDOWS, OnRefreshWindows)
    ON_THREAD_MESSAGE(WM_RWT_THREAD_OP, OnThreadOp)
END_MESSAGE_MAP()


// CRefreshWindowThread message handlers

void CRefreshWindowThread::RefreshWindows(HWND hWnd /* = NULL */)
{
    CWindowIterator *pIter(hWnd ? &m_WindowIterator : &m_TopWindowIterator);
    if (!pIter->IsSearching())
        PostThreadMessage(WM_RWT_REFRESH_WINDOWS, (WPARAM)hWnd, 0);
}

void CRefreshWindowThread::DoThreadedOp(ThreadedFn fn, void *pData)
{
    if (fn)
        PostThreadMessage(WM_RWT_THREAD_OP, (WPARAM)fn, (LPARAM)pData);
}

