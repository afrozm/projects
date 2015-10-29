#include "StdAfx.h"
#include "StringMatcher.h"
#include "SystemUtils.h"

static CString WildCardToRegExp(LPCTSTR wildCard)
{
	LPTSTR regExp = new TCHAR[6*lstrlen(wildCard)+1];
	unsigned len = 0;

	while (*wildCard) {
		TCHAR extraCharToAdd = 0;

		switch (*wildCard) {
		case '*':
			extraCharToAdd = '.';
			break;
		case '.':
			extraCharToAdd = '\\';
			break;
		}
		if (extraCharToAdd)
			regExp[len++] = extraCharToAdd;
		regExp[len++] = *wildCard++;
	}
	regExp[len] = 0;
	CString regExpStr(regExp);

	delete[] regExp;

	return regExpStr;
}

CString WildCardExpToRegExp(LPCTSTR wildCardExp)
{
	CString exp(wildCardExp);
	CString token;
	int curPos = 0;
	token = exp.Tokenize(_T(";"), curPos);
	CString regExp;
	while (token != _T("")) {
		regExp += _T("(") + WildCardToRegExp(token) + _T(")");
		token = exp.Tokenize(_T(";"), curPos);
		if (token != _T("")) {
			regExp +=_T("|");
		}
	}
	return regExp;
}
bool IsWildCardExp(const CString &wildCardExp)
{
	return wildCardExp.FindOneOf(_T(";*[]|:")) >= 0;
}

#define MW_MATCH_EXACT 4
#define MW_MATCH_SOUNDING (MW_MATCH_EXACT-1)
#define MW_WHITE_SPACE (MW_MATCH_SOUNDING-1)
#define MW_NON_WHITE_SPACE (MW_WHITE_SPACE-1)
#define MW_MATCH_IN_ORDER (MW_WHITE_SPACE*2+1)
#define MW_MATCH_CHAR_IN_ORDER (MW_MATCH_EXACT>>1)

int StringMatcher::GetWordMatchCharWeight( TCHAR ch )
{
	int wt(0);
	if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\n')
		wt += MW_WHITE_SPACE;
	else if (!_istalnum(ch))
		wt += MW_NON_WHITE_SPACE;
	return wt;
}
int StringMatcher::GetWordMatchWeight(const CString &matchString, int startPos /* = 0 */, int lengthMatch /* = -1 */)
{
	int wMWt(0);
	if (startPos == 0)
		wMWt += MW_NON_WHITE_SPACE;
	else if (startPos > 0 && startPos < matchString.GetLength())
		wMWt += GetWordMatchCharWeight(matchString[startPos-1]);
	if (lengthMatch < 0)
		lengthMatch = matchString.GetLength();
	int endPos(startPos+lengthMatch);
	if (endPos >= matchString.GetLength())
		wMWt += MW_NON_WHITE_SPACE;
	else
		wMWt += GetWordMatchCharWeight(matchString[endPos]);
	return wMWt;
}

int StringMatcher::GetWordIndex( LPCTSTR inStr, int startPos /*= 0*/)
{
	if (!GetWordMatchCharWeight(inStr[startPos]) && startPos > 0) {					// Already in word
		for (++startPos; inStr[startPos]; ++startPos) { // skip all word letters
			if (GetWordMatchCharWeight(inStr[startPos]))
				break;
		}
	}
	if (GetWordMatchCharWeight(inStr[startPos])) { // if non-word char
		for (;inStr[startPos]; ++startPos) { // skip all non-word letters
			if (!GetWordMatchCharWeight(inStr[startPos]))
				break;
		}
	}
	return startPos;
}

