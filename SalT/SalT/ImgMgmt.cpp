#include "stdafx.h"
#include "ImgMgmt.h"
#include <set>

HBITMAP CopyHwndToBitmap(HWND hWnd, bool bEntire /* = true */, LPRECT area /* = NULL */)
{
	HDC         hScrDC(NULL), hMemDC(NULL);         // screen DC and memory DC     
	//	HBITMAP     hBitmap; //, 
	//	HBITMAP     hBitmap;
	//	HBITMAP     hOldBitmap;    // handles to deice-dependent bitmaps     
	int         nWidth(0), nHeight(0);        // DIB width and height     

	HGDIOBJ     hOldBitmap(NULL), hBitmap(NULL);

	// create a DC for the screen and create     
	// a memory DC compatible to screen DC          

	if (bEntire)
		hScrDC = GetDCEx(hWnd, NULL, DCX_WINDOW);
	if (hScrDC == NULL) {
		hScrDC = GetDC(hWnd);
		bEntire = false;
	}
	hMemDC = CreateCompatibleDC(hScrDC);      // get points of rectangle to grab  

	RECT rc;
	if (area == NULL) {
		area = &rc;
		if (bEntire)
			GetWindowRect(hWnd, &rc);
		else
			GetClientRect(hWnd, &rc);
	}

	nWidth = area->right - area->left;
	nHeight = area->bottom - area->top;
	if (area == &rc) {
		area->left = 0;
		area->top = 0;
	}
	// create a bitmap compatible with the screen DC     

	hBitmap = CreateCompatibleBitmap(hScrDC, nWidth, nHeight);

	// select new bitmap into memory DC     

	hOldBitmap = SelectObject(hMemDC, hBitmap);

	// bitblt screen DC to memory DC     

	BitBlt(hMemDC, 0, 0, nWidth, nHeight, hScrDC, area->left, area->top, SRCCOPY);

	// select old bitmap back into memory DC and get handle to     
	// bitmap of the screen          

	hBitmap = SelectObject(hMemDC, hOldBitmap);

	// clean up      

	DeleteDC(hScrDC);
	DeleteDC(hMemDC);

	// return handle to the bitmap      

	return (HBITMAP)hBitmap;

}
using namespace Gdiplus;
static int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT  num = 0;          // number of image encoders
	UINT  size = 0;         // size of the image encoder array in bytes

	ImageCodecInfo* pImageCodecInfo = NULL;

	GetImageEncodersSize(&num, &size);
	if (size == 0)
		return -1;  // Failure

	pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
	if (pImageCodecInfo == NULL)
		return -1;  // Failure

	GetImageEncoders(num, size, pImageCodecInfo);

	for (UINT j = 0; j < num; ++j)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;  // Success
		}
	}

	free(pImageCodecInfo);
	return -1;  // Failure
}

