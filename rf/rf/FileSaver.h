#pragma once
#include "Path.h"
class CFileSaver
{
public:
	CFileSaver();
	~CFileSaver(void);
	BOOL SetSavePath(LPCTSTR saveDir);
	BOOL OpenNew(LPCTSTR ext = NULL, LPCTSTR prefix = NULL);
    BOOL WriteFrom(LPCTSTR inFromFile, long long startPos = 0, long long endPos = -1);
	BOOL Write(const void *buffer, size_t size);
	void Close();
private:
	int mid;
	Path m_SavePath;
	HANDLE m_hFile;
};

