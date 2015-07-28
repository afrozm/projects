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
	Path FileNameWithoutExt() const;
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
	Path GetUniqueFileName(int &statNum, LPCTSTR ext = NULL, LPCTSTR prefix = NULL) const;
	Path GetNextUniqueFileName() const;
	int Delete(bool bRecusrive = false, LPCTSTR pattern = NULL, LPCTSTR excludePattern = NULL) const;
	INT64 GetSize() const;
	bool CreateShortCut(const Path &shortCutPath, LPCTSTR pszTargetargs = NULL, LPCTSTR pszDescription = NULL,
		int iShowmode = 0, LPCTSTR pszCurdir = NULL,
		LPCTSTR pszIconfile = NULL, int iIconindex = 0) const;
	enum FileTimeType {
		CreationTime,
		AccessTime,
		ModifiedTime
	};
	ULONGLONG GetFileTime(FileTimeType fileType = CreationTime) const;
};

bool operator == (const Path& p1, const Path& p2);
bool operator != (const Path& p1, const Path& p2);


struct FindData {
	WIN32_FIND_DATA *pFindData;
	const lstring &fullPath;
	bool fileMatched;
	FindData(WIN32_FIND_DATA *pFD, const lstring &fp, bool fm)
		: pFindData(pFD), fullPath(fp), fileMatched(fm)
	{}
};
#define FCBRV_CONTINUE 0
#define FCBRV_ABORT 1
#define FCBRV_SKIPDIR 2
#define FCBRV_STOP 3
typedef int(*FindCallBack)(FindData&, void *pUserParam);
struct Finder {
	Finder(FindCallBack fcb, void *pUserParam = NULL, LPCTSTR pattern = NULL, LPCTSTR excludePattern = NULL);
	int StartFind(const lstring &dir);
	CAtlRegExp<> mRegExp;
	CAtlRegExp<> mExcludeRegExp;
	LPCTSTR mExcludePattern;
	FindCallBack mFindCallBack;
	void *m_pUserParam;
};
