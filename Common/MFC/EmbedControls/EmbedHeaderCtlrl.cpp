// EmbedHeaderCtlrl.cpp : implementation file
//

#include "stdafx.h"
#include "EmbedHeaderCtlrl.h"
#include "EmbedListCtrl.h"

// CEmbedHeaderCtlrl

IMPLEMENT_DYNAMIC(CEmbedHeaderCtlrl, CHeaderCtrl)

CEmbedHeaderCtlrl::CEmbedHeaderCtlrl(CEmbedListCtrl *pListControl)
: m_pListControl(pListControl)
{

}

CEmbedHeaderCtlrl::~CEmbedHeaderCtlrl()
{
}


BEGIN_MESSAGE_MAP(CEmbedHeaderCtlrl, CHeaderCtrl)
END_MESSAGE_MAP()



// CEmbedHeaderCtlrl message handlers


LRESULT CEmbedHeaderCtlrl::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult = CHeaderCtrl::WindowProc(message, wParam, lParam);
	switch (message) {
	case HDM_HITTEST:
		if (lResult >= 0) {
			HDHITTESTINFO *htInfo = (HDHITTESTINFO *)lParam;
			int iCol = (int)lResult;
			if (iCol >= 0 && (htInfo->flags & HHT_ONHEADER)
				&& m_pListControl->HeaderHasCheckBox(iCol)) {
				int colLeft = 0;
				for (int i = 0; i < iCol; ++i)
					colLeft += m_pListControl->GetColumnWidth(i);
				if (htInfo->pt.x < colLeft + 20)
					htInfo->flags |= HHT_ONCHECKBOX;
			}
		}
		break;
	case WM_LBUTTONDOWN:
		{
			HDHITTESTINFO htInfo = {0};
			htInfo.pt.x = GET_X_LPARAM(lParam);
			htInfo.pt.y = GET_Y_LPARAM(lParam);
			int iCol = HitTest(&htInfo);
			if (iCol >= 0 && (htInfo.flags & HHT_ONCHECKBOX))
				m_pListControl->CheckRowColCheckBox(iCol, -1,
					!m_pListControl->IsChecked(iCol));
		}
		break;
	}
	return lResult;
}