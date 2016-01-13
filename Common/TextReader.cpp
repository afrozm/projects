#include "StdAfx.h"
#include "TextReader.h"

#define API_FAIL_EXIT(x) if (x) {bEndOfFileReached = true; goto API_EXIT;}

CTextReader::CTextReader(LPCTSTR fileName)
:mFileEncoding(FileEncoding_ANSI), bEndOfFileReached(false)
{
	m_hFile = CreateFile(fileName, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (m_hFile == INVALID_HANDLE_VALUE)
		return;
	int encoding = 0;
	DWORD nBytesRead = 0;
	ReadFile(m_hFile, &encoding, sizeof(int), &nBytesRead, NULL);
	switch (encoding & 0xffff) {
	case 0xbbef:
		if ((encoding >> 16 & 0xff) == 0xbf) {
			mFileEncoding = FileEncoding_UTF8;
			SetFilePos(3);
		}
		else
			SetFilePos(0);
		break;
	case 0xFFFE:
		mFileEncoding = FileEncoding_UNICODE_BIG;
		SetFilePos(2);
		break;
	case 0xFEFF:
		mFileEncoding = FileEncoding_UNICODE;
		SetFilePos(2);
		break;
	default: // Assume ANSI
		if ((encoding & 0xff00ff00)== 0)
			mFileEncoding = FileEncoding_UNICODE;
		else if ((encoding & 0x00ff00ff) == 0)
			mFileEncoding = FileEncoding_UNICODE_BIG;
		SetFilePos(0);
	}
}

CTextReader::~CTextReader(void)
{
	if (m_hFile != INVALID_HANDLE_VALUE)
		CloseHandle(m_hFile);
}

LONGLONG CTextReader::SetFilePos(LONGLONG filePos, DWORD dwMoveMethod)
{
	LARGE_INTEGER fp;
	fp.QuadPart = filePos;
	LARGE_INTEGER newFilePos = {0};
	if (m_hFile != INVALID_HANDLE_VALUE) {
		SetFilePointerEx(m_hFile, fp, &newFilePos, dwMoveMethod);
	}
	bEndOfFileReached = false;
	return newFilePos.QuadPart;
}

LONGLONG CTextReader::GetFilePos()
{
	return SetFilePos(0, FILE_CURRENT);
}
LONGLONG CTextReader::GetFileSize()
{
	LARGE_INTEGER fileSize = {0};
	::GetFileSizeEx(m_hFile, &fileSize);
	return fileSize.QuadPart;
}
TCHAR CTextReader::ReadChar()
{
	TCHAR tch = 0;
	DWORD nBytesRead = 0;
	API_FAIL_EXIT(bEndOfFileReached);
	switch (mFileEncoding) {
	case FileEncoding_ANSI:
	case FileEncoding_UTF8:
		{
			unsigned char ch[2];
			ReadFile(m_hFile, ch, sizeof(char), &nBytesRead, NULL);
			API_FAIL_EXIT(nBytesRead == 0);
			tch = ch[0];
#if 0//defined(_UNICODE) || defined(UNICODE)
			if (mFileEncoding == FileEncoding_UTF8
				&& ch[0] > 127) {
				ReadFile(m_hFile, ch+1, sizeof(char), &nBytesRead, NULL);
				API_FAIL_EXIT(nBytesRead == 0);
				tch |= ((TCHAR)ch[1]) << 8;
			}
#endif
		}
		break;
	case FileEncoding_UNICODE:
	case FileEncoding_UNICODE_BIG:
		{
			WCHAR ch = 0;
			ReadFile(m_hFile, &ch, sizeof(WCHAR), &nBytesRead, NULL);
			API_FAIL_EXIT(nBytesRead == 0);
			if (mFileEncoding == FileEncoding_UNICODE_BIG)
				ch = (ch << 8) | (ch >> 8);
#if defined(_UNICODE) || defined(UNICODE)
			tch = ch;
#else
			WideCharToMultiByte( CP_ACP, 0, &ch, -1,
				&tch, sizeof (TCHAR), NULL, NULL );
#endif
		}
		break;
	}
API_EXIT:
	return tch;
}

lstring CTextReader::ReadLine()
{
	if (mFileEncoding == FileEncoding_UTF8)
		return ReadLineUTF8();
	TCHAR line[1024];
	lstring str;
	int c = 0;
	TCHAR ch = ReadChar();
	// Skip any new line char
	while (!bEndOfFileReached && (ch == '\n' || ch == '\r'))
		ch = ReadChar();
	while (!bEndOfFileReached && (ch != '\n' && ch != '\r')) {
		if (c < 1023) {
			line[c++] = ch;
		}
		else {
			line[c] = 0;
			str += line;
			c = 0;
		}
		ch = ReadChar();
	}
	line[c] = 0;
	str += line;
	return str;
}
lstring CTextReader::ReadLine(LONGLONG atPos)
{
	SetFilePos(atPos);
	return ReadLine();
}
lstring CTextReader::ReadLineUTF8()
{
	int len = 1024;
	char *line = (char *)malloc(len);
	lstring str;
	int c = 0;
	TCHAR ch = ReadChar();
	// Skip any new line char
	while (!bEndOfFileReached && (ch == '\n' || ch == '\r'))
		ch = ReadChar();
	while (!bEndOfFileReached && (ch != '\n' && ch != '\r')) {
		if (c < len-1) {
			line[c++] = (char)ch;
		}
		else {
			len += 1024;
			line = (char *)realloc(line, len);
			line[c++] = (char)ch;
		}
		ch = ReadChar();
	}
	line[c] = 0;
	if (c > 0) {
		TCHAR *convString = (TCHAR *)line;
#if defined(_UNICODE) || defined(UNICODE)
		len = MultiByteToWideChar(CP_UTF8, 0, line, c, NULL, 0);
		if (len > 0) {
			convString = new TCHAR[len+1];
			len = MultiByteToWideChar(CP_UTF8, 0, line, c, convString, len);
			convString[len] = 0;
		}
#endif
		str += convString;
		if (convString != (TCHAR *)line)
			delete[] convString;
	}
	free(line);
	return str;
}

int CTextReader::LineCount()
{
	CAutoFilePos autoFilePos(this);
	int lineCount = 0;

	while (!ReadLine().empty())
		lineCount++;

	return lineCount;
}