// FindReplaceDialogEx.cpp : implementation file
//

#include "stdafx.h"
#include "Find.h"
#include "FindReplaceDialogEx.h"
#include "SaveListResultCtrl.h"

// CFindReplaceDialogEx

IMPLEMENT_DYNAMIC(CFindReplaceDialogEx, CFindReplaceDialog)

CFindReplaceDialogEx::CFindReplaceDialogEx(CSaveListResultCtrl *pList, FindStringType currentStringType)
: m_pList(pList), mCurrentStringType(currentStringType)
{

}

CFindReplaceDialogEx::~CFindReplaceDialogEx()
{
}


BEGIN_MESSAGE_MAP(CFindReplaceDialogEx, CFindReplaceDialog)
END_MESSAGE_MAP()

// These IDs were found through spy++.
#define IDC_FIND_CHECK_MATCH_WHOLE_WORD 0x410
#define IDC_FIND_CHECK_MATCH_CASE 0x411
#define IDC_FIND_EDIT_FIND 0x480
#define IDC_FIND_SELECTALL 0x4000
#define IDC_FIND_CBX_STRTYPE 0x4001

// CFindReplaceDialogEx message handlers

void SetClearFlag(DWORD &flag, UINT uFlagVal, BOOL bSet = TRUE)
{
	if (bSet)
		flag |= uFlagVal;
	else
		flag &= ~uFlagVal;
}

BOOL CFindReplaceDialogEx::Create(BOOL bFindDialogOnly, // TRUE for Find, FALSE for FindReplace
			LPCTSTR lpszFindWhat,
			LPCTSTR lpszReplaceWith,
			DWORD dwFlags,
			CWnd* pParentWnd)
{
	m_fr.wFindWhatLen = lpszFindWhat ? ((WORD)lstrlen(lpszFindWhat)+1) : 256;
	m_fr.lpstrFindWhat = new TCHAR[m_fr.wFindWhatLen];
	if (lpszFindWhat)
		StringCchCopy(m_fr.lpstrFindWhat, m_fr.wFindWhatLen, lpszFindWhat);
	else
		m_fr.lpstrFindWhat[0] = 0;
	BOOL bRet = CFindReplaceDialog::Create(bFindDialogOnly, lpszFindWhat,
		lpszReplaceWith, dwFlags, pParentWnd);
	return bRet;
}

LRESULT CFindReplaceDialogEx::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_INITDIALOG:
		{
			SetDlgItemText(IDC_FIND_CHECK_MATCH_WHOLE_WORD, _T("&Match in full path"));
			RECT rc;
			GetClientRect(&rc);
			rc.bottom += SystemUtils::GetTranslatedDPIPixelY(50);
			SetWindowPos(NULL, 0, 0, rc.right-rc.left, rc.bottom-rc.top, SWP_NOMOVE|SWP_NOZORDER);
			GetClientRect(&rc);
			rc.bottom -= SystemUtils::GetTranslatedDPIPixelY(5);
			rc.top = rc.bottom - SystemUtils::GetTranslatedDPIPixelY(22);
			//rect.right -= 1;
			rc.left += SystemUtils::GetTranslatedDPIPixelX(6);
			rc.right = rc.left + SystemUtils::GetTranslatedDPIPixelX(120);
			HWND hWnd = ::CreateWindow(_T("COMBOBOX"), _T("Plain Text"), WS_CHILD| WS_VISIBLE|WS_TABSTOP|CBS_DROPDOWNLIST,
				rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, m_hWnd, (HMENU)IDC_FIND_CBX_STRTYPE,
				NULL, NULL);
			::SendMessage(hWnd, WM_SETFONT, (WPARAM)SendMessage(WM_GETFONT), FALSE);
			::SendMessage(hWnd, CB_ADDSTRING, 0, (LPARAM)_T("Plain Text"));
			::SendMessage(hWnd, CB_ADDSTRING, 0, (LPARAM)_T("Wild Card"));
			::SendMessage(hWnd, CB_ADDSTRING, 0, (LPARAM)_T("Regular Expression"));
			::SendMessage(hWnd, CB_SETCURSEL, mCurrentStringType, 0);
			GetClientRect(&rc);
			rc.bottom -= SystemUtils::GetTranslatedDPIPixelY(5);
			rc.top = rc.bottom - SystemUtils::GetTranslatedDPIPixelY(22);
			//rect.right -= 1;
			rc.left = rc.right - SystemUtils::GetTranslatedDPIPixelX(120);
			hWnd = ::CreateWindow(_T("Button"), _T("&Select all matches"), WS_CHILD| WS_VISIBLE|WS_TABSTOP,
				rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, m_hWnd, (HMENU)IDC_FIND_SELECTALL,
				NULL, NULL);
			::SendMessage(hWnd, WM_SETFONT, (WPARAM)SendMessage(WM_GETFONT), FALSE);
			if (hWnd == NULL)
				hWnd = NULL;
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_FIND_SELECTALL:
			{
				CString str;
				GetDlgItemText(IDC_FIND_EDIT_FIND, str);
				StringCchCopy(m_fr.lpstrFindWhat, m_fr.wFindWhatLen, str);
			}
			SetClearFlag(m_fr.Flags, FR_MATCHCASE, IsDlgButtonChecked(IDC_FIND_CHECK_MATCH_CASE));
			SetClearFlag(m_fr.Flags, FR_WHOLEWORD, IsDlgButtonChecked(IDC_FIND_CHECK_MATCH_WHOLE_WORD));
			m_pList->FindNext(true, true);
			break;
		case IDC_FIND_EDIT_FIND:
			switch (HIWORD(wParam)) {
			case EN_CHANGE:
				{
					CString str;
					GetDlgItemText(IDC_FIND_EDIT_FIND, str);
					CWnd *pSelectAllButton = GetDlgItem(IDC_FIND_SELECTALL);
					if (pSelectAllButton != NULL)
						pSelectAllButton->EnableWindow(!str.IsEmpty());
				}
				break;
			}
		break;
		}
		break;
	case WM_DESTROY:
		if (m_fr.lpstrFindWhat)
			delete m_fr.lpstrFindWhat;
		m_fr.lpstrFindWhat = NULL;
		break;
	}
	return CFindReplaceDialog::WindowProc(message, wParam, lParam);
}

BOOL CFindReplaceDialogEx::OnInitDialog()
{
	BOOL bRet = CFindReplaceDialog::OnInitDialog();
	return bRet;
}
CFindReplaceDialogEx::FindStringType CFindReplaceDialogEx::GetFindStringType()
{
	return (CFindReplaceDialogEx::FindStringType)((CComboBox*)GetDlgItem(IDC_FIND_CBX_STRTYPE))->GetCurSel();
}
