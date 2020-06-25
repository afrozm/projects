// EditFindCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "LogEditCtrl.h"

// CLogEditCtrl

IMPLEMENT_DYNAMIC(CLogEditCtrl, CEditFindCtrl)

CLogEditCtrl::CLogEditCtrl()
{

}

CLogEditCtrl::~CLogEditCtrl()
{
}

void CLogEditCtrl::Append(const CString& inStr)
{
	CString curText;
	GetWindowText(curText);
	int lenExceeding(curText.GetLength() > 100*1024*1024); // 100MB limit
	if (lenExceeding > 0) 
		curText.Delete(0, lenExceeding);
	curText += inStr;
	SetWindowText(curText);
	SendMessage(WM_VSCROLL, SB_BOTTOM, 0);
}

BEGIN_MESSAGE_MAP(CLogEditCtrl, CEditFindCtrl)
END_MESSAGE_MAP()



// CLogEditCtrl message handlers
