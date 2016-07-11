#include "stdafx.h"
#include "NotificationManagerWin.h"
#include "resource.h"

NotificationManagerWin::NotificationManagerWin()
    : m_hWndDialog(NULL), m_pWindowThread(NULL)
{
}

static INT_PTR CALLBACK NotificationManager_DialogProc(
    _In_ HWND   hwndDlg,
    _In_ UINT   uMsg,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam
)
{
    return ((NotificationManagerWin&)NotificationManager::GetInstance()).DialogProc(hwndDlg, uMsg, wParam, lParam);
}
#define WM_NM_HANDLE_NOTIFCATION WM_USER+0x147
#define WM_NM_HANDLE_NOTIFCATION_UPDATE WM_USER+0x148
INT_PTR NotificationManagerWin::DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_INITDIALOG:
        m_hWndDialog = hwndDlg;
        ShowWindow(hwndDlg, SW_HIDE);
        SetWindowText(hwndDlg, _T("NotificationWindowHelper"));
        UpdateRegisteredWnds();
        PostMyWndToOther(TRUE);
        break;
    case WM_COPYDATA:
    {
        COPYDATASTRUCT *pcs((COPYDATASTRUCT*)lParam);
        if (NULL == pcs)
            break;
        NotificationData notData(new Paramters(std::string((const char *)pcs->lpData, pcs->cbData)));
        {
            std::lock_guard<std::mutex> lk(mMutextQueue);
            mQueuedData.push_back(notData);
        }
        mcvQueue.notify_one();
        SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, TRUE);
        return TRUE;
    }
    break;
    case WM_NM_HANDLE_NOTIFCATION:
        SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, TRUE);
        return TRUE;
    case WM_NM_HANDLE_NOTIFCATION_UPDATE:
        EnumWindowsProc((HWND)wParam, lParam);
        SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, TRUE);
        return TRUE;

    case WM_DESTROY:
        PostMyWndToOther(FALSE);
        break;
    default:
        break;
    }
    return FALSE;
}
static BOOL CALLBACK NotificationManager_EnumWindowsProc(
    _In_ HWND   hwnd,
    _In_ LPARAM lParam
)
{
    return ((NotificationManagerWin&)NotificationManager::GetInstance()).EnumWindowsProc(hwnd, lParam);
}

static BOOL IsNotificationWindow(HWND hWnd)
{
    TCHAR buffer[MAX_PATH];
    buffer[0] = 0;
    return GetClassName(hWnd, buffer, _countof(buffer)) > 0
        && !lstrcmpi(buffer, _T("#32770"))                  // Dialog class
        && GetWindowText(hWnd, buffer, _countof(buffer))
        && !lstrcmpi(buffer, _T("NotificationWindowHelper"))     // Notification window
        && SendMessage(hWnd, WM_NM_HANDLE_NOTIFCATION, 0, 0);  // Handles notifications
}

BOOL NotificationManagerWin::EnumWindowsProc(HWND hWnd, LPARAM lParam)
{
    if (lParam) { // Add
        if (hWnd == m_hWndDialog
            || mRegisteredWnds.find(hWnd) == mRegisteredWnds.end()
            && IsNotificationWindow(hWnd)) { // Add notification window to list
            std::lock_guard<std::mutex> guard(mLock);
            mRegisteredWnds.insert(hWnd);
        }
    }
    else if (hWnd != m_hWndDialog) { // Remove
        std::lock_guard<std::mutex> guard(mLock);
        mRegisteredWnds.erase(hWnd);
    }
    return TRUE;
}

int NotificationManagerWin::SendNotification(const std::string &notData)
{
    SetHWND listHwnd;
    GetRegWindows(listHwnd);
    for (const auto &hWnd : listHwnd) {
        COPYDATASTRUCT cpData = { 1, (DWORD)notData.length() * sizeof(char), (PVOID)notData.c_str() };
        SendMessage(hWnd, WM_COPYDATA, (WPARAM)m_hWndDialog, (LPARAM)&cpData);
    }
    return 0;
}


void NotificationManagerWin::WindowThreadProc()
{
    extern HMODULE ghModule;
    CreateDialog(ghModule, MAKEINTRESOURCE(IDD_DIALOG_NOTIFICATION), NULL, NotificationManager_DialogProc);
    MSG msg = { 0 };
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    DestroyWindow(m_hWndDialog);
    m_hWndDialog = NULL;
}

void NotificationManagerWin::UpdateRegisteredWnds()
{
    {
        std::lock_guard<std::mutex> guard(mLock);
        mRegisteredWnds.clear();
    }
    EnumWindows(NotificationManager_EnumWindowsProc, TRUE);
}

void NotificationManagerWin::PostMyWndToOther(BOOL bAdd)
{
    SetHWND listHwnd;
    GetRegWindows(listHwnd);
    for (const auto &hWnd : listHwnd)
        PostMessage(hWnd, WM_NM_HANDLE_NOTIFCATION_UPDATE, (WPARAM)m_hWndDialog, bAdd);
}

void NotificationManagerWin::GetRegWindows(SetHWND &outRegWindows)
{
    std::lock_guard<std::mutex> guard(mLock);
    outRegWindows = mRegisteredWnds;
}

NotificationManagerWin::~NotificationManagerWin()
{
}

int NotificationManagerWin::Initialize()
{
    if (NULL == m_pWindowThread) {
        m_pWindowThread = new std::thread(&NotificationManagerWin::WindowThreadProc, this);
        int retCount(10);
        // Wait a nit to get dialog loaded
        while (--retCount > 0 && m_hWndDialog == NULL)
            Sleep(10);
    }
    return __super::Initialize();
}

int NotificationManagerWin::Finalize()
{
    if (NULL != m_hWndDialog)
        PostThreadMessage(GetWindowThreadProcessId(m_hWndDialog, NULL), WM_QUIT, 0, 0);
    if (NULL != m_pWindowThread) {
        m_pWindowThread->join();
        delete m_pWindowThread;
        m_pWindowThread = NULL;
    }
    return __super::Finalize();
}

