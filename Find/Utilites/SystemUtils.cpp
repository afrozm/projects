#include "StdAfx.h"
#include "SystemUtils.h"
#include <shlobj.h>
#include <vector>
/**
Query the Windows shell for a CSIDL path
*/
bool
SystemUtils::GetSpecialFolderPath(int inFolderID, bool inCreate , std::wstring &outPath)
{
	bool bRet(false);
	LPITEMIDLIST lpItemID(NULL);
	HRESULT hret = SHGetFolderLocation(NULL, inFolderID, NULL, 0, &lpItemID);
	if (SUCCEEDED(hret))
	{
		wchar_t pathValue[MAX_PATH];
		if (SHGetPathFromIDList(lpItemID, pathValue))
		{
			//::PathAddBackslash(pathValue);
			outPath.assign(pathValue);
			bRet = true;
		}
		CoTaskMemFree(lpItemID);
	}
	else
	{
		wchar_t pathValue[MAX_PATH];
		if (SUCCEEDED(SHGetFolderPath(NULL, inFolderID, NULL, SHGFP_TYPE_DEFAULT, pathValue)))
		{
			//::PathAddBackslash(pathValue);
			outPath.assign(pathValue);
			bRet = true;
		}
	}
	if (!outPath.empty())
	{
		if (outPath[outPath.size()-1] == L'\\')
			outPath.resize(outPath.size()-1);

		if (inCreate && !::PathFileExists(outPath.c_str()))
		{
			bRet = (ERROR_SUCCESS == ::SHCreateDirectoryEx(NULL, outPath.c_str(), NULL));
		}
	}
	return bRet;
}

bool SystemUtils::FileReadLine(FILE *inFile, CString &line)
{
	bool bRet(false);
	line = _T("");
	TCHAR strLine[1024];
	while (_fgetts(strLine, sizeof(strLine)/sizeof(TCHAR), inFile)) {
		int len = lstrlen(strLine);
		bool bLineCharFound(false);
		while (len-- > 0) {
			if (strLine[len] != '\n' && strLine[len] != '\r') {
				break;
			}
			bLineCharFound = true;
		}
		len++;
		if (bLineCharFound) {
			strLine[len] = 0;
		}
		line += strLine;
		bRet = true;
		if (bLineCharFound)
			break;
	}
	return bRet;
}

std::string SystemUtils::UnicodeToUTF8(const wchar_t *unicodeString)
{
	std::string sRet;
	if (unicodeString != NULL && unicodeString[0])
	{
		int kMultiByteLength = WideCharToMultiByte(CP_UTF8, 0, unicodeString, -1, 0, 0, NULL, NULL);
		std::vector<char> vecChar(kMultiByteLength);
		if( WideCharToMultiByte(CP_UTF8, 0, unicodeString, -1, &vecChar[0], (int)vecChar.size(), NULL, NULL))
		{
			sRet.assign(&vecChar[0], vecChar.size());
		}
	}
	return sRet;	
}
std::wstring SystemUtils::UTF8ToUnicode(const char *utf8String)
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
				sRet.assign(&vecWide[0], vecWide.size());
			}
		}
	}
	return sRet;
}
CString SystemUtils::UTF8ToUnicodeCString(const char *utf8String)
{
	CString		sRet;
	if (utf8String != NULL && utf8String[0])
	{
		int	kAllocate = MultiByteToWideChar(CP_UTF8, 0, utf8String, -1, NULL, 0);
		if (kAllocate)
		{
			std::vector<wchar_t> vecWide(kAllocate);
			
			int kCopied = MultiByteToWideChar(CP_UTF8, 0, utf8String, -1, &vecWide[0], (int)vecWide.size());
			if (kCopied)
			{
				sRet = (wchar_t *)&vecWide[0];
			}
		}
	}
	return sRet;
}
void SystemUtils::FindAndReplace(CString &inStr, const CString &findStr, const CString &replaceStr)
{
	int start = 0;
	int findLen = findStr.GetLength();
	int replaceLen = replaceStr.GetLength();
	while ((start = inStr.Find(findStr, start)) >= 0) {
		inStr.Delete(start, findLen);
		inStr.Insert(start, replaceStr);
		start += replaceLen;
	}
}
CString SystemUtils::IntToString(int no)
{
	TCHAR number[256];
	_stprintf_s(number, sizeof(number)/sizeof(TCHAR), _T("%d"), no);
	return CString(number);
}
int SystemUtils::StringToInt(LPCTSTR number)
{
	return _ttoi(number);
}
LONGLONG SystemUtils::StringToLongLong(LPCTSTR number)
{
	return _ttoi64(number);
}
CString SystemUtils::LongLongToString(LONGLONG no)
{
	CString strNum;
	strNum.Format(_T("%I64d"), no);
	return strNum;
}

