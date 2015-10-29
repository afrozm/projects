// EmbedConboBox.cpp : implementation file
//

#include "stdafx.h"
#include "EmbedComboBox.h"
#include "EmbedListCtrl.h"

// CEmbedComboBox

IMPLEMENT_DYNAMIC(CEmbedComboBox, CComboBox)

CEmbedComboBox::CEmbedComboBox()
: mEmbedEditControl(NULL)
{

}

CEmbedComboBox::~CEmbedComboBox()
{
}


BEGIN_MESSAGE_MAP(CEmbedComboBox, CComboBox)
END_MESSAGE_MAP()



// CEmbedComboBox message handlers

LRESULT CEmbedComboBox::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult(0);
	lResult = CComboBox::WindowProc(message, wParam, lParam);
	switch (message) {
	case WM_DESTROY:
		if (mEmbedEditControl != NULL) {
			delete mEmbedEditControl;
			mEmbedEditControl = NULL;
		}
		break;
	case WM_SHOWWINDOW:
		if (!mEmbedEditControl) {
			CWnd *pChild(GetWindow(GW_CHILD));
			if (pChild) {
				mEmbedEditControl = new CEmbedEditControl();
				mEmbedEditControl->SubclassWindow(pChild->m_hWnd);
			}
		}
		if (wParam)
			ShowDropDown();
		break;
	case WM_KILLFOCUS:
		{
			CWnd *pChild(GetWindow(GW_CHILD));
			if (!pChild || pChild->m_hWnd != (HWND)wParam)
				GetParent()->SendMessage(WM_ENABLE_EMBED_CONTROL, 0, 0);
		}
		break;
	case WM_ENABLE_EMBED_CONTROL:
		GetParent()->SendMessage(WM_ENABLE_EMBED_CONTROL, wParam, lParam);
		break;
	case WM_KEYDOWN:
		{
			int commit(0);
			switch (wParam) {
			case VK_ESCAPE:
			case VK_CLEAR:
				commit = ECF_DONOT_COMMIT; // No commit
			case VK_ACCEPT:
			case VK_RETURN:
				GetParent()->SendMessage(WM_ENABLE_EMBED_CONTROL, commit, 0);
			}
		}
		break;
	}
	return lResult;
}


