// FindOptionDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Find.h"
#include "FindOptionDlg.h"
#include "SystemUtils.h"

#define SIZE_OPERATOR_GREATER_THAN 0
#define SIZE_OPERATOR_LESSER_THAN 1
#define SIZE_OPERATOR_EQUAL_TO 2
#define SIZE_OPERATOR_GREATER_THAN_OR_EUQAL_TO 3
#define SIZE_OPERATOR_LESSER_THAN_OR_EUQAL_TO 4
#define SIZE_OPERATOR_IN_BETWEEN 5

#define SIZE_SELECTED_KB 0
#define SIZE_SELECTED_MB 1
#define SIZE_SELECTED_GB 2
#define SIZE_SELECTED_BYTES 3

#define TIME_COMPARE_MODIFIED 0
#define TIME_COMPARE_CREATED 1
#define TIME_COMPARE_ACCESSED 2
#define TIME_COMPARE_DBUPDATE 3

#define SIZE_INCR_ON_SPIN 100

// CFindOptionDlg dialog
static int GenericCompareFindOptionCatagoryFn(const void * elem1, const void * elem2)
{
	return ((FindOptionCatagory*)elem1)->number - ((FindOptionCatagory*)elem2)->number;
}

IMPLEMENT_DYNAMIC(CFindOptionDlg, CDialog)

CFindOptionDlg::CFindOptionDlg(CWnd* pParent /*=NULL*/)
: CDialog(CFindOptionDlg::IDD, pParent), m_bSizeChecked(false),
m_uSizeOperator(SIZE_OPERATOR_GREATER_THAN), m_llSize(0), m_llSize2(0),
m_iSizeSelected(SIZE_SELECTED_KB), m_bModifiedDateTimeChecked(false),
m_uTimeToCompare(TIME_COMPARE_MODIFIED), m_uDTOpenrator(IDC_RADIO_BETWEEN),
m_timeFrom(CTime::GetCurrentTime()), m_timeTo(CTime::GetCurrentTime()), mbSearchZip(false),
mCatagories((GenericCompareFn)GenericCompareFindOptionCatagoryFn), mCatagoryListCtrl(NULL)
{
}

CFindOptionDlg::~CFindOptionDlg()
{
}

void CFindOptionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BOOL CFindOptionDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	// Update size controls
	CheckDlgButton(IDC_CHECK_SIZE, m_bSizeChecked ? BST_CHECKED : BST_UNCHECKED);
	((CComboBox *)GetDlgItem(IDC_COMBO_SIZE))->SetCurSel(m_uSizeOperator);
	((CComboBox *)GetDlgItem(IDC_COMBO_SIZE_OPTION))->SetCurSel(m_iSizeSelected);
	UpdateSizeControls();
	UINT size = (UINT)m_llSize;
	UINT size2 = (UINT)m_llSize2;
	if (m_iSizeSelected != SIZE_SELECTED_BYTES) {
		size = (UINT)(m_llSize >> ((m_iSizeSelected+1)*10));
		size2 = (UINT)(m_llSize2 >> ((m_iSizeSelected+1)*10));
	}
	SetDlgItemInt(IDC_EDIT_SIZE, size,FALSE);
	SetDlgItemInt(IDC_EDIT_SIZE2, size2,FALSE);
	CSpinButtonCtrl *pSpinCtrl = (CSpinButtonCtrl *)GetDlgItem(IDC_SPIN_SIZE);
	UDACCEL udAccel = {0, 100};
	pSpinCtrl->SetRange32(UD_MAXVAL, 0);
	pSpinCtrl->SetPos32(size);
	pSpinCtrl->SetAccel(1, &udAccel);
	pSpinCtrl = (CSpinButtonCtrl *)GetDlgItem(IDC_SPIN_SIZE2);
	pSpinCtrl->SetRange32(UD_MAXVAL, 0);
	pSpinCtrl->SetPos32(size2);
	pSpinCtrl->SetAccel(1, &udAccel);
	// Update Date controls
	CheckDlgButton(IDC_CHECK_DATE, m_bModifiedDateTimeChecked ? BST_CHECKED : BST_UNCHECKED);
	((CComboBox *)GetDlgItem(IDC_COMBO_FILE_TIME))->SetCurSel(m_uTimeToCompare);
	CheckDlgButton(m_uDTOpenrator, BST_CHECKED);
	((CDateTimeCtrl *)GetDlgItem(IDC_DATETIMEPICKER_FROM))->SetTime(&m_timeFrom);
	((CDateTimeCtrl *)GetDlgItem(IDC_DATETIMEPICKER_TO))->SetTime(&m_timeTo);
	OnDateRadioButtonClicked(m_uDTOpenrator);
	EnableDisableControlsOnCheck(0);
	EnableDisableControlsOnCheck(1);
	CEmbedListCtrl *pEmbedListCtrl(new CEmbedListCtrl());
	mCatagoryListCtrl = pEmbedListCtrl;
	pEmbedListCtrl->SubclassDlgItem(IDC_LIST_CATAGORIES, this);
	mCatagoryListCtrl->SetExtendedStyle(mCatagoryListCtrl->GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP);
	pEmbedListCtrl->InsertColumn(0, _T("Name"));
	pEmbedListCtrl->SetColumnWidth(0, LVSCW_AUTOSIZE_USEHEADER);
	pEmbedListCtrl->CheckRowColCheckBox(0);
	CheckDlgButton(IDC_CHECK_SEARCH_ZIP, mbSearchZip ? BST_CHECKED : BST_UNCHECKED);
	LoadCatagories(FindDataBase::GetCacheDBPath());
	return TRUE;
}

