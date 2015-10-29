#pragma once
#include "Finder.h"
#include "FindDataBase.h"
#include "EmbedListCtrl.h"
#include "FileMetaDataProvider.h"
#include "resource.h"
// CFindOptionDlg dialog

struct FindOptionCatagory {
	FindOptionCatagory()
		: number(0), bChecked(true)
	{
	}
	FindOptionCatagory(int cnumber)
		: number(cnumber), bChecked(true)
	{
	}
	bool operator > (const FindOptionCatagory & inSH) const {return Compare(inSH) > 0;}
	bool operator < (const FindOptionCatagory & inSH) const {return Compare(inSH) < 0;}
	int Compare(const FindOptionCatagory &inFC) const {return number - inFC.number;}
	int number;
	CString name;
	bool bChecked;
};
typedef CSortedArray<FindOptionCatagory> CSortedArrayFindOptionCatagory;


class CFindOptionDlg : public CDialog
{
	DECLARE_DYNAMIC(CFindOptionDlg)

public:
	CFindOptionDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CFindOptionDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_FIND_OPTIONS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	void LoadCatagories(LPCTSTR path = NULL);
	FindOptionCatagory* GetCatagory(int catagoryNum);
	afx_msg void OnDestroy();
	DECLARE_MESSAGE_MAP()
	bool CheckFileSizeSatisfies(const LONGLONG &fileSize);
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnCbnSelchangeComboSize();
	bool CheckFileSatisfies(const CFileMetaDataProvider &findFile);
	afx_msg void OnDeltaposSpinSize(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDeltaposSpinSize2(NMHDR *pNMHDR, LRESULT *pResult);
	void UpdateSizeControls();
	void OnSpinDeltaPos(LPNMUPDOWN pNMUpDown, int nEditControl);
	void OnEditSizeChnage(int nEditControl, int nSpinControl);
	void OnDateRadioButtonClicked(int nRadioControl);
	void EnableDisableControlsOnCheck(int iCheck);
	int SavePrefToFile(FindDataBase &findDb, const CString &preferenceName);
	int LoadPrefFromFile(FindDataBase &findDb, const CString &preferenceName);
	int ItrPreferencesTableRowsCallbackFn(sqlite3_stmt *statement, void *pUserData);
	int ItrCatagoryTableRowsCallbackFn(sqlite3_stmt *statement, void *pUserData);
	void SaveDefault(FindDataBase &findDb);
	void LoadDefault(FindDataBase &findDb);
	CString GetCatogoryQueryCondition() const;
	bool IsSearchZipEnabled() const {return mbSearchZip;}
private:
	bool m_bSizeChecked;
	int m_uSizeOperator; // GT, LT, EQ, ...
	LONGLONG m_llSize;
	LONGLONG m_llSize2;
	int m_iSizeSelected;
	bool m_bModifiedDateTimeChecked;
	int m_uTimeToCompare; // Modifed Time, Created Time, Accessed Time
	int m_uDTOpenrator; // In between, Before, After
	CTime m_timeFrom;
	CTime m_timeTo;
	CSortedArrayFindOptionCatagory mCatagories;
	CEmbedListCtrl *mCatagoryListCtrl;
	CString mSelectedCatagories; 
	bool mbSearchZip;

	afx_msg void OnEnChangeEditSize();
	afx_msg void OnEnChangeEditSize2();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedRadioBetween();
	afx_msg void OnBnClickedRadioBefore();
	afx_msg void OnBnClickedRadioAfter();
	afx_msg void OnBnClickedCheckSize();
	afx_msg void OnBnClickedCheckDate();
	void UpdateCacheLocationEditCtrl(LPCTSTR path = NULL);
public:
	afx_msg void OnBnClickedButtonBrowseCache();
	afx_msg void OnBnClickedCheckUseLocal();
};
