#pragma once
#include "NotificationManager.h"
class NotificationManagerWin :
    public NotificationManager
{
public:
    INT_PTR DialogProc(
        _In_ HWND   hwndDlg,
        _In_ UINT   uMsg,
        _In_ WPARAM wParam,
        _In_ LPARAM lParam
    );
    BOOL EnumWindowsProc(HWND hWnd, LPARAM lParam);
    NotificationManagerWin();
    ~NotificationManagerWin();
    virtual int Initialize() override;
    virtual int Finalize() override;
protected:
    virtual int SendNotification(const std::string &notData) override;
private:
    void WindowThreadProc();
    HWND m_hWndDialog;
    std::thread *m_pWindowThread;
};