CRegExpMatcher::CRegExpMatcher(LPCTSTR lpExpression, bool bExpressionIsRegExp, BOOL bCaseSensitive)
: mRegExp(NULL), mRegExpException(NULL), mbCaseSensitive(bCaseSensitive)
{
	SetExpression(lpExpression, bExpressionIsRegExp, bCaseSensitive);
}
void CRegExpMatcher::SetExpression(LPCTSTR lpExpression, bool bExpressionIsRegExp, BOOL bCaseSensitive)
{
	SetCaseSensitive(bCaseSensitive);
	Free();
	if (lpExpression == NULL)
		lpExpression = _T("*");
	if (lstrcmp(lpExpression, _T("*"))) {
		CString expr(lpExpression);
		CString excp;
		int exceptPos = expr.Find(':');
		if (exceptPos >= 0) {
			excp = expr.Right(expr.GetLength()-(exceptPos+1));
			expr = expr.Left(exceptPos);
		}
		if (expr.GetLength() > 0) {
			mRegExp = new CAtlRegExp<>();
			if (bExpressionIsRegExp)
				mRegExp->Parse(expr, mbCaseSensitive);
			else {
				mRegExp->Parse(WildCardExpToRegExp(expr), mbCaseSensitive);
			}
		}
		if (excp.GetLength() > 0) {
			mRegExpException = new CAtlRegExp<>();
			if (bExpressionIsRegExp)
				mRegExpException->Parse(excp, mbCaseSensitive);
			else {
				mRegExpException->Parse(WildCardExpToRegExp(excp), mbCaseSensitive);
			}
		}
	}
}
CRegExpMatcher::~CRegExpMatcher(void)
{
	Free();
}
void CRegExpMatcher::Free(void)
{
	if (mRegExp != NULL)
		delete mRegExp;
	mRegExp = NULL;
	if (mRegExpException != NULL)
		delete mRegExpException;
	mRegExpException = NULL;
}
bool CRegExpMatcher::Match(LPCTSTR matchString)
{
	bool bMatched(true);
	if (mRegExp) {
		CAtlREMatchContext<> mc;
		bMatched = mRegExp->Match(matchString, &mc) ? true : false;
	}
	if (bMatched && mRegExpException) {
		CAtlREMatchContext<> mc;
		bMatched = mRegExpException->Match(matchString, &mc) ? false : true;
	}
	return bMatched;
}

CStringMatcherList::CStringMatcherList(LPCTSTR lpExpression /* = NULL */)
	: mMatchWeight(0)
{
	SetExpression(lpExpression);
}
void CStringMatcherList::SetExpression(LPCTSTR lpExpression /* = NULL */)
{
	mMatchWeight = 0;
	mStrListToMatch.RemoveAll();
	if (lpExpression) {
		CString exp(lpExpression);
		exp.MakeLower();
		SystemUtils::SplitString(exp, mStrListToMatch, _T(" "));
		for (INT_PTR i = 0; i < mStrListToMatch.GetCount(); ++i) {
			mStrListToMatch[i].Trim();
			if (mStrListToMatch[i].IsEmpty()) {
				mStrListToMatch.RemoveAt(i, 1);
				--i;
			}
		}
	}
}
bool CStringMatcherList::Match(LPCTSTR matchString) 
{
	CString mS(matchString);
	mS.MakeLower();
	mMatchWeight = 0;
	int matchPos(-1);
	int matchCount(0);
	for (INT_PTR i = 0; i < mStrListToMatch.GetCount(); ++i) {
		int curMatchPos(mS.Find(mStrListToMatch[i], 0));
		if (curMatchPos >= 0) {
			++matchCount;
			mMatchWeight += GetWordMatchWeight(mS, curMatchPos, mStrListToMatch[i].GetLength());
			mS.SetAt(curMatchPos, ':');
			if (curMatchPos > matchPos) {
				mMatchWeight += MW_MATCH_IN_ORDER;
				++matchCount;
			}
			matchPos = curMatchPos;
		}
	}
	return matchCount > mStrListToMatch.GetCount();
}

