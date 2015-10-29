#pragma once


// CEditFindCtrl

class CEditFindCtrl : public CRichEditCtrl
{
	DECLARE_DYNAMIC(CEditFindCtrl)
	CString mStringToFind;
public:
	CEditFindCtrl();
	virtual ~CEditFindCtrl();
	bool FindText(unsigned uFlags = 0);
protected:
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	DECLARE_MESSAGE_MAP()
};


