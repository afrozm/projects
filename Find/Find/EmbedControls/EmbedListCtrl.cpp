// EmbedListCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "EmbedListCtrl.h"
#include "EmbedEditControl.h"
#include "EmbedComboBox.h"
#include "EmbedHeaderCtlrl.h"

// CEmbedListCtrl

#pragma comment( lib, "UxTheme")

IMPLEMENT_DYNAMIC(CEmbedListCtrl, CListCtrlUtil)

CEmbedListCtrl::CEmbedListCtrl()
: mEmbedControlCallBackFn(NULL), m_pUserData(NULL), m_pHeaderControl(NULL),
mCheckBoxIndex(-1), m_iConSize(SystemUtils::GetTranslatedDPIPixel(14))
{
}

CEmbedListCtrl::~CEmbedListCtrl()
{
	if (m_pHeaderControl != NULL) {
		delete m_pHeaderControl;
	}
}
void CEmbedListCtrl::SetEmbedControlCallBackFn(EmbedControlCallBackFn embedControlCallBackFn, LPVOID pUserData)
{
	InitHeaderControl();
	mEmbedControlCallBackFn = embedControlCallBackFn;
	m_pUserData = pUserData;
}
void CEmbedListCtrl::InitHeaderControl()
{
	if (m_pHeaderControl == NULL) {
		m_pHeaderControl = new CEmbedHeaderCtlrl(this);
		CHeaderCtrl *pHeader(GetHeaderCtrl());
		int id = GetWindowLong(pHeader->m_hWnd, GWL_ID);
		m_pHeaderControl->SubclassDlgItem(id, pHeader->GetParent());
	}
}
bool CEmbedListCtrl::EnableControl(int row, int col, unsigned int uEnable)
{
	if (mEmbedControlCallBackFn == NULL)
		return false;
	bool bSuccess(false);
	if (uEnable & ECF_ENABLE) {
		const int rowCount(GetItemCount());
		const int colCount(GetColumnCount());
		if (row == -1 || col == -1
			|| (uEnable & (ECF_ENABLE_NEXT | ECF_ENABLE_PREV))) {
			if (row < 0)
				row = uEnable & ECF_ENABLE_PREV ? rowCount : 0;
			col = -1;
			if (mEmbedControlsArray.GetCount() > 0) {
				INT_PTR i = mEmbedControlsArray.GetCount() -1;
				row = mEmbedControlsArray[i].row;
				col = mEmbedControlsArray[i].col;
			}
			if (uEnable & ECF_ENABLE_PREV) {
				--col;
				if (col < 0) {
					col = colCount -1;
					--row;
					if (row < 0)
						row = rowCount-1;
				}
			}
			else {
				++col;
				if (col >= colCount) {
					col = 0;
					++row;
					if (row >= rowCount)
						row = 0;
				}
			}
		}
		bSuccess = EnableControl(-1, -1, false); // Remove all
		if (row < 0 || row >= rowCount || col < 0 || col >= colCount) {
			return false;
		}
		EmbedControlType ect = (EmbedControlType)mEmbedControlCallBackFn(this, EMT_AddControl, row, col, m_pUserData);
		CWnd *pControl(NULL);
		RECT bound, cl;
		EnsureVisible(row, FALSE);
		EnsureColVisible(col);
		GetItemRect(row, col, &bound);
		if (HasCheckBox(col, row))
			bound.left += 20;
		GetClientRect(&cl);
		if (bound.right > cl.right)
			bound.right = cl.right;
		UINT id((UINT)mEmbedControlsArray.GetCount()+1);
		CString itemText(GetItemText(row, col));
		switch (ect) {
		case ECT_Edit:
		{
			CEdit *pEdit = new CEmbedEditControl();
			pEdit->Create(WS_CHILD|ES_WANTRETURN|ES_AUTOHSCROLL, bound, this, id);
			pEdit->SetWindowText(itemText);
			pControl = pEdit;
			break;
		}
		case ECT_ComboBox:
		case ECT_DropDownComboBox:
		{
			CEmbedComboBox *pComboBox = new CEmbedComboBox();
			DWORD style = ect == ECT_ComboBox
				? CBS_DROPDOWN|CBS_AUTOHSCROLL
				: CBS_DROPDOWNLIST;
			pComboBox->Create(WS_CHILD|style, bound, this, id);
			pComboBox->SetWindowText(itemText);
			COMBOBOXINFO cbInfo = {0};
			pControl = pComboBox;
			break;
		}
		}
		if (pControl) {
			EmbedControlInfo ecInf;
			ecInf.mType = ect;
			ecInf.row = row;
			ecInf.col = col;
			ecInf.mControl = pControl;
			mEmbedControlsArray.Add(ecInf);
			mEmbedControlCallBackFn(this, EMT_InitControl, (WPARAM)&ecInf, 0, m_pUserData);
			switch (ect) {
			case ECT_DropDownComboBox:
				{
					CEmbedComboBox *pComboBox = (CEmbedComboBox *)pControl;
					pComboBox->SelectString(-1, itemText);
				}
				break;
			}
			pControl->SetFont(GetFont(), FALSE);
			pControl->ShowWindow(SW_SHOW);
			pControl->SetFocus();
			bSuccess = true;
		}
	}
	else {
		if (row == -1 || col == -1) {
			for (int i= 0; i < mEmbedControlsArray.GetCount(); ++i) {
				bSuccess = bSuccess | OnDeleteEmbedControl(i, uEnable);
			}
		}
		else {
			int i = 0;
			for (i= 0; i < mEmbedControlsArray.GetCount(); ++i)
				if (mEmbedControlsArray[i].row == row && mEmbedControlsArray[i].col == col)
					break;
			bSuccess = OnDeleteEmbedControl(i, uEnable);
		}
	}
	return bSuccess;
}

