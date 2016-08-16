#pragma once

#include "StringMatcher.h"
#include "Path.h"

#define FIND_CONTINUE_SEARCH 0
#define FIND_ABORT_SEARCH 1
#define FIND_NOTNETWORK_NODE 2
#define FIND_SKIP_SEARCH 3

#define FIND_MAX_THREAD_COUNT 21

#define FCB_CONTINUE 0
#define FCB_ABORT 1
#define FCB_DORECURSIVE 2
#define FCB_NORECURSIVE 3

CString RootNameFromPath(LPCTSTR path);
LPCTSTR FileNameFromPath(LPCTSTR path);
CString ParentDirectoryFromPath(LPCTSTR path);
LONGLONG GetSizeFromString(LPCTSTR pszSize);

class CFileFindEx;
typedef int (*FindCallBack)(CFileFindEx*, bool fileMatched, void *pUserParam);

class CFileFindEx : public CFileFind
{
public:
	CFileFindEx(bool bSearchZip = false) : mbSearchZip(bSearchZip), m_iMatchWeight(0) {}
	virtual ~CFileFindEx() {Close();}
	virtual LONGLONG GetFileSize(void) const;
	virtual ULONGLONG GetLength() const {return __super::GetLength();}
	virtual CString GetFileName() const;
	virtual CString GetFilePath() const {return __super::GetFilePath();}
	virtual CString GetFileTitle() const {return __super::GetFileTitle();}
	virtual CString GetFileURL() const {return __super::GetFileURL();}
	virtual CString GetRoot() const {return __super::GetRoot();}

	virtual BOOL GetLastWriteTime(FILETIME* pTimeStamp) const {return __super::GetLastWriteTime(pTimeStamp);}
	virtual BOOL GetLastAccessTime(FILETIME* pTimeStamp) const {return __super::GetLastAccessTime(pTimeStamp);}
	virtual BOOL GetCreationTime(FILETIME* pTimeStamp) const {return __super::GetCreationTime(pTimeStamp);}
	virtual BOOL GetLastWriteTime(CTime& refTime) const {return __super::GetLastWriteTime(refTime);}
	virtual BOOL GetLastAccessTime(CTime& refTime) const {return __super::GetLastAccessTime(refTime);}
	virtual BOOL GetCreationTime(CTime& refTime) const {return __super::GetCreationTime(refTime);}

	virtual BOOL MatchesMask(DWORD dwMask) const {return __super::MatchesMask(dwMask);}

	virtual BOOL IsDots() const {return __super::IsDots();}
	// these aren't virtual because they all use MatchesMask(), which is
	virtual BOOL IsReadOnly() const {return __super::IsReadOnly();}
	virtual BOOL IsDirectory() const;
	virtual BOOL HasSize() const;
	virtual BOOL IsCompressed() const {return __super::IsCompressed();}
	virtual BOOL IsSystem() const {return __super::IsSystem();}
	virtual BOOL IsHidden() const {return __super::IsHidden();}
	virtual BOOL IsTemporary() const {return __super::IsTemporary();}
	virtual BOOL IsNormal() const {return __super::IsNormal();}
	virtual BOOL IsArchived() const {return __super::IsArchived();}

	// Operations
	virtual void Close() {__super::Close();}
	virtual BOOL FindFile(LPCTSTR pstrName = NULL, DWORD dwUnused = 0) {return __super::FindFile(pstrName, dwUnused);}
	virtual BOOL FindNextFile() {return __super::FindNextFile();}
	virtual LPCTSTR GetDirectory() const {return mDirectory;}
	virtual void SetDirectory(LPCTSTR directory) {mDirectory=directory;}
	virtual BOOL JumpToChildPath(LPCTSTR childPath);

	int GetMatchWeight() const { return m_iMatchWeight; }
	void SetMatchWeight(int iMatchWeight = 0) { m_iMatchWeight = iMatchWeight; }
protected:
	virtual void CloseContext() {__super::CloseContext();}
	CString mDirectory;
	bool mbSearchZip;
	int m_iMatchWeight;
};

class CZipFileFinder : public CFileFindEx {
public:
	CZipFileFinder() : mbRecrursive(true), m_pFileInfo(NULL), mbFound(false) {}
	virtual ~CZipFileFinder() {Close();}
	virtual ULONGLONG GetLength() const;
	virtual CString GetFileName() const;
	virtual CString GetFilePath() const;
	virtual CString GetFileTitle() const;
	virtual CString GetFileURL() const {return __super::GetFileURL();}
	virtual CString GetRoot() const {return __super::GetRoot();}

	virtual BOOL GetLastWriteTime(FILETIME* pTimeStamp) const;
	virtual BOOL GetLastAccessTime(FILETIME* pTimeStamp) const;
	virtual BOOL GetCreationTime(FILETIME* pTimeStamp) const;
	virtual BOOL GetLastWriteTime(CTime& refTime) const;
	virtual BOOL GetLastAccessTime(CTime& /*refTime*/) const {return FALSE;}
	virtual BOOL GetCreationTime(CTime& /*refTime*/) const {return FALSE;}

