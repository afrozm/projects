#pragma once


// CEmbedEditControl

class CEmbedEditControl : public CEdit
{
	DECLARE_DYNAMIC(CEmbedEditControl)

public:
	CEmbedEditControl();
	virtual ~CEmbedEditControl();

protected:
	DECLARE_MESSAGE_MAP()
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
};


