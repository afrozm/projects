#pragma once
#include "FileSaver.h"

class CFileRecover
{
public:
	CFileRecover(void);
	~CFileRecover(void);
	virtual bool ParseBuffer(const void *buffer, unsigned int size) = 0;
	BOOL SetSavePath(LPCTSTR saveDir)
	{	return mFileSaver.SetSavePath(saveDir);		}
protected:
	CFileSaver mFileSaver;
};

