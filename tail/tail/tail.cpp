// tail.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "TextReader.h"

static int Help()
{
	_tprintf(_T("tail <file path> [size]"));
	return 1;
}
//int _tmain(int argc, _TCHAR* argv[])
//{
//	if (argc < 2 || !lstrcmp(argv[1], _T("/?")))
//		return Help();
//	FILE *pFile(NULL);
//	_tfopen_s(&pFile, argv[1], _T("r"));
//	if (pFile == NULL) {
//		_tprintf(_T("Cannot open file \"%s\" to read. Error: %d"), argv[1], GetLastError());
//		return GetLastError();
//	}
//	if (argc > 2) {
//		__int64 pos = _ttoi64(argv[2]);
//		if (pos < 0)
//			pos = -pos;
//		if (pos > 0)
//			_fseeki64(pFile, pos, SEEK_END);
//	}
//	while (1) {
//		if (_kbhit() && _getch() == 27) { // ESC pressed
//			break;
//		}
//		TCHAR readChar[1024];
//		if (_fgetts(readChar, 1024, pFile)) {
//			_tprintf(_T("%s"), readChar);
//		}
//		else Sleep(100);
//	}
//	fclose(pFile);
//	return 0;
//}

int _tmain(int argc, _TCHAR* argv[])
{
	if (argc < 2 || !lstrcmp(argv[1], _T("/?")))
		return Help();
	CTextReader textReader(argv[1]);
	if (!textReader) {
		_tprintf(_T("Cannot open file \"%s\" to read. Error: %d"), argv[1], GetLastError());
		return GetLastError();
	}
	if (argc > 2) {
		__int64 pos = _ttoi64(argv[2]);
		if (pos > 0)
			pos = -pos;
		if (pos < 0) {
			LARGE_INTEGER li;
			li.QuadPart = pos;
			textReader.SetFilePos(pos, FILE_END);
		}
	}
	while (1) {
		if (_kbhit() && _getch() == 27) { // ESC pressed
			break;
		}
		lstring line(textReader.ReadLine());
		if (!line.empty()) {
			_tprintf(_T("%s\n"), line.c_str());
		}
		else Sleep(100);
		textReader.SetFilePos(0, FILE_CURRENT);
	}
	return 0;
}


