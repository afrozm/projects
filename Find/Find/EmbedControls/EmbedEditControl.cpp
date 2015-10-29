// EmbedEditControl.cpp : implementation file
//

#include "stdafx.h"
#include "EmbedEditControl.h"
#include "EmbedListCtrl.h"

// CEmbedEditControl

IMPLEMENT_DYNAMIC(CEmbedEditControl, CEdit)

CEmbedEditControl::CEmbedEditControl()
{

}

CEmbedEditControl::~CEmbedEditControl()
{
}


BEGIN_MESSAGE_MAP(CEmbedEditControl, CEdit)
END_MESSAGE_MAP()



// CEmbedEditControl message handlers

LRESULT CEmbedEditControl::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult(0);
	lResult = CEdit::WindowProc(message, wParam, lParam);
	switch (message) {
	case WM_KILLFOCUS:
		GetParent()->SendMessage(WM_ENABLE_EMBED_CONTROL, 0, 0);
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

