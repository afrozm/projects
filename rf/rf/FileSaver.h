#pragma once
#include "Path.h"
class CFileSaver
{
public:
	CFileSaver();
	~CFileSaver(void);
	BOOL SetSavePath(LPCTSTR saveDir);
	BOOL OpenNew(LPCTSTR ext = NULL, LPCTSTR prefix = NULL);
	BOOL Write(const void *buffer, int size);
	void Close();
private:
	int mid;
	Path m_SavePath;
	HANDLE m_hFile;
};

