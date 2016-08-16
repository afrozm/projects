#include "StdAfx.h"
#include "Finder.h"
#include "LoggerFactory.h"
#include "unzip/Unzipper.h"

LPCTSTR FileNameFromPath(LPCTSTR path)
{
	LPCTSTR pathStart = path;
	path += lstrlen(path);
	while (path > pathStart) {
		if (*path == '\\' || *path == '/') {
			path++;
			break;
		}
		--path;
	}
	return path;
}

CString RootNameFromPath(LPCTSTR path)
{
	CString root;
	int len = lstrlen(path);

	if (len > 1) {
		len = 2;
		while (path[len] && !(path[len] == '\\' || path[len] == '/' || path[len] == ':')) {
			len++;
		}
		root.SetString(path, len);
	}
	return root;
}

CString ParentDirectoryFromPath(LPCTSTR path)
{
	LPTSTR parentPath = new TCHAR[lstrlen(path)+1];
	lstrcpy(parentPath, path);
	LPTSTR startPath = parentPath;
	path = startPath;
	parentPath += lstrlen(parentPath);
	if (startPath[0] == '\\' && startPath[1] == '\\') // Network path
		startPath += 2;
	else if (startPath[1] == ':')
		startPath += 2;
	/* Skip trailing \ */
	while (parentPath-- > startPath && *parentPath == '\\');
	/* Skip till \ */
	while (parentPath-- > startPath && *parentPath != '\\');
	*parentPath = 0;

	CString parentPathStr(path);

	delete[] path;

	return parentPathStr;
}

LONGLONG CFileFindEx::GetFileSize() const
{
	LONGLONG size = GetLength();
	if (size == 0 && IsDirectory()) {
		size = -1;
	}
	return size;
}
CString CFileFindEx::GetFileName() const
{
	CString fileName(CFileFind::GetFileName());
	if (fileName.IsEmpty()) {
		if (m_pFoundInfo != NULL)
			fileName = ((LPWIN32_FIND_DATA) m_pFoundInfo)->cAlternateFileName;
		if (fileName.IsEmpty())
			fileName = _T("???");
	}
	return fileName;
}
static BOOL PathIsInZip(const Path &inDir)
{
	BOOL bFound(FALSE);
	Path childPath, dir(inDir);
	while (!dir.IsEmpty() && !dir.Exists()) {
		childPath = dir.FileName().Append(childPath);
		dir = dir.Parent();
	}
	if (dir.Exists() && !dir.IsDir()) {
		CZipFileFinder zf;
		bFound = zf.FindFile(dir);
		if (bFound && !childPath.IsEmpty())
			bFound = zf.JumpToChildPath(childPath);
	}
	return bFound;
}
BOOL CFileFindEx::IsDirectory() const {
	BOOL isDir = __super::IsDirectory();
	if (!isDir && mbSearchZip) {
		isDir = PathIsInZip(Path(GetFilePath()));
		if (isDir) {
			if (m_pFoundInfo != NULL)
				((LPWIN32_FIND_DATA) m_pFoundInfo)->dwFileAttributes |= FILE_ATTRIBUTE_DIRECTORY;
		}
	}
	return isDir;
}
BOOL CFileFindEx::HasSize() const
{
	return !__super::IsDirectory();
}
BOOL CFileFindEx::JumpToChildPath(LPCTSTR childPath)
{
	BOOL bSuccess(TRUE);
	Path cPath(childPath);
	while (!cPath.IsEmpty() && bSuccess) {
		Path root(cPath.GetRoot());
		if (GetFileName() == root) {
			cPath = cPath.RemoveRoot();
			bSuccess = FindFile(Path(GetRoot()).Append(root.Append(Path(_T("*")))));
		}
		else {
			bSuccess = FindNextFile();
		}
	}
	return bSuccess;
}

