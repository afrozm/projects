// MergeFolder.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "cMD5.h"
#include "Path.h"
#include "Progress.h"

#ifndef FLAGBIT
#define FLAGBIT(n) (1<<(n))
#endif
// Merge folder flasg bits
#define MFF_SKIP_FILE_TIME_CHECK FLAGBIT(0) // Skip file time comparion check
#define MFF_DIFF_ONLY FLAGBIT(1) // Print diff only, Do not perform file operation like copy/delete
#define MFF_NO_VERBOSE FLAGBIT(2) // Print minimal information
#define MFF_CREATE_EMPTY_DIR FLAGBIT(3) // Print minimal information
#define MFF_CONTENT_COMPARE FLAGBIT(4) // Compare contents before copying
#define MFF_DELETE_MISSING FLAGBIT(5) // Compare contents before copying

unsigned guGlags(0);
inline bool CheckFlag(unsigned uFlag)
{
	return (guGlags & uFlag) != 0;
}
inline void SetFlag(unsigned uFlag, bool bSet = true)
{
	if (bSet)
		guGlags |= uFlag;
	else
		guGlags &= ~uFlag;
}

std::string UnicodeToUTF8(const wchar_t *unicodeString)
{
	std::string sRet;
	if (unicodeString != NULL && unicodeString[0])
	{
		int len(lstrlen(unicodeString));
		int kMultiByteLength = WideCharToMultiByte(CP_UTF8, 0, unicodeString, -1, 0, 0, NULL, NULL);
		std::vector<char> vecChar(kMultiByteLength);
		if (WideCharToMultiByte(CP_UTF8, 0, unicodeString, -1, &vecChar[0], (int)vecChar.size(), NULL, NULL))
		{
			sRet = &vecChar[0];
		}
	}
	return sRet;
}
std::wstring UTF8ToUnicode(const char *utf8String)
{
	std::wstring		sRet;
	if (utf8String != NULL && utf8String[0])
	{
		int	kAllocate = MultiByteToWideChar(CP_UTF8, 0, utf8String, -1, NULL, 0);
		if (kAllocate)
		{
			std::vector<wchar_t> vecWide(kAllocate);

			int kCopied = MultiByteToWideChar(CP_UTF8, 0, utf8String, -1, &vecWide[0], (int)vecWide.size());
			if (kCopied)
			{
				sRet = &vecWide[0];
			}
		}
	}
	return sRet;
}
lstring FormatString(LPCTSTR format, ...)
{
	va_list args;
	int     len;
	wchar_t    *buffer;

	// retrieve the variable arguments
	va_start(args, format);

	len = _vsctprintf(format, args) // _vscprintf doesn't count
		+ 1; // terminating '\0'

	buffer = (wchar_t*)malloc(len * sizeof(wchar_t));

	_vstprintf_s(buffer, len, format, args); // C4996
											 // Note: vsprintf is deprecated; consider using vsprintf_s instead
	lstring outStr(buffer);

	free(buffer);

	return outStr;
}
enum LogLevel {
	Info,
	Trace
};
int LogConsole(LogLevel logLevel, LPCTSTR format, ...)
{
	int nLen(0);
	if (logLevel == Trace && CheckFlag(MFF_NO_VERBOSE))
		return nLen;
	va_list args;
	int     len;
	wchar_t    *buffer;

	// retrieve the variable arguments
	va_start(args, format);

	len = _vsctprintf(format, args) // _vscprintf doesn't count
		+ 1; // terminating '\0'

	buffer = (wchar_t*)malloc(len * sizeof(wchar_t));

	_vstprintf_s(buffer, len, format, args); // C4996
											 // Note: vsprintf is deprecated; consider using vsprintf_s instead
	if (buffer[0])
		nLen = _tprintf(_T("%s\n"), buffer);

	free(buffer);

	return nLen;
}

bool CreateEmotyDiredtiesFlagSet()
{
	return !CheckFlag(MFF_DIFF_ONLY) && CheckFlag(MFF_CREATE_EMPTY_DIR);
}


