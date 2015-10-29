// EditFindCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "Find.h"
#include "EditFindCtrl.h"
#include "StdUtils.h"

// CEditFindCtrl

IMPLEMENT_DYNAMIC(CEditFindCtrl, CRichEditCtrl)

CEditFindCtrl::CEditFindCtrl()
{

}

CEditFindCtrl::~CEditFindCtrl()
{
}


BEGIN_MESSAGE_MAP(CEditFindCtrl, CRichEditCtrl)
	ON_WM_CHAR()
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()



// CEditFindCtrl message handlers
static bool AnyModifierPresed()
{
	return GetAsyncKeyState(VK_CONTROL) || GetAsyncKeyState(VK_SHIFT) || GetAsyncKeyState(VK_MENU);
}

void CEditFindCtrl::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	bool bFound(false);
	if (nChar >= 20) {
		TCHAR str[] = {(TCHAR)nChar, 0};
		mStringToFind += str;
		bFound = FindText();
	}
	if (!bFound)
		__super::OnChar(nChar, nRepCnt, nFlags);
}
void CEditFindCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	bool bFound(false);
	if (nChar == VK_F3) {
		unsigned uFlags(GetAsyncKeyState(VK_SHIFT) ? 2 : 1);
		bFound = FindText(uFlags);
	}
	if (!bFound)
		__super::OnKeyDown(nChar, nRepCnt, nFlags);
}
bool CEditFindCtrl::FindText(unsigned uFlags)
{
	CHARRANGE sel = {0};
	GetSel(sel);
	CHARRANGE orgSel = sel;
	unsigned flag(FR_DOWN);
	if (uFlags & 1)
		sel.cpMin++;
	else if (uFlags & 2) {
		flag = 0;
	}
	int nSel(sel.cpMax-sel.cpMin);
	if (mStringToFind.IsEmpty()) {
		if (nSel > 0) {
			mStringToFind = GetSelText();
		}
		else mStringToFind = GetClipboardString().c_str();
	}
	FINDTEXTEX ft = {{sel.cpMin,-1}, mStringToFind};
	int pos = __super::FindText(flag, &ft);
	if (pos < 0) {
		if (flag == FR_DOWN)
			ft.chrg.cpMin = 0;
		else
			ft.chrg.cpMin = ((unsigned long)~0)>>1;
		pos = __super::FindText(flag, &ft);
	}
	bool bFound = true;
	if (pos < 0) {
		mStringToFind.Empty();
		pos = orgSel.cpMin;
		MessageBeep(0xFFFFFFFF);
		bFound = false;
	}
	sel.cpMin = pos;
	sel.cpMax = pos+mStringToFind.GetLength();
	SetSel(sel);
	return bFound;
}
