// Preferences.cpp : implementation file
//

#include "stdafx.h"
#include "Find.h"
#include "Preferences.h"
#include "FindDlg.h"
#include "SystemUtils.h" 

// CPreferences dialog

IMPLEMENT_DYNAMIC(CPreferences, CDialog)

CPreferences::CPreferences(LPCTSTR defPref, CWnd* pParent /*=NULL*/)
	: CDialog(CPreferences::IDD, pParent)
{
	if (defPref)
		mLoadedPreferenceName = defPref;
}

CPreferences::~CPreferences()
{
}

void CPreferences::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CPreferences, CDialog)
	ON_BN_CLICKED(IDOK, &CPreferences::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CPreferences::OnBnClickedCancel)
END_MESSAGE_MAP()


// CPreferences message handlers

void CPreferences::OnBnClickedOk()
{
	// Call the funtion
	/// check which check box is sleated
	// Load/Save
	if (IsDlgButtonChecked(IDC_RADIO_SAVE)) { // Save
		CString prefName;
		GetDlgItemText(IDC_EDIT_PREFERENCE_NAME, prefName);
		CListBox *listBox = (CListBox *)GetDlgItem(IDC_LIST_PREFERENCES);
		int indexInListBox = listBox->FindString(-1, prefName);
		if (indexInListBox >= 0) { // Already has this preference name
			if (MessageBox(prefName + _T(" already exists.\nDo you want to overwrite."),
				0, MB_YESNO) == IDNO) {
					CEdit *pEdit((CEdit*)GetDlgItem(IDC_EDIT_PREFERENCE_NAME));
					pEdit->SetFocus();
					pEdit->SetSel(0, -1);
					return;
			}
		}
		SavePreference(prefName);
		listBox->AddString(prefName);
		((CComboBox *)GetDlgItem(IDC_COMBO_DEFPREF))->AddString(prefName);
	}
	else if (IsDlgButtonChecked(IDC_RADIO_LOAD)) { // Load
		CListBox *listBox = (CListBox *)GetDlgItem(IDC_LIST_PREFERENCES);
		int curSel = listBox->GetCurSel();
		if (curSel >= 0) { // User has selected
			CString prefName;
			listBox->GetText(curSel, prefName);
			LoadPreference(prefName);
		}
	}
	else if (IsDlgButtonChecked(IDC_RADIO_DELETE)) { // Delete
		CListBox *listBox = (CListBox *)GetDlgItem(IDC_LIST_PREFERENCES);
		int curSel = listBox->GetCurSel();
		if (curSel >= 0) { // User has selected
			CString prefName;
			listBox->GetText(curSel, prefName);
			if (DeletePreference(prefName)) {
				listBox->DeleteString(curSel);
				CComboBox *pComboBox((CComboBox *)GetDlgItem(IDC_COMBO_DEFPREF));
				pComboBox->DeleteString(pComboBox->FindString(1, prefName));
			}
		}
	}
}


BOOL CPreferences::OnInitDialog()
{
	BOOL bRet = CDialog::OnInitDialog();
	LoadPreferences();
	return bRet;
}
TableItertatorClass(CPreferences);
int CPreferences::ItrPreferencesTableRowsCallbackFn(sqlite3_stmt *statement, void *pUserData)
{
    UNREFERENCED_PARAMETER(pUserData);
	CListBox *listBox = (CListBox *)GetDlgItem(IDC_LIST_PREFERENCES);
	CString prefName =
		SystemUtils::UTF8ToUnicode((const char *)sqlite3_column_text(statement, Preferences_PreferenceName)).c_str();
	listBox->AddString(prefName);
	CComboBox *pComboBox((CComboBox *)GetDlgItem(IDC_COMBO_DEFPREF));
	pComboBox->AddString(prefName);
	return 0;
}

void CPreferences::LoadPreferences()
{
	CListBox *listBox = (CListBox *)GetDlgItem(IDC_LIST_PREFERENCES);
	int prefCount = listBox->GetCount();
	// Remove all existing pre before reloading
	listBox->DeleteString(prefCount);
	ItrTableRowsCallbackData_CPreferences itSHTable(this,
		&CPreferences::ItrPreferencesTableRowsCallbackFn);
	FindDataBase prefDataBase(FDB_PrefDatabase, true);
	if (prefDataBase.Open() == 0) {
		itSHTable.IterateTableRows(prefDataBase, "Preferences");
	}
	CComboBox *pComboBox((CComboBox *)GetDlgItem(IDC_COMBO_DEFPREF));
	pComboBox->InsertString(0, _T(""));
	pComboBox->SelectString(0, mLoadedPreferenceName);
}

void CPreferences::SavePreference(const CString &preferenceName)
{
	CFindDlg *pMainWndDlg((CFindDlg *)AfxGetMainWnd());
	if (pMainWndDlg == NULL)
		return;
	FindDataBase findDb;
	if (findDb.Open() == 0) {
		pMainWndDlg->SavePrefToFile(findDb, preferenceName);
		findDb.Commit();
	}
}
void CPreferences::LoadPreference(const CString &preferenceName)
{
	CFindDlg *pMainWndDlg((CFindDlg *)AfxGetMainWnd());
	
	if (pMainWndDlg == NULL)
		return;
	FindDataBase findDb(FDB_PrefDatabase, true);
	if (findDb.Open() == 0) {
		pMainWndDlg->LoadPrefFromFile(findDb, preferenceName);
	}
}
BOOL CPreferences::DeletePreference(const CString &preferenceName)
{
	BOOL bSuccess(FALSE);
	FindDataBase findDb;
	if (findDb.Open() == 0) {
		bSuccess = findDb.QueryNonRows("DELETE from Preferences where Preferencename = '%s'",
			SystemUtils::UnicodeToUTF8(preferenceName).c_str()) == 0;
		findDb.Commit();
	}
	return bSuccess;
}

void CPreferences::OnBnClickedCancel()
{
	GetDlgItemText(IDC_COMBO_DEFPREF, mLoadedPreferenceName);
	OnCancel();
}
