#include "stdafx.h"
#include "ImgMgmt.h"

static HBITMAP CopyHwndToBitmap(HWND hWnd, bool bEntire = true, LPRECT area = NULL)
{
	HDC         hScrDC(NULL), hMemDC(NULL);
	int         nWidth(0), nHeight(0);

	HGDIOBJ     hOldBitmap(NULL), hBitmap(NULL);

	if (bEntire)
		hScrDC = GetDCEx(hWnd, NULL, DCX_WINDOW);
	if (hScrDC == NULL) {
		hScrDC = GetDC(hWnd);
		bEntire = false;
	}
	hMemDC = CreateCompatibleDC(hScrDC);

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

	hBitmap = CreateCompatibleBitmap(hScrDC, nWidth, nHeight);
	hOldBitmap = SelectObject(hMemDC, hBitmap);
	BitBlt(hMemDC, 0, 0, nWidth, nHeight, hScrDC, area->left, area->top, SRCCOPY);
	hBitmap = SelectObject(hMemDC, hOldBitmap);

	DeleteDC(hScrDC);
	DeleteDC(hMemDC);

	return (HBITMAP)hBitmap;
}

using namespace Gdiplus;

static int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT  num = 0;
	UINT  size = 0;

	ImageCodecInfo* pImageCodecInfo = NULL;

	GetImageEncodersSize(&num, &size);
	if (size == 0)
		return -1;

	pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
	if (pImageCodecInfo == NULL)
		return -1;

	GetImageEncoders(num, size, pImageCodecInfo);

	for (UINT j = 0; j < num; ++j)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;
		}
	}

	free(pImageCodecInfo);
	return -1;
}

static BOOL SaveBitmapAsJpeg(HBITMAP hBmp, LPCTSTR filePath)
{
	Gdiplus::Bitmap bmp(hBmp, NULL);
	CLSID pngClsid;
	GetEncoderClsid(L"image/jpeg", &pngClsid);
	return bmp.Save(filePath, &pngClsid, NULL) == Ok;
}

bool CaptureAndSaveWindowImage(const Path &filePath)
{
	HWND hWnd = GetForegroundWindow();
	if (hWnd == NULL)
		return false;
	HBITMAP hBmp = CopyHwndToBitmap(hWnd);
	if (hBmp == NULL)
		return false;
	BOOL bRet = SaveBitmapAsJpeg(hBmp, filePath.c_str());
	DeleteObject(hBmp);
	return bRet != FALSE;
}
