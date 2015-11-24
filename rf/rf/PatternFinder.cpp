#include "StdAfx.h"
#include "JpegFinder.h"


CJpegFinder::CJpegFinder(void)
	: mState(FindStartMark)
{
}


CJpegFinder::~CJpegFinder(void)
{
}
bool CJpegFinder::ParseBuffer(const void *buffer, unsigned int inSize)
{
	unsigned int size = inSize;
	const unsigned char *cBuffer = (const unsigned char *)buffer;
	bool bSuccess(false);
	switch (mState) {
	case FindStartMark:
	{
		while (size) {
			const unsigned int *iBuffer = (const unsigned int *)cBuffer;
			if (*iBuffer == 0xe1ffd8ff) {
				mState = FindFirstEndMark;
				break;
			}
			else {
				++cBuffer;
				--size;
			}
		}
		if (mState != FindStartMark) {
			mFileSaver.OpenNew(_T("jpg"), _T("IMG"));
		}
		else break;
	}
	case FindFirstEndMark:
	case FindEndMark:
	{
		buffer = cBuffer;
		while (size) {
			const unsigned short *iBuffer = (const unsigned short *)cBuffer;
			if (*iBuffer == 0xd9ff) {
				mState = (State)(mState + 1);
				++cBuffer;
				--size;
			}
			++cBuffer;
			--size;
			if (mState == (FindEndMark+1)) {
				break;
			}
		}
		mFileSaver.Write(buffer, (const unsigned char *)cBuffer - (const unsigned char *)buffer);
		if (mState == (FindEndMark+1)) {
			bSuccess = true;
			mState = FindStartMark;
			mFileSaver.Close();
		}
		else
			break;
	}
	}
	return bSuccess;
}