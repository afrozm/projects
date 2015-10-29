#pragma once

class CIcon {
public:
	CIcon(HICON hIcon = NULL, bool bAutoDestroy = false);
	explicit CIcon(CIcon&);
	~CIcon();
	void SetIcon(HICON hIcon);
	HICON GetIcon() const {return m_hIcon;}
	operator HICON() const {return GetIcon();}
	BOOL LoadIcon(LPCTSTR lpIconName, HINSTANCE hIns = NULL);
	//CIcon operator=(const CIcon& other) {return CIcon(other);}
	bool operator==(const CIcon& other) {return m_hIcon == other.GetIcon();}
	CPoint GetSize() const;
	void SetAutoDestroy(bool bAuto = true) {mbAutoDestroy=bAuto;}
	bool IsAutoDestroy() const {return mbAutoDestroy;}
protected:
	HICON m_hIcon;
	bool mbAutoDestroy;
};

class FileIconMgr {
public:
	static FileIconMgr& GetInstance();
	int GetFolderIconIndex();
	int GetIconIndex(const CString& filePath);
	CImageList* GetImageList() const {return m_pImageList;};
	void SetImageList(CImageList *pImageList);
private:
	typedef CMap<CString, LPCTSTR, int, int&> MapStrVsInt;
	MapStrVsInt mExtVsIconIndex;
	CImageList *m_pImageList;
	CImageList mPrivateImageList;
	CArray<CIcon> mFileExtIcons;

	FileIconMgr();
	int InsertIcon(HICON hIcon);
	void InitKnownExtnList();
	int mFolderIconIndex;
};