CString SystemUtils::GetReadableSize(LONGLONG size)
{
	if (size < 0)
		return CString();
	LPCTSTR extString[] = {_T("TB"), _T("GB"), _T("MB"), _T("KB"), _T("Bytes")};
	const int kExtStringSize = sizeof (extString)/sizeof(LPCTSTR);
	INT64 sizeDecimal = 0;
	int i = 0;
	for (i = 0; i < kExtStringSize; i++) {
		INT64 mask = 1;
		int shift = (kExtStringSize-1-i) * 10;
		mask <<= shift;
		mask -= 1;
		if (size & ~mask) {
			sizeDecimal = size >> shift;
			size &= mask;
			size >>= shift - 10;
			break;
		}
	}
	CString strSize(IntToString((int)sizeDecimal));
	size = (INT64)((size * 10.0f) / 1024.0f);
	if (size > 0) {
		strSize += _T(".") + IntToString((int)size);
	}
	strSize += _T(" ");
	if (i >= kExtStringSize)
		i = kExtStringSize-1;
	strSize += extString[i];

	return strSize;
}
LONGLONG SystemUtils::GetSizeFromString(LPCTSTR pszSize)
{
	LONGLONG size = -1;
	if (*pszSize) {
		size = _ttoi64(pszSize);
		while (*pszSize && (!_istalpha(*pszSize)))
			++pszSize;
		switch (*pszSize) {
			case 'T':
			case 't':
				size <<=40;
				break;
			case 'G':
			case 'g':
				size <<=30;
				break;
			case 'M':
			case 'm':
				size <<=20;
				break;
			case 'K':
			case 'k':
				size <<=10;
				break;
		}
	}
	return size;
}
#define STR_SKIP_TILL(s,c) while (*s && *s!=c) ++s
#define STR_SKIP_CHAR(s,c) while (*s && *s==c) ++s
#define STR_SKIP_NEXT(s,c) STR_SKIP_TILL(s,c);STR_SKIP_CHAR(s,c);
CTime SystemUtils::StringToDate(const char *date)
{
	int year(0), month(0), day(0);
	day = atoi(date);
	STR_SKIP_NEXT(date,'/');
	month = atoi(date);
	STR_SKIP_NEXT(date,'/');
	year = atoi(date);
	return CTime(year, month, day, 0, 0, 0);
}
CTime SystemUtils::StringToDate(const CString &date)
{
	return SystemUtils::StringToDate(SystemUtils::UnicodeToUTF8(date).c_str());
}
CString SystemUtils::DateToString(const CTime &date)
{
	return date.Format(_T("%d/%m/%Y"));
}
CString SystemUtils::DateToRString(const CTime &date, LPCTSTR fmt)
{
	if (fmt == NULL)
		fmt = _T("%d/%m/%Y %I:%M %p");
	return date.Format(fmt);
}
CTime SystemUtils::IntToTime(INT_PTR time)
{
	return CTime(time);
}
INT_PTR SystemUtils::TimeToInt(const CTime &time)
{
	return time.GetTime();
}
INT_PTR SystemUtils::SplitString(const CString& inString, CArrayCString &outStrings, LPCTSTR sep)
{
	outStrings.RemoveAll();
	CString str(inString);
	while (1) {
		int pos = str.FindOneOf(sep);
		if (pos >= 0) {
			outStrings.Add(str.Left(pos));
			str.Delete(0, pos+1);
		}
		else break;
	}
	if (!str.IsEmpty())
		outStrings.Add(str);
	return outStrings.GetCount();
}

CString SystemUtils::CombineString(const CArrayCString &inStringArray, LPCTSTR sep /* = _T */, INT_PTR startIndex /*= 0*/, INT_PTR endIndex /*= -1*/)
{
	CString outStr;
	if (endIndex < 0)
		endIndex = inStringArray.GetCount();
	for (INT_PTR i = startIndex; i < endIndex; ++i)
		outStr += inStringArray.GetAt(i) + sep;
	if (outStr.GetLength() > 0) {
		int sepLen(lstrlen(sep));
		outStr.Delete(outStr.GetLength()-sepLen, sepLen);
	}
	return outStr;
}