struct MergeFolderFCBData {
	Path dst;
	int nFilesCopied;
	MergeFolderFCBData()
		: nFilesCopied(0)
	{
		dst = dst.CurrentDir();
	}
	MergeFolderFCBData(const Path& inPath)
		: dst(inPath), nFilesCopied(0)
	{
		if (dst.empty()) {
			dst = dst.CurrentDir();
		}
	}
};
DWORD WINAPI MergeCopyProgressRoutine(
                    LARGE_INTEGER TotalFileSize,
                    LARGE_INTEGER TotalBytesTransferred,
                    LARGE_INTEGER StreamSize,
                    LARGE_INTEGER StreamBytesTransferred,
                    DWORD dwStreamNumber,
                    DWORD dwCallbackReason,
                    HANDLE hSourceFile,
                    HANDLE hDestinationFile,
                    LPVOID lpData
)
{
	int *lastPrgressDone((int*)lpData);
	int curProgress = (int)((TotalBytesTransferred.QuadPart*100)/TotalFileSize.QuadPart);
	if (*lastPrgressDone != curProgress) {
		_tprintf(_T("\r%d%%"), curProgress);
		*lastPrgressDone = curProgress;
	}
	return PROGRESS_CONTINUE;
}

bool IsFileCopyRequired(const FindData& srcFileData, const Path &dstFile)
{
	bool bCopyRequired(!dstFile.Exists()); // File does not exists -- copy the new file
	if (!bCopyRequired && !CheckFlag(MFF_SKIP_FILE_TIME_CHECK)) { // File aleardy exists - check lasr modifed time
		FILETIME lastWriteTime = {0};
		if (dstFile.GetFileTime(NULL, NULL, &lastWriteTime)) {
			bCopyRequired = CompareFileTime(&srcFileData.pFindData->ftLastWriteTime, &lastWriteTime) > 0;
		}
	}
	return bCopyRequired;
}

LONGLONG FileGetFileSize(LPCTSTR srcFile)
{
	LARGE_INTEGER fileSize = { 0 };
	HANDLE hFile(CreateFile(srcFile, GENERIC_READ, FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL));
	if (hFile != INVALID_HANDLE_VALUE) {
		GetFileSizeEx(hFile, &fileSize);
		CloseHandle(hFile);
	}
	return fileSize.QuadPart;
}
BOOL MFCopyFile(LPCTSTR srcFile, LPCTSTR dstFile, LONGLONG llFileSize = -1)
{
	BOOL bCancelled(FALSE);
	LPPROGRESS_ROUTINE progFn(NULL);
	int curProgrss(0);
	if (llFileSize < 0)
		llFileSize = FileGetFileSize(srcFile);
	if (llFileSize > 50 * 1024 * 1024) // File is greater than 50Mb
		progFn = (LPPROGRESS_ROUTINE)MergeCopyProgressRoutine;
	_tprintf(_T("Copying file %s to %s\n"), srcFile, dstFile);
	Path(dstFile).Parent().CreateDir();
	BOOL bSuccess(CopyFileEx(srcFile, dstFile, progFn, &curProgrss,
		&bCancelled, 0));
	if (bSuccess) {
		_tprintf(_T("Done\n"));
	}
	else {
		_tprintf(_T("Failed: Error: %d\n"), GetLastError());
	}
	return bSuccess;
}
int FindCallBack_MergeFolder(FindData& findData, void *pUserParam)
{
	MergeFolderFCBData *pMFData((MergeFolderFCBData*)pUserParam);
	if (findData.pFindData == NULL) {// exit of dir
		pMFData->dst = pMFData->dst.Parent();
	}
	else {
		Path dst(pMFData->dst.Append(Path(findData.pFindData->cFileName)));
		if (findData.pFindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			pMFData->dst = dst;
			if (CreateEmotyDiredtiesFlagSet()) {
				pMFData->dst.CreateDir();
			}
		}
		else if (findData.fileMatched) {
			if (IsFileCopyRequired(findData, dst)) {
				if (CheckFlag(MFF_DIFF_ONLY)) {
					_tprintf(_T("File Copy required from %s to %s\n"), findData.fullPath.c_str(), dst.c_str());
					pMFData->nFilesCopied++;
				}
				else if (MFCopyFile(findData.fullPath.c_str(), dst.c_str(), findData.pFindData->nFileSizeLow)) {
					pMFData->nFilesCopied++;
				}
			}
			else if (!CheckFlag(MFF_NO_VERBOSE)) {
				_tprintf(_T("Already present. Skipping file Copy from %s to %s\n"), findData.fullPath.c_str(), dst.c_str());
			}
		}
	}
	return 0;
}
typedef std::map<std::string, std::set<lstring>> MapStrVsSet;