BEGIN_MESSAGE_MAP(CFindOptionDlg, CDialog)
	ON_CBN_SELCHANGE(IDC_COMBO_SIZE, &CFindOptionDlg::OnCbnSelchangeComboSize)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_SIZE, &CFindOptionDlg::OnDeltaposSpinSize)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_SIZE2, &CFindOptionDlg::OnDeltaposSpinSize2)
	ON_EN_CHANGE(IDC_EDIT_SIZE, &CFindOptionDlg::OnEnChangeEditSize)
	ON_EN_CHANGE(IDC_EDIT_SIZE2, &CFindOptionDlg::OnEnChangeEditSize2)
	ON_BN_CLICKED(IDOK, &CFindOptionDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_RADIO_BETWEEN, &CFindOptionDlg::OnBnClickedRadioBetween)
	ON_BN_CLICKED(IDC_RADIO_BEFORE, &CFindOptionDlg::OnBnClickedRadioBefore)
	ON_BN_CLICKED(IDC_RADIO_AFTER, &CFindOptionDlg::OnBnClickedRadioAfter)
	ON_BN_CLICKED(IDC_CHECK_SIZE, &CFindOptionDlg::OnBnClickedCheckSize)
	ON_BN_CLICKED(IDC_CHECK_DATE, &CFindOptionDlg::OnBnClickedCheckDate)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_CACHE, &CFindOptionDlg::OnBnClickedButtonBrowseCache)
	ON_BN_CLICKED(IDC_CHECK_USE_LOCAL, &CFindOptionDlg::OnBnClickedCheckUseLocal)
END_MESSAGE_MAP()
void CFindOptionDlg::UpdateSizeControls()
{
	m_uSizeOperator = ((CComboBox *)GetDlgItem(IDC_COMBO_SIZE))->GetCurSel();
	int nShow = SW_SHOW;
	if (m_uSizeOperator != SIZE_OPERATOR_IN_BETWEEN) {
		nShow = SW_HIDE;
	}
	GetDlgItem(IDC_EDIT_SIZE2)->ShowWindow(nShow);
	GetDlgItem(IDC_STATIC_AND)->ShowWindow(nShow);
	GetDlgItem(IDC_SPIN_SIZE2)->ShowWindow(nShow);
}

// CFindOptionDlg message handlers