CSimpleStringMatcher::CSimpleStringMatcher(LPCTSTR strToSearch /* = NULL */, BOOL bCaseSensitive /* = FALSE */, BOOL bMatchWholeWord /* = FALSE */, MatchCallback mcb /* = NULL */, void *pUserParam /* = NULL */)
	: muFlags(0)
{
	SetCallBack(mcb, pUserParam);
	SET_UNSET_FLAGBIT(bCaseSensitive, muFlags,  caseSensitive);
	SET_UNSET_FLAGBIT(bMatchWholeWord, muFlags,  matchWholeWord);
	if (strToSearch != NULL)
		mStrToSearch = strToSearch;
	if (!IS_FLAGBIT_SET(muFlags, caseSensitive))
		mStrToSearch.MakeLower();
}
void CSimpleStringMatcher::SetCallBack(MatchCallback mcb /* = NULL */, void *pUserParam /* = NULL */)
{
	mMatchCallback = mcb;
	m_pUserParam = pUserParam;
}
bool CSimpleStringMatcher::Match(LPCTSTR matchString)
{
	CString matchStr(matchString);
	if (!IS_FLAGBIT_SET(muFlags, caseSensitive))
		matchStr.MakeLower();
	bool bMatch(false);
	int startPos = 0;
	int lenStrToSearch = mStrToSearch.GetLength();
	while (startPos >= 0) {
		if (mMatchCallback != NULL)
			if (mMatchCallback(-1, m_pUserParam))
				break;
		startPos = matchStr.Find(mStrToSearch, startPos);
		if (startPos < 0)
			continue;
		bool bStrMatch(true);
		if (IS_FLAGBIT_SET(muFlags, matchWholeWord)) {
			bStrMatch = SystemUtils::IsCompleteWord(matchStr, startPos, lenStrToSearch);
		}
		if (!bStrMatch)
			continue;
		bMatch = true;
		if (mMatchCallback != NULL)
			if (mMatchCallback(startPos, m_pUserParam))
				break;
		startPos += lenStrToSearch;
	}
	return bMatch;
}
static bool IsVowel(TCHAR ch)
{
	return ch == 'a' || ch == 'e' || ch == 'i' || ch == 'o' || ch == 'u' || ch == 'y';
}
CString CPhoneticStringMatcher::GetPhoneticString(LPCTSTR inStr, PhoneticData &outPhoneticData, bool bUpdateCounts /* = true */)
{
	outPhoneticData.consonantCount = outPhoneticData.vowelCount = 0;
	CString outStr(inStr);
	outStr.MakeLower();
	outPhoneticData.mStrVowel = outStr;
	for (int i = 0; i < outStr.GetLength(); ++i) {
		if (IsVowel(outStr[i])) {
			outStr.SetAt(i, ':');
			outPhoneticData.vowelCount++;
		}
		else {
			outPhoneticData.mStrVowel.SetAt(i, ':');
			if (bUpdateCounts) {
				bool incrCount(true);
				if (i > 0 && outStr[i] == 'h')
					incrCount = false;
				if (incrCount)
					outPhoneticData.consonantCount++;
			}
		}
	}
	return outStr;
}

CPhoneticStringMatcher::GetPhoneticData::GetPhoneticData( TCHAR inCh /*= 0*/, LPCTSTR inChStr /*= NULL*/, int inChIndex /*= -1*/, LPCTSTR inTargetStr /*= NULL*/, int inTargerIndex /*= -1*/ )
	: ch(inCh), weight(0)
{
	chString[0] = inChStr;
	chString[1] = inTargetStr;
	chIndex[0] = inChIndex;
	chIndex[1] = inTargerIndex;
	lenIndex[0] = inChStr ? lstrlen(inChStr) : 0;
	lenIndex[1] = inTargetStr ? lstrlen(inTargetStr) : 0;
}

TCHAR CPhoneticStringMatcher::GetPhoneticData::GetChar( int idStr /*= 0*/, int relIndex /*= 1*/ )
{
	ASSERT(ch == chString[0][chIndex[0]]);
	TCHAR outCh(0);
	relIndex = chIndex[idStr]+relIndex;
	if (relIndex >= 0 && relIndex < lenIndex[idStr])
		outCh = chString[idStr][relIndex];
	return outCh;
}

