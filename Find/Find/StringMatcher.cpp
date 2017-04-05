#include "StdAfx.h"
#include "StringMatcher.h"
#include "WordParser.h"


bool IsWildCardExp(LPCTSTR wildCardExp)
{
	return StdString(wildCardExp).find_first_of(_T(";*[]|:")) != StdString::npos;
}

StringMatcher* StringMatcher_GetStringMatcher(LPCTSTR inString)
{
    if (IsWildCardExp(inString))
        return new CRegExpMatcher(inString);
    if (StringMatcher_IsPhonetic(inString))
        return new CPhoneticStringMatcherList(inString+1);
    if (StdString(inString).find(' ') != StdString::npos) // has spaces
        return new CStringMatcherList(inString);
    if (inString != nullptr && *inString)
        return new CSimpleStringMatcher(inString);
    return nullptr;
}
bool StringMatcher_IsSimpleMatch(LPCTSTR inString)
{
    if (inString == nullptr || *inString == 0)
        return true;
    if (IsWildCardExp(inString))
        return false;
    if (StringMatcher_IsPhonetic(inString))
        return false;
    return StdString(inString).find(' ') == StdString::npos;
}


bool StringMatcher_IsPhonetic(LPCTSTR inString)
{
    return !inString && inString[0] == '?';
}

#define MW_MATCH_EXACT 4
#define MW_MATCH_SOUNDING (MW_MATCH_EXACT-1)
#define MW_WHITE_SPACE (MW_MATCH_SOUNDING-1)
#define MW_NON_WHITE_SPACE (MW_WHITE_SPACE-1)
#define MW_MATCH_IN_ORDER (MW_WHITE_SPACE*2+1)
#define MW_MATCH_CHAR_IN_ORDER (MW_MATCH_EXACT>>1)

StringMatcher::StringMatcher()
    : mMatchWeight(0)
{

}

int StringMatcher::GetWordMatchCharWeight(TCHAR ch)
{
	int wt(0);
	if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\n')
		wt += MW_WHITE_SPACE;
	else if (WordParser::IsWordSep(ch))
		wt += MW_NON_WHITE_SPACE;
	return wt;
}
int StringMatcher::GetWordMatchWeight(const StdString& matchString, int startPos /* = 0 */, int lengthMatch /* = -1 */)
{
	int wMWt(0);
	if (startPos == 0)
		wMWt += MW_NON_WHITE_SPACE;
	else if (startPos > 0 && startPos < matchString.length())
		wMWt += GetWordMatchCharWeight(matchString[startPos-1]);
	if (lengthMatch < 0)
		lengthMatch = matchString.length();
	int endPos(startPos+lengthMatch);
	if (endPos >= matchString.length())
		wMWt += MW_NON_WHITE_SPACE;
	else
		wMWt += GetWordMatchCharWeight(matchString[endPos]);
	return wMWt;
}

int StringMatcher::GetWordIndex( const StdString& inStr, int startPos /*= 0*/)
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
		StdString expr(lpExpression);
		StdString excp;
		size_t exceptPos = expr.find(':');
		if (exceptPos != StdString::npos) {
			excp = expr.substr(expr.length()-(exceptPos+1));
			expr = expr.substr(0,exceptPos);
		}
		if (expr.length() > 0) {
			mRegExp = new std::wregex(bExpressionIsRegExp ? expr : StringUtils::WildCardExpToRegExp(expr.c_str()),
                mbCaseSensitive ? std::regex_constants::icase : std::regex_constants::ECMAScript);
		}
		if (excp.length() > 0) {
            mRegExpException = new std::wregex(bExpressionIsRegExp ? excp : StringUtils::WildCardExpToRegExp(excp.c_str()),
                mbCaseSensitive ? std::regex_constants::icase : std::regex_constants::ECMAScript);
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
        std::wcmatch m;
        bMatched = std::regex_match(matchString, m, *mRegExp);
        if (bMatched)
            mMatchString= m[0].first;
    }
	if (bMatched && mRegExpException)
        bMatched = !std::regex_match(matchString, *mRegExpException);
	return bMatched;
}

