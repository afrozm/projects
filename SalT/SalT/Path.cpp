#include "StdAfx.h"
#include "Path.h"
#include <shlwapi.h>
#include <shlobj.h>

Path::Path()
{
}
Path::Path(const lstring &inPath)
	: lstring(inPath)
{
}
Path::Path(const Path& p)
: lstring(p.c_str())
{
}
Path::Path(LPCTSTR inPath)
: lstring(inPath)
{
}
Path Path::Parent() const
{
	TCHAR *path = new TCHAR[length()+1];
	lstrcpy(path, c_str());
	PathRemoveFileSpec(path);
	Path parent(path);
	delete []path;
	return parent;
}
Path Path::FileName() const
{
	return Path(PathFindFileName(c_str()));
}

Path Path::FileNameWithoutExt() const
{
	return FileName().RenameExtension();
}

bool Path::Exists() const
{
	return PathFileExists(c_str()) == TRUE;
}
bool Path::IsDir() const
{
	DWORD fa = GetFileAttributes(c_str());
	return fa != INVALID_FILE_ATTRIBUTES && (fa & FILE_ATTRIBUTE_DIRECTORY);
}
bool Path::CreateDir() const
{
	bool bSuccess(false);
	if (Exists()) {
		bSuccess = IsDir();
	}
	else {
		bSuccess = SHCreateDirectoryEx(NULL, c_str(), NULL) == ERROR_SUCCESS;
	}
	return bSuccess;
}
Path Path::Append(const Path &append) const
{
	TCHAR *newPath = new TCHAR[length()+append.length()+10];
	lstrcpy(newPath, c_str());
	if (*newPath == '.') {
		if (newPath[length() - 1] != '\\'
			&& append.length() > 0 && append[0] != '\\')
			lstrcat(newPath, _T("\\"));
		lstrcat(newPath, append.c_str());
	}
	else
		PathAppend(newPath, append.c_str());
	Path appended(newPath);
	delete []newPath;
	return appended;
}
Path Path::CurrentDir()
{
	TCHAR curDir[4*MAX_PATH];
	curDir[0] = 0;
	GetCurrentDirectory(4*MAX_PATH, curDir);
	return Path(curDir);
}
bool Path::GetFileTime(LPFILETIME lpCreationTime, LPFILETIME lpLastAccessTime, LPFILETIME lpLastWriteTime) const
{
	bool bSuccess(false);
	HANDLE hFile(CreateFile(c_str(), GENERIC_READ, FILE_SHARE_READ, NULL,
		OPEN_EXISTING, 0, NULL));
	if (hFile != INVALID_HANDLE_VALUE) {
		bSuccess = ::GetFileTime(hFile, lpCreationTime, lpLastAccessTime, lpLastWriteTime) == TRUE;
		CloseHandle(hFile);
	}
	return bSuccess;
}
Path Path::RenameExtension(LPCTSTR newExtn) const
{
	TCHAR *path = new TCHAR[length()+4+lstrlen(newExtn ? newExtn : _T(""))];
	lstrcpy(path, c_str());
	PathRemoveExtension(path);
	if (newExtn != NULL && newExtn[0] != 0) {
		PathAddExtension(path, newExtn);
	}
	Path newpath(path);
	delete []path;
	return newpath;
}
Path Path::GetExtension() const
{
	return Path(PathFindExtension(c_str()));
}
int Path::Compare(const Path &p) const
{
	Path p1(MakeFullPath());
	Path p2(p.MakeFullPath());
	return lstrcmpi(p1.c_str(), p2.c_str());
}
Path Path::GetRoot() const
{
	TCHAR root[MAX_PATH];
	PathCanonicalize(root, c_str());
	PathStripToRoot(root);
	return Path(root);
}

