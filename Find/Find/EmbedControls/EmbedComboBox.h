#pragma once


// CEmbedComboBox
#include "EmbedEditControl.h"

class CEmbedComboBox : public CComboBox
{
	DECLARE_DYNAMIC(CEmbedComboBox)

public:
	CEmbedComboBox();
	virtual ~CEmbedComboBox();

protected:
	DECLARE_MESSAGE_MAP()
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	CEmbedEditControl *mEmbedEditControl;
};


