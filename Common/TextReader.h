#pragma once
#include "DataReader.h"

typedef enum {
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
	TCHAR ReadChar();
	lstring ReadLine();
	lstring ReadLine(LONGLONG atPos);
	LONGLONG GetFilePos();
	LONGLONG SetFilePos(LONGLONG, DWORD dwMoveMethod = FILE_BEGIN);
	LONGLONG GetFileSize();
	bool operator!() {return m_pDataReader == NULL;}
	int LineCount();
private:
	lstring ReadLineUTF8();
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