bool SystemUtils::IsCompleteWord(const CString& inString, int pos, int len)
{
	if (pos == 0 || _istalnum(inString[pos])) {
		if (inString.GetLength()<=pos+len)
			return true;
		return _istalnum(inString[pos+len]) != 0;
	}
	return false;
}
BOOL SystemUtils::GetFileVersion(LPCTSTR filePath, DWORD &outFileVersionMS, DWORD &outFileVersionLS)
{
	DWORD dwHandle(0);
	DWORD dwSize  = GetFileVersionInfoSize(filePath, &dwHandle);
	BOOL bRet(FALSE);
	if (dwSize)
	{
		std::vector<char>		vecBuf(dwSize);
		ZeroMemory(&vecBuf[0], dwSize);
		if(GetFileVersionInfo(filePath,NULL,dwSize,&vecBuf[0]))
		{
			void* pBuffer;
			unsigned int ulFixedFileInfoSize;
			if(VerQueryValue((void*)&vecBuf[0],TEXT("\\"),&pBuffer, &ulFixedFileInfoSize))
			{
				VS_FIXEDFILEINFO *pFixedFileInfo = reinterpret_cast<VS_FIXEDFILEINFO*>(pBuffer);
				outFileVersionMS = pFixedFileInfo->dwFileVersionMS;
				outFileVersionLS = pFixedFileInfo->dwFileVersionLS;
				bRet = TRUE;
			}
		}
	}
	return bRet;
}
static void DefaultLogMessageHandler(const wchar_t *msg)
{
	_tprintf(_T("%s\n"), msg);
#ifdef _DEBUG
	OutputDebugString(msg);
	OutputDebugString(_T("\r\n"));
#endif
}
SystemUtils::LogMessageHandlerFn sLogMessageHandlerFn = DefaultLogMessageHandler;
void SystemUtils::LogMessage(const wchar_t *msg, ...)
{
	if (msg != NULL) {
		va_list arg;
		va_start(arg, msg);
		int len = _vsctprintf(msg, arg) + 4*sizeof(TCHAR); // _vscprintf doesn't count + 1; terminating '\0'
		TCHAR *buf = new TCHAR[len];
		_vstprintf_s(buf, len, msg, arg);
		sLogMessageHandlerFn(buf);
		delete buf;
	}
}
void SystemUtils::LogMessage(const char *msg, ...)
{
	if (msg != NULL) {
		va_list arg;
		va_start(arg, msg);
		int len = _vscprintf(msg, arg) + 4*sizeof(char); // _vscprintf doesn't count + 1; terminating '\0'
		char *buf = new char[len];
		vsprintf_s(buf, len, msg, arg);
		strcat_s(buf, len, "\r\n");
		sLogMessageHandlerFn(UTF8ToUnicode(buf).c_str());
		delete buf;
	}
}
void SystemUtils::SetLogMessageHandler(LogMessageHandlerFn logFn)
{
	if (logFn)
		sLogMessageHandlerFn = logFn;
	else
		sLogMessageHandlerFn = DefaultLogMessageHandler;
}
typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
bool SystemUtils::IsOS64()
{
	BOOL bIsWow64 = FALSE;

	LPFN_ISWOW64PROCESS fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(
		GetModuleHandle(TEXT("kernel32")),"IsWow64Process");

	if (NULL != fnIsWow64Process)
	{
		if (!fnIsWow64Process(GetCurrentProcess(),&bIsWow64))
		{
			// handle error
		}
	}
	return bIsWow64 != FALSE;
}