void CFindOptionDlg::OnCbnSelchangeComboSize()
{
	UpdateSizeControls();
}
void CFindOptionDlg::OnSpinDeltaPos(LPNMUPDOWN pNMUpDown, int nEditControl)
{
	SetDlgItemInt(nEditControl, pNMUpDown->iPos);
}
void CFindOptionDlg::OnEditSizeChnage(int nEditControl, int nSpinControl)
{
	CSpinButtonCtrl *pSpinCtrl = (CSpinButtonCtrl *)GetDlgItem(nSpinControl);
	pSpinCtrl->SetPos32(GetDlgItemInt(nEditControl, NULL, FALSE));
}
void CFindOptionDlg::OnDeltaposSpinSize(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	OnSpinDeltaPos(pNMUpDown, IDC_EDIT_SIZE);
	*pResult = 0;
}

void CFindOptionDlg::OnDeltaposSpinSize2(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	OnSpinDeltaPos(pNMUpDown, IDC_EDIT_SIZE2);
	*pResult = 0;
}
bool CFindOptionDlg::CheckFileSizeSatisfies(const LONGLONG &fileSize)
{
	bool bPassed = true;
	if (m_bSizeChecked) {
		switch (m_uSizeOperator) {
		case SIZE_OPERATOR_GREATER_THAN:
			bPassed = fileSize > m_llSize;
			break;
		case SIZE_OPERATOR_LESSER_THAN:
			bPassed = fileSize < m_llSize;
			break;
		case SIZE_OPERATOR_EQUAL_TO:
			bPassed = fileSize == m_llSize;
			break;
		case SIZE_OPERATOR_GREATER_THAN_OR_EUQAL_TO:
			bPassed = fileSize >= m_llSize;
			break;
		case SIZE_OPERATOR_LESSER_THAN_OR_EUQAL_TO:
			bPassed = fileSize <= m_llSize;
			break;
		case SIZE_OPERATOR_IN_BETWEEN:
			{
				bPassed = fileSize >= m_llSize && fileSize <= m_llSize2;
			}
			break;
		}
	}
	return bPassed;
}
bool CFindOptionDlg::CheckFileSatisfies(const CFileMetaDataProvider &findFile)
{
	bool bPassed = CheckFileSizeSatisfies(findFile.GetFileSize());
	if (bPassed && m_bModifiedDateTimeChecked) {
		CTime fileTime;
		switch (m_uTimeToCompare) {
		case TIME_COMPARE_MODIFIED:
			findFile.GetLastWriteTime(fileTime);
			break;
		case TIME_COMPARE_CREATED:
			findFile.GetCreationTime(fileTime);
			break;
		case TIME_COMPARE_ACCESSED:
			findFile.GetLastAccessTime(fileTime);
			break;
		case TIME_COMPARE_DBUPDATE:
			findFile.GetLastUpdateTime(fileTime);
			break;
		}
		switch (m_uDTOpenrator) {
		case IDC_RADIO_BETWEEN:
			bPassed = fileTime >= m_timeFrom && fileTime <= m_timeTo;
			break;
		case IDC_RADIO_BEFORE:
			bPassed = fileTime < m_timeFrom;
			break;
		case IDC_RADIO_AFTER:
			bPassed = fileTime > m_timeFrom;
			break;
		}
	}
	return bPassed;
}
void CFindOptionDlg::OnEnChangeEditSize()
{
	OnEditSizeChnage(IDC_EDIT_SIZE, IDC_SPIN_SIZE);
}

