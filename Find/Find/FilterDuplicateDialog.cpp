// FilterDuplicateDialog.cpp : implementation file
//

#include "stdafx.h"
#include "Find.h"
#include "FilterDuplicateDialog.h"


// CFilterDuplicateDialog dialog

IMPLEMENT_DYNAMIC(CFilterDuplicateDialog, CDialog)

CFilterDuplicateDialog::CFilterDuplicateDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CFilterDuplicateDialog::IDD, pParent), m_uFlag(0)
{

}

CFilterDuplicateDialog::~CFilterDuplicateDialog()
{
}

void CFilterDuplicateDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BOOL CFilterDuplicateDialog::OnInitDialog()
{
	CDialog::OnInitDialog();
	CheckDlgButton(IDC_CHECK_FILE_NAME, BST_CHECKED);
	m_uFlag = DF_MATCH_FILE_NAME;
	CheckDlgButton(IDC_RADIO_DUPLICATE, BST_CHECKED);
	return TRUE;
}


BEGIN_MESSAGE_MAP(CFilterDuplicateDialog, CDialog)
	ON_BN_CLICKED(IDC_CHECK_SIZE, &CFilterDuplicateDialog::OnBnClickedCheckSize)
	ON_BN_CLICKED(IDC_CHECK_FILE_NAME, &CFilterDuplicateDialog::OnBnClickedCheckFileName)
	ON_BN_CLICKED(IDC_CHECK_FULL_PATH, &CFilterDuplicateDialog::OnBnClickedCheckFullPath)
	ON_BN_CLICKED(IDOK, &CFilterDuplicateDialog::OnBnClickedOk)
END_MESSAGE_MAP()


// CFilterDuplicateDialog message handlers
void CFilterDuplicateDialog::CommonCheck(int nIDButton, UINT uFlag)
{
	if (IsDlgButtonChecked(nIDButton)) {
		m_uFlag |= uFlag;
	}
	else {
		m_uFlag &= ~uFlag;
		if ((m_uFlag & DF_MATCH_MASK) == 0) {
			m_uFlag |= uFlag;
			CheckDlgButton(nIDButton, BST_CHECKED);
		}
	}
}
void CFilterDuplicateDialog::CommonCheckConflicts(int nIDButton, UINT uFlag)
{
	if (m_uFlag & uFlag) {
		m_uFlag &= ~uFlag;
		CheckDlgButton(nIDButton, BST_UNCHECKED);
	}
}
void CFilterDuplicateDialog::OnBnClickedCheckSize()
{
	CommonCheck(IDC_CHECK_SIZE, DF_MATCH_FILE_SIZE);
}

void CFilterDuplicateDialog::OnBnClickedCheckFileName()
{
	CommonCheck(IDC_CHECK_FILE_NAME, DF_MATCH_FILE_NAME);
	CommonCheckConflicts(IDC_CHECK_FULL_PATH, DF_MATCH_PATH);
}

void CFilterDuplicateDialog::OnBnClickedCheckFullPath()
{
	CommonCheck(IDC_CHECK_FULL_PATH, DF_MATCH_PATH);
	CommonCheckConflicts(IDC_CHECK_FILE_NAME, DF_MATCH_FILE_NAME);
}

void CFilterDuplicateDialog::OnBnClickedOk()
{
	CommonCheck(IDC_RADIO_UNIQUE, DF_PRINT_UNIQUE_ONLY);
	OnOK();
}
