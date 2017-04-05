#pragma once

#include <string>

namespace SystemUtils
{
	bool GetSpecialFolderPath(int inFolderID, bool inCreate, std::wstring &outPath);
	bool FileReadLine(FILE *inFile, CString &line);
	std::string UnicodeToUTF8(const wchar_t *unicodeString, int len = -1);
	std::wstring UTF8ToUnicode(const char *utf8String, int len=-1);
	CString UTF8ToUnicodeCString(const char *utf8String);
	void FindAndReplace(CString &inStr, const CString &findStr, const CString &replaceStr);
	CString GetReadableSize(LONGLONG size);
	CString IntToString(int no);
	int StringToInt(LPCTSTR number);
    wchar_t StringGetAt(const CString &inStr, int index);
	LONGLONG GetSizeFromString(LPCTSTR pszSize);
	LONGLONG StringToLongLong(LPCTSTR number);
	CString LongLongToString(LONGLONG no);
	CTime StringToDate(const char *date);
	CTime StringToDate(const CString &date);
	CString DateToString(const CTime &date);
	CString DateToRString(const CTime &date, LPCTSTR fmt = _T("%d/%m/%Y %I:%M %p"));
	CTime IntToTime(INT_PTR time);
	INT_PTR TimeToInt(const CTime &time);
	INT_PTR SplitString(const CString& inString, CArrayCString &outStrings, LPCTSTR sep = _T(","));
	CString CombineString(const CArrayCString &inStringArray, LPCTSTR sep = _T(","), INT_PTR startIndex = 0, INT_PTR endIndex = -1, bool bInlcudeEmpty = true);
    CString StringFindOneOf(const CString &inStr, const CString &inFindStr);
	BOOL GetFileVersion(LPCTSTR filePath, DWORD &outFileVersionMS, DWORD &outFileVersionLS);
	void LogMessage(const wchar_t *msg, ...);
	void LogMessage(const char *msg, ...);
	typedef void (*LogMessageHandlerFn)(const wchar_t *msg);
	void SetLogMessageHandler(LogMessageHandlerFn logFn);
	bool IsOS64();
	bool IsModule64(LPCTSTR modulePath = NULL);
	CString FirstDriveFromMask( ULONG unitmask, int *startPos = NULL);
	template<class T, class I>
	T MakeDataType(I a, I b)
	{
		T r = a;
		const unsigned int shift = ((1<<sizeof(T)) >> 1);
		r <<= shift;
		r |= b;
		return r;
	}

#define  DPI_RESIZE_FLAG_NO_LEFT	1
#define  DPI_RESIZE_FLAG_NO_TOP		2
#define  DPI_RESIZE_FLAG_NO_WIDTH	4
#define  DPI_RESIZE_FLAG_NO_HEIGHT	8
	bool IsHiDPI();
	float GetDPIRatioX();
	float GetDPIRatioY();
	int GetCurrentDPIX();
	int GetCurrentDPIY();
	int GetTranslatedDPIPixel(int pixels, bool bVertical = true);
	int GetTranslatedDPIPixelX(int);
	int GetTranslatedDPIPixelY(int);
	void GetTranslatedDPIRect(LPRECT rect);
	BOOL SetWindowPos(HWND wnd, HWND pWndInsertAfter, int x, int y, int cx, int cy, UINT nFlags, UINT reposFlags = 0);
	void MoveWindow(HWND wnd, LPCRECT lpRect, BOOL bRepaint, UINT reposFlags = 0);
}
