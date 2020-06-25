#include "StdAfx.h"
#include "Find.h"
#include "DupFileFilter.h"
#include "FilterDuplicateDialog.h"
#include "Percentage.h"
#include "cMD5.h"
#include "StringUtils.h"

__int64 FileSize(HANDLE pFile)
{
	LARGE_INTEGER li = {0};
	GetFileSizeEx(pFile, &li);
	return li.QuadPart;
}

CDupFileFilter::CDupFileFilter(CSaveListResultCtrl *pListCtrl, CDialog *pFindDlg)
: mListCtrl(pListCtrl), m_uFlags(DF_MATCH_FILE_NAME), m_pDialog((CFindDlg *)pFindDlg)
{
}

CDupFileFilter::~CDupFileFilter(void)
{
}
#define FSIF_ISDUPLICATE 1
#define FSIF_OUTPUT 2
struct FileSizeInfo {
	int nItem;
	UINT m_uFlag;
	FileSizeInfo()
		: m_uFlag(0)
	{
	}
};
void FilterDuplicateUpdateCallback(double curPercentage, void *pUserData)
{
	((CFindDlg*)pUserData)->SetStatusMessage(_T("Filtering duplicate files: %.2f%% done"),
		curPercentage);
}

void CDupFileFilter::ApplyFilter()
{
	if (m_pDialog->IsSearchStarted()) // Already searching some files? - Then dont do
		return;
	{
		CFilterDuplicateDialog filterDupDialog(mListCtrl);
		if (filterDupDialog.DoModal() != IDOK)
			return;
		m_uFlags = filterDupDialog.m_uFlag;
	}
	mFileHash.RemoveAll();
	m_pDialog->SetSearchStartedImpl();
	m_pDialog->SetStatusMessage(_T("Sorting list..."));
	int startIndex = 0;
	if (m_uFlags & DF_MATCH_FILE_SIZE) {
		startIndex = 2;
	}
	else if (m_uFlags & DF_MATCH_PATH) {
		startIndex = 1;
	}
	mListCtrl->SortItemsEx(startIndex, true);
	if (m_pDialog->IsSearchCancelled())
		return;
	startIndex = 0;
	int listItemCount = mListCtrl->GetItemCount();
	// Skip all directories
	if (m_uFlags & DF_MATCH_FILE_SIZE) {
		for (startIndex = 0; startIndex < listItemCount; startIndex++) {
			if (mListCtrl->GetFileSize(startIndex) >= 0)
				break;
			if (m_pDialog->IsSearchCancelled())
				return;
		}
		listItemCount -= startIndex;
	}
	CAutoDisableNotificaltion autoDisableNotification(mListCtrl);
	ULONGLONG itemCount(listItemCount);
	itemCount *= 2;
	CPercentage percentage(itemCount, FilterDuplicateUpdateCallback, m_pDialog);
	itemCount = 0;
	CArray<FileSizeInfo> fileSizeInfoArray;
	fileSizeInfoArray.SetSize(listItemCount);
	// Initialize file size array
	for (int i = 0; i < listItemCount; i++) {
		fileSizeInfoArray[i].nItem = startIndex + i;
		CString fileName = mListCtrl->GetItemText(startIndex + i, 0);
		if (fileName[0] == '*') {
			fileName.Delete(0); // Remove exisitn astrik
			mListCtrl->SetItemText(startIndex + i, 0, fileName);
		}
		if (m_pDialog->IsSearchCancelled())
			return;
	}
	// Now start duplicates
	for (int i = 0; i < listItemCount && !m_pDialog->IsSearchCancelled(); i++) {
		percentage.Update(itemCount++);
		FileSizeInfo &fsInfo = fileSizeInfoArray[i];
		if (m_uFlags & DF_PRINT_UNIQUE_ONLY)
			fsInfo.m_uFlag |= FSIF_OUTPUT;
		bool bHasDuplicate = false;
		int next;
		for (next = i+1; next < listItemCount && !m_pDialog->IsSearchCancelled(); next++) {
			FileSizeInfo &fsInfoNext = fileSizeInfoArray[next];
			if (!FilePartialMatch(fsInfo.nItem, fsInfoNext.nItem))
				break;
			if (CompareFiles(fsInfo.nItem, fsInfoNext.nItem) == 0) {
				fsInfoNext.m_uFlag |= FSIF_ISDUPLICATE;
				if (!(m_uFlags & DF_PRINT_UNIQUE_ONLY))
					fsInfoNext.m_uFlag |= FSIF_OUTPUT;
				bHasDuplicate = true;
			}
		}
		i = next-1; // Skip all duplicates found
		if (bHasDuplicate && !(m_uFlags & DF_PRINT_UNIQUE_ONLY)) {
			// Add '*' astric to indicate duplicates
			CString fileName = mListCtrl->GetItemText(fsInfo.nItem, 0);
			fileName = _T("*")+fileName;
			mListCtrl->SetItemText(fsInfo.nItem, 0, fileName);
			fsInfo.m_uFlag |= FSIF_OUTPUT;
		}
	}
	mFileHash.RemoveAll();
	m_pDialog->SetStatusMessage(_T("Updating list..."));
	mListCtrl->DisablePaint(true);
	// Now Updade the List
	for (int i = 0; i < startIndex && !m_pDialog->IsSearchCancelled(); i++) {
		mListCtrl->DeleteItem(0);
		percentage.Update(itemCount++);
	}
	for (int i = 0; i < listItemCount && !m_pDialog->IsSearchCancelled(); i++) {
		FileSizeInfo &fsInfo = fileSizeInfoArray[i];
		if (!(fsInfo.m_uFlag & FSIF_OUTPUT)) {
			mListCtrl->DeleteItem(fsInfo.nItem-startIndex);
			startIndex++;
		}
		percentage.Update(itemCount++);
	}
	mListCtrl->DisablePaint(false);
	listItemCount = mListCtrl->GetItemCount();
	m_pDialog->SetStatusMessage(_T("%d files found"), listItemCount);
}
void FileCompareUpdateCallback(double curPercentage, void *pUserData)
{
	((CFindDlg*)pUserData)->SetStatusMessage(_T("Comparing files content: %.2f%% done"),
		curPercentage);
}