CStringMatcherList::CStringMatcherList(LPCTSTR lpExpression /* = NULL */)
	: mMinMatchCount(0)
{
	SetExpression(lpExpression);
}
void CStringMatcherList::SetExpression(LPCTSTR lpExpression /* = NULL */)
{
	mMatchWeight = 0;
	mStrListToMatch.clear();
	if (lpExpression) {
		StdString exp(lpExpression);
		exp.MakeLower();
		StringUtils::SplitString(mStrListToMatch, exp, _T(" "));
        
		for (auto it = mStrListToMatch.begin(); it != mStrListToMatch.end();) {
            ((StdString&)(*it)).Trim();
            if (it->empty())
                it = mStrListToMatch.erase(it);
            else
                ++it;
		}
	}
    if (mMinMatchCount == 0 || mMinMatchCount > GetWordCount())
        mMinMatchCount = (unsigned)GetWordCount() / 2;
    if (mMinMatchCount == 0)
        mMinMatchCount = 1;
}
bool CStringMatcherList::Match(LPCTSTR matchString) 
{
	StdString mS(matchString);
	mS.MakeLower();
	mMatchWeight = 0;
    mMatchString.clear();
	int matchPos(-1);
	unsigned matchCount(0);
	for (size_t i = 0; i < mStrListToMatch.size(); ++i) {
		int curMatchPos((int)mS.find(mStrListToMatch[i], 0));
		if (curMatchPos >= 0) {
            int matchLEnght(mStrListToMatch[i].length());
            if (matchLEnght > mMatchString.length())
                mMatchString.assign(matchString, curMatchPos, matchLEnght);
            ++matchCount;
			mMatchWeight += GetWordMatchWeight(mS, curMatchPos, matchLEnght);
			mS[curMatchPos]= ':';
			if (curMatchPos > matchPos) {
				mMatchWeight += MW_MATCH_IN_ORDER;
				++matchCount;
			}
			matchPos = (int)curMatchPos;
		}
	}
	return matchCount > mMinMatchCount;
}