ULONGLONG CZipFileFinder::GetLength() const
{
	UZ_FileInfo *pUzFileInfo((UZ_FileInfo *)m_pFoundInfo);
	return pUzFileInfo->dwUncompressedSize;
}
CString CZipFileFinder::GetFileName() const
{
	UZ_FileInfo *pUzFileInfo((UZ_FileInfo *)m_pFoundInfo);
	return Path(pUzFileInfo->szFileName).FileName();
}
CString CZipFileFinder::GetFilePath() const
{
	UZ_FileInfo *pUzFileInfo((UZ_FileInfo *)m_pFoundInfo);
	return Path(m_strRoot).Append(Path(pUzFileInfo->szFileName));
}
CString CZipFileFinder::GetFileTitle() const
{
	return Path(GetFileName()).RenameExtension();
}
BOOL CZipFileFinder::GetLastWriteTime(FILETIME* pTimeStamp) const
{
	CTime t;
	BOOL b = GetLastWriteTime(t);
	SYSTEMTIME st;
	t.GetAsSystemTime(st);
	SystemTimeToFileTime(&st, pTimeStamp);
	return b;
}
BOOL CZipFileFinder::GetLastAccessTime(FILETIME* pTimeStamp) const
{
	CTime t;
	BOOL b = GetLastAccessTime(t);
	SYSTEMTIME st;
	t.GetAsSystemTime(st);
	SystemTimeToFileTime(&st, pTimeStamp);
	return b;
}
BOOL CZipFileFinder::GetCreationTime(FILETIME* pTimeStamp) const
{
	CTime t;
	BOOL b = GetCreationTime(t);
	SYSTEMTIME st;
	t.GetAsSystemTime(st);
	SystemTimeToFileTime(&st, pTimeStamp);
	return b;
}
BOOL CZipFileFinder::GetLastWriteTime(CTime& refTime) const
{
	UZ_FileInfo *pUzFileInfo((UZ_FileInfo *)m_pFoundInfo);
	CTime t(pUzFileInfo->dwDosDate);
	refTime = t;
	return TRUE;
}
BOOL CZipFileFinder::MatchesMask(DWORD dwMask) const
{
	UZ_FileInfo *pUzFileInfo((UZ_FileInfo *)m_pFoundInfo);
	return (pUzFileInfo->dwExternalAttrib & dwMask) == dwMask;
}
BOOL CZipFileFinder::IsDots() const
{
	CString fileName(GetFileName());
	return fileName == _T(".") || fileName == _T("..");
}
BOOL CZipFileFinder::IsDirectory() const
{
	UZ_FileInfo *pUzFileInfo((UZ_FileInfo *)m_pFoundInfo);
	return pUzFileInfo->bFolder;
}
BOOL CZipFileFinder::HasSize() const
{
	UZ_FileInfo *pUzFileInfo((UZ_FileInfo *)m_pFoundInfo);
	return !pUzFileInfo->bFolder;
}

void CZipFileFinder::CloseContext()
{
	if (m_pFoundInfo)
		delete (UZ_FileInfo*)m_pFoundInfo;
	if (m_pFileInfo)
		delete (UZ_FileInfo*)m_pFileInfo;
	m_pFoundInfo = NULL;
	m_pNextInfo = NULL;
    m_pFileInfo = NULL;
	CUnzipper *pUnzipper((CUnzipper *)m_hContext);
	if (pUnzipper != NULL) {
		delete pUnzipper;
	}
	m_hContext = NULL;
    mRootItems.RemoveAll();
}
BOOL CZipFileFinder::JumpToChildPath(LPCTSTR childPath)
{
	mChildPath = Path(childPath);
	CUnzipper *pUnzipper((CUnzipper *)m_hContext);
	if (pUnzipper != NULL)
		pUnzipper->GotoFirstFile();
	return FindNext();
}

BOOL CZipFileFinder::FindFile(LPCTSTR pstrName, DWORD /*dwUnused*/)
{
	Close();
	CUnzipper *pUnzipper = new CUnzipper(pstrName);
	m_hContext = (HANDLE)pUnzipper;
	BOOL bSuccess(pUnzipper->IsOpen());
	if (bSuccess) {
		m_strRoot = pstrName;
		bSuccess = pUnzipper->GotoFirstFile();
		if (bSuccess) {
			m_pFoundInfo = new UZ_FileInfo;
			m_pFileInfo= new UZ_FileInfo;
			m_pNextInfo = m_pFileInfo;
			bSuccess = FindNext();
		}
	}
	if (!bSuccess) {
		Close();
	}
	return bSuccess;
}
BOOL CZipFileFinder::FindNext()
{
	CUnzipper *pUnzipper((CUnzipper *)m_hContext);
	UZ_FileInfo *pUzFileInfo((UZ_FileInfo *)m_pNextInfo);
	mbFound = false;
	while (m_pNextInfo != NULL && !mbFound) {
		BOOL bSuccess = bSuccess = pUnzipper->GetFileInfo(*pUzFileInfo);
		if (bSuccess) {
			bSuccess = pUnzipper->GotoNextFile();
			if (!bSuccess) {
				m_pNextInfo = NULL;
			}
			if (!mbRecrursive) {
				mbFound = mChildPath == Path(pUzFileInfo->szFileName).Parent();
                if (!mbFound) {
                    CString rootItem(Path(pUzFileInfo->szFileName).GetRoot());
                    if (mRootItems.Find(rootItem) < 0) {
                        mRootItems.InsertUnique(rootItem);
                        pUzFileInfo->bFolder = true;
                        _tcscpy_s(pUzFileInfo->szFileName, rootItem);
                        mbFound = true;
                    }
                }
			}
			else mbFound = mChildPath.IsEmpty() || mChildPath.IsParentOf(Path(pUzFileInfo->szFileName));
		}
	}
	return mbFound;
}
BOOL CZipFileFinder::FindNextFile()
{
	CUnzipper *pUnzipper((CUnzipper *)m_hContext);
    UNREFERENCED_PARAMETER(pUnzipper);
	BOOL bSuccess = FALSE;
	if (mbFound)
		memcpy(m_pFoundInfo, m_pFileInfo, sizeof(UZ_FileInfo));
	if (m_pNextInfo != NULL) {
		bSuccess = FindNext();
	}
	return bSuccess;
}

