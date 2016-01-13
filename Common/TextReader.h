#pragma once

typedef enum {
	FileEncoding_ANSI,
	FileEncoding_UTF8,
	FileEncoding_UNICODE,
	FileEncoding_UNICODE_BIG,
} FileEncoding;

class CTextReader
{
public:
	CTextReader(LPCTSTR fileName);
	~CTextReader(void);
	TCHAR ReadChar();
	lstring ReadLine();
	lstring ReadLine(LONGLONG atPos);
	LONGLONG GetFilePos();
	LONGLONG SetFilePos(LONGLONG, DWORD dwMoveMethod = FILE_BEGIN);
	LONGLONG GetFileSize();
	bool operator!() {return m_hFile == INVALID_HANDLE_VALUE;}
	int LineCount();
private:
	lstring ReadLineUTF8();
	HANDLE m_hFile;
	FileEncoding mFileEncoding;
	bool bEndOfFileReached;
};


class CAutoFilePos
{
public:
	CAutoFilePos(CTextReader *pTextReader)
		:m_pTextReader(pTextReader)
	{
		m_i64FilePos = m_pTextReader->GetFilePos();
	}
	~CAutoFilePos()
	{
		m_pTextReader->SetFilePos(m_i64FilePos);
	}
private:
	CTextReader *m_pTextReader;
	LONGLONG m_i64FilePos;
};