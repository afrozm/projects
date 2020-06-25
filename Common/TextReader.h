#pragma once
#include "DataReader.h"

typedef enum {
    FileEncoding_NotInitialized,
    FileEncoding_Invalid, // If ansi and not text file
	FileEncoding_ANSI,
	FileEncoding_UTF8,
	FileEncoding_UNICODE,
	FileEncoding_UNICODE_BIG,
} FileEncoding;

class CTextReader
{
public:
	CTextReader(LPCTSTR fileName = NULL);
    CTextReader(const BinaryData *pData);
    BOOL SetInput(LPCTSTR fileName = NULL, const BinaryData *pData = NULL);
    virtual BOOL SetFile(LPCTSTR fileName) { return SetInput(fileName); }
	TCHAR ReadChar();
	lstring ReadLine(LPCTSTR includeLineFeed = nullptr);
	lstring ReadLine(LONGLONG atPos);
    lstring Read(DWORD nBytes = 64*1024);
	LONGLONG GetFilePos();
	LONGLONG SetFilePos(LONGLONG, DWORD dwMoveMethod = FILE_BEGIN);
	LONGLONG GetFileSize();
	bool operator!() {return m_pDataReader == NULL;}
	int LineCount();
    BOOL IsValidTextFile();
private:
	lstring ReadLineUTF8(LPCTSTR includeLineFeed = nullptr);
    DataReader *m_pDataReader;
    BinaryDataReader mBinaryDataReader;
    FileDataReader mFileDataReader;
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

class CTextLineReader : public CTextReader {
public:
    CTextLineReader(LPCTSTR fileName = NULL);
    lstring ReadLine();
    BOOL SetFile(LPCTSTR fileName = NULL);
    int GetCurrentLineNumber() const { return m_iLineCount + 1; }
    int GetLineCount() const { return m_iLineCount; }
private:
    lstring mReadBuffer;
    int m_iStartPos;
    int m_iLineCount;
};
