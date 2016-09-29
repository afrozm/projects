// RefreshWindowThread.cpp : implementation file
//

#include "stdafx.h"
#include "WindowFinder.h"
#include "RefreshWindowThread.h"


// CRefreshWindowThread
#define WM_RWT_REFRESH_WINDOWS WM_USER+0x348

CRefreshWindowThread* CRefreshWindowThread::m_pThreadInstance = NULL;

IMPLEMENT_DYNCREATE(CRefreshWindowThread, CWinThread)

CRefreshWindowThread::CRefreshWindowThread()
    : m_iLastRefreshTime(0)
{
}

CRefreshWindowThread::~CRefreshWindowThread()
{
}

BOOL CRefreshWindowThread::InitInstance()
{
	return TRUE;
}

int CRefreshWindowThread::ExitInstance()
{
    m_pThreadInstance = NULL;
	return CWinThread::ExitInstance();
}

CRefreshWindowThread* CRefreshWindowThread::GetInstance()
{
    if (m_pThreadInstance == NULL)
        m_pThreadInstance = (CRefreshWindowThread*)AfxBeginThread(RUNTIME_CLASS(CRefreshWindowThread));
    return m_pThreadInstance;
}

void CRefreshWindowThread::Shutdown()
{
    if (m_pThreadInstance) {
        HANDLE hThread(m_pThreadInstance->m_hThread);
        m_pThreadInstance->CancelRefresh();
        m_pThreadInstance->PostThreadMessage(WM_QUIT, 0, 0);
        WaitForSingleObject(hThread, 5000);
    }
}

void CRefreshWindowThread::CancelRefresh()
{
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
    m_iLastRefreshTime = GetTickCount();
    AfxGetMainWnd()->PostMessage(WM_WINDOW_ITER_OP, 0, wParam);
}


BEGIN_MESSAGE_MAP(CRefreshWindowThread, CWinThread)
    ON_THREAD_MESSAGE(WM_RWT_REFRESH_WINDOWS, OnRefreshWindows)
END_MESSAGE_MAP()


// CRefreshWindowThread message handlers

void CRefreshWindowThread::RefreshWindows(HWND hWnd /* = NULL */)
{
    CWindowIterator *pIter(hWnd ? &m_WindowIterator : &m_TopWindowIterator);
    if (!pIter->IsSearching())
        PostThreadMessage(WM_RWT_REFRESH_WINDOWS, (WPARAM)hWnd, 0);
}

