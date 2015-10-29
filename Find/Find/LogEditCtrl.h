#pragma once

#include "EditFindCtrl.h"

// CLogEditCtrl

class CLogEditCtrl : public CEditFindCtrl
{
	DECLARE_DYNAMIC(CLogEditCtrl)
public:
	CLogEditCtrl();
	~CLogEditCtrl();
	void Append(const CString& inStr);
protected:
	DECLARE_MESSAGE_MAP()
};