CSimpleStringMatcher::CSimpleStringMatcher(LPCTSTR strToSearch /* = NULL */, BOOL bCaseSensitive /* = FALSE */, BOOL bMatchWholeWord /* = FALSE */, MatchCallback mcb /* = NULL */, void *pUserParam /* = NULL */)
	: muFlags(0)
{
	SetCallBack(mcb, pUserParam);
	SET_UNSET_FLAGBIT(muFlags,  caseSensitive, bCaseSensitive);
	SET_UNSET_FLAGBIT(muFlags,  matchWholeWord, bMatchWholeWord);
    if (strToSearch != NULL)
        mStrToSearch = strToSearch;
    mOrgStrToSearch = mStrToSearch;
	if (!IS_FLAGBIT_SET(muFlags, caseSensitive))
		mStrToSearch = StdString(mStrToSearch).MakeLower();
}
void CSimpleStringMatcher::SetCallBack(MatchCallback mcb /* = NULL */, void *pUserParam /* = NULL */)
{
	mMatchCallback = mcb;
	m_pUserParam = pUserParam;
}
#define SM_CHAR_IS_WHITE_SPACE(c) ((c)==' '||(c)=='\t'||(c)=='\n'||(c)=='\r')
bool CSimpleStringMatcher::Match(LPCTSTR matchString)
{
    mMatchWeight = 0;
    mMatchString.clear();
    if (mStrToSearch.empty())
        return true;
	StdString matchStr(matchString);
    if (matchStr == mOrgStrToSearch) { // match case and whole word
        mMatchString = matchStr;
        mMatchWeight = 5; // match + case + whole word
        return true;
    }
	if (!IS_FLAGBIT_SET(muFlags, caseSensitive))
        matchStr.MakeLower();
    if (matchStr == mStrToSearch) { // match case and whole word
        mMatchString = matchString;
        mMatchWeight = 4; // match + whole word - case
        return true;
    }
    bool bMatch(false);
	int startPos = 0;
	int lenStrToSearch = mStrToSearch.length(), lenInStr(matchStr.length());
	while (startPos >= 0) {
		if (mMatchCallback != NULL)
			if (mMatchCallback(-1, m_pUserParam))
				break;
		startPos = (int)matchStr.find(mStrToSearch, startPos);
		if (startPos < 0)
			continue;
		bool bStrMatch(true);
        auto ch(matchStr[startPos]);
        if (startPos == 0 || SM_CHAR_IS_WHITE_SPACE(ch)) // word starting
            mMatchWeight += 2;
		if (IS_FLAGBIT_SET(muFlags, matchWholeWord)) {
			bStrMatch = WordParser::IsCompleteWord(matchString, startPos, lenStrToSearch);
		}
        if (!bStrMatch) {
            startPos += lenStrToSearch;
            continue;
        }
		bMatch = true;
        if (mMatchString.empty())
            mMatchString = WordParser::GetCompleteWord(matchString, startPos, lenStrToSearch);
		if (mMatchCallback != NULL)
			if (mMatchCallback(startPos, m_pUserParam))
				break;
		startPos += lenStrToSearch;
        ch = startPos < lenInStr ? matchStr[startPos] : 0;
        if (ch == 0 || SM_CHAR_IS_WHITE_SPACE(ch)) // word end
            mMatchWeight++;
	}
	return bMatch;
}
static bool IsVowel(TCHAR ch)
{
	return ch == 'a' || ch == 'e' || ch == 'i' || ch == 'o' || ch == 'u' || ch == 'y';
}
StdString CPhoneticStringMatcher::GetPhoneticString(const StdString& inStr, PhoneticData &outPhoneticData, bool bUpdateCounts /*= true*/)
{
	outPhoneticData.consonantCount = outPhoneticData.vowelCount = 0;
	StdString outStr(inStr);
	outStr.MakeLower();
	outPhoneticData.mStrVowel = outStr;
	for (int i = 0; i < outStr.length(); ++i) {
		if (IsVowel(outStr[i])) {
			outStr[i]=':';
			outPhoneticData.vowelCount++;
		}
		else {
			outPhoneticData.mStrVowel[i]=':';
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
CPhoneticStringMatcher::CPhoneticStringMatcher(LPCTSTR lpExpression /*= NULL*/ )
	: m_iMatchStartInddex(-1)
{
	SetExpression(lpExpression);
}

void CPhoneticStringMatcher::SetExpression(LPCTSTR lpExpression /*= NULL*/ )
{
	StdString vowels;
	mStrOrgExpression = StdString(lpExpression).MakeLower();
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
    mMatchString.clear();
	if (mStrPhonetic.length() <= 0)
		return false;
	const bool bDoFullMatch(mPhoneticData.consonantCount < 3);
	if (mPhoneticData.consonantCount == 0) {
		StdString str(StdString(matchString).MakeLower());
		m_iMatchStartInddex = (int)str.find(mStrOrgExpression, 0);
		if (m_iMatchStartInddex >= 0) {
			mMatchWeight = mStrOrgExpression.length() << 1;
			mMatchWeight += GetWordMatchWeight(str, m_iMatchStartInddex, str.length());
            mMatchString.assign(matchString, m_iMatchStartInddex, str.length());
			return true;
		}
		return false;
	}
	PhoneticData phd;
	int startIndex(0);
	StdString phoneticStr(GetPhoneticString(matchString, phd, false));
	int matchIndex(0);
	int matchCount(0);
	startIndex = GetWordIndex(matchString, startIndex);
	TCHAR prevCh(0), prevCh2(0);
	int orderWt(0);
	while ( true ) {
		SKIP_TILL_MATCH(phoneticStr, startIndex, ':');
		SKIP_TILL_MATCH(mStrPhonetic, matchIndex, ':');
		TCHAR ch2(startIndex < phoneticStr.length() ? phoneticStr[startIndex++] : 0);
		TCHAR ch(matchIndex < mStrPhonetic.length() ? mStrPhonetic[matchIndex++] : 0);
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
        mMatchString.assign(matchString, m_iMatchStartInddex, startIndex - m_iMatchStartInddex);
		StdString &vowels(phd.mStrVowel);
		matchCount = bDoFullMatch ? startIndex : vowels.length();
		startIndex = m_iMatchStartInddex;
		matchIndex = 0;
		StdString &mStrVowel(mPhoneticData.mStrVowel);
		while ( startIndex < matchCount && matchIndex < mStrVowel.length() ) {
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


bool CPhoneticStringMatcher::StringHasVowels(const StdString& str)
{
	StdString vowels(str);
	vowels.MakeLower();
	return vowels.find_first_of(_T("aeiouy")) >= 0;
}

CPhoneticStringMatcherList::CPhoneticStringMatcherList(LPCTSTR lpExpression /*= NULL*/ )
{
	SetExpression(lpExpression);
}

void CPhoneticStringMatcherList::SetExpression(LPCTSTR lpExpression /*= NULL*/ )
{
	__super::SetExpression(lpExpression);
	mPhoneticMatchers.clear();
	for (INT_PTR i = 0; i < GetWordCount(); ++i)
		mPhoneticMatchers.push_back(CPhoneticStringMatcher(mStrListToMatch[i]));
}

bool CPhoneticStringMatcherList::Match( LPCTSTR matchString )
{
	StdString mS(StdString(matchString).MakeLower()), strMatch;
	mMatchWeight = 0;
    mMatchString.clear();
	int matchPos(-1);
	unsigned localWight(0);
	for (size_t i = 0; i < mPhoneticMatchers.size(); ++i) {
		if (mPhoneticMatchers[i].Match(mS.c_str())) {
			mMatchWeight += mPhoneticMatchers[i].GetMatchWeight();
            if (mMatchString.length() > strMatch.length())
                strMatch = mMatchString;
			int curMatchPos(mPhoneticMatchers[i].GetMatchIndex());
			if (curMatchPos >= 0) {
				mS[curMatchPos]='?';
				++localWight;
				if (curMatchPos > matchPos) {
					mMatchWeight += MW_MATCH_IN_ORDER;
					++localWight;
				}
				matchPos = curMatchPos;
			}
		}
	}
    mMatchString = strMatch;
	return localWight > mMinMatchCount;
}