TCHAR CPhoneticStringMatcher::GetPhoneticChar(GetPhoneticData &inOutPhoticCharData)
{
	struct PhoneticCharData {
		PhoneticCharData(TCHAR c = 0, int w = 0) : ch(c), weight(w) {}
		TCHAR ch;
		int weight;
	};
	TCHAR ch(inOutPhoticCharData.ch);
	static PhoneticCharData sMapPhoneticChars['z'-'a'+1];
	if (sMapPhoneticChars['c'-'a'].ch == 0) {
		sMapPhoneticChars['c'-'a']=PhoneticCharData('s',MW_MATCH_SOUNDING);
		sMapPhoneticChars['s'-'a']=PhoneticCharData('c',MW_MATCH_SOUNDING);
		sMapPhoneticChars['k'-'a']=PhoneticCharData('q',MW_MATCH_SOUNDING);
		sMapPhoneticChars['q'-'a']=PhoneticCharData('k',MW_MATCH_SOUNDING);
		sMapPhoneticChars['v'-'a']=PhoneticCharData('w',MW_MATCH_SOUNDING);
		sMapPhoneticChars['w'-'a']=PhoneticCharData('v',MW_MATCH_SOUNDING);
		sMapPhoneticChars['j'-'a']=PhoneticCharData('z',MW_MATCH_SOUNDING-1);
		sMapPhoneticChars['z'-'a']=PhoneticCharData('j',MW_MATCH_SOUNDING-1);
	}
	PhoneticCharData pd;
	if (ch >= 'a' && ch <= 'z')
		pd = sMapPhoneticChars[ch-'a'];
	inOutPhoticCharData.weight = pd.weight;
	return pd.ch;
}
CPhoneticStringMatcher::CPhoneticStringMatcher( LPCTSTR lpExpression /*= NULL*/ )
	: mMatchWeight(0), m_iMatchStartInddex(-1)
{
	SetExpression(lpExpression);
}

void CPhoneticStringMatcher::SetExpression( LPCTSTR lpExpression /*= NULL*/ )
{
	CString vowels;
	mStrOrgExpression = lpExpression;
	mStrOrgExpression.MakeLower();
	mStrPhonetic = GetPhoneticString(lpExpression, mPhoneticData);
	mMatchWeight = 0;
	m_iMatchStartInddex = -1;
}
#define SKIP_TILL_MATCH(s,i,c) while(s[i] && s[i]==c) ++i


int CPhoneticStringMatcher::MatchChar( TCHAR ch, TCHAR ch2 )
{
	int wt(0);
	GetPhoneticData phd(ch);
	if (ch || ch2) {
		if (ch == ch2)
			wt = MW_MATCH_EXACT;
		else if (ch == GetPhoneticChar(phd))
			wt = phd.weight;
	}
	return wt;
}

