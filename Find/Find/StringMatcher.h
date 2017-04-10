#pragma once
#include "Common.h"
#include "StringUtils.h"

bool IsWildCardExp(LPCTSTR wildCardExp);

class StringMatcher {
public:
    StringMatcher();
	virtual bool Match(LPCTSTR matchString) = 0;
	virtual unsigned GetMatchWeight() const {return mMatchWeight;}
	virtual ~StringMatcher() {}
    virtual const StdString& GetMatchString() const { return mMatchString; }
protected:
	int GetWordMatchCharWeight(TCHAR ch);
	int GetWordMatchWeight(const StdString& matchString, int startPos = 0, int lengthMatch = -1);
	int GetWordIndex(const StdString& inStr, int startPos = 0);

    unsigned mMatchWeight;
    StdString mMatchString; // string matched
};

class CRegExpMatcher : public StringMatcher
{
public:
	CRegExpMatcher(LPCTSTR lpExpression = NULL, bool bExpressionIsRegExp = false, BOOL bCaseSensitive = FALSE);
	~CRegExpMatcher(void);
	bool Match(LPCTSTR matchString);
	void SetExpression(LPCTSTR lpExpression, bool bExpressionIsRegExp = false, BOOL bCaseSensitive = FALSE);
protected:
	void Free();
	std::wregex *mRegExp;
    std::wregex *mRegExpException;
};

class CPhoneticStringMatcher : public StringMatcher {
public:
    CPhoneticStringMatcher(LPCTSTR lpExpression = NULL);
	void SetExpression(LPCTSTR lpExpression = NULL);
	bool Match(LPCTSTR matchString);
	unsigned GetMatchWeight() const {return mMatchWeight;}
	int GetMatchIndex() const {return m_iMatchStartInddex;}
	struct PhoneticData {
		StdString mStrVowel;
		int vowelCount;
		int consonantCount;
	};
	static StdString GetPhoneticString(const StdString& inStr, PhoneticData &outPhoneticData, bool bUpdateCounts = true);
	static bool StringHasVowels(const StdString& str);
	class GetPhoneticData {
	public:
		GetPhoneticData(TCHAR inCh = 0, LPCTSTR inChStr = NULL, int inChIndex = -1, LPCTSTR inTargetStr = NULL, int inTargerIndex = -1);
		TCHAR GetChar(int idStr = 0, int relIndex = 1);
		TCHAR ch;
		int weight;
        LPCTSTR chString[2]; // 0 - ch string, 1 - target string
		int chIndex[2];
		int lenIndex[2];
	};
	static TCHAR GetPhoneticChar(GetPhoneticData &inOutPhoticCharData);
private:
	int MatchChar(TCHAR ch, TCHAR ch2);
	StdString mStrOrgExpression;
	StdString mStrPhonetic;
	PhoneticData mPhoneticData;
	int m_iMatchStartInddex;
};

class CStringMatcherList : public StringMatcher {
public:
    CStringMatcherList(LPCTSTR lpExpression = NULL);
	void SetExpression(LPCTSTR lpExpression = NULL);
	bool Match(LPCTSTR matchString);
	unsigned GetMatchWeight() {return mMatchWeight;}
	INT_PTR GetWordCount() const {return mStrListToMatch.size();}
    void SetMinimumMatchCount(unsigned mc) { mMinMatchCount = mc; }
protected:
	StringUtils::VecString mStrListToMatch;
    unsigned mMinMatchCount;
};
class CPhoneticStringMatcherList : public CStringMatcherList {
public:
    CPhoneticStringMatcherList(LPCTSTR lpExpression = NULL);
	void SetExpression(LPCTSTR lpExpression = NULL);
	bool Match(LPCTSTR matchString);
protected:
	typedef std::vector<CPhoneticStringMatcher> CArrayPhonetilstringMatcher;
	CArrayPhonetilstringMatcher mPhoneticMatchers;
};
class CSimpleStringMatcher : public StringMatcher {
public:
	typedef int (*MatchCallback)(int startPos, void *pUserParam);
	CSimpleStringMatcher(LPCTSTR strToSearch = NULL, BOOL bCaseSensitive = FALSE, BOOL bMatchWholeWord = FALSE, MatchCallback mcb = NULL, void *pUserParam = NULL);
	bool Match(LPCTSTR matchString);
	void SetCallBack(MatchCallback mcb = NULL, void *pUserParam = NULL);
protected:
	enum {
		caseSensitive,
		matchWholeWord
	};
	StdString mStrToSearch, mOrgStrToSearch;
	UINT muFlags;
	MatchCallback mMatchCallback;
	void *m_pUserParam;
};

StringMatcher* StringMatcher_GetStringMatcher(LPCTSTR inString);
bool StringMatcher_IsSimpleMatch(LPCTSTR inString);
bool StringMatcher_IsPhonetic(LPCTSTR inString);