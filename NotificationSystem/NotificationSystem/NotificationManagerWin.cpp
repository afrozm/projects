#include "stdafx.h"
#include "NotificationManagerWin.h"
#include "resource.h"
#include "Property.h"

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
INT_PTR NotificationManagerWin::DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_INITDIALOG:
        ShowWindow(hwndDlg, SW_HIDE);
        SetWindowText(hwndDlg, _T("NotificationWindowHelper"));
        break;
    case WM_COPYDATA:
    {
        COPYDATASTRUCT *pcs((COPYDATASTRUCT*)lParam);
        if (NULL == pcs)
            break;
        PropertySetStreamer pss;
        PropertySet ps;
        pss.SetPropertySetStream(ps);
        pss.ReadFromString(lstring((LPCTSTR)pcs->lpData, pcs->cbData));
        NotificationData notData(CreateNotificationData());
        *((Property*)notData) = ps.GetProperty(_T("data"));
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
BOOL NotificationManagerWin::EnumWindowsProc(HWND hWnd, LPARAM lParam)
{
    TCHAR buffer[MAX_PATH];
    buffer[0] = 0;
    if (GetClassName(hWnd, buffer, _countof(buffer)) > 0
        && !lstrcmpi(buffer, _T("#32770"))                  // Dialog class
        && GetWindowText(hWnd, buffer, _countof(buffer))
        && !lstrcmpi(buffer, _T("NotificationWindowHelper"))) {    // Notification window
        if (SendMessage(hWnd, WM_NM_HANDLE_NOTIFCATION, 0, 0)) { // Handle notification
            lstring *pData((lstring*)lParam);
            COPYDATASTRUCT cpData = { 1, (DWORD)pData->length() * sizeof(TCHAR), (PVOID)pData->c_str() };
            SendMessage(hWnd, WM_COPYDATA, (WPARAM)m_hWndDialog, (LPARAM)&cpData);
        }
    }
    return TRUE;
}

int NotificationManagerWin::SendNotification(const lstring &notData)
{
    EnumWindows(NotificationManager_EnumWindowsProc, (LPARAM)&notData);
    return 0;
}


void NotificationManagerWin::WindowThreadProc()
{
    extern HMODULE ghModule;
    m_hWndDialog = CreateDialog(ghModule, MAKEINTRESOURCE(IDD_DIALOG_NOTIFICATION), NULL, NotificationManager_DialogProc);
    MSG msg = { 0 };
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    DestroyWindow(m_hWndDialog);
    m_hWndDialog = NULL;
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