bool operator == (const Path& p1, const Path& p2)
{
	return p1.Compare(p2) == 0;
}
bool operator != (const Path& p1, const Path& p2)
{
	return p1.Compare(p2) != 0;
}
Path Path::GetSpecialFolderPath(int inFolderID, bool inCreate)
{
	Path outPath;
	bool bRet(false);
	LPITEMIDLIST lpItemID(NULL);
	HRESULT hret = SHGetFolderLocation(NULL, inFolderID, NULL, 0, &lpItemID);
	if (SUCCEEDED(hret))
	{
		wchar_t pathValue[MAX_PATH];
		if (SHGetPathFromIDList(lpItemID, pathValue))
		{
			//::PathAddBackslash(pathValue);
			outPath.assign(pathValue);
			bRet = true;
		}
		CoTaskMemFree(lpItemID);
	}
	else
	{
		wchar_t pathValue[MAX_PATH];
		if (SUCCEEDED(SHGetFolderPath(NULL, inFolderID, NULL, SHGFP_TYPE_DEFAULT, pathValue)))
		{
			//::PathAddBackslash(pathValue);
			outPath.assign(pathValue);
			bRet = true;
		}
	}
	if (!outPath.empty())
	{
		if (outPath[outPath.size()-1] == L'\\')
			outPath.resize(outPath.size()-1);

		if (inCreate && !::PathFileExists(outPath.c_str()))
		{
			bRet = (ERROR_SUCCESS == ::SHCreateDirectoryEx(NULL, outPath.c_str(), NULL));
		}
	}
	return outPath;
}
bool Path::IsRelativePath() const
{
	return ::PathIsRelative(c_str()) == TRUE;
}
Path Path::MakeFullPath() const
{
	Path outPath(*this);
	if (IsRelativePath()) {
		outPath = CurrentDir().Append(*this);
	}
	outPath = outPath.Canonicalize();
	return outPath;
}
Path Path::Canonicalize() const
{
	TCHAR cp[MAX_PATH];
	if (PathCanonicalize(cp, c_str())) {
		return Path(cp);
	}
	return *this;
}
bool Path::IsPreFixOf(const Path &preFixPath) const
{
	Path curPath(MakeFullPath());
	Path preFix(preFixPath.MakeFullPath());
	return PathIsPrefix(preFix.c_str(), curPath.c_str()) == TRUE;
}
bool
Path::DeleteDirectory() const
{
	bool bSuccess = ::RemoveDirectory(c_str()) == TRUE;
	if (!bSuccess && GetLastError() == ERROR_ACCESS_DENIED) {
		// Try to remove read only attribute
		SetFileAttributes(FILE_ATTRIBUTE_NORMAL);
		bSuccess = ::RemoveDirectory(c_str()) == TRUE;
	}
	return bSuccess;
}

bool 
Path::SetFileAttributes(DWORD inAttribute ) const
{
	return (TRUE == ::SetFileAttributes(c_str(),inAttribute));
}

bool 
Path::DeleteDirectoryRecursive() const
{
	WIN32_FIND_DATA findFileData;
	Path fsPath(*this);
	fsPath = fsPath.Append(L"*");
	HANDLE hFind = FindFirstFile(fsPath.c_str(), &findFileData);
	bool bRet = true;
	if (hFind != INVALID_HANDLE_VALUE && bRet) {
		do {
			if (lstrcmp(findFileData.cFileName, _T(".")) && lstrcmp(findFileData.cFileName, _T(".."))) {
				fsPath = Append(findFileData.cFileName);
				if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
					bRet = fsPath.DeleteDirectoryRecursive();
				}
				else {
					bRet = fsPath.DeleteFile();
				}
			}
		} while (FindNextFile(hFind, &findFileData));
		FindClose(hFind);
	}
	bRet = DeleteDirectory();
	return bRet;
}
bool
Path::DeleteFile() const
{
	BOOL bRet = ::DeleteFile(c_str());
	if (!bRet) {
		bRet = SetFileAttributes(FILE_ATTRIBUTE_NORMAL);
		bRet = ::DeleteFile(c_str());
	}
	return (TRUE == bRet);
}
lstring IntToString(int no)
{
	TCHAR number[256];
	_stprintf_s(number, sizeof(number)/sizeof(TCHAR), _T("%d"), no);
	return lstring(number);
}

Path Path::GetUniqueFileName(int &statNum, LPCTSTR ext, LPCTSTR prefix) const
{
	Path fileName;
	while (1) {
		fileName.clear();
		if (prefix)
			fileName = prefix;
		fileName += IntToString(statNum);
		if (ext) {
			if (*ext != '.')
				fileName += lstring(_T("."));
			fileName += lstring(ext);
		}
		fileName = Append(fileName);
		++statNum;
		if (!fileName.Exists())
			break;
	}
	return fileName;
}

Path Path::GetNextUniqueFileName() const
{
	if (Exists()) {
		int nextFileName(0);
		return GetUniqueFileName(nextFileName, GetExtension().c_str(), FileNameWithoutExt().c_str());
	}
	return *this;
}

