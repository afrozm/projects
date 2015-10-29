#include "StdAfx.h"
#include "FileIconMgr.h"
#include "Path.h"

CIcon::CIcon(HICON hIcon /* = NULL */, bool bAutoDestroy)
	: m_hIcon(NULL), mbAutoDestroy(bAutoDestroy)
{
	SetIcon(hIcon);
}
CIcon::~CIcon()
{
	SetIcon(NULL);
}
void CIcon::SetIcon(HICON hIcon)
{
	if (m_hIcon != NULL && mbAutoDestroy)
		DestroyIcon(m_hIcon);
	m_hIcon = hIcon;
}
CIcon::CIcon(CIcon &icon)
	: m_hIcon(NULL), mbAutoDestroy(true)
{
	SetIcon(::CopyIcon(icon));
}
BOOL CIcon::LoadIcon(LPCTSTR lpIconName, HINSTANCE hIns /* = NULL */)
{
	mbAutoDestroy = true;
	SetIcon(::LoadIcon(hIns, lpIconName));
	return m_hIcon != NULL;
}
CPoint CIcon::GetSize() const
{
	CPoint pt;
	ICONINFO iconInfo;
	GetIconInfo(m_hIcon, &iconInfo);
	CBitmap *pBitmap(CBitmap::FromHandle(iconInfo.hbmColor ? iconInfo.hbmColor : iconInfo.hbmMask));
	if (pBitmap != NULL) {
		BITMAP bm;
		if (pBitmap->GetBitmap(&bm))
			pt.SetPoint(bm.bmWidth, bm.bmHeight);
	}
	return pt;
}

FileIconMgr& FileIconMgr::GetInstance()
{
	static FileIconMgr sFileIconMgr;
	return sFileIconMgr;
}

FileIconMgr::FileIconMgr()
{
	SetImageList(NULL);
}
void FileIconMgr::InitKnownExtnList()
{
	if (mExtVsIconIndex.IsEmpty()) {
		mExtVsIconIndex[_T("")] = -2;
		DWORD dwIndex(0);
		while (1) {
			TCHAR keyName[MAX_PATH];
			DWORD knL(ARRAYSIZE(keyName));
			LONG ret = RegEnumKeyEx(HKEY_CLASSES_ROOT, dwIndex++, keyName, &knL, NULL, NULL, NULL, NULL);
			if (ret)
				break;
			if (keyName[0] == '.') {
				mExtVsIconIndex[keyName] = -2;
			}
		}
	}
}
void FileIconMgr::SetImageList(CImageList *pImageList)
{
	mFolderIconIndex = -1;
	if (pImageList != NULL)
		m_pImageList = pImageList;
	m_pImageList = &mPrivateImageList;
}
int FileIconMgr::GetIconIndex(const CString& filePath)
{
	InitKnownExtnList();
	CString ext(Path(filePath).GetExtension().MakeLower());
	MapStrVsInt::CPair *pVal(mExtVsIconIndex.PLookup(ext));
	if (pVal == NULL)
		pVal = mExtVsIconIndex.PLookup(_T(""));
	int iconIndex(-1);
	if (pVal != NULL) {
		iconIndex = pVal->value;
		if (iconIndex < -1) {
			iconIndex = -1;
			// Index not found
			Path file(Path::TempFile(Path::TempPath(), _T("find"), ext));
			HANDLE hFile = CreateFile(file, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			if (hFile != INVALID_HANDLE_VALUE) {
				HICON hIcon(file.GetIcon());
				iconIndex = InsertIcon(hIcon);
				CloseHandle(hFile);
				file.DeletePath();
			}
			mExtVsIconIndex[ext] = iconIndex;
		}
	}
	return iconIndex;
}
int FileIconMgr::GetFolderIconIndex()
{
	if (mFolderIconIndex < 0) {
		mFolderIconIndex = InsertIcon(Path::TempPath().GetIcon());
	}
	return mFolderIconIndex;
}
int FileIconMgr::InsertIcon(HICON hIcon)
{
	int iconIndex(-1);
	if (hIcon != NULL) {
		CIcon icon(hIcon);
		if (m_pImageList->m_hImageList == NULL) {
			CPoint pt(icon.GetSize());
			if (pt.x > 0 && pt.y > 0) {
				m_pImageList->Create(pt.x, pt.y, ILC_COLOR32, 1, 1);
			}
		}
		if (m_pImageList->m_hImageList != NULL) {
			iconIndex = m_pImageList->Add(hIcon);
			if (iconIndex >= 0) {
				mFileExtIcons[mFileExtIcons.Add(icon)].SetAutoDestroy();
			}
		}
		if (iconIndex < 0)
			icon.SetAutoDestroy(); // Destroy icon
	}
	return iconIndex;
}
