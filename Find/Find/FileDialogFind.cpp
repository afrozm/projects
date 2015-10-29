// FileDialogFind.cpp : implementation file
//

#include "stdafx.h"
#include "Find.h"
#include "FileDialogFind.h"


// CFileDialogFind

IMPLEMENT_DYNAMIC(CFileDialogFind, CFileDialog)

CFileDialogFind::CFileDialogFind(BOOL bOpenFileDialog, LPCTSTR lpszDefExt, LPCTSTR lpszFileName,
		DWORD dwFlags, LPCTSTR lpszFilter, CWnd* pParentWnd) :
		CFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd),
		m_uAppendChecked(0)
{
	OPENFILENAME &ofn = GetOFN();
	ofn.Flags |= OFN_ENABLETEMPLATE;
	ofn.lpTemplateName = MAKEINTRESOURCE(IDD_DIALOG_APPEND);
}

CFileDialogFind::~CFileDialogFind()
{
}


BEGIN_MESSAGE_MAP(CFileDialogFind, CFileDialog)
END_MESSAGE_MAP()



// CFileDialogFind message handlers

void CFileDialogFind::OnInitDone()
{
	CFileDialog::OnInitDone();
	HWND hdlg = m_hWnd;
	HWND hParent = ::GetParent(hdlg);
	RECT rect;
	::GetWindowRect(::GetDlgItem(hParent, lst1), &rect);
	::MapWindowPoints(HWND_DESKTOP, hParent, (LPPOINT)&rect, 2);
	int left = rect.left;
	::GetWindowRect(hdlg, &rect);
	::MapWindowPoints(HWND_DESKTOP, hParent, (LPPOINT)&rect, 2);
	left = ::SetWindowPos(hdlg, NULL, rect.left+left, rect.top, 0, 0, SWP_NOSIZE|SWP_NOZORDER);
}

BOOL CFileDialogFind::OnFileNameOK()
{
	m_uAppendChecked = IsDlgButtonChecked(IDC_CHECK_APPEND);
	return CFileDialog::OnFileNameOK();
}