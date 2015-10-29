#pragma once

#include "FindDataBase.h"
// CPreferences dialog

class CPreferences : public CDialog
{
	DECLARE_DYNAMIC(CPreferences)

public:
	CPreferences(LPCTSTR defPref = NULL, CWnd* pParent = NULL);   // standard constructor
	virtual ~CPreferences();

// Dialog Data
	enum { IDD = IDD_DIALOG_PREFERENCES };
	CString mLoadedPreferenceName;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	CString GetPreferencesFilePahthFromName(const CString &preferenceName)
	{return FindDataBase::GetPreferencesFolderPahth() + _T("\\") + preferenceName + _T(".prf");}
	void LoadPreferences(); // Loads all preferences into list ontrol
	void SavePreference(const CString &preferenceName);
	BOOL DeletePreference(const CString &preferenceName);
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	void LoadPreference(const CString &preferenceName);
	int ItrPreferencesTableRowsCallbackFn(sqlite3_stmt *statement, void *pUserData);
	afx_msg void OnBnClickedCancel();
};
