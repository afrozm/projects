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

Path Path::GetUniqueFileName(int &statNum, LPCTSTR ext, LPCTSTR prefix)
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