BEGIN_MESSAGE_MAP(CEmbedListCtrl, CListCtrlUtil)
	ON_WM_SETFOCUS()
	ON_MESSAGE(WM_ENABLE_EMBED_CONTROL, OnEnableControl)
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()



// CEmbedListCtrl message handlers

void CEmbedListCtrl::OnSetFocus(CWnd *pOldWnd)
{
	EnableControl(-1, -1, false);
	__super::OnSetFocus(pOldWnd);
}
LRESULT CEmbedListCtrl::OnEnableControl(WPARAM wParam, LPARAM /*lParam*/)
{
	return EnableControl(-1, -1, (unsigned int)wParam);
}
void CEmbedListCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	__super::OnLButtonDown(nFlags, point);
	int x = point.x;
	int y = point.y;
	LVHITTESTINFO hitTestInfo = {0};
	hitTestInfo.pt.x = x;
	hitTestInfo.pt.y = y;
	int row = SubItemHitTest(&hitTestInfo);
	if (row >= 0) {
		RECT colRect;
		int iCol = hitTestInfo.iSubItem;
		GetHeaderCtrl()->GetItemRect(iCol, &colRect);
		::MapWindowPoints(GetHeaderCtrl()->m_hWnd, m_hWnd, (LPPOINT)&colRect, 2);
		if (HasCheckBox(hitTestInfo.iSubItem, row)
			&& x < colRect.left + m_iConSize) {
				CheckRowColCheckBox(iCol, row, !IsChecked(iCol, row));
		}
		else {
			EnableControl(row, iCol);
		}
	}
}