BOOL SaveBitmapAsJpeg(HBITMAP hBmp, LPCTSTR filePath)
{
	Gdiplus::Bitmap bmp(hBmp, NULL);
	CLSID pngClsid;
	GetEncoderClsid(L"image/jpeg", &pngClsid);
	return bmp.Save(filePath, &pngClsid, NULL) == Ok;
}
struct cleanupFileInfo {
	ULONGLONG ft;
	INT64 fileSize;
	Path filePath;
	cleanupFileInfo() : fileSize(0), ft(0) {}
	bool operator < (const cleanupFileInfo &rt) const
	{
		return ft < rt.ft;
	}
	void setFT(const FILETIME &inFT)
	{
		ULARGE_INTEGER ul = { inFT.dwLowDateTime, inFT.dwHighDateTime };
		ft = ul.QuadPart;
	}
	void setSize(DWORD fileSizeHigh, DWORD fileSizeLow)
	{
		LARGE_INTEGER li = { fileSizeLow, (LONG)fileSizeHigh };
		fileSize = li.QuadPart;
	}
};
static int FindCallBack__cleanupImgFolder(FindData &findData, void *pUserParam)
{
	if (findData.pFindData != NULL) {
		if (!(findData.pFindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			std::multiset<cleanupFileInfo> *setcleanupFileInfo((std::multiset<cleanupFileInfo> *)pUserParam);
			cleanupFileInfo cfi;
			cfi.filePath = findData.fullPath;
			cfi.setSize(findData.pFindData->nFileSizeHigh, findData.pFindData->nFileSizeLow);
			cfi.setFT(findData.pFindData->ftCreationTime);
			setcleanupFileInfo->insert(cfi);
		}
	}
	return 0;
}
const INT64 kMaxSize(8 * 1024 * 1024 * 1024LL); // 8 GB
static DWORD WINAPI ThreadProc_cleanupImgFolder(
	_In_  LPVOID lpParameter
	)
{
	Path imgPath(GetImagePath());
	INT64 folderSize(imgPath.GetSize());
	if (folderSize > kMaxSize)  {
		std::multiset<cleanupFileInfo> setcleanupFileInfo;
		Finder(FindCallBack__cleanupImgFolder, &setcleanupFileInfo).StartFind(imgPath);
		for (auto cit(setcleanupFileInfo.begin()); cit != setcleanupFileInfo.end() && (folderSize > (kMaxSize >> 1)); ++cit) {
			folderSize -= cit->fileSize;
			cit->filePath.Delete();
		}
	}
	return 0;
}

static void cleanupImgFolder()
{
	static DWORD lastCleanup(0);
	static HANDLE hThread(NULL);
	DWORD curTickCount(GetTickCount());
	DWORD timeElapsed(curTickCount - lastCleanup);
	if (timeElapsed < 60 * 60 * 1000) {// 1hr or folder size is exceeding
		bool bRet(true);
		if (timeElapsed > 5 * 60 * 1000) {// 5 min
			bRet = GetImagePath().GetSize() < kMaxSize;
		}
		if (bRet)
			return;
	}
	lastCleanup = curTickCount;
	if (WaitForSingleObject(hThread, 1) == WAIT_TIMEOUT) // thread is running
		return;
	hThread = CreateThread(NULL, 0, ThreadProc_cleanupImgFolder, NULL, 0, NULL);
}

void ReplaceStr(lstring& str, const lstring& oldStr, const lstring& newStr)
{
	for (auto cit(oldStr.begin()); cit != oldStr.end();) {
		bool bMatchFound(false);
		size_t pos = 0;
		while ((pos = str.find(*cit, pos)) != lstring::npos)
		{
			str.replace(pos, 1, newStr);
			pos += newStr.length();
			bMatchFound = true;
		}
		if (!bMatchFound)
			++cit;
	}
}

static lstring getPrefix(HWND hWnd)
{
	TCHAR wndText[256] = { 0 };
	GetWindowText(hWnd, wndText, ARRAYSIZE(wndText));
	TCHAR className[256] = { 0 };
	GetClassName(hWnd, className, ARRAYSIZE(className));
	Path immgPath(GetImagePath());
	int remLength = 250 - immgPath.length() - 12;
	lstring prefix(_T("SS"));
	if (remLength > 0) {
		LPCTSTR pStr = wndText[0] ? wndText : className;
		lstring morePrefix(pStr);
		if ((int)morePrefix.length() > remLength)
			morePrefix.erase(remLength);
		ReplaceStr(morePrefix, _T("\\/:*?\"<>|"), _T(""));
		if (morePrefix.length() > 0)
			prefix += _T(".") + morePrefix;
	}
	return prefix;
}
static ULONGLONG getSystemTimeAsMS()
{
	SYSTEMTIME st;
	GetSystemTime(&st);
	FILETIME ft;
	SystemTimeToFileTime(&st, &ft);
	ULARGE_INTEGER ul = { ft.dwLowDateTime, ft.dwHighDateTime };
	return ul.QuadPart;
}
BOOL SaveHwndImage(HWND hWnd, int msg)
{
	static DWORD lastSavedTime(0);
	if (GetTickCount() - lastSavedTime < 60 * 1000) {// 1 min
			return FALSE;
	}
	lstring prefix(getPrefix(hWnd));
	HBITMAP hBmp(CopyHwndToBitmap(hWnd));
	int imgNum(0);
	Path immgPath = GetImagePath().GetUniqueFileName(imgNum, _T(".JPG"), prefix.c_str());
	BOOL bRet(SaveBitmapAsJpeg(hBmp, immgPath.c_str()));
	if (bRet)
		lastSavedTime = GetTickCount();
	DeleteObject(hBmp);
	cleanupImgFolder();
	return bRet;
}


// Image Path

Path GetImagePath()
{
	Path outPath(Path::GetSpecialFolderPath(CSIDL_COMMON_APPDATA));
	outPath = outPath.Append(_T("salt"));
	outPath = outPath.Append(_T("ss"));
	outPath.CreateDir();
	return outPath;
}