__int64 CDupFileFilter::CompareFiles(HANDLE pFiles[], unsigned int nFiles)
{
	__int64 pos = FileSize(pFiles[0]);
	char buffer1[4096];
	DWORD nRead1 = sizeof(buffer1)/sizeof(char);
	bool bUpdateProgress = pos >= 1024*1024; // 1Mb
	CPercentage percentage(pos, FileCompareUpdateCallback, m_pDialog);
	pos = 0;
	while (ReadFile(pFiles[0], buffer1, nRead1, &nRead1, NULL) && nRead1 > 0) {
		pos += nRead1;
		for (unsigned int i=1; i < nFiles; ++i) {
			char buffer2[4096];
			ReadFile(pFiles[i], buffer2, nRead1, &nRead1, NULL);
			if (nRead1 == 0 || memcmp(buffer1, buffer2, nRead1)) {
				return pos;
			}
		}
		if (bUpdateProgress)
			percentage.Update(pos);
	}
	return 0;
}

__int64 CDupFileFilter::CompareFiles(int nItem1, int nItem2)
{
	__int64 pos = 0;
	if ((m_uFlags & DF_MATCH_FILE_SIZE) && mListCtrl->GetFileSize(nItem1) > 0)
		return GetFileMD5(mListCtrl->GetItemText(nItem1, 1)).Compare(GetFileMD5(mListCtrl->GetItemText(nItem2, 1)));
	return pos;
}

bool CDupFileFilter::FilePartialMatch(int nItem1, int nItem2)
{
	bool bPartialMatch = true;
	int col = -1;
	if (m_uFlags & DF_MATCH_FILE_NAME) {
		col = 0;
	}
	else if (m_uFlags & DF_MATCH_PATH) {
		col = 1;
	}
	if (col >= 0) {
		bPartialMatch = mListCtrl->GetItemText(nItem1, col).CompareNoCase(mListCtrl->GetItemText(nItem2, col)) == 0;
	}
	if (bPartialMatch && (m_uFlags & DF_MATCH_FILE_SIZE)) {
		bPartialMatch = (mListCtrl->GetFileSize(nItem1) == mListCtrl->GetFileSize(nItem2));
	}
	return bPartialMatch;
}

static void Md5PercentUpdateCallback(double curPercentage, void *pUserData)
{
    ((CFindDlg*)pUserData)->SetStatusMessage(_T("Computing file checksum: %.2f%% done"), curPercentage);
}

class MD5FileCallback : public MD5Callback
{
public:
    MD5FileCallback(CDupFileFilter *pDupFilter) : pFilter(pDupFilter)
    {
        percentage.SetCallback(Md5PercentUpdateCallback, pFilter->GetDlg());
        percentage.SetFlag(PF_UPDATE_AT_TIME_OUT);
    }
    virtual int Status() override
    {
        percentage.Update(GetCurrent());
        return pFilter->GetDlg()->IsSearchCancelled();
    }


protected:
    CDupFileFilter *pFilter;
    CPercentage percentage;
    virtual void SetTotal(MD5ULL total) override
    {
        __super::SetTotal(total);
        percentage.SetTototal(total);
    }

};


const CString& CDupFileFilter::GetFileMD5(const CString &filePath)
{
	if (mFileHash.PLookup(filePath) == NULL) {
        MD5FileCallback callback(this);
		cMD5 cmd5(&callback);
		mFileHash[filePath] = UTF8_TO_UNICODE(cmd5.CalcMD5FromFile(filePath)).c_str();
	}
	return mFileHash[filePath];
}