bool SystemUtils::IsModule64(LPCTSTR modulePath)
{
	bool bModuleIs64(false);
	if (modulePath == NULL) {
#ifdef _WIN64
		bModuleIs64 = true;
#endif
	}
	else {

	}
	return bModuleIs64;
}
CString SystemUtils::FirstDriveFromMask( ULONG unitmask, int *startPos)
{
	int i = startPos ? *startPos : 0;
	for (; i < 26; ++i)
	{
		if (unitmask & 0x1)
			break;
		unitmask = unitmask >> 1;
	}
	if (startPos)
		*startPos = i+1;
    TCHAR drive[3] = {0};
    drive[0] = (TCHAR)i + 'A';
    drive[1] = ':';
    drive[2] = 0;
	return CString(drive);
}
CString SystemUtils::GetLine(LPCTSTR inStr, int startPos /* = 0 */)
{
	if (inStr == NULL || startPos >= lstrlen(inStr))
		return CString();
	LPCTSTR curStr(inStr+startPos);
	STR_SKIP_LINE(curStr);
	LPCTSTR endStr(curStr);
	STR_SKIP_TILL_LINE(endStr);
	STR_SKIP_TILL_LINE_REV(curStr, inStr);
	CString outStr(curStr, (int)(endStr-curStr));
	return outStr;
}

int SystemUtils::GetCurrentDPIX()
{
	static int _dpiX = 0;
	if (_dpiX == 0)
	{
		HDC hdc = GetDC(NULL);
		_dpiX = GetDeviceCaps(hdc, LOGPIXELSX);
		ReleaseDC(NULL, hdc);
	}
	return _dpiX;
}

int SystemUtils::GetCurrentDPIY()
{
	static int _dpiY = 0;
	if (_dpiY == 0)
	{
		HDC hdc = GetDC(NULL);
		_dpiY = GetDeviceCaps(hdc, LOGPIXELSY);
		ReleaseDC(NULL, hdc);
	}
	return _dpiY;
}
#define STD_DISPALY_PIXELS_PER_INCH 96

int SystemUtils::GetTranslatedDPIPixelX(int x)
{
	return MulDiv(x, SystemUtils::GetCurrentDPIX(), STD_DISPALY_PIXELS_PER_INCH);
}

int SystemUtils::GetTranslatedDPIPixelY(int y)
{
	return MulDiv(y, SystemUtils::GetCurrentDPIY(), STD_DISPALY_PIXELS_PER_INCH);
}

int SystemUtils::GetTranslatedDPIPixel(int pixels, bool bVertical /* = true */)
{
	if (bVertical)
		pixels = GetTranslatedDPIPixelY(pixels);
	else
		pixels = GetTranslatedDPIPixelX(pixels);
	return pixels;
}
BOOL SystemUtils::SetWindowPos(HWND wnd, HWND pWndInsertAfter, int x, int y, int cx, int cy, UINT nFlags, UINT reposFlags/* = 0*/)
{
	if(!(reposFlags & DPI_RESIZE_FLAG_NO_LEFT))
	{
		x = GetTranslatedDPIPixelX(x);
	}
	if(!(reposFlags & DPI_RESIZE_FLAG_NO_WIDTH))
	{
		cx = GetTranslatedDPIPixelX(cx);
	}
	if(!(reposFlags & DPI_RESIZE_FLAG_NO_TOP))
	{
		y = GetTranslatedDPIPixelY(y);
	}
	if(!(reposFlags & DPI_RESIZE_FLAG_NO_HEIGHT))
	{
		cy = GetTranslatedDPIPixelY(cy);
	}

	return ::SetWindowPos(wnd, pWndInsertAfter, x, y, cx, cy, nFlags);
}

void SystemUtils::MoveWindow(HWND wnd, LPCRECT lpRect, BOOL /*bRepaint*/, UINT reposFlags/* = 0*/)
{
	SystemUtils::SetWindowPos(wnd, NULL, lpRect->left, lpRect->top, lpRect->right - lpRect->left, lpRect->bottom - lpRect->top, SWP_NOZORDER, reposFlags);
}

#define HIDPI_PIXELS 96
bool SystemUtils::IsHiDPI()
{
	return GetCurrentDPIY() > HIDPI_PIXELS || GetCurrentDPIX() > HIDPI_PIXELS;
}

float SystemUtils::GetDPIRatioX()
{
	return (float)GetCurrentDPIX() / (float)STD_DISPALY_PIXELS_PER_INCH;
}

float SystemUtils::GetDPIRatioY()
{
	return (float)GetCurrentDPIY() / (float)STD_DISPALY_PIXELS_PER_INCH;
}
void SystemUtils::GetTranslatedDPIRect(LPRECT rect)
{
	LONG *dimenstions((LONG*)rect);
	for (int i = 0; i < 4; ++i, ++dimenstions)
		*dimenstions = GetTranslatedDPIPixel(*dimenstions, (i%2)!=0);
}