static int FindCallBack_DeleteFolder(FindData& findData, void *pUserParam)
{
	int &nFilesDeleted(*(int*)pUserParam);
	if (findData.pFindData == NULL) {// exit of dir
		if (RemoveDirectory(findData.fullPath.c_str()))
			++nFilesDeleted;
	}
	else if (findData.fileMatched && !(findData.pFindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
		if (DeleteFile(findData.fullPath.c_str()))
			++nFilesDeleted;
	}
	return 0;
}
int Path::Delete(bool bRecusrive /*= false*/, LPCTSTR pattern /*= NULL*/, LPCTSTR excludePattern /*= NULL*/) const
{
	int nFilesDeleted(0);
	if (IsDir()) {
		if (bRecusrive) {
			Finder rd(FindCallBack_DeleteFolder, &nFilesDeleted, pattern, excludePattern);
			rd.StartFind(*this);
		}
		if (RemoveDirectory(c_str()))
			++nFilesDeleted;
	}
	else if (::DeleteFile(c_str()))
		++nFilesDeleted;
	return nFilesDeleted;
}

static int FindCallBack_PathGetSize(FindData &findData, void *pUserParam)
{
	if (findData.pFindData != NULL) {
		INT64 fileSize(findData.pFindData->nFileSizeHigh);
		fileSize <<= 32;
		fileSize |= findData.pFindData->nFileSizeLow;
		*((INT64*)pUserParam) += fileSize;
	}
	return 0;
}
INT64 Path::GetSize() const
{
	INT64 fileSize(-1);
	if (IsDir()) {
		fileSize = 0;
		Path root(GetRoot());
		if (root == *this) {
			ULARGE_INTEGER li = { 0 };
			GetDiskFreeSpaceEx(root.c_str(), NULL, &li, NULL);
			fileSize = li.QuadPart;
		}
		else
			Finder(FindCallBack_PathGetSize, &fileSize).StartFind(c_str());
	}
	else {
		HANDLE hFile(CreateFile(c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL));
		if (hFile != INVALID_HANDLE_VALUE) {
			LARGE_INTEGER li = { 0 };
			GetFileSizeEx(hFile, &li);
			CloseHandle(hFile);
			fileSize = li.QuadPart;
		}
	}
	return fileSize;
}

bool Path::CreateShortCut(const Path &shortCutPath, LPCTSTR pszTargetargs /*= NULL*/, LPCTSTR pszDescription /*= NULL*/, int iShowmode /*= 0*/, LPCTSTR pszCurdir /*= NULL*/, LPCTSTR pszIconfile /*= NULL*/, int iIconindex /*= 0*/) const
{
	HRESULT       hRes;                  /* Returned COM result code */
	IShellLink*   pShellLink;            /* IShellLink object pointer */
	IPersistFile* pPersistFile;          /* IPersistFile object pointer */
	LPCTSTR pszTargetfile(c_str()), pszLinkfile(shortCutPath.c_str());
	hRes = E_INVALIDARG;
	bool bCoInitializedCalled(false);
	if (
		(pszTargetfile != NULL) && (lstrlen(pszTargetfile) > 0) &&
		(pszLinkfile != NULL) && (lstrlen(pszLinkfile) > 0)
		)
	{
		hRes = CoCreateInstance(
			CLSID_ShellLink,     /* pre-defined CLSID of the IShellLink
								 object */
			NULL,                 /* pointer to parent interface if part of
								  aggregate */
			CLSCTX_INPROC_SERVER, /* caller and called code are in same
								  process */
			IID_IShellLinkW,      /* pre-defined interface of the
								  IShellLink object */
			(LPVOID*)&pShellLink);         /* Returns a pointer to the IShellLink
										   object */
		if (!SUCCEEDED(hRes)) {
			CoInitialize(NULL);
			bCoInitializedCalled = true;
			hRes = CoCreateInstance(
				CLSID_ShellLink,     /* pre-defined CLSID of the IShellLink
									 object */
				NULL,                 /* pointer to parent interface if part of
									  aggregate */
				CLSCTX_INPROC_SERVER, /* caller and called code are in same
									  process */
				IID_IShellLinkW,      /* pre-defined interface of the
									  IShellLink object */
				(LPVOID*)&pShellLink);         /* Returns a pointer to the IShellLink
											   object */
		}
		if (SUCCEEDED(hRes))
		{
			/* Set the fields in the IShellLink object */
			hRes = pShellLink->SetPath(pszTargetfile);
			if (pszTargetargs != NULL)
				hRes = pShellLink->SetArguments(pszTargetargs);
			if (lstrlen(pszDescription) > 0)
			{
				hRes = pShellLink->SetDescription(pszDescription);
			}
			if (iShowmode > 0)
			{
				hRes = pShellLink->SetShowCmd(iShowmode);
			}
			if (pszCurdir != NULL && lstrlen(pszCurdir) > 0)
			{
				hRes = pShellLink->SetWorkingDirectory(
					pszCurdir);
			}
			if (pszIconfile != NULL && lstrlen(pszIconfile) > 0 && iIconindex >= 0)
			{
				hRes = pShellLink->SetIconLocation(
					pszIconfile, iIconindex);
			}

			/* Use the IPersistFile object to save the shell link */
			hRes = pShellLink->QueryInterface(
				/* existing IShellLink object */
				IID_IPersistFile,         /* pre-defined interface of the
										  IPersistFile object */
				(void **)&pPersistFile);            /* returns a pointer to the
													IPersistFile object */
			if (SUCCEEDED(hRes))
			{
				hRes = pPersistFile->Save(pszLinkfile, TRUE);
				pPersistFile->Release();
			}
			pShellLink->Release();
		}
	}
	if (bCoInitializedCalled)
		CoUninitialize();
	return SUCCEEDED(hRes);
}

ULONGLONG Path::GetFileTime(FileTimeType fileType) const
{
	FILETIME fileTime[3] = { 0 };
	GetFileTime(fileTime, fileTime + 1, fileTime + 2);
	if (fileType < CreationTime || fileType > ModifiedTime)
		fileType = CreationTime;
	ULONGLONG outFileTime(fileTime[fileType].dwHighDateTime);
	outFileTime <<= 32;
	outFileTime |= fileTime[fileType].dwLowDateTime;
	return outFileTime;
}

lstring WildCardToRegExp(LPCTSTR wildCard)
{
	LPTSTR regExp = new TCHAR[6 * lstrlen(wildCard) + 1];
	unsigned len = 0;

	while (*wildCard) {
		TCHAR extraCharToAdd = 0;

		switch (*wildCard) {
		case '*':
			extraCharToAdd = '.';
			break;
		case '.':
			extraCharToAdd = '\\';
			break;
		}
		if (extraCharToAdd)
			regExp[len++] = extraCharToAdd;
		if (_istalpha(*wildCard)) {
			regExp[len++] = '[';
			regExp[len++] = _totlower(*wildCard);
			regExp[len++] = _totupper(*wildCard++);
			regExp[len++] = ']';
		}
		else
			regExp[len++] = *wildCard++;
	}
	regExp[len] = 0;
	lstring regExpStr(regExp);

	delete[] regExp;

	return regExpStr;
}

lstring WildCardExpToRegExp(LPCTSTR wildCardExp)
{
	TCHAR *exp = new TCHAR[lstrlen(wildCardExp) + 1];
	lstrcpy(exp, wildCardExp);
	LPTSTR nexttoken(NULL);
	LPTSTR token = _tcstok_s(exp, _T(";"), &nexttoken);
	lstring regExp;
	while (token != NULL) {
		regExp += _T("(") + WildCardToRegExp(token) + _T(")");
		token = _tcstok_s(NULL, _T(";"), &nexttoken);
		if (token != NULL) {
			regExp += _T("|");
		}
	}
	return regExp;
}

Finder::Finder(FindCallBack fcb, void *pUserParam, LPCTSTR inpattern, LPCTSTR excludePattern)
	: mExcludePattern(excludePattern)
{
	lstring pat;
	if (inpattern)
		pat = inpattern;
	if (pat.empty())
		pat = _T("*");
	mRegExp.Parse(WildCardExpToRegExp(pat.c_str()).c_str(), FALSE);
	if (mExcludePattern)
		mExcludeRegExp.Parse(WildCardExpToRegExp(mExcludePattern).c_str(), FALSE);
	m_pUserParam = pUserParam;
	mFindCallBack = fcb;
}
int Finder::StartFind(const lstring &dir)
{
	WIN32_FIND_DATA findFileData;
	HANDLE hFind = FindFirstFile((dir + _T("\\*")).c_str(), &findFileData);
	int c = 0;
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if (lstrcmp(findFileData.cFileName, _T(".")) && lstrcmp(findFileData.cFileName, _T(".."))) {
				CAtlREMatchContext<> mc;
				lstring file = dir + _T("\\");
				file += findFileData.cFileName;
				bool bMatched = mRegExp.Match(findFileData.cFileName, &mc) == TRUE;
				if (mExcludePattern)
					bMatched = bMatched && mExcludeRegExp.Match(findFileData.cFileName, &mc) == FALSE;
				int fcbRetVal(mFindCallBack(FindData(&findFileData, file, bMatched), m_pUserParam));
				while (fcbRetVal == FCBRV_STOP) {
					fcbRetVal = mFindCallBack(FindData(&findFileData, file, bMatched), m_pUserParam);
					Sleep(10);
				}
				if (fcbRetVal == FCBRV_ABORT)
					break;
				if ((findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					&& fcbRetVal != FCBRV_SKIPDIR) {
					c += StartFind(file);
					mFindCallBack(FindData(NULL, file, false), m_pUserParam);
				}
				if (bMatched)
					c++;
			}
		} while (FindNextFile(hFind, &findFileData));
		FindClose(hFind);
	}
	return c;
}