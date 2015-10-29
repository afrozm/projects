#pragma once

#include "StringMatcher.h"

class CFileContentFinderCallback {
public:
	struct MatchData {
		MatchData() : iLineNo(0), bMatched(false), iMatchCount(0) {}
		int iLineNo;
		CString strLine;
		bool bMatched;
		int iMatchCount;
	};
	virtual int MatchCallback(const MatchData &inMatchData) = 0;
};

class CFileContentFinder
{
public:
	CFileContentFinder(CFileContentFinderCallback *pCallback = NULL);
	~CFileContentFinder(void);
	int Find(LPCTSTR fileName, LPCTSTR matchString);
	void SetCallBack(CFileContentFinderCallback *pCallback = NULL);
protected:
	CFileContentFinderCallback *m_pFinderCallback;
	StringMatcher *m_pStringMatcher;
};

