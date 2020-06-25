#pragma once
#include "Path.h"

// Images
HBITMAP CopyHwndToBitmap(HWND hWnd, bool bEntire  = true , LPRECT area  = NULL );
BOOL SaveBitmapAsJpeg(HBITMAP hBmp, LPCTSTR filePath);
BOOL SaveHwndImage(HWND hWnd, int msg = 0);

// Image Path
Path GetImagePath();
