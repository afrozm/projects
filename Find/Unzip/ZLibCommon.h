
#pragma once

#ifdef _WIN32
#include "windows.h"
#include <tchar.h>
#else
#include "CommonDef.h"
#endif
// message
#define ZLIBCBFN_START 0
#define ZLIBCBFN_FILE_START 1
#define ZLIBCBFN_FILE 2
#define ZLIBCBFN_FILE_FINISH 3
#define ZLIBCBFN_FINISH 4

// Return value
#define ZLIBCBFN_CONTINUE 0
#define ZLIBCBFN_STOP 1

struct ZLibCBData {
	UINT message;
	DWORD szBytesProcessed;
	void *pUserData;
	ZLibCBData()
		: message(ZLIBCBFN_START), szBytesProcessed(0), pUserData(NULL)
	{	}
};
typedef int (*ZLibCallback)(ZLibCBData *pData);
