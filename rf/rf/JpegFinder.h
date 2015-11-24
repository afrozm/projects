#pragma once

#include "FileRecover.h"

class CJpegFinder : public CFileRecover
{
public:
	CJpegFinder(void);
	~CJpegFinder(void);
	bool ParseBuffer(const void *buffer, unsigned int size);
protected:
	enum State {
		FindStartMark,
		FindFirstEndMark,
		FindSecEndMark,
		FindEndMark
	};
	State mState;
};

