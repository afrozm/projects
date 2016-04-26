#include "StdAfx.h"
#include "Path.h"
#include <shlwapi.h>
#include <shlobj.h>

Path::Path()
{
}
Path::Path(const CString &inPath)
	: CString(inPath)
{
}
Path Path::Parent() const
{
	TCHAR *path = new TCHAR[GetLength()+1];
	lstrcpy(path, (LPCTSTR)*this);
	PathRemoveFileSpec(path);
	Path parent(path);
	delete []path;
	return parent;
}
Path Path::FileName() const
{
	return Path(PathFindFileName((LPCTSTR)*this));
}
bool Path::Exists() const
{
	return PathFileExists((LPCTSTR)*this) == TRUE;
}
bool Path::IsDir() const
{
	DWORD fileAttribute(GetFileAttributes((LPCTSTR)*this));
	if (fileAttribute != INVALID_FILE_ATTRIBUTES)
		return (fileAttribute & FILE_ATTRIBUTE_DIRECTORY) != 0;
	return false;
}
bool Path::CreateDir() const
{
	bool bSuccess(false);
	if (Exists()) {
		bSuccess = IsDir();
	}
	else {
		bSuccess = SHCreateDirectoryEx(NULL, (LPCTSTR)*this, NULL) == ERROR_SUCCESS;
	}
	return bSuccess;
}
Path Path::Append(const Path &append) const
{
	TCHAR *newPath = new TCHAR[GetLength()+append.GetLength()+10];
	lstrcpy(newPath, (LPCTSTR)*this);
	PathAppend(newPath, append);
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
	TCHAR tempDir[4*MAX_PATH];
	tempDir[0] = 0;
	GetTempPath(4*MAX_PATH, tempDir);
	return Path(tempDir);
}
Path Path::TempFile(const Path &inPath, const CString& preFix, const CString &ext, unsigned long startNum)
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
	HANDLE hFile(CreateFile((LPCTSTR)*this, GENERIC_READ, FILE_SHARE_READ, NULL,
		OPEN_EXISTING, 0, NULL));
	if (hFile != INVALID_HANDLE_VALUE) {
		bSuccess = ::GetFileTime(hFile, lpCreationTime, lpLastAccessTime, lpLastWriteTime) == TRUE;
		CloseHandle(hFile);
	}
	return bSuccess;
}
Path Path::RenameExtension(LPCTSTR newExtn) const
{
	TCHAR *path = new TCHAR[GetLength()+4+lstrlen(newExtn ? newExtn : _T(""))];
	lstrcpy(path, (LPCTSTR)*this);
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
	return Path(PathFindExtension(*this));
}
Path Path::GetRoot() const
{
	TCHAR *path = new TCHAR[GetLength()+1];
	TCHAR *root(path);
	lstrcpy(path, (LPCTSTR)*this);
	while (*path == '\\' || *path == '/') // Skip leading slashes
		++path;
	while (*path && *path != '\\' && *path != '/') // Skip till slashes
		++path;
	*path = 0;
	Path newpath(root);
	delete []root;
	return newpath;
}
Path Path::RemoveRoot() const
{
	Path retVal(*this);
	int start=0;
	while (retVal[start] == '\\' || retVal[start] == '/')
		++start;
	retVal.Delete(0, start);
	start = retVal.FindOneOf(_T("\\/"));
	if (start > 0)
		retVal.Delete(0, start+1);
	else
		retVal.Empty();
	start=0;
	while (retVal[start] == '\\' || retVal[start] == '/')
		++start;
	retVal.Delete(0, start);
	return retVal;
}
unsigned Path::NextComponent(Path &component, unsigned pos) const
{
	component.Empty();
	if (pos >= (unsigned)GetLength())
		return pos;
	LPCTSTR curPath(*this);
	curPath += pos;
	// Reverse back to seperator
	while (pos && *curPath != '\\' && *curPath != '/') {
		--pos;
		--curPath;
	}
	// Skip seperators
	while (*curPath && (*curPath == '\\' || *curPath == '/')) {
		++curPath;
		++pos;
	}
	unsigned startPos(0);
	// Skip till seperators
	while (*curPath && curPath[startPos] != '\\' && curPath[startPos] != '/') {
		++pos;
		++startPos;
	}
	component = CString(curPath, startPos);
	return pos;
}
int ComparePath(const CString &path1, const CString &path2, bool checkPath1IsSubPath /* = false */)
{
	int curPos1 = 0;
	CString resToken1= path1.Tokenize(_T("\\/"),curPos1);
	int curPos2 = 0;
	CString resToken2= path2.Tokenize(_T("\\/"),curPos2);
	int res = 0;
	while (!resToken1.IsEmpty() || !resToken2.IsEmpty())
	{
		res = resToken1.CompareNoCase(resToken2);
		if (res)
			break;
		resToken1= path1.Tokenize(_T("\\/"),curPos1);
		resToken2= path2.Tokenize(_T("\\/"),curPos2);
	};
    if (checkPath1IsSubPath && resToken1.IsEmpty())
        res = 0;
	return res;
}
bool Path::IsUNC() const
{
	return ::PathIsUNC(*this) == TRUE;
}
LONGLONG Path::GetFileSize() const
{
	HANDLE hFile = CreateFile((LPCTSTR)this, GENERIC_READ, FILE_SHARE_READ, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	LONGLONG sizeL = -1;
	if (hFile != INVALID_HANDLE_VALUE) {
		LARGE_INTEGER size = {0};
		GetFileSizeEx(hFile, &size);
		CloseHandle(hFile);
		sizeL = size.QuadPart;
	}
	return sizeL;
}
Path Path::RemoveParent() const
{
	Path outPath(*this);
	outPath.TrimLeft(_T("\\/"));
	int pos = outPath.FindOneOf(_T("\\/"));
	if (pos > 0) {
		outPath.Delete(0, pos);
	}
	outPath.TrimLeft(_T("\\/"));
	return outPath;
}
bool Path::IsParentOf(const Path &childPath) const
{
	return !IsEmpty() && !childPath.IsEmpty() && PathCommonPrefix((LPCTSTR)*this, (LPCTSTR)childPath.Parent(), NULL) == GetLength();
}
Path Path::AddBackSlash() const
{
	Path outPath(RemoveBackSlash());
	outPath += _T("\\");
	return outPath;
}
Path Path::RemoveBackSlash() const
{
	Path outPath(*this);
	int len = outPath.GetLength();
	while (1) {
		outPath.TrimRight('\\');
		outPath.TrimRight('/');
		int nlen = outPath.GetLength();
		if (nlen != len)
			len = nlen;
		else break;
	}
	return outPath;
}
Path Path::GetURL() const
{
	if (IsURL())
		return *this;
	DWORD len = GetLength()+MAX_PATH;
	TCHAR *newPath = new TCHAR[len];
	lstrcpy(newPath, (LPCTSTR)*this);
	UrlCreateFromPath((LPCTSTR)*this, newPath, &len, 0);
	Path outPath(newPath);
	delete []newPath;
	return outPath;
}
bool Path::IsURL() const
{
	return PathIsURL((LPCTSTR)*this) == TRUE;
}
bool Path::IsFileURL() const
{
	return UrlIsFileUrl((LPCTSTR)*this) == TRUE;
}
Path Path::GetPathFromURL() const
{
	if (!IsURL())
		return *this;
	DWORD len = GetLength()+MAX_PATH;
	TCHAR *newPath = new TCHAR[len];
	lstrcpy(newPath, (LPCTSTR)*this);
	PathCreateFromUrl((LPCTSTR)*this, newPath, &len, 0);
	Path outPath(newPath);
	delete []newPath;
	return outPath;
}
bool Path::DeletePath(bool bRecursive) const
{
	bool bSuccess(false);
	if (IsDir()) {
		if (bRecursive) {
			CFileFind finder;
			BOOL bWorking = finder.FindFile();
			while (bWorking)
			{
				bWorking = finder.FindNextFile();
				bSuccess = Append(finder.GetFileName()).DeletePath(bRecursive);
			} 
		}
		else 
			bSuccess = RemoveDirectory((LPCTSTR)*this) == TRUE;
	}
	else
		bSuccess = DeleteFile((LPCTSTR)*this) == TRUE;
	return bSuccess;
}
HICON Path::GetIcon(bool bSmallIcon) const
{
	SHFILEINFO si = {0};
	UINT uFlags(SHGFI_ICON);
	if (bSmallIcon)
		uFlags |= SHGFI_SMALLICON;
	SHGetFileInfo((LPCTSTR)*this, 0, &si, sizeof(si), uFlags);
	return si.hIcon;
}

Path Path::GetMachineNameFromUNCPath() const
{
	Path outPath(GetRoot());
	outPath.TrimLeft(_T("\\/"));
	return outPath;
}

Path Path::MakeUNCPath() const
{
	Path outPath(*this);
	outPath.TrimLeft(_T("\\/"));
	outPath = _T("\\\\") + outPath;
	return outPath;
}
