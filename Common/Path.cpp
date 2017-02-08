#include "StdAfx.h"
#include "Path.h"
#include <shlwapi.h>
#include <shlobj.h>
#include "StringUtils.h"

#pragma comment(lib, "Shlwapi.lib")

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
: lstring(inPath ? inPath : _T(""))
{
}

Path::Path(const otherstring &inPath)
{
    lstring &thisString(*this);
    STLUtils::ChangeType(inPath, thisString);
}
Path::Path(const otherchar *inPath)
{
    otherstring inPathStr(inPath ? inPath : OTHER_T(""));
    lstring &thisString(*this);
    STLUtils::ChangeType(inPathStr, thisString);
}

Path::operator otherstring() const
{
    otherstring outStr;
    const lstring &thisString(*this);
    STLUtils::ChangeType(thisString, outStr);
    return outStr;
}

Path Path::Parent() const
{
    const size_t len(length() + 1);
	TCHAR *path = new TCHAR[len];
    _tcscpy_s(path, len, c_str());
	PathRemoveFileSpec(path);
	Path parent(path);
	delete []path;
	return parent;
}

bool Path::IsParentOf(const Path &childPath) const
{
    return !IsEmpty() && !childPath.IsEmpty() && PathCommonPrefix((LPCTSTR)*this, (LPCTSTR)childPath.Parent(), NULL) == length();
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

bool Path::IsUNC() const
{
    return ::PathIsUNC(c_str()) == TRUE;
}

bool Path::IsURL() const
{
    return PathIsURL((LPCTSTR)*this) == TRUE;
}

Path Path::GetURL() const
{
    if (IsURL())
        return *this;
    DWORD len = (DWORD)length() + MAX_PATH;
    TCHAR *newPath = new TCHAR[len];
    lstrcpy(newPath, (LPCTSTR)*this);
    UrlCreateFromPath((LPCTSTR)*this, newPath, &len, 0);
    Path outPath(newPath);
    delete[]newPath;
    return outPath;
}

Path Path::MakeUNCPath() const
{
    Path outPath(*this);
    while (!outPath.empty() && (outPath[0] == '\\' || outPath[0] == '/'))
        outPath.erase(0, 1);
    outPath = _T("\\\\") + outPath;
    return outPath;
}

Path Path::GetMachineNameFromUNCPath() const
{
    Path outPath(GetRoot());
    while (!outPath.empty() && (outPath[0] == '\\' || outPath[0] == '/'))
        outPath.erase(0, 1);
    return outPath;
}

Path Path::RemoveRoot() const
{
    Path retVal(*this);
    
    size_t start = retVal.find_first_not_of(_T("\\/"));
    if (start != npos && start > 0)
        retVal.erase(0, start);
    start = retVal.find_first_of(_T("\\/"));
    if (start != npos)
        retVal.erase(0, start + 1);
    else
        retVal.clear();
    start = retVal.find_first_not_of(_T("\\/"));
    if (start != npos && start > 0)
        retVal.erase(0, start);
    return retVal;
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
    const size_t len(length() + append.length() + 10);
	TCHAR *newPath = new TCHAR[len];
	_tcscpy_s(newPath, len, c_str());
	if (*newPath == '.') {
		if (newPath[length() - 1] != '\\'
			&& append.length() > 0 && append[0] != '\\')
            _tcscat_s(newPath, len, _T("\\"));
        _tcscat_s(newPath, len, append.c_str());
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

Path Path::TempPath()
{
    TCHAR tempDir[4 * MAX_PATH];
    tempDir[0] = 0;
    ::GetTempPath(4 * MAX_PATH, tempDir);
    return Path(tempDir);
}

Path Path::TempFile(const Path & inPath, LPCTSTR preFix, LPCTSTR ext, unsigned long startNum)
{
    Path path;

    do {
        path = inPath.Append(preFix);
        CString num;
        num.Format(_T("%u"), startNum++);
        path += num;
        path = path.RenameExtension(ext);
    } while (path.Exists());

    return path;
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
    const size_t len(length() + 4 + lstrlen(newExtn ? newExtn : _T("")));
	TCHAR *path = new TCHAR[len];
	_tcscpy_s(path, len, c_str());
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
int Path::CompareExtension(LPCTSTR extn) const
{
    if (extn == NULL)
        return -1;
    Path thisExtn(GetExtension());
    LPCTSTR thisExtnStr(thisExtn.c_str());
    if (*thisExtnStr == '.')
        ++thisExtnStr;
    if (*extn == '.')
        ++extn;
    return lstrcmpi(thisExtnStr, extn);
}
Path Path::GetRoot() const
{
    TCHAR *path = new TCHAR[length() + 1];
    TCHAR *root(path);
    lstrcpy(path, (LPCTSTR)*this);
    while (*path == '\\' || *path == '/') // Skip leading slashes
        ++path;
    while (*path && *path != '\\' && *path != '/') // Skip till slashes
        ++path;
    *path = 0;
    Path newpath(root);
    delete[]root;
    return newpath;
}

Path Path::NextComponent(unsigned *inoutpos /*= nullptr*/) const
{
    unsigned tempPos(0);
    unsigned &pos(inoutpos ? *inoutpos : tempPos);
    Path component;
    if (pos >= (unsigned)length())
        return component;
    LPCTSTR curPath(*this);
    curPath += pos;
    // Reverse back to separator
    while (pos && *curPath != '\\' && *curPath != '/') {
        --pos;
        --curPath;
    }
    // Skip separators
    while (*curPath && (*curPath == '\\' || *curPath == '/')) {
        ++curPath;
        ++pos;
    }
    unsigned startPos(0);
    // Skip till separators
    while (*curPath && curPath[startPos] != '\\' && curPath[startPos] != '/') {
        ++pos;
        ++startPos;
    }
    component = lstring(curPath, startPos);
    return component;
}

int ComparePath(LPCTSTR path1, LPCTSTR path2, bool checkPath1IsSubPath /*= false */)
{
    StringUtils::VecString resToken1, resToken2;
    StringUtils::SplitString(resToken1, path1, _T("\\/"));
    StringUtils::SplitString(resToken2, path2, _T("\\/"));
    int res = 0;
    while (!resToken1.empty() && !resToken2.empty())
    {
        res = lstrcmpi(resToken1.front().c_str(), resToken2.front().c_str());
        if (res)
            break;
        resToken1.erase(resToken1.begin());
        resToken2.erase(resToken2.begin());
    };
    if (checkPath1IsSubPath && resToken1.empty())
        res = 0;
    return res;
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

Path Path::GetModuleFilePath(HMODULE hModule /*= NULL*/)
{
    TCHAR modulePath[MAX_PATH] = { 0 };
    ::GetModuleFileName(hModule, modulePath, MAX_PATH);
    if (hModule != NULL && modulePath[0] == 0) {
        DWORD nPNSize(MAX_PATH);
        QueryFullProcessImageName(hModule, 0, modulePath, &nPNSize);
    }
    return modulePath;
}

Path Path::GetTempPath()
{
    TCHAR tempPAth[MAX_PATH] = { 0 };
    ::GetTempPath(_countof(tempPAth), tempPAth);
    return Path(tempPAth);
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
		return Parent().GetUniqueFileName(nextFileName, GetExtension().c_str(), FileNameWithoutExt().c_str());
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

bool Path::Move(const Path & inNewLocation) const
{
    return MoveFileEx(c_str(), inNewLocation.c_str(), MOVEFILE_COPY_ALLOWED) != FALSE;
}

bool Path::CopyFile(const Path & newFilePath) const
{
    return ::CopyFile(c_str(), newFilePath.c_str(), FALSE) != FALSE;
}

bool Path::OpenInExplorer() const
{
    Path path(*this);
    std::wstring cParams;
    SHELLEXECUTEINFO shExInfo = { sizeof(SHELLEXECUTEINFO) };
    BOOL isDir(IsDir() || GetExtension().IsEmpty() || PathIsNetworkPath(*this));
    if (!isDir) {
        cParams = _T("/select,") + path;
        path = Path(_T("explorer.exe"));
        shExInfo.lpParameters = cParams.c_str();
    }
    else
        shExInfo.lpVerb = _T("open");
    shExInfo.nShow = SW_SHOWDEFAULT;
    shExInfo.lpFile = path;
    BOOL l = ShellExecuteEx(&shExInfo);
    return l == TRUE;
}

HICON Path::GetIcon(bool bSmallIcon) const
{
    SHFILEINFO si = { 0 };
    UINT uFlags(SHGFI_ICON);
    if (bSmallIcon)
        uFlags |= SHGFI_SMALLICON;
    SHGetFileInfo((LPCTSTR)*this, 0, &si, sizeof(si), uFlags);
    return si.hIcon;
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
    const size_t len(lstrlen(wildCardExp) + 1);
	TCHAR *exp = new TCHAR[len];
	_tcscpy_s(exp, len, wildCardExp);
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


long long FindData::GetFileSize() const
{
    if (pFindData) {
        long long fileSize(pFindData->nFileSizeHigh);
        fileSize <<= sizeof(fileSize) << 1;
        fileSize |= pFindData->nFileSizeLow;
        return fileSize;
    }
    return Path(fullPath).GetSize();
}

Finder::Finder(PathFindCallBack fcb, void *pUserParam, LPCTSTR inpattern, LPCTSTR excludePattern)
	: mExcludePattern(excludePattern)
{
	lstring pat;
	if (inpattern)
		pat = inpattern;
	if (pat.empty())
		pat = _T("*");
    mRegExp.assign(WildCardExpToRegExp(pat.c_str()).c_str(), std::regex_constants::icase);
	if (mExcludePattern)
        mExcludeRegExp.assign(WildCardExpToRegExp(mExcludePattern).c_str(), std::regex_constants::icase);
	m_pUserParam = pUserParam;
	mFindCallBack = fcb;
}
int Finder::StartFind(const Path &dir)
{
    WIN32_FIND_DATA findFileData = {};
	HANDLE hFind = FindFirstFile(dir.Append(_T("*")).c_str(), &findFileData);
	int c = 0;
    Path srcDir(dir);
    bool bSrcIsFile((!srcDir.IsDir() && srcDir.Exists()));
    if (bSrcIsFile) {
        _tcscpy_s(findFileData.cFileName, srcDir.FileName().c_str());
        findFileData.dwFileAttributes = GetFileAttributes(srcDir.c_str());
        LARGE_INTEGER li = {0};
        li.QuadPart = (srcDir.GetSize());
        findFileData.nFileSizeLow = li.LowPart;
        findFileData.nFileSizeHigh = li.HighPart;
        srcDir.GetFileTime(&findFileData.ftCreationTime, &findFileData.ftLastAccessTime, &findFileData.ftLastWriteTime);
        srcDir = srcDir.Parent();
    }
	if (hFind != INVALID_HANDLE_VALUE || bSrcIsFile) {
		do {
			if (lstrcmp(findFileData.cFileName, _T(".")) && lstrcmp(findFileData.cFileName, _T(".."))) {
				lstring file = srcDir.Append(findFileData.cFileName);
                bool bMatched = std::regex_match(findFileData.cFileName, mRegExp);
				if (mExcludePattern)
                    bMatched = bMatched && std::regex_match(findFileData.cFileName, mExcludeRegExp);
                FindData fd(&findFileData, file, bMatched);
				int fcbRetVal(mFindCallBack(fd, m_pUserParam));
				if (fcbRetVal == FCBRV_ABORT)
					break;
				if ((findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					&& fcbRetVal != FCBRV_SKIPDIR) {
					c += StartFind(file);
                    fd.pFindData = NULL;
                    fd.fileMatched = false;
					mFindCallBack(fd, m_pUserParam);
				}
				if (bMatched)
					c++;
			}
		} while (hFind != INVALID_HANDLE_VALUE && FindNextFile(hFind, &findFileData));
        if (hFind != INVALID_HANDLE_VALUE)
		    FindClose(hFind);
        hFind = INVALID_HANDLE_VALUE;
	}
	return c;
}