void CFindOptionDlg::OnEnChangeEditSize2()
{
	OnEditSizeChnage(IDC_EDIT_SIZE2, IDC_SPIN_SIZE2);
}
template <class T>
void Swap(T &a, T &b)
{
	T t = a;
	a = b;
	b = t;
}
void CFindOptionDlg::OnBnClickedOk()
{
	// Update size controls
	m_bSizeChecked = IsDlgButtonChecked(IDC_CHECK_SIZE) == BST_CHECKED;
	m_uSizeOperator = ((CComboBox *)GetDlgItem(IDC_COMBO_SIZE))->GetCurSel();
	m_iSizeSelected = ((CComboBox *)GetDlgItem(IDC_COMBO_SIZE_OPTION))->GetCurSel();
	m_llSize = (UINT)GetDlgItemInt(IDC_EDIT_SIZE);
	m_llSize2 = (UINT)GetDlgItemInt(IDC_EDIT_SIZE2);
	if (m_iSizeSelected != SIZE_SELECTED_BYTES) {
		m_llSize <<= (m_iSizeSelected+1)*10;
		m_llSize2 <<= (m_iSizeSelected+1)*10;
	}
	if (m_uSizeOperator == SIZE_OPERATOR_IN_BETWEEN
		&& m_llSize > m_llSize2)
		Swap(m_llSize, m_llSize2);
	// Update Date controls
	m_bModifiedDateTimeChecked = IsDlgButtonChecked(IDC_CHECK_DATE) == BST_CHECKED;
	m_uTimeToCompare = ((CComboBox *)GetDlgItem(IDC_COMBO_FILE_TIME))->GetCurSel();
	((CDateTimeCtrl *)GetDlgItem(IDC_DATETIMEPICKER_FROM))->GetTime(m_timeFrom);
	((CDateTimeCtrl *)GetDlgItem(IDC_DATETIMEPICKER_TO))->GetTime(m_timeTo);
	if (m_uDTOpenrator == IDC_RADIO_BETWEEN && m_timeFrom > m_timeTo)
		Swap(m_timeFrom, m_timeTo);
	for (int i = 0; i < mCatagoryListCtrl->GetItemCount(); ++i) {
		FindOptionCatagory *pCatagory((FindOptionCatagory *)mCatagoryListCtrl->GetItemData(i));
		pCatagory->bChecked = mCatagoryListCtrl->IsChecked(0, i);
	}
	mbSearchZip = IsDlgButtonChecked(IDC_CHECK_SEARCH_ZIP) == BST_CHECKED;
	OnOK();
}

void CFindOptionDlg::OnBnClickedRadioBetween()
{
	OnDateRadioButtonClicked(IDC_RADIO_BETWEEN);
}

void CFindOptionDlg::OnBnClickedRadioBefore()
{
	OnDateRadioButtonClicked(IDC_RADIO_BEFORE);
}

void CFindOptionDlg::OnBnClickedRadioAfter()
{
	OnDateRadioButtonClicked(IDC_RADIO_AFTER);
}

void CFindOptionDlg::OnDateRadioButtonClicked(int nRadioControl)
{
	m_uDTOpenrator = nRadioControl;
	int nShow = SW_SHOW;
	if (m_uDTOpenrator != IDC_RADIO_BETWEEN) {
		nShow = SW_HIDE;
	}
	GetDlgItem(IDC_STATIC_FROM)->ShowWindow(nShow);
	GetDlgItem(IDC_STATIC_TO)->ShowWindow(nShow);
	GetDlgItem(IDC_DATETIMEPICKER_TO)->ShowWindow(nShow);
}

void CFindOptionDlg::EnableDisableControlsOnCheck(int iCheck)
{
	const int iControls[] = {IDC_CHECK_SIZE, IDC_CHECK_DATE};
	const int iControlsToEnDs_Size[] = {IDC_COMBO_SIZE, IDC_EDIT_SIZE, IDC_SPIN_SIZE,
		IDC_COMBO_SIZE_OPTION, IDC_STATIC_AND, IDC_EDIT_SIZE2, IDC_SPIN_SIZE2
	};
	const int iControlsToEnDs_Date[] = {IDC_COMBO_FILE_TIME, IDC_RADIO_BETWEEN, IDC_RADIO_BEFORE, IDC_RADIO_AFTER,
		IDC_STATIC_FROM, IDC_DATETIMEPICKER_FROM, IDC_STATIC_TO, IDC_DATETIMEPICKER_TO
	};
	const int iSizes[] = {sizeof(iControlsToEnDs_Size)/sizeof(int), sizeof(iControlsToEnDs_Date)/sizeof(int)};
	const int *iControlsArray[] = {iControlsToEnDs_Size, iControlsToEnDs_Date};
	BOOL bEnable = IsDlgButtonChecked(iControls[iCheck]) == BST_CHECKED;
	for (int i = 0 ; i < iSizes[iCheck]; i++) {
		GetDlgItem(iControlsArray[iCheck][i])->EnableWindow(bEnable);
	}
}
void CFindOptionDlg::OnBnClickedCheckSize()
{
	EnableDisableControlsOnCheck(0);
}

