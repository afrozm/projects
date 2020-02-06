#pragma once

class CSaveListResultCtrl;
// CFindReplaceDialogEx

class CFindReplaceDialogEx : public CFindReplaceDialog
{
	DECLARE_DYNAMIC(CFindReplaceDialogEx)

public:
	virtual BOOL OnInitDialog();
	virtual ~CFindReplaceDialogEx();
	virtual BOOL Create(BOOL bFindDialogOnly, // TRUE for Find, FALSE for FindReplace
			LPCTSTR lpszFindWhat,
			LPCTSTR lpszReplaceWith = NULL,
			DWORD dwFlags = FR_DOWN,
			CWnd* pParentWnd = NULL);
	enum FindStringType {
		PlainText,
		WildCard,
		RegularExpression,
	};
	CFindReplaceDialogEx(CSaveListResultCtrl *pList, FindStringType currentStringType = PlainText);
	FindStringType GetFindStringType();
protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
	void SetFindStr(LPCTSTR findStr = NULL);
	CSaveListResultCtrl *m_pList;
	FindStringType mCurrentStringType;
	LPTSTR m_lpszFindStr = NULL;
};


