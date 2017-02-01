// md5.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "cMD5.h"
#include <conio.h>
#include "Path.h"
#include "Progress.h"
COORD GetConsolePos(void)
{
	CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE),
		&csbiInfo);
	return csbiInfo.dwCursorPosition;
}

void SetConsolePos(COORD newPos)
{
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), newPos);
}
void MoveConsoleCursor(int x, int y)
{
	COORD cp = GetConsolePos();
	cp.X += x;
	cp.Y += y;
	SetConsolePos(cp);
}
class AutoConsolePos {
	COORD mCurPos;
public:
	AutoConsolePos(PCOORD setPos = NULL)
	{
		mCurPos = GetConsolePos();
		if (setPos)
			SetConsolePos(*setPos);
	}
	~AutoConsolePos()
	{
		SetConsolePos(mCurPos);
	}
};
class ConsolePrinter
{
	COORD printPos;
	TCHAR mStr[256];
	int strLen;
/*	void AllocateSpaceForStr(int newLen)
	{
		if (newLen > strLen) {
			if (mStr)
				delete[] mStr;
			mStr = new TCHAR[newLen+1];
			mStr[0] = 0;
			strLen = newLen+1;
		}
		else if (newLen <= 0 && mStr) {
			delete[] mStr;
			mStr = NULL;
			strLen = 0;
		}
	}*/
	void CopyStr(const TCHAR *str)
	{
	//	AllocateSpaceForStr(lstrlen(str));
		lstrcpy(mStr, str);
	}
public:
	ConsolePrinter()
	{
		printPos = GetConsolePos();
		mStr[0] = 0;
		strLen = 0;
	}
	void Print(const TCHAR *str)
	{
		if (lstrcmp(mStr, str)) {
			Erase();
			CopyStr(str);
			AutoConsolePos acp(&printPos);
			lprintf(mStr);
		}
	}
	void Erase()
	{
		int len = lstrlen(mStr);
		AutoConsolePos acp(&printPos);
		while (len-- > 0) {
			lprintf(_T(" "));
		}
	}
	~ConsolePrinter(void)
	{
//		AllocateSpaceForStr(0); // Free space
	}
};

struct ConsoleProgress {
	COORD printPos;
	COORD printDotPos;
	int m_iPercentDone;
	ConsoleProgress() {
		lprintf(_T("\r\t\t\t\t\t"));
		printPos = GetConsolePos();
		printDotPos = printPos;
		printDotPos.X = 0;
		m_iPercentDone = -1;
		lprintf(_T("\r"));
	}
	~ConsoleProgress() {
	}
	void ShowPercentage(int percentDone)
	{
		if (percentDone > 100)
			percentDone = 100;
		if (m_iPercentDone != percentDone) {
			m_iPercentDone = percentDone;
			AutoConsolePos acp;
			if (printDotPos.X < 80) {
				int newX = (4 * m_iPercentDone) / 5;
				SetConsolePos(printDotPos);
				while (printDotPos.X < newX) {
					lprintf(_T("."));
					printDotPos.X++;
				}
			}
			SetConsolePos(printPos);
			lprintf(_T(" %d%% "), percentDone);
		}
	}
};
int CopyA2T(const char *src, LPTSTR dst)
{
	int len = 0;
	while (*dst++ = *src++)
		len++;
	return len;
}
int CopyT2A(LPCTSTR src, char *dst)
{
	int len = lstrlen(src);
#if defined(UNICODE) || (_UNICODE)
	len = WideCharToMultiByte(CP_ACP, 0, src, -1, dst, len+1, NULL, NULL);
#else
	lstrcpy(dst, src);
#endif
	return len;
}

class MD5CallbackProgress : public MD5Callback {
public:
	int Status() {
		progress.ShowPercentage((int)((GetCurrent() * 100)/GetTotal()));
		return 0;
	}
private:
	ConsoleProgress progress;
};

struct FileInfo {
	lstring path;
	MD5ULL size;
	FileInfo(const lstring inPath, MD5ULL inSize)
		: path(inPath), size(inSize)
	{
	}
};
typedef std::vector<FileInfo> VecFileInfo;
struct FileList {
	VecFileInfo files;
	MD5ULL totalSize;
	FileList()
		: totalSize(0)
	{
	}
};