bool CPhoneticStringMatcher::Match( LPCTSTR matchString )
{
	mMatchWeight = 0;
	m_iMatchStartInddex = -1;
	if (mStrPhonetic.GetLength() <= 0)
		return false;
	const bool bDoFullMatch(mPhoneticData.consonantCount < 3);
	if (mPhoneticData.consonantCount == 0) {
		CString str(matchString);
		str.MakeLower();
		m_iMatchStartInddex = str.Find(mStrOrgExpression, 0);
		if (m_iMatchStartInddex >= 0) {
			mMatchWeight = mStrOrgExpression.GetLength() << 1;
			mMatchWeight += GetWordMatchWeight(str, m_iMatchStartInddex, str.GetLength());
			return true;
		}
		return false;
	}
	PhoneticData phd;
	int startIndex(0);
	CString phoneticStr(GetPhoneticString(matchString, phd, false));
	int matchIndex(0);
	int matchCount(0);
	startIndex = GetWordIndex(matchString, startIndex);
	TCHAR prevCh(0), prevCh2(0);
	int orderWt(0);
	while ( true ) {
		SKIP_TILL_MATCH(phoneticStr, startIndex, ':');
		SKIP_TILL_MATCH(mStrPhonetic, matchIndex, ':');
		TCHAR ch2(startIndex < phoneticStr.GetLength() ? phoneticStr[startIndex++] : 0);
		TCHAR ch(matchIndex < mStrPhonetic.GetLength() ? mStrPhonetic[matchIndex++] : 0);
		int mw(MatchChar(ch, ch2));
		if (mw); // Char match
		else if (prevCh && (ch == 'h' || MatchChar(ch, prevCh))) {
			mw = -1;
			if (ch2)
				--startIndex;
			if (ch == 'h' && prevCh != ch)
				prevCh = 0;
		}
		else if (prevCh2 && (ch2 == 'h' || MatchChar(ch2, prevCh2))) {
			mw = -1;
			if (ch)
				--matchIndex;
			if (ch == 'h' && prevCh2 != ch2)
				prevCh2 = 0;
		}
		if (mw > 0) { // then add weight
			mMatchWeight += mw + orderWt;
			if (m_iMatchStartInddex < 0)
				m_iMatchStartInddex = startIndex-1;
			++matchCount;
			prevCh2 = ch2;
			prevCh = ch;
			orderWt = MW_MATCH_CHAR_IN_ORDER;
		}
		else if (mw == 0) { // restart from beginning
			bool dDone(false);
			if (ch == 0) {// all matched
				dDone = true;
				if (bDoFullMatch)
					dDone = GetWordMatchCharWeight(ch2) > 0;
			}
			if (dDone) {
				if (ch2)
					--startIndex;
				break;
			}
			if (matchCount > 0)
				--startIndex;
			if (bDoFullMatch)
				startIndex = GetWordIndex(matchString, startIndex);
			matchIndex = 0;
			matchCount = 0;
			m_iMatchStartInddex = -1;
			mMatchWeight = 0;
			prevCh = prevCh2 = 0;
			if (ch2 == 0)
				break;
		}
		if (mw <= 0)
			orderWt = 0;
	}
	if (mMatchWeight > 0) {
		mMatchWeight += GetWordMatchWeight(matchString, m_iMatchStartInddex, startIndex-m_iMatchStartInddex) + orderWt;
		CString &vowels(phd.mStrVowel);
		matchCount = bDoFullMatch ? startIndex : vowels.GetLength();
		startIndex = m_iMatchStartInddex;
		matchIndex = 0;
		CString &mStrVowel(mPhoneticData.mStrVowel);
		while ( startIndex < matchCount && matchIndex < mStrVowel.GetLength() ) {
			TCHAR ch2(vowels[startIndex++]);
			if (ch2 == ':') {// Skip escape char
				SKIP_TILL_MATCH(vowels, startIndex, ':');
				continue;
			}
			TCHAR ch(mStrVowel[matchIndex++]);
			if (ch == ':') { // Skip escape char
				SKIP_TILL_MATCH(mStrVowel, matchIndex, ':');
				--startIndex;
				continue;
			}
			int mw(0);
			if (ch == ch2) // Char match
				mw = MW_MATCH_EXACT;
			//else { // Later for vowel phonetic match
			//	ch2 = GetPhoneticChar(ch2, &mw); // or phonetic char match
			//	if (ch != ch2)
			//		mw = 0;
			//}
			if (mw) // then add weight
				mMatchWeight += mw;
		}
	}
	return mMatchWeight > 0;
}

bool CPhoneticStringMatcher::StringHasVowels( LPCTSTR str )
{
	CString vowels(str);
	vowels.MakeLower();
	return vowels.FindOneOf(_T("aeiouy")) >= 0;
}

CPhoneticStringMatcherList::CPhoneticStringMatcherList( LPCTSTR lpExpression /*= NULL*/ )
{
	SetExpression(lpExpression);
}

void CPhoneticStringMatcherList::SetExpression( LPCTSTR lpExpression /*= NULL*/ )
{
	__super::SetExpression(lpExpression);
	mPhoneticMatchers.RemoveAll();
	for (INT_PTR i = 0; i < GetWordCount(); ++i)
		mPhoneticMatchers.Add(CPhoneticStringMatcher(mStrListToMatch[i]));
}

bool CPhoneticStringMatcherList::Match( LPCTSTR matchString )
{
	CString mS(matchString);
	mS.MakeLower();
	mMatchWeight = 0;
	int matchPos(-1);
	int localWight(0);
	for (INT_PTR i = 0; i < mPhoneticMatchers.GetCount(); ++i) {
		if (mPhoneticMatchers[i].Match(mS)) {
			mMatchWeight += mPhoneticMatchers[i].GetMatchWeight();
			int curMatchPos(mPhoneticMatchers[i].GetMatchIndex());
			if (curMatchPos >= 0) {
				mS.SetAt(curMatchPos, '?');
				++localWight;
				if (curMatchPos > matchPos) {
					mMatchWeight += MW_MATCH_IN_ORDER;
					++localWight;
				}
				matchPos = curMatchPos;
			}
		}
	}
	return localWight > mStrListToMatch.GetCount();
}
