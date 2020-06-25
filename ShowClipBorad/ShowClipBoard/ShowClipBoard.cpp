// ShowClipBorad.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


int _tmain(int argc, _TCHAR* argv[])
{
	if (!OpenClipboard(NULL)) {
		_tprintf(_T("Cannot open clip board (%d)\n"), GetLastError());
		return 1;
	}
	UINT uFormat = 0;
	TCHAR name[1024];
	if (IsClipboardFormatAvailable(CF_HDROP)) {
		_tprintf(_T("CF_HROP format available. Files are\n"));
		HDROP hDrop = (HDROP)GetClipboardData(CF_HDROP);
		if (hDrop) {
			int nFiles(DragQueryFile(hDrop, -1, NULL, 0));
			for (int i = 0; i < nFiles; ++i) {
				name[0] = 0;
				DragQueryFile(hDrop, i, name, sizeof(name) / sizeof(name[0]));
				if (name[0])
					_tprintf(_T("%s\n"), name);
			}
		}
	}
	else if (IsClipboardFormatAvailable(CF_UNICODETEXT)) {
		_tprintf(_T("CF_UNICODETEXT format available. Text:\n"));
		HANDLE hDrop = GetClipboardData(CF_UNICODETEXT);
		if (hDrop) {
			LPCTSTR pDropFiles = (LPCTSTR)GlobalLock(hDrop);
			_tprintf(_T("%s\n"), pDropFiles);
			GlobalUnlock(hDrop);
		}
	}
	else if (IsClipboardFormatAvailable(CF_TEXT)) {
		_tprintf(_T("CF_TEXT format available. Text:\n"));
		HANDLE hDrop = GetClipboardData(CF_TEXT);
		if (hDrop) {
			LPCSTR pDropFiles = (LPCSTR)GlobalLock(hDrop);
			printf("%s\n", pDropFiles);
			GlobalUnlock(hDrop);
		}
	}
	while (uFormat = EnumClipboardFormats(uFormat)) {
		name[0] = 0;
		GetClipboardFormatName(uFormat, name, sizeof(name)/sizeof(TCHAR));
		if (name[0])
			_tprintf(_T("Clip Board Data format: %s\n"), name);
	}
	CloseClipboard();
	return 0;
}