void CFindOptionDlg::OnBnClickedCheckDate()
{
	EnableDisableControlsOnCheck(1);
}
void CFindOptionDlg::OnDestroy()
{
	delete mCatagoryListCtrl;
}
int CFindOptionDlg::SavePrefToFile(FindDataBase &findDb, const CString &preferenceName)
{
	std::string prefName(SystemUtils::UnicodeToUTF8(preferenceName));
	char buffer[1024];
	char condition[256];
	sprintf_s(condition, 256, " where PreferenceName = '%s'", prefName.c_str());
	if(m_bSizeChecked) {
		sprintf_s(buffer, 1024,
			"update Preferences set SizeMin=%I64d, SizeMax=%I64d, SizeCondition=%d%s",
			m_llSize, m_llSize2, m_uSizeOperator, condition);
		findDb.QueryNonRows(buffer);
	}
	if (m_bModifiedDateTimeChecked) {
		char dateTo[256] = {0};
		if (m_uDTOpenrator == IDC_RADIO_BETWEEN)
			sprintf_s(dateTo, 256, "%d %d %d %d %d",
				m_timeTo.GetYear(), m_timeTo.GetMonth(), m_timeTo.GetDay(),
				m_timeTo.GetHour(), m_timeTo.GetMinute());
		sprintf_s(buffer, 1024,
			"update Preferences set DateFrom='%d %d %d %d %d', DateTo='%s', DateCondition=%d, DateType=%d%s",
			m_timeFrom.GetYear(), m_timeFrom.GetMonth(), m_timeFrom.GetDay(),
			m_timeFrom.GetHour(), m_timeFrom.GetMinute(),
			dateTo,
			m_uTimeToCompare, m_uDTOpenrator, condition);
		findDb.QueryNonRows(buffer);
	}
	return 0;
}
TableItertatorClass(CFindOptionDlg);
int CFindOptionDlg::ItrPreferencesTableRowsCallbackFn(sqlite3_stmt *statement, void *pUserData)
{
    UNREFERENCED_PARAMETER(pUserData);
	const char *colText((const char *)sqlite3_column_text(statement, Preferences_SizeMin));
	m_bSizeChecked = colText != NULL && colText[0];
	// SizeMin
	sscanf_s(colText, "%I64d", &m_llSize);
	colText = (const char *)sqlite3_column_text(statement, Preferences_SizeMax);
	// SizeMex
	sscanf_s(colText, "%I64d", &m_llSize2);
	colText = (const char *)sqlite3_column_text(statement, Preferences_SizeCondition);
	// m_uSizeOperator
	sscanf_s(colText, "%d", &m_uSizeOperator);
	m_iSizeSelected = SIZE_SELECTED_BYTES;
	colText = (const char *)sqlite3_column_text(statement, Preferences_DateFrom);
	m_bModifiedDateTimeChecked = colText != NULL && colText[0];
	int y,m,d,h,min;
	if (colText[0]) {
		sscanf_s(colText, "%d %d %d %d %d",
			&y, &m, &d, &h, &min);
		m_timeFrom = CTime(y,m,d,h,min, 0);
	}
	colText = (const char *)sqlite3_column_text(statement, Preferences_DateTo);
	if (colText[0]) {
		sscanf_s(colText, "%d %d %d %d %d",
			&y, &m, &d, &h, &min);
		m_timeTo = CTime(y,m,d,h,min, 0);
	}
	colText = (const char *)sqlite3_column_text(statement, Preferences_DateCondition);
	sscanf_s(colText, "%d", &m_uTimeToCompare);
	colText = (const char *)sqlite3_column_text(statement, Preferences_DateType);
	sscanf_s(colText, "%d", &m_uDTOpenrator);
	return 0;
}