	virtual BOOL MatchesMask(DWORD dwMask) const;

	virtual BOOL IsDots() const;
	// these aren't virtual because they all use MatchesMask(), which is
	virtual BOOL IsReadOnly() const {return __super::IsReadOnly();}
	virtual BOOL IsDirectory() const;
	virtual BOOL HasSize() const;
	virtual BOOL IsCompressed() const {return __super::IsCompressed();}
	virtual BOOL IsSystem() const {return __super::IsSystem();}
	virtual BOOL IsHidden() const {return __super::IsHidden();}
	virtual BOOL IsTemporary() const {return __super::IsTemporary();}
	virtual BOOL IsNormal() const {return __super::IsNormal();}
	virtual BOOL IsArchived() const {return __super::IsArchived();}


	// Operations
	virtual void Close() {CloseContext();}
	virtual BOOL FindFile(LPCTSTR pstrName = NULL, DWORD dwUnused = 0);
	virtual BOOL FindNextFile();
	virtual BOOL JumpToChildPath(LPCTSTR childPath);
	void SetRecursive(bool bSetRecursive = true) {mbRecrursive=bSetRecursive;}
	bool IsRecursize() const {return mbRecrursive;}
protected:
	BOOL FindNext();
	virtual void CloseContext();
	Path mChildPath;
	bool mbRecrursive;
	bool mbFound;
	void *m_pFileInfo;
    CSortedArrayCString mRootItems;
};
/*class CRegExpContainer
{
public:
	bool MatchPath(LPCTSTR lpPath);
	bool MatchPathFile(LPCTSTR lpDirectory, LPCTSTR lpFile);
	bool bDirectoryValid;
	CAtlRegExp<> mRegDirectoryExp;
	CAtlRegExp<> mRegFileExp;
};

class CRegExpMatcher
{
public:
	CRegExpMatcher(LPCTSTR lpExpression);
	bool MatchPath(LPCTSTR lpPath);
	bool MatchPathFile(LPCTSTR lpDirectory, LPCTSTR lpFile);
private:
	CArray<CRegExpContainer> mRegExpContainerArray;
};*/
class CFinder
{
public:
	enum MatchType {
		WildCard,
		RegularExp,
		Phonetic
	};
	CFinder(LPCTSTR lpExpression = NULL, FindCallBack fcb = NULL, bool recurse = true, void *pUserParam = NULL, MatchType mt = WildCard);
	~CFinder(void);
	int Find(CString lpDirectory, bool bSearchZip = false);
	bool IsAborted() {return m_bAborted;}
	void SetCallback(FindCallBack fcb = NULL) {mFindCallBack=fcb;}
	void* GetUserData() const {return m_pUserParam;}
	void SetUserData(void *pUserData = NULL) {m_pUserParam=pUserData;}
protected:
	int DoFindCallBack(CFileFindEx* pFindFile, bool fileMatched, void *pUserParam);
	FindCallBack mFindCallBack;
	StringMatcher *m_pStringMatcher;
	bool m_bRecursive;
	void *m_pUserParam;
	bool m_bAborted;
};

#define FinderClass(ClassName) \
static int FinderCBFn_##ClassName(CFileFindEx*, bool fileMatched, void *pUserParam);\
struct FinderCallbackData_##ClassName {\
	ClassName *classPointer;		\
	int (ClassName::*FinderCallbackFn)(CFileFindEx*, bool fileMatched, void *pUserParam);\
	void *mpUserData;\
	FinderCallbackData_##ClassName(ClassName *inCPt,\
	int (ClassName::*mCBFn)(CFileFindEx*, bool, void *),\
	void *pUserData = NULL)\
	: classPointer(inCPt), FinderCallbackFn(mCBFn), mpUserData(pUserData)\
{\
}\
void SetCallbackFn(int (ClassName::*mCBFn)(CFileFindEx*, bool fileMatched, void *pUserParam))\
{\
	FinderCallbackFn = mCBFn;\
}\
int Find(CFinder &inFinder, CString lpDirectory, bool bSearchZip = false)\
{\
	inFinder.SetCallback(FinderCBFn_##ClassName);\
	if (mpUserData == NULL)\
		mpUserData  = inFinder.GetUserData();\
	inFinder.SetUserData(this);\
	return inFinder.Find(lpDirectory, bSearchZip);\
}\
};\
static int FinderCBFn_##ClassName(CFileFindEx *pFinder, bool fileMatched, void *pUserData)\
{\
	FinderCallbackData_##ClassName *pData = (FinderCallbackData_##ClassName*)pUserData;\
	return (pData->classPointer->*(pData->FinderCallbackFn))(pFinder, fileMatched, pData->mpUserData);\
}
