#pragma once

class Path : public lstring
{
public:
	Path(const Path& p);
	Path();
	Path(const lstring &inPath);
	Path(LPCTSTR inPath);
	Path Parent() const;
	Path FileName() const;
	bool Exists() const;
	bool IsDir() const;
	bool CreateDir() const;
	Path Append(const Path &append) const;
	static Path CurrentDir();
	bool GetFileTime(LPFILETIME lpCreationTime, LPFILETIME lpLastAccessTime, LPFILETIME lpLastWriteTime) const;
	Path RenameExtension(LPCTSTR newExtn = NULL) const;
	Path GetExtension() const;
	int Compare(const Path &p) const;
	Path GetRoot() const;
	static Path GetSpecialFolderPath(int inFolderID, bool inCreate = false);
	Path MakeFullPath() const;
	bool IsRelativePath() const;
	Path Canonicalize() const;
	bool IsPreFixOf(const Path &preFixPath) const;
	bool DeleteDirectory() const;
	bool SetFileAttributes(DWORD inAttribute ) const;
	bool DeleteDirectoryRecursive() const;
	bool DeleteFile() const;
	Path GetUniqueFileName(int &statNum, LPCTSTR ext = NULL, LPCTSTR prefix = NULL);
};

bool operator == (const Path& p1, const Path& p2);
bool operator != (const Path& p1, const Path& p2);
