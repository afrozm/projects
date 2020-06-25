#pragma once

#define DF_MATCH_FILE_SIZE 1
#define DF_MATCH_FILE_NAME 2
#define DF_MATCH_PATH 4
#define DF_MATCH_MASK (DF_MATCH_FILE_SIZE|DF_MATCH_FILE_NAME|DF_MATCH_PATH)
#define DF_PRINT_UNIQUE_ONLY 8
#define DF_DELETE_FILES 16

// CFilterDuplicateDialog dialog

class CFilterDuplicateDialog : public CDialog
{
	DECLARE_DYNAMIC(CFilterDuplicateDialog)

public:
	CFilterDuplicateDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CFilterDuplicateDialog();

// Dialog Data
	enum { IDD = IDD_DIALOG_FILTER_DUPLICATE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	void CommonCheck(int nIDButton, UINT uFlag);
	void CommonCheckConflicts(int nIDButton, UINT uFlag);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedCheckSize();
public:
	afx_msg void OnBnClickedCheckFileName();
public:
	afx_msg void OnBnClickedCheckFullPath();
	UINT m_uFlag;
public:
	afx_msg void OnBnClickedOk();
};
