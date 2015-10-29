// ListCtrlUtil.cpp : implementation file
//

#include "stdafx.h"
#include "Find.h"
#include "ListCtrlUtil.h"


// CListCtrlUtil

IMPLEMENT_DYNAMIC(CListCtrlUtil, CListCtrl)

CListCtrlUtil::CListCtrlUtil()
: m_iColToAdjust(1), mbNotificationDisabled(false)
{

}

CListCtrlUtil::~CListCtrlUtil()
{
}
bool CListCtrlUtil::DisableNotification(bool bDisable)
{
	if (mbNotificationDisabled != bDisable) {
		mbNotificationDisabled = bDisable;
		return true;
	}
	return false;
}

BOOL CListCtrlUtil::EnsureColVisible(int col)
{
	BOOL bSuccess(FALSE);
	bool bScrolledRight(false);
	RECT cr;
	GetClientRect(&cr);
	while (1) {
		RECT colRect;
		bSuccess = GetHeaderCtrl()->GetItemRect(col, &colRect);
		::MapWindowPoints(GetHeaderCtrl()->m_hWnd, m_hWnd, (LPPOINT)&colRect, 2);
		if (bSuccess) {
			if (colRect.left < 0) {
				WPARAM wParam = SB_LINELEFT;
				if (-colRect.left >= cr.right)
					wParam = SB_PAGELEFT;
				SendMessage(WM_HSCROLL, wParam);
				if (bScrolledRight)
					break;
			}
			else {
				RECT clientRect;
				GetClientRect(&clientRect);
				if (colRect.right > clientRect.right) {
					int toScroll = colRect.right - clientRect.right;
					if (toScroll > colRect.left)
						toScroll = colRect.left;
					WPARAM wParam = SB_LINERIGHT;
					if (toScroll >= cr.right)
						wParam = SB_PAGERIGHT;
					SendMessage(WM_HSCROLL, wParam);
					bScrolledRight = true;
				}
				else break;
			}
		}
	}
	return bSuccess;
}
void CListCtrlUtil::SelectAllItems()
{
	CAutoDisableNotificaltion autoDisableNotificaltion(this);
	int count = GetItemCount();
	for (int i = 0; i < count; i++) {
		SetItemState(i,LVIS_SELECTED, LVIS_SELECTED);
	}
}
void CListCtrlUtil::InvertSelection()
{
	CAutoDisableNotificaltion autoDisableNotificaltion(this);
	int count = GetItemCount();
	for (int i = 0; i < count; i++) {
		UINT state = GetItemState(i, LVIS_SELECTED);
		state ^= LVIS_SELECTED;
		SetItemState(i, state, LVIS_SELECTED);
	}
}
void CListCtrlUtil::RemoveAllSelection()
{
	CAutoDisableNotificaltion autoDisableNotificaltion(this);
	POSITION pos;
	while ((pos = GetFirstSelectedItemPosition()) != NULL) {
		SetItemState(GetNextSelectedItem(pos), 0, LVIS_SELECTED); // Remove selection;
	}
}
BOOL CListCtrlUtil::SetItemTextIfModified(int nItem, int nSubItem, LPCTSTR lpszText)
{
	CString itemText(GetItemText(nItem, nSubItem));
	if (itemText != lpszText)
		return SetItemText(nItem, nSubItem, lpszText);
	return FALSE;
}

BEGIN_MESSAGE_MAP(CListCtrlUtil, CListCtrl)
	ON_WM_SIZE()
END_MESSAGE_MAP()



// CListCtrlUtil message handlers
void CListCtrlUtil::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);
	AdjustColumnWidth();
}

void CListCtrlUtil::AdjustColumnWidth()
{
	RECT clr;
	GetClientRect(&clr);
	if (clr.right-clr.left < SystemUtils::GetTranslatedDPIPixelX(500))
		return;
	int nCol = GetHeaderCtrl()->GetItemCount();
	int totalColWidht = 0;
	for (int i = 0; i < nCol; i++) {
		if (i!=1) // Path
			totalColWidht += GetColumnWidth(i);
	}
	totalColWidht = clr.right-clr.left - totalColWidht;
	if (totalColWidht > SystemUtils::GetTranslatedDPIPixelX(300))
		SetColumnWidth(m_iColToAdjust, totalColWidht);
}

void CListCtrlUtil::OnKeyDown(UINT nChar, UINT nReptCnt, UINT nFlags)
{
	bool bCtrlPressed = GetAsyncKeyState(VK_CONTROL) != 0;
	switch (nChar) {
	case 'A':
		if (bCtrlPressed) {
			CAutoDisableNotificaltion autoDisableNotificaltion(this);
			SelectAllItems();
		}
		break;
	case 'I':
		if (bCtrlPressed)
			InvertSelection();
		break;
	}
	if (!IsNotificationDisabled()) {
		CAutoDisableNotificaltion autoDisableNotificaltion(this);
		__super::OnKeyDown(nChar, nReptCnt, nFlags);
	}
	else {
		__super::OnKeyDown(nChar, nReptCnt, nFlags);
	}
}

int CListCtrlUtil::GetItemIndex(LPVOID pItemData, int startIndex /* = 0 */, bool bIncr /* = true */, bool bMatch /* = true */)
{
	int incr(bIncr ? 1 : -1);
	for (int i = startIndex; i >= 0 && i < GetItemCount(); i += incr) {
		if (((LPVOID)GetItemData(i) == pItemData) == bMatch)
			return i;
	}
	return -1;
}
