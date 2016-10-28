#include "stdafx.h"
#include "WindowIterator.h"
#include "WindowFinderDlg.h"
#include "ProcessUtil.h"
#include "Path.h"
//////////////////////////// CWindowEntry //////////////////////////////

CWindowEntry::CWindowEntry(HWND hWnd /*= NULL*/)
    : m_hWnd(hWnd)
{

}

const CString& CWindowEntry::GetDesc(Desc desc /* = Short */) const
{
    switch (desc)
    {
    case CWindowEntry::Short:
        return mDesc;
    case CWindowEntry::Long:
        return mDescLong;
    default:
        break;
    }
    return mDesc;
}

CWindowEntry::operator HWND() const
{
    return m_hWnd;
}

static CString GetProcessNameFromWindow(HWND hWnd)
{
    DWORD pid(0);
    GetWindowThreadProcessId(hWnd, &pid);
    TCHAR processImageName[1024] = { 0 };
    ProcessUtil::GetProcessExePath(pid, processImageName, _countof(processImageName));
    return processImageName;
}

void CWindowEntry::UpdateDesc(LPCTSTR procName /*= NULL*/)
{
    if (mDesc.IsEmpty()) {
        extern CWindowFinderDlg *theMainDlg;
        mDesc.Format(_T("0x%x|"), m_hWnd);
        bool bIsHanged(false);
        CString text;
        text += theMainDlg->getWindowText(m_hWnd, bIsHanged, 512);
        mDesc += text.Left(32);
        mDescLong += text;
        TCHAR className[256] = { 0 };
        GetClassName(m_hWnd, className, _countof(className));
        if (className[0]) {
            text = className;
            mDesc += CString(_T("|")) + text.Left(32);
            mDescLong += _T("|") + text;
        }
        if (procName)
            text = procName;
        else
            text = GetProcessNameFromWindow(m_hWnd);
        if (!text.IsEmpty()) {
            mDesc += CString(_T("|")) + Path((LPCTSTR)text).FileNameWithoutExt();
            mDescLong += _T("|") + text;
        }
        mDesc += CString(_T("|")) +
            ((GetWindowLong(m_hWnd, GWL_STYLE)&WS_VISIBLE)
            ? _T("visible") : _T("hidden"));
        mDescLong = mDesc + _T("|") + mDescLong;
        mDescLong.MakeLower();
    }
}

//////////////////////////// CWindowIterator //////////////////////////////

CWindowIterator::CWindowIterator(Callback callback /* = NULL */)
    : mCalllback(callback), m_uFlags(0), m_hWnd(NULL)
{
}


CWindowIterator::~CWindowIterator()
{
}

void CWindowIterator::SetCallback(Callback callback /*= nullptr*/)
{
    mCalllback = callback;
}

static BOOL CALLBACK EnumChildProc_WindowIterator(
    _In_ HWND   hwnd,
    _In_ LPARAM lParam
)
{
    return ((CWindowIterator*)lParam)->Callback_Iterator(hwnd) == 0;
}

int CWindowIterator::Iterate(HWND hWnd)
{
    if (IsSearching())
        return 1;
    m_hWnd = hWnd;
    SetSearching();
    mProcessName.Empty();
    mWindows.RemoveAll();
    if (hWnd)
        mProcessName = _T(""); //GetProcessNameFromWindow(hWnd);
    EnumChildWindows(hWnd, EnumChildProc_WindowIterator, (LPARAM)this);
    SetSearching(false);
    return 0;
}

int CWindowIterator::Callback_Iterator(HWND hWnd)
{
    if (m_hWnd && GetParent(hWnd) != m_hWnd)
        return 0;
    CWindowEntry &we(mWindows.GetAt(mWindows.Add(hWnd)));
    we.UpdateDesc(mProcessName);
    return mCalllback ? mCalllback(we) : 0;
}

const CArray<CWindowEntry>& CWindowIterator::GetWindowList() const
{
    static const CArray<CWindowEntry> sEmptyList;
    return IsSearching() ? sEmptyList : mWindows;
}