class MD5CallbackProgressFiles : public MD5Callback {
public:
	MD5CallbackProgressFiles()
		: currentDone(0)
	{
	}
	int Status() {
		if (progress.UpdateProgress(currentDone + GetCurrent())) {
			cprogress.ShowPercentage((int)(progress.GetCurrentPercentageDone()));
		}
		return 0;
	}
	void SetTotalSize(MD5ULL totalSize)
	{
		progress.SetTask(totalSize);
	}
	void UpdateCurrentDone(MD5ULL inCurrentDone)
	{
		currentDone += inCurrentDone;
	}
	ConsoleProgress cprogress;
	Progress progress;
	MD5ULL currentDone;
};

static int FindCallBack_Files(FindData &fd, void *pUserParam)
{
	if (fd.fileMatched && !(fd.pFindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
		MD5ULL size = fd.pFindData->nFileSizeHigh;
		size <<= sizeof(MD5ULL)<<3;
		size |= fd.pFindData->nFileSizeLow;
		FileList *pFiles((FileList*)pUserParam);
		pFiles->files.push_back(FileInfo(fd.fullPath, size));
		pFiles->totalSize += size;
	}
	return FCBRV_CONTINUE;
}
static int GetFiles(const lstring &folderPath, FileList &outFiles, LPCTSTR pattern = NULL, LPCTSTR excludePattern = NULL)
{
	Finder fd(FindCallBack_Files, &outFiles, pattern, excludePattern);
	return fd.StartFind(folderPath);
}

int _tmain(int argc, _TCHAR* argv[])
{
	if (argc < 2) {
		_ftprintf(stderr, _T("Usage: md5 <file/directory path> or <string>\n")
			_T("If direcotry is given: optional parmaters:\n")
			_T("md5 <directory> [match string] [except string]\n")
			_T("eg md5 <directory> * desktop.ini;.DS_Store\n")
			_T("will calculate md5 of all files in the given directory except 'desktop.ini' and '.DS_Store'\n")
			_T("To skip file check use -s for strings\nmd5 \"Some String\" -s\n"));
	}
	else {
		cMD5 cmd5;
		TCHAR tmd5[128];
		lstring postString;
		char *c_md5 = "";
		if (PathFileExists(argv[1]) && (argc <3 || lstrcmpi(argv[2], _T("-s")))) {
			if (!PathIsDirectory(argv[1])) {
				MD5CallbackProgress md5Progress;
				cmd5.SetMD5Callback(&md5Progress);
				c_md5 = cmd5.CalcMD5FromFile(argv[1]);
			}
			else { // A directory - recurse and calc md5 of each files
				LPCTSTR mf(argc > 2 ? argv[2] : NULL), ef(argc > 3 ? argv[3] : NULL);
				FileList files;
				GetFiles(argv[1], files, mf, ef);
				MD5CallbackProgressFiles progress;
				progress.SetTotalSize(files.totalSize);
				cmd5.SetMD5Callback(&progress);
				for (VecFileInfo::const_iterator cit = files.files.begin(); cit != files.files.end(); ++cit) {
					cmd5.CalcMD5FromFile(cit->path.c_str(), false);
					progress.UpdateCurrentDone(cit->size);
				}
				int nFiles((int)files.files.size());
				if (nFiles)
					c_md5 = cmd5.MD5FinalToString();
				TCHAR buf[128] = {0};
				postString += _T("Number of Files: ");
				_itot_s(nFiles, buf, 128, 10);
				postString += buf;
				postString += _T("\n");
			}
		}
		else {
			char *src = (char *)malloc(sizeof(char)*(lstrlen(argv[1])+1));
			if (src == NULL)
				return -2;
			CopyT2A(argv[1], src);
			c_md5 = cmd5.CalcMD5FromString(src);
			free(src);
		}
		CopyA2T(c_md5, tmd5);
		{
			AutoConsolePos acp;
			_tprintf(_T("\r                                                                                \r"));
		}
		_tprintf(_T("%s\n%s"), tmd5, postString.c_str());
	}
	return 0;
}

