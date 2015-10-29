#include "StdAfx.h"
#include "TextReader.h"

#define API_FAIL_EXIT(x) if (x) {bEndOfFileReached = true; goto API_EXIT;}

#define TR_BUFFER_SIZE 512*1024 // 512KB

CTextReader::CTextReader(LPCTSTR fileName)
:mFileEncoding(FileEncoding_ANSI), m_hFile(INVALID_HANDLE_VALUE)
{
	SetFile(fileName);
}
BOOL CTextReader::SetFile(LPCTSTR fileName)
{
	mFileEncoding = FileEncoding_ANSI;
	if (m_hFile != INVALID_HANDLE_VALUE)
		CloseHandle(m_hFile);
	m_hFile = INVALID_HANDLE_VALUE;
	BOOL bSuccess(FALSE);
	if (fileName && *fileName) {
		m_hFile = CreateFile(fileName, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE, NULL,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (m_hFile != INVALID_HANDLE_VALUE) {
			bSuccess = TRUE;
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
	}
	return bSuccess;
}

CTextReader::~CTextReader(void)
{
	SetFile();
}

LONGLONG CTextReader::SetFilePos(LONGLONG filePos, DWORD dwMoveMethod)
{
	LARGE_INTEGER fp;
	fp.QuadPart = filePos;
	LARGE_INTEGER newFilePos = {0};
	if (m_hFile != INVALID_HANDLE_VALUE) {
		SetFilePointerEx(m_hFile, fp, &newFilePos, dwMoveMethod);
	}
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
CString CTextReader::Read(DWORD nBytes)
{
	CString outStr;
	char *pBuffer = new char[nBytes+2*sizeof(TCHAR)];
	ReadFile(m_hFile, pBuffer, nBytes, &nBytes, NULL);
	if (nBytes > 0) {
		TCHAR *convString = (TCHAR *)pBuffer;
		{
			TCHAR *lastChar = (TCHAR*)(pBuffer+nBytes);
			lastChar[0] = 0;
			lastChar[1] = 0;
		}
		bool bfreeConvStr(false);
		switch (mFileEncoding) {
		case FileEncoding_ANSI:
		case FileEncoding_UTF8:
		{
#if defined(_UNICODE) || defined(UNICODE)
			int len = MultiByteToWideChar(CP_UTF8, 0, pBuffer, -1, NULL, 0);
			if (len > 0) {
				convString = new TCHAR[len+1];
				len = MultiByteToWideChar(CP_UTF8, 0, pBuffer, -1, convString, len);
				convString[len] = 0;
				bfreeConvStr = true;
			}
#endif
		break;
		}
		case FileEncoding_UNICODE_BIG:
			convString = (TCHAR *)(pBuffer+1);
			break;
		}
		outStr = convString;
		if (bfreeConvStr)
			delete []convString;
	}
	delete []pBuffer;
	return outStr;
}


CTextLineReader::CTextLineReader( LPCTSTR fileName /*= NULL*/ )
	: CTextReader(fileName), m_iStartPos(0), m_iLineCount(0)
{

}

CString CTextLineReader::ReadLine()
{
	CString outStr;
	while (true) {
		LPCTSTR pBuffer(mReadBuffer);
		pBuffer += m_iStartPos;
		TCHAR pCH(0);
		while (IS_LINE_CHAR(*pBuffer)) {
			if (!(*pBuffer == '\n' && pCH == '\r'))
				m_iLineCount++;
			pCH = *pBuffer;
			++pBuffer;
		}
		LPCTSTR startBuf(pBuffer);
		STR_SKIP_TILL_LINE(pBuffer);
		int iLen((int)(pBuffer-startBuf));
		if (iLen > 0) {
			outStr += CString(startBuf, iLen);
			m_iStartPos = (int)(pBuffer - (LPCTSTR)mReadBuffer);
			break;
		}
		else {
			m_iStartPos = 0;
			mReadBuffer = Read();
			if (mReadBuffer.IsEmpty()) {
				break;
			}
		}
	}
	return outStr;
}

BOOL CTextLineReader::SetFile( LPCTSTR fileName /*= NULL*/ )
{
	m_iStartPos = m_iLineCount = 0;
	mReadBuffer.Empty();
	return __super::SetFile(fileName);
}
