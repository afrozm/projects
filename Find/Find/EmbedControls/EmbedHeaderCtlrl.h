#pragma once


// CEmbedHeaderCtlrl
class CEmbedListCtrl;

#define HHT_ONCHECKBOX          0x100000

class CEmbedHeaderCtlrl : public CHeaderCtrl
{
	DECLARE_DYNAMIC(CEmbedHeaderCtlrl)

public:
	CEmbedHeaderCtlrl(CEmbedListCtrl *pListControl);
	virtual ~CEmbedHeaderCtlrl();

protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
	CEmbedListCtrl *m_pListControl;
};