bool CEmbedListCtrl::OnDeleteEmbedControl(unsigned int iIndex, unsigned uCommit)
{
	bool bDeleted(false);
	if (iIndex < (unsigned int)mEmbedControlsArray.GetCount()) {
		EmbedControlInfo &embedControl(mEmbedControlsArray[iIndex]);
		if (!(uCommit & ECF_DONOT_COMMIT)) {
			switch (embedControl.mType) {
			case ECT_Edit:
			case ECT_ComboBox:
			case ECT_DropDownComboBox:
			case ECT_HotKey:
			case ECT_IPAddress:
			{
				CString text;
				embedControl.mControl->GetWindowText(text);
				SetItemText(embedControl.row, embedControl.col, text);
			}
				break;
			}
		}
		mEmbedControlCallBackFn(this, EMT_RemoveControl, (WPARAM)&embedControl, uCommit, m_pUserData);
		CWnd *pWnd(embedControl.mControl);
		mEmbedControlsArray.RemoveAt(iIndex);
		pWnd->DestroyWindow();
		delete pWnd;
		bDeleted = true;
	}
	return bDeleted;
}
BOOL CEmbedListCtrl::PreTranslateMessage(MSG* pMsg)
{
	BOOL bProcessed(FALSE);
	if (pMsg->message == WM_KEYDOWN) {
		{
			int commit(0);
			switch (pMsg->wParam) {
			case VK_ESCAPE:
			case VK_CLEAR:
				commit = ECF_DONOT_COMMIT; // No commit
			case VK_ACCEPT:
			case VK_RETURN:
				bProcessed = EnableControl(-1, -1, commit);
				break;
			case VK_TAB:
				if (GetAsyncKeyState(VK_CONTROL))
				{
					CWnd *pFocusWindow = CWnd::GetFocus();
					if (pFocusWindow && pFocusWindow != this)
						pFocusWindow = pFocusWindow->GetParent();
					if (pFocusWindow && pFocusWindow != this)
						pFocusWindow = pFocusWindow->GetParent();
					if (pFocusWindow != this)
						break;
					commit = ECF_ENABLE;
					if (GetAsyncKeyState(VK_SHIFT))
						commit |= ECF_ENABLE_PREV;
					else
						commit |= ECF_ENABLE_NEXT;
					bProcessed = EnableControl(-1, -1, commit);
				}
				break;
			case VK_F2:
				{
					int row = GetSelectionMark();
					bProcessed = EnableControl(row, -1);
				}
				break;
			}
		}
	}
	return bProcessed;
}
bool CEmbedListCtrl::AddCheckBox()
{
	bool bSuccess(false);
	if (mImageList.m_hImageList == NULL || mCheckBoxIndex < 0) {
		const int kImageSize(SystemUtils::GetTranslatedDPIPixel(20));
		if (mImageList.m_hImageList == NULL) {
			mImageList.Create(m_iConSize, m_iConSize, ILC_COLOR32, 2, 1);
			SetImageList(&mImageList, LVSIL_SMALL);
		}
		// Fill imahe list
		HTHEME hTheme(OpenThemeData(NULL, _T("BUTTON")));
		if (hTheme != NULL) {
			CBitmap bmpCheckboxes;
			int states[] = {CBS_UNCHECKEDNORMAL, CBS_CHECKEDNORMAL};
			CDC dcMem, *windowDC(GetDC());
			dcMem.CreateCompatibleDC(windowDC);
			bmpCheckboxes.CreateCompatibleBitmap(windowDC, m_iConSize<<1, m_iConSize);
			CBitmap* pOldBmp = dcMem.SelectObject(&bmpCheckboxes);
			RECT rect = {0};
			rect.left = (m_iConSize-kImageSize)>>1;
			rect.top = rect.left;
			rect.right = rect.bottom = rect.left + kImageSize;
			for (int i = 0; i < sizeof(states)/sizeof(states[0]); i++) {
				HRESULT hResult(0);
				hResult = DrawThemeBackground(hTheme, dcMem, BP_CHECKBOX, states[i], &rect, NULL);
				if (SUCCEEDED(hResult)) {
				}
				rect.left += m_iConSize;
				rect.right += m_iConSize;
			}
			dcMem.SelectObject(pOldBmp);
			mCheckBoxIndex = mImageList.Add(&bmpCheckboxes, RGB(0,0,0));
            ReleaseDC(windowDC);
			CloseThemeData(hTheme);
		}
		InitHeaderControl();
	}
	return bSuccess;
}

bool CEmbedListCtrl::CheckRowColCheckBox(int iCol, int iRow, int iChecked)
{
	LVCOLUMN lvCol = {0};
	lvCol.mask = LVCF_IMAGE;
	bool bAdd(false);
	if (iChecked == BST_REMOVE)
		lvCol.iImage = -1;
	else {
		bAdd = (iChecked & BST_ADD) != 0;
		iChecked &= MASKBIT(4);
		AddCheckBox();
		lvCol.iImage = mCheckBoxIndex + iChecked;
	}
	int nCol(GetColumnCount());
	int nRow(GetItemCount());
	if (iCol < 0 || iCol >= nCol)
		iCol = 0;
	else
		nCol = iCol + 1;
	if (iRow < 0 || iRow >= nRow)
		iRow = 0;
	else
		nRow = iRow + 1;
	bool bSuccess(false);
	for (int row = iRow; row < nRow; ++row) {
		for (int col = iCol; col < nCol; ++col) {
			if (iChecked == BST_REMOVE || bAdd || HasCheckBox(col, row))
				bSuccess = SetIcon(lvCol.iImage, row, col) == TRUE;
		}
	}
	lvCol.iImage = mCheckBoxIndex + IsChecked(iCol);
	bSuccess = SetColumn(iCol, &lvCol) && bSuccess;
	NMLISTVIEW nmLv = {{GetSafeHwnd(), (UINT_PTR)GetDlgCtrlID(), LVN_COLUMNCLICK_ON_CHECK_BOX}, iRow, iCol};
	GetParent()->SendMessage(WM_NOTIFY, (WPARAM)GetSafeHwnd(), (LPARAM)&nmLv);
	return bSuccess;
}
bool CEmbedListCtrl::IsChecked(int iCol, int iRow)
{
	bool bIsChecked(false);
	if (iRow < 0 || iRow >= GetItemCount()) {
		for (iRow = 0; iRow < GetItemCount(); ++iRow) {
			int iConIndex(GetIcon(iRow, iCol));
			if (iConIndex < 0)
				continue;
			if (iConIndex != mCheckBoxIndex + 1) {
				break;
			}
		}
		bIsChecked = GetItemCount() && iRow == GetItemCount();
	}
	else {
		bIsChecked = GetIcon(iRow, iCol) == mCheckBoxIndex + 1;
	}
	return bIsChecked;
}
bool CEmbedListCtrl::HasCheckBox(int iCol, int iRow)
{
	bool bHasCheck(false);
	int nCol(GetColumnCount());
	int nRow(GetItemCount());
	if (iCol < 0 || iCol >= nCol)
		iCol = 0;
	else
		nCol = iCol + 1;
	if (iRow < 0 || iRow >= nRow)
		iRow = 0;
	else
		nRow = iRow + 1;
	for (int row = iRow; row < nRow; ++row) {
		for (int col = iCol; col < nCol; ++col) {
			int iCon = GetIcon(row, col);
			bHasCheck = iCon == mCheckBoxIndex + BST_UNCHECKED || iCon == mCheckBoxIndex + BST_CHECKED;
			if (!bHasCheck)
				break;
		}
		if (!bHasCheck)
			break;
	}
	return bHasCheck;
}
BOOL CEmbedListCtrl::GetItemRect(int nItem, int iCol, LPRECT lpRect, UINT nCode)
{
	BOOL bSuccess(CListCtrlUtil::GetItemRect(nItem, lpRect, nCode));
	if (bSuccess) {
		RECT colRect;
		bSuccess = GetHeaderCtrl()->GetItemRect(iCol, &colRect);
		::MapWindowPoints(GetHeaderCtrl()->m_hWnd, m_hWnd, (LPPOINT)&colRect, 2);
		if (bSuccess) {
			lpRect->left = colRect.left;
			lpRect->right = colRect.right;
		}
	}
	return bSuccess;
}
int CEmbedListCtrl::AddIcon(HICON hIcon)
{
	if (mImageList.m_hImageList == NULL) {
		mImageList.Create(m_iConSize, m_iConSize, ILC_COLOR32, 1, 1);
		SetImageList(&mImageList, LVSIL_SMALL);
	}
	return mImageList.Add(hIcon);
}

BOOL CEmbedListCtrl::SetIcon(int iconIndex, int iRow, int iCol)
{
	BOOL bSuccess(FALSE);
	int nCol(GetColumnCount());
	int nRow(GetItemCount());
	if (iCol < 0 || iCol >= nCol)
		iCol = 0;
	else
		nCol = iCol + 1;
	if (iRow < 0 || iRow >= nRow)
		iRow = 0;
	else
		nRow = iRow + 1;
	for (int row = iRow; row < nRow; ++row) {
		for (int col = iCol; col < nCol; ++col) {
			bSuccess = SetItem(row, col, LVIF_IMAGE, NULL, iconIndex, 0, 0, NULL) ;
		}
	}
	return bSuccess;
}
int CEmbedListCtrl::GetIcon(int iRow, int iCol)
{
	LVITEM lvItem = {0};
	lvItem.mask = LVIF_IMAGE;
	lvItem.iItem = iRow;
	lvItem.iSubItem = iCol;
	lvItem.iImage = -1;
	GetItem(&lvItem);
	return lvItem.iImage;
}
int CEmbedListCtrl::SetFontSize(int fontSize)
{
	LOGFONT lf = {0};
	GetFont()->GetLogFont(&lf);
	int oldFontSize(lf.lfHeight);
	lf.lfHeight = fontSize;
	mFont.CreateFontIndirect(&lf);
	SetFont(&mFont);
	return oldFontSize;
}
bool CEmbedListCtrl::HeaderHasCheckBox(int iCol)
{
	LVCOLUMN col = {LVCF_IMAGE};
	return GetColumn(iCol, &col) && col.iImage >= 0;
}