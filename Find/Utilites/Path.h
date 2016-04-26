#pragma once
int ComparePath(const CString &path1, const CString &path2, bool checkPath1IsSubPath = false);

class Path : public CString
{
public:
	Path();
	Path(const CString &inPath);
	Path Parent() const;
	Path FileName() const;
	bool Exists() const;
	bool IsDir() const;
	bool CreateDir() const;
	bool DeletePath(bool bRecursive = false) const;
	Path Append(const Path &append) const;
	static Path CurrentDir();
	static Path TempPath();
	static Path TempFile(const Path &inPath, const CString& preFix, const CString &ext, unsigned long startNum = 0);
	bool GetFileTime(LPFILETIME lpCreationTime, LPFILETIME lpLastAccessTime, LPFILETIME lpLastWriteTime) const;
	Path RenameExtension(LPCTSTR newExtn = NULL) const;
	Path GetExtension() const;
	Path GetRoot() const;
	Path RemoveRoot() const;
	unsigned NextComponent(Path &component, unsigned pos = 0) const;
	bool IsUNC() const;
	LONGLONG GetFileSize() const;
	Path RemoveParent() const;
	bool IsParentOf(const Path &childPath) const; // true of childPath is sub path of this path e.g c:\abc is parent of c:\abc\xyz\tx.xtx and/or c:\abc\t.xt
	Path AddBackSlash() const;
	Path RemoveBackSlash() const;
	int ComparePath(const Path &path) const
	{
		return ::ComparePath(*this, path);
	}
	bool operator == (const Path &path) const {
		return ComparePath(path) == 0;
	}
	Path GetURL() const;
	bool IsURL() const;
	bool IsFileURL() const;
	Path GetPathFromURL() const;
	HICON GetIcon(bool bSmallIcon = true) const;
	Path GetMachineNameFromUNCPath() const;
	Path MakeUNCPath() const;
};