/*
CRegExpMatcher::CRegExpMatcher(LPCTSTR lpExpression)
{
	CString exp(lpExpression);
	CString token;
	int curPos = 0;
	int state = 0;
	do {
		token = exp.Tokenize(_T("\\"), curPos);
		CString regExp;
		bool bIsDirectory = exp[curPos-1]=='\\';
		CAtlRegExp<> mRegDirectoryExp;
		CAtlRegExp<> mRegFileExp;
		while (token != _T("")) {
			regExp += _T("(") + WildCardToRegExp(token) + _T(")");
			token = exp.Tokenize(_T(";\\"), curPos);
			if (token != _T("")) {
				regExp +=_T("|");
			}
		}
		mRegExp.Parse(regExp);
	}

}
*/
CFinder::CFinder(LPCTSTR lpExpression /* = NULL */, FindCallBack fcb /* = NULL */, bool recurse /* = true */, void *pUserParam /* = NULL */, MatchType mt /* = WildCard */)
: mFindCallBack(fcb), m_bRecursive(recurse), m_pUserParam(pUserParam), m_bAborted(false), m_pStringMatcher(NULL)
{
	switch (mt)
	{
	case CFinder::WildCard:
	case CFinder::RegularExp:
		m_pStringMatcher = new CRegExpMatcher(lpExpression, mt == RegularExp);
		break;
	case CFinder::Phonetic:
		m_pStringMatcher = new CPhoneticStringMatcher(lpExpression);
		break;
	default:
		break;
	}
}
CFinder::~CFinder(void)
{
	if (m_pStringMatcher != NULL)
		delete m_pStringMatcher;
	m_pStringMatcher = NULL;
}
static void MakeLongPath(CString &path)
{
	if (path[2] != '?') {
		if (path[0] == '\\' && path[1] == '\\') {
			path.Delete(0);
			path = _T("\\\\?\\UNC") + path;
		}
		else 
			path = _T("\\\\?\\") + path;
	}
}
int CFinder::Find(CString lpDirectory, bool bSearchZip)
{
	CFileFindEx cf(bSearchZip);
	CZipFileFinder zf;
	zf.SetRecursive(m_bRecursive);
	CFileFindEx *pcf(&cf);
	bool bIsZip(false);
	BOOL bFound(FALSE);
	if (lpDirectory.GetLength() > MAX_PATH) {
		return 0;
	}
	int count = 0;
	try {
		bFound = pcf->FindFile(lpDirectory + _T("\\*"));
		if (!bFound && bSearchZip) {
			Path dir(lpDirectory);
			Path childPath;
			while (!dir.IsEmpty() && !dir.Exists()) {
				childPath = dir.FileName().Append(childPath);
				dir = dir.Parent();
			}
			if (dir.Exists() && !dir.IsDir()) {
				pcf = &zf;
				bFound = pcf->FindFile(dir);
				bIsZip = bFound == TRUE;
				if (bFound && !childPath.IsEmpty())
					bFound = pcf->JumpToChildPath(childPath);
				bSearchZip = false;
			}
		}
		pcf->SetDirectory(lpDirectory);
		while (bFound && !m_bAborted) {
			bFound = pcf->FindNextFile();
			if (pcf->IsDots())
				continue;
			bool bRecursive = m_bRecursive;
			bool bMatched(true);
			if (m_pStringMatcher) {
				bMatched = m_pStringMatcher->Match(pcf->GetFileName());
				pcf->SetMatchWeight(m_pStringMatcher->GetMatchWeight());
			}
			// Call callback
			switch (DoFindCallBack(pcf, bMatched, m_pUserParam)) {
			case FCB_ABORT:
				m_bAborted = true;
				break;
			case FCB_DORECURSIVE:
				bRecursive = true;
				break;
			case FCB_NORECURSIVE:
				bRecursive = false;
				zf.SetRecursive(bRecursive);
				break;
			}
			BOOL bIsDir(pcf->IsDirectory());
			if (bRecursive && !m_bAborted && 
				(bIsDir&&!bIsZip || bSearchZip)) {
					count += Find(pcf->GetFilePath(), bSearchZip);
			}
			count++;
		}
	}
	catch (...) {
		bFound = FALSE;
	}
	return count;
}

int CFinder::DoFindCallBack( CFileFindEx* pFindFile, bool fileMatched, void *pUserParam )
{
	if (mFindCallBack)
		return mFindCallBack(pFindFile, fileMatched, pUserParam);
	return 0;
}
