#pragma once

typedef enum {
	FileEncoding_ANSI,
	FileEncoding_UTF8,
	FileEncoding_UNICODE,
	FileEncoding_UNICODE_BIG,
} FileEncoding;


#define TR_DEFAULT_CHUNK_SIZE 64*1024

class CTextReader
{
public:
	CTextReader(LPCTSTR fileName = NULL);
	~CTextReader(void);
	virtual BOOL SetFile(LPCTSTR fileName = NULL);
	LONGLONG GetFileSize();
	CString Read(DWORD nBytes = TR_DEFAULT_CHUNK_SIZE);
	LONGLONG SetFilePos(LONGLONG filePos, DWORD dwMoveMethod = FILE_BEGIN);
	LONGLONG GetFilePos();
private:
	HANDLE m_hFile;
	FileEncoding mFileEncoding;
};

class CTextLineReader : public CTextReader {
public:
	CTextLineReader(LPCTSTR fileName = NULL);
	CString ReadLine();
	BOOL SetFile(LPCTSTR fileName = NULL);
	int GetCurrentLineNumber() const {return m_iLineCount+1;}
	int GetLineCount() const {return m_iLineCount;}
private:
	CString mReadBuffer;
	int m_iStartPos;
	int m_iLineCount;
};