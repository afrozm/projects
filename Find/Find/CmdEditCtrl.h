#pragma once


// CCmdEditCtrl

class CCmdEditCtrl : public CEdit
{
	DECLARE_DYNAMIC(CCmdEditCtrl)

public:
	CCmdEditCtrl();
	virtual ~CCmdEditCtrl();
	void AddCommand(const CString &cmd, bool bUpdate = false);
	void RemoveCommand(const CString &cmd, bool bUpdate = false);
	void Clear();
	void GetLines();
	const CArrayCString& GetCommands() const {return mLines;};
	void UpdateWindowText();
	void SetWindowText(LPCTSTR lpszString);
protected:
	int FindCommand(const CString &cmd);
	DECLARE_MESSAGE_MAP()
	CArrayCString mLines;
};


