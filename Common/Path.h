#pragma once
#include "Common.h"

#include <regex>
#ifndef _WIN32
#include <sys/stat.h>
typedef timespec FILETIME;
typedef FILETIME*  LPFILETIME;
#else
#include <windows.h>
#endif

class Path : public lstring
{
public:
	Path(const Path& p);
	Path();
	Path(const lstring &inPath);
	Path(LPCTSTR inPath);
    operator LPCTSTR () const {return c_str();}
    Path(const otherstring &inPath);
    Path(const otherchar *inPath);
    operator otherstring() const;
    Path Parent() const;
    bool IsParentOf(const Path &childPath) const;
	Path FileName() const;
	Path FileNameWithoutExt() const;
	bool Exists() const;
	bool IsDir() const;
    bool IsUNC() const;
    bool IsURL() const;
    Path GetURL() const;
    Path MakeUNCPath() const;
    Path GetMachineNameFromUNCPath() const;
    bool IsEmpty() const { return empty(); }
    Path RemoveRoot() const;
	bool CreateDir() const;
	Path Append(const Path &append) const;
	static Path CurrentDir();
    static Path TempPath();
    static Path TempFile(const Path &inPath, LPCTSTR preFix = nullptr, LPCTSTR ext = nullptr, unsigned long startNum = 0);
	bool GetFileTime(LPFILETIME lpCreationTime, LPFILETIME lpLastAccessTime, LPFILETIME lpLastWriteTime) const;
	Path RenameExtension(LPCTSTR newExtn = NULL) const;
	Path GetExtension() const;
	int Compare(const Path &p) const;
    int CompareExtension(LPCTSTR extn) const;
	Path GetRoot() const;
    Path NextComponent(unsigned *inoutpos = nullptr) const;
	static Path GetSpecialFolderPath(int inFolderID, bool inCreate = false);
    static Path GetModuleFilePath(HMODULE hModule = NULL);
    static Path GetTempPath();
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
    bool Move(const Path &inNewLocation) const;
    bool CopyFile(const Path &newFilePath) const;
    bool OpenInExplorer() const;
#ifdef _WIN32
    HICON GetIcon(bool bSmallIcon = true) const;
#endif // _WIN32

};
int ComparePath(LPCTSTR path1, LPCTSTR path2, bool checkPath1IsSubPath = false );

bool operator == (const Path& p1, const Path& p2);
bool operator != (const Path& p1, const Path& p2);


struct FindData {
	const Path &fullPath;
	bool fileMatched;
#ifdef _WIN32
    typedef WIN32_FIND_DATA PLAT_FIND_DATA;
#else
    struct PLAT_FIND_DATA {
    };
#endif
    PLAT_FIND_DATA *pFindData;
	FindData(PLAT_FIND_DATA *pFD, const Path &fp, bool fm)
		: fullPath(fp), fileMatched(fm), pFindData(pFD)
	{}
    long long GetFileSize() const;
};
#define FCBRV_CONTINUE 0
#define FCBRV_ABORT 1
#define FCBRV_SKIPDIR 2

typedef int(*PathFindCallBack)(FindData&, void *pUserParam);
struct Finder {
	Finder(PathFindCallBack fcb, void *pUserParam = NULL, LPCTSTR pattern = NULL, LPCTSTR excludePattern = NULL);
    int StartFind(const Path &dir);
    std::basic_regex<TCHAR> mRegExp, mExcludeRegExp;
	LPCTSTR mExcludePattern;
    PathFindCallBack mFindCallBack;
	void *m_pUserParam;
};
