#include "StdAfx.h"
#include "FileContentFinder.h"
#include "TextReader.h"



CFileContentFinder::CFileContentFinder(CFileContentFinderCallback *pCallback /*= NULL*/)
	: m_pStringMatcher(NULL)
{
	SetCallBack(pCallback);
}
void CFileContentFinder::SetCallBack(CFileContentFinderCallback *pCallback /* = NULL */)
{
	m_pFinderCallback = pCallback;
}
int CFileContentFinder::Find(LPCTSTR fileName, LPCTSTR matchString)
{
	CSimpleStringMatcher sm(matchString);
	bool bSetSMToNull(m_pStringMatcher == NULL);
	if (bSetSMToNull)
		m_pStringMatcher = &sm;
	CFileContentFinderCallback::MatchData md;
	CTextLineReader textReader(fileName);
	while (true) {
		md.strLine = textReader.ReadLine().c_str();
		if (md.strLine.IsEmpty())
			break;
		md.bMatched = m_pStringMatcher->Match(md.strLine);
		if (md.bMatched)
			++md.iMatchCount;
		md.iLineNo = textReader.GetCurrentLineNumber();
		if (m_pFinderCallback != NULL) {
			if (m_pFinderCallback->MatchCallback(md))
				break;
		}
	}
	if (bSetSMToNull)
		m_pStringMatcher = NULL;
	return md.iMatchCount;
}

CFileContentFinder::~CFileContentFinder(void)
{
}