class CountTimer {
	DWORD mLastTime;
	DWORD mCurTime;
	__int64 mTimeDuration;
	DWORD mTimerUpdateDuration;
	bool m_bCountDownWards;
public:
	CountTimer(bool bCountDownWards = false, DWORD timerUpdateDuration = 1000)
	{
		m_bCountDownWards = bCountDownWards;
		mLastTime = GetTickCount();
		mTimeDuration = 0;
		mTimerUpdateDuration = timerUpdateDuration;
	}
	void SetTimeDuration(__int64 timeDuration)
	{
		m_bCountDownWards = true; // its a count down timer
		mTimeDuration = timeDuration;
	}
	__int64 GetTimeDuration(void)
	{
		UpdateTimeDuration();
		return mTimeDuration;
	}

	bool UpdateTimeDuration()
	{
		DWORD curTime = GetTickCount();
		if (curTime - mCurTime > mTimerUpdateDuration) {
			DWORD interval = curTime - mLastTime;
			if (m_bCountDownWards) {
				mTimeDuration -= interval;
				if (mTimeDuration < 0)
					mTimeDuration = 0;
				mLastTime = curTime;
			}
			else {
				mTimeDuration = interval;
			}
			mCurTime = curTime;
			return true;
		}
		return false;
	}
};

class MF_MD5Status : public MD5Callback
{
private:
	virtual int Status() override
	{
		if (mcT.UpdateTimeDuration()
			&& mcT.GetTimeDuration() > 10*1000 // At least 10 sec before display progress
			&& mProg.UpdateProgress(GetCurrent())) {
			_tprintf(_T("\r%.2f\r"), mProg.GetCurrentPercentageDone());
		}
		return 0;
	}

	virtual void SetTotal(MD5ULL total) override
	{
		__super::SetTotal(total);
		mProg.Reset();
		mProg.SetTask(total);
	}

	CountTimer mcT;
	Progress mProg;
};

int FindCallBack_MD5List(FindData& findData, void *pUserParam)
{
	MapStrVsSet *pData((MapStrVsSet*)pUserParam);
	if (findData.fileMatched
		&& findData.pFindData != NULL
		&& !(findData.pFindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
		LogConsole(Trace, _T("%s"), findData.fullPath.c_str());
		MF_MD5Status md5Status;
		std::string md5Str(cMD5(&md5Status).CalcMD5FromFile(findData.fullPath.c_str()));
		(*pData)[md5Str].insert(findData.fullPath);
		LogConsole(Trace, _T("%s"), UTF8ToUnicode(md5Str.c_str()).c_str());
	}
	return 0;
}
int GetMD5List(MapStrVsSet &outMd5List, LPCTSTR dstDir, LPCTSTR matchPattern = NULL, LPCTSTR excludePattern = NULL)
{
	LogConsole(Info, _T("Calculating md5 for %s"), dstDir);
	Finder finder(FindCallBack_MD5List, &outMd5List, matchPattern, excludePattern);
	return finder.StartFind(dstDir);
}
MapStrVsSet gMapMd5SrcList, gMapMd5DstList;
int MergeFolderContent(LPCTSTR srcDir, LPCTSTR dstDir, LPCTSTR matchPattern = NULL, LPCTSTR excludePattern = NULL)
{
	GetMD5List(gMapMd5SrcList, srcDir, matchPattern, excludePattern);
	if (dstDir == NULL)
		dstDir = _T(".");
	if (srcDir == NULL)
		srcDir = _T(".");
	GetMD5List(gMapMd5DstList, dstDir, matchPattern, excludePattern);
	const int srcLen(lstrlen(srcDir));
	int nFilesCopeed(0);
	for (auto citS(gMapMd5SrcList.begin()); citS != gMapMd5SrcList.end(); ++citS) {
		if (gMapMd5DstList.find(citS->first) != gMapMd5DstList.end()) // Already present
			continue;
		// Copy file
		Path srcFile(*citS->second.begin());
		Path dstFile(Path(dstDir).Append(Path(srcFile.c_str() + srcLen)).GetNextUniqueFileName());
		if (CheckFlag(MFF_DIFF_ONLY)) {
			LogConsole(Trace, _T("File Copy required from %s to %s"), srcFile.c_str(), dstFile.c_str());
			++nFilesCopeed;
		}
		else if (MFCopyFile(srcFile.c_str(), dstFile.c_str()))
			++nFilesCopeed;
	}
	return nFilesCopeed;
}
int MergeFolder(LPCTSTR srcDir, LPCTSTR dstDir, LPCTSTR matchPattern = NULL, LPCTSTR excludePattern = NULL)
{
	int nFiles(0);
	if (CheckFlag(MFF_CONTENT_COMPARE)) {
		nFiles = MergeFolderContent(srcDir, dstDir, matchPattern, excludePattern);
	}
	else {
		Path dst(dstDir != NULL ? dstDir : _T(""));
		MergeFolderFCBData mfdt(dst);
		Finder finder(FindCallBack_MergeFolder, &mfdt, matchPattern, excludePattern);
		if (CreateEmotyDiredtiesFlagSet()) {
			mfdt.dst.CreateDir();
		}
		finder.StartFind(srcDir);
		nFiles = mfdt.nFilesCopied;
	}
	return nFiles;
}
struct DeleteFolderFCBData : public MergeFolderFCBData {
	LPCTSTR matchPattern;
	LPCTSTR excludePattern;
	DeleteFolderFCBData() 
		: matchPattern(NULL), excludePattern(NULL)
	{
	}
	DeleteFolderFCBData(const Path& inPath, LPCTSTR mp = NULL, LPCTSTR ep = NULL)
		: MergeFolderFCBData(inPath), matchPattern(mp), excludePattern(ep)
	{
	}
};
static int FindCallBack_DeleteMissing(FindData& findData, void *pUserParam)
{
	DeleteFolderFCBData *pMFData((DeleteFolderFCBData*)pUserParam);
	if (findData.pFindData == NULL) {// exit of dir
		pMFData->dst = pMFData->dst.Parent();
	}
	else {
		Path dst(pMFData->dst.Append(Path(findData.pFindData->cFileName)));
		if (findData.pFindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			if (!dst.Exists()) {
				if (!CheckFlag(MFF_DIFF_ONLY)) {
					_tprintf(_T("Deleteing folder %s as it is not present in %s\n"), findData.fullPath.c_str(), dst.c_str());
					int nDeleted = Path(findData.fullPath).Delete(true, pMFData->matchPattern, pMFData->excludePattern);
					pMFData->nFilesCopied += nDeleted;
					if (nDeleted > 0) {
						_tprintf(_T("Done\n"));
					}
					else {
						_tprintf(_T("Failed: Error: %d\n"), GetLastError());
					}
				}
				else {
					_tprintf(_T("Folder is required to be deleted from %s which is not present in %s\n"), findData.fullPath.c_str(), dst.c_str());
					pMFData->nFilesCopied++;
				}
			}
			pMFData->dst = dst;
		}
		else if (findData.fileMatched) {
			if (!dst.Exists()) {
				if (CheckFlag(MFF_DIFF_ONLY)) {
					_tprintf(_T("File is required to be deleted from %s which is not present in %s\n"), findData.fullPath.c_str(), dst.c_str());
					pMFData->nFilesCopied++;
				}
				else {
					_tprintf(_T("Deleteing file %s as it is not present in %s\n"), findData.fullPath.c_str(), dst.c_str());
					BOOL bSuccess (Path(findData.fullPath).Delete() > 0);
					if (bSuccess) {
						pMFData->nFilesCopied++;
						Path(findData.fullPath).Parent().Delete();
						_tprintf(_T("Done\n"));
					}
					else {
						_tprintf(_T("Failed: Error: %d\n"), GetLastError());
					}
				}
			}
			else if (!CheckFlag(MFF_NO_VERBOSE)) {
				_tprintf(_T("File from %s is already present in %s. Skipping file delete.\n"), dst.c_str(), findData.fullPath.c_str());
			}
		}
	}
	return 0;
}
int DeleteMissing(LPCTSTR srcDir, LPCTSTR dstDir, LPCTSTR matchPattern = NULL, LPCTSTR excludePattern = NULL)
{
	int nFiles(0);
	if (srcDir == NULL)
		srcDir = _T(".");
	if (CheckFlag(MFF_CONTENT_COMPARE)) {
		if (dstDir == NULL)
			dstDir = _T(".");
		for (auto citD(gMapMd5DstList.begin()); citD != gMapMd5DstList.end(); ++citD) {
			if (gMapMd5SrcList.find(citD->first) != gMapMd5SrcList.end()) // Already present
				continue;
			for (auto cit(citD->second.begin()); cit != citD->second.end(); ++cit) {
				Path dstFilePath(*cit);
				if (CheckFlag(MFF_DIFF_ONLY)) {
					LogConsole(Trace, _T("File is required to be deleted from %s which is not present in %s"), dstFilePath.c_str(), dstDir);
					++nFiles;
				}
				else {
					LogConsole(Trace, _T("Deleteing file %s as it is not present in %s"), dstFilePath.c_str(), dstDir);
					if (dstFilePath.DeleteFile()) {
						dstFilePath.Parent().DeleteDirectory();
						++nFiles;
						LogConsole(Trace, _T("Done"));
					}
					else
						LogConsole(Info, _T("Failed to delete file '%s': Error: %d"), dstFilePath.c_str(), GetLastError());
				}
			}
		}
	}
	else {
		Path dst(dstDir != NULL ? dstDir : _T(""));
		DeleteFolderFCBData mfdt(dst);
		Finder finder(FindCallBack_DeleteMissing, &mfdt, matchPattern, excludePattern);
		finder.StartFind(srcDir);
		nFiles = mfdt.nFilesCopied;
	}
	return nFiles;
}
int FindArg(int argc, _TCHAR* argv[], LPCTSTR arg)
{
	int len(lstrlen(arg));
	while (--argc > 0) {
		if (_tcsncicmp(argv[argc], arg, len) == 0)
			break;
	}
	return argc;
}
int FindNextArg(int argc, _TCHAR* argv[], int startArg)
{
	while (++startArg < argc) {
		if (argv[startArg][0] != '-')
			break;
	}
	if (startArg >= argc)
		startArg =0;
	return startArg;
}
void Help()
{
	_tprintf(_T("MergeFolder <src dir> [dst dir] [-mp=<match pattern>] [-ep=<exclude pattern>] [-st] [-cd]\n"));
	_tprintf(_T("-st : To Skip file modified time check.\n"));
	_tprintf(_T("-cd : Only show diff.\n"));
	_tprintf(_T("-dm : Delete missing files that are not present in the source and are present in the dst dir.\n"));
	_tprintf(_T("-nv : Suppress verbose output.\n"));
	_tprintf(_T("-ced : Create empty directories.\n"));
	_tprintf(_T("-cc : Compare contents.\n"));
}
int _tmain(int argc, _TCHAR* argv[])
{
	if (argc < 2 || !lstrcmpi(argv[1], _T("/?"))) {
		Help();
		return -1;
	}
	int nextArg = FindNextArg(argc, argv, 0);
	LPCTSTR src(NULL),dst(NULL), mp(NULL), ep(NULL);
	if (nextArg == 0) {
		_tprintf(_T("src dir not given\n"));
		Help();
		return -2;
	}
	src = argv[nextArg];
	nextArg = FindNextArg(argc, argv, nextArg);
	if (nextArg != 0)
		dst = argv[nextArg];
	nextArg = FindArg(argc, argv, _T("-mp="));
	if (nextArg > 0) {
		mp = argv[nextArg]+4;
	}
	nextArg = FindArg(argc, argv, _T("-ep="));
	if (nextArg > 0) {
		ep = argv[nextArg]+4;
	}
	SetFlag(MFF_SKIP_FILE_TIME_CHECK, FindArg(argc, argv, _T("-st")) > 0);
	SetFlag(MFF_DIFF_ONLY, FindArg(argc, argv, _T("-cd")) > 0);
	SetFlag(MFF_NO_VERBOSE, FindArg(argc, argv, _T("-nv")) > 0);
	SetFlag(MFF_CREATE_EMPTY_DIR, FindArg(argc, argv, _T("-ced")) > 0);
	SetFlag(MFF_CONTENT_COMPARE, FindArg(argc, argv, _T("-cc")) > 0);
	SetFlag(MFF_DELETE_MISSING, FindArg(argc, argv, _T("-dm")) > 0);
	int nFilesCopied(MergeFolder(src, dst, mp, ep));
	int totalFileOp(nFilesCopied);
	if (CheckFlag(MFF_DELETE_MISSING)) {
		int nFilesDeleted = DeleteMissing(dst, src, mp, ep);
		_tprintf(_T("\t%d files %s.\n"), nFilesDeleted, CheckFlag(MFF_DIFF_ONLY) ? _T("require delete") : _T("deleted"));
		totalFileOp += nFilesDeleted;
	}
	_tprintf(_T("\t%d files %s.\n"), nFilesCopied, CheckFlag(MFF_DIFF_ONLY) ? _T("require copy") : _T("copied"));
	_tprintf(_T("Total file operations: %d\n"), totalFileOp);
	return nFilesCopied;
}