int CFindOptionDlg::LoadPrefFromFile(FindDataBase &findDb, const CString &preferenceName)
{
	ItrTableRowsCallbackData_CFindOptionDlg itSHTable(this,
		&CFindOptionDlg::ItrPreferencesTableRowsCallbackFn);
	std::string prefName(SystemUtils::UnicodeToUTF8(preferenceName));
	char condition[256];
	sprintf_s(condition, 256, " where PreferenceName = '%s'", prefName.c_str());
	itSHTable.IterateTableRows(findDb, "Preferences", condition);

	return 0;
}
int CFindOptionDlg::ItrCatagoryTableRowsCallbackFn(sqlite3_stmt *statement, void *pUserData)
{
    UNREFERENCED_PARAMETER(pUserData);
	int flag = sqlite3_column_int(statement, Catagory_Flags);
	if (flag & 1) {
		FindOptionCatagory fc(sqlite3_column_int(statement, Catagory_CatagoryNumber));
		fc.name = SystemUtils::UTF8ToUnicodeCString((const char *)sqlite3_column_text(statement, Catagory_Name));
		mCatagories.Insert(fc);
	}
	return 0;
}

FindOptionCatagory* CFindOptionDlg::GetCatagory(int catagoryNum)
{
	FindOptionCatagory *pOutCatagory(NULL);
	for (INT_PTR i = 0; i < mCatagories.GetCount(); ++i) {
		FindOptionCatagory *pCatagory(&mCatagories.GetAt(i));
		if (pCatagory->number == catagoryNum) {
			pOutCatagory = pCatagory;
			break;
		}
	}
	return pOutCatagory;
}

void CFindOptionDlg::LoadCatagories(LPCTSTR path)
{
	bool bReload(false);
	CString csPath;
	if (path != NULL)
		csPath = path;
	bReload = csPath != FindDataBase::GetCacheDBPath();
	UpdateCacheLocationEditCtrl(path);
	if (bReload) {
		mCatagories.RemoveAll();
		mSelectedCatagories.Empty();
	}
	if (mCatagories.IsEmpty()) {
		ItrTableRowsCallbackData_CFindOptionDlg itSHTable(this,
			&CFindOptionDlg::ItrCatagoryTableRowsCallbackFn);
		FindDataBase fdb(FDB_CacheDatabase, true);
		if (fdb.Open() == 0) {
			itSHTable.IterateTableRows(fdb, "Category");
			CString lastUpdated(fdb.GetProperty(_T("LastUpdateTime")));
			if (!lastUpdated.IsEmpty()) {
				CString editText;
				GetDlgItemText(IDC_EDIT_CACHE_LOCATION, editText);
				editText = _T("Last updated ") + lastUpdated + _T("   ")   + editText;
				SetDlgItemText(IDC_EDIT_CACHE_LOCATION, editText);
			}
		}
	}
	if (!mSelectedCatagories.IsEmpty()) {
		for (INT_PTR i = 0; i < mCatagories.GetCount(); ++i) {
			mCatagories.GetAt(i).bChecked = false;
		}
		CArrayCString selectedCatagories;
		SystemUtils::SplitString(mSelectedCatagories, selectedCatagories);
		mSelectedCatagories.Empty();
		for (INT_PTR i = 0; i < selectedCatagories.GetCount(); ++i) {
			FindOptionCatagory *pCatagory(GetCatagory(SystemUtils::StringToInt(selectedCatagories[i])));
			if (pCatagory)
				pCatagory->bChecked = true;
		}
	}
	mCatagoryListCtrl->DeleteAllItems();
	for (INT_PTR i = 0; i < mCatagories.GetCount(); ++i) {
		FindOptionCatagory *pCatagory(&mCatagories.GetAt(i));
		int row = mCatagoryListCtrl->InsertItem(0x7fffffff, pCatagory->name);
		mCatagoryListCtrl->CheckRowColCheckBox(0, row, pCatagory->bChecked);
		mCatagoryListCtrl->SetItemData(row, (DWORD_PTR)pCatagory);
	}
}

