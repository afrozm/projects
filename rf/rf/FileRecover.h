#pragma once
#include "PatternData.h"

class CFileRecover
{
public:
	CFileRecover(void);
	~CFileRecover(void);
	bool ParseBuffer(const void *buffer, unsigned int size);
	//BOOL SetSavePath(LPCTSTR saveDir)
	//{	return mFileSaver.SetSavePath(saveDir);		}
private:

};

