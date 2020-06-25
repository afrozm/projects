// CmdEditCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "Find.h"
#include "CmdEditCtrl.h"


// CCmdEditCtrl

IMPLEMENT_DYNAMIC(CCmdEditCtrl, CEdit)

CCmdEditCtrl::CCmdEditCtrl()
{

}

CCmdEditCtrl::~CCmdEditCtrl()
{
}
void CCmdEditCtrl::AddCommand(const CString &cmd, bool bUpdate)
{
	RemoveCommand(cmd);
	mLines.Add(cmd);
	if (bUpdate)
		UpdateWindowText();
}
void CCmdEditCtrl::RemoveCommand(const CString &cmd, bool bUpdate)
{
    if (bUpdate)
	    GetLines();
	while (1) {
		int iCmd(FindCommand(cmd));
		if (iCmd >= 0)
			mLines.RemoveAt(iCmd);
		else break;
	}
	if (bUpdate)
		UpdateWindowText();
}
void CCmdEditCtrl::Clear()
{
	mLines.RemoveAll();
	UpdateWindowText();
}
int CCmdEditCtrl::FindCommand(const CString &cmd)
{
	// command format - command:path\reexpression
	CString path;
	int pos(cmd.Find(_T(":")));
	if (pos >= 0) {
		path = cmd;
		path.Delete(0, pos+1);
	}
	if (path.IsEmpty())
		return -1;
	for (int i = 0; i < mLines.GetCount(); ++i) {
		if (mLines.GetAt(i).Find(path) >= 0)
			return i;
	}
	return -1;
}

void CCmdEditCtrl::UpdateWindowText()
{
	CString text;
	for (int i = 0; i < mLines.GetCount(); ++i)
		text += mLines.GetAt(i) + _T("\r\n");
	CEdit::SetWindowText(text);
}
void CCmdEditCtrl::SetWindowText(LPCTSTR lpszString)
{
	CEdit::SetWindowText(lpszString);
	GetLines();
}
void CCmdEditCtrl::GetLines()
{
	mLines.RemoveAll();
	CString text;
	CEdit::GetWindowText(text);
	int pos(0);
	while ((pos = text.Find(_T("\n"))) != -1)
	{
		CString line(text.Left(pos-1));
		if (!line.IsEmpty())
			mLines.Add(line);
		text.Delete(0, pos+1);
	};
	if (!text.IsEmpty())
		mLines.Add(text);
}

BEGIN_MESSAGE_MAP(CCmdEditCtrl, CEdit)
END_MESSAGE_MAP()



// CCmdEditCtrl message handlers