void CFindOptionDlg::OnBnClickedButtonBrowseCache()
{
	CFileDialog cfd(TRUE, _T("txt"), NULL, 4|2, NULL, this);
	if (cfd.DoModal() != IDOK)
		return;
	LoadCatagories(cfd.GetPathName());
}
void CFindOptionDlg::UpdateCacheLocationEditCtrl(LPCTSTR path)
{
	FindDataBase::SetCacheDBPath(path);
	const CString &cacheLocation(FindDataBase::GetCacheDBPath());
	if (cacheLocation.IsEmpty()) {
		SetDlgItemText(IDC_EDIT_CACHE_LOCATION, _T("Local Cache"));
		GetDlgItem(IDC_CHECK_USE_LOCAL)->ShowWindow(SW_HIDE);
		CheckDlgButton(IDC_CHECK_USE_LOCAL, BST_CHECKED);
	}
	else {
		SetDlgItemText(IDC_EDIT_CACHE_LOCATION, cacheLocation);
		GetDlgItem(IDC_CHECK_USE_LOCAL)->ShowWindow(SW_SHOW);
		CheckDlgButton(IDC_CHECK_USE_LOCAL, BST_UNCHECKED);
	}
}
void CFindOptionDlg::OnBnClickedCheckUseLocal()
{
	if (IsDlgButtonChecked(IDC_CHECK_USE_LOCAL)) {
		LoadCatagories();
	}
}
void CFindOptionDlg::SaveDefault(FindDataBase &findDb)
{
	const CString &cacheLocation(FindDataBase::GetCacheDBPath());
	if (cacheLocation.IsEmpty()) {
		findDb.RemoveProperty(_T("CacheDBPath"));
	}
	else
		findDb.SetProperty(_T("CacheDBPath"), cacheLocation);

	if (mCatagories.GetCount() > 0) {
		bool bAllCheck(true);
		int nCatagoriesSelected(0);
		CString selectedCatagories;
		for (INT_PTR i = 0; i < mCatagories.GetCount(); ++i) {
			const FindOptionCatagory *pCatagory(&mCatagories.GetAt(i));
			if (pCatagory->bChecked) {
				if (nCatagoriesSelected)
					selectedCatagories += _T(",");
				selectedCatagories += SystemUtils::IntToString(pCatagory->number);
				++nCatagoriesSelected;
			}
			else bAllCheck = false;
		}
		if (bAllCheck)
			findDb.RemoveProperty(_T("SelectedCatagories"));
		else
			findDb.SetProperty(_T("SelectedCatagories"), selectedCatagories);
	}
	findDb.SetProperty(_T("ZipSearch"), mbSearchZip ? _T("1") : _T("0"));
}
void CFindOptionDlg::LoadDefault(FindDataBase &findDb)
{
	FindDataBase::SetCacheDBPath(findDb.GetProperty(_T("CacheDBPath")));
	mSelectedCatagories = findDb.GetProperty(_T("SelectedCatagories"));
	mbSearchZip = findDb.GetProperty(_T("ZipSearch")) == _T("1");
}
CString CFindOptionDlg::GetCatogoryQueryCondition() const
{
	CString condition(_T("CatagoryNumber IN ("));
	bool bAllCheck(mSelectedCatagories.IsEmpty());
	if (bAllCheck) {
		int nCatagoriesSelected(0);
		for (INT_PTR i = 0; i < mCatagories.GetCount(); ++i) {
			const FindOptionCatagory *pCatagory(&mCatagories.GetAt(i));
			if (pCatagory->bChecked) {
				if (nCatagoriesSelected)
					condition += _T(",");
				condition += SystemUtils::IntToString(pCatagory->number);
				++nCatagoriesSelected;
			}
			else bAllCheck = false;
		}
	}
	else {
		condition += mSelectedCatagories;
	}
	if (bAllCheck)
		condition.Empty();
	else
		condition += _T(")");
	return condition;
}

