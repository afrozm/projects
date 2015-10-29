#pragma once

CString WildCardExpToRegExp(LPCTSTR wildCardExp);
bool IsWildCardExp(const CString &wildCardExp);

class StringMatcher {
public:
	virtual bool Match(LPCTSTR matchString) = 0;
	virtual unsigned GetMatchWeight() const {return 0;}
	virtual ~StringMatcher() {}
protected:
	int GetWordMatchCharWeight(TCHAR ch);
	int GetWordMatchWeight(const CString &matchString, int startPos = 0, int lengthMatch = -1);
	int GetWordIndex(LPCTSTR inStr, int startPos = 0);
};

class CRegExpMatcher : public StringMatcher
{
public:
	CRegExpMatcher(LPCTSTR lpExpression = NULL, bool bExpressionIsRegExp = false, BOOL bCaseSensitive = FALSE);
	~CRegExpMatcher(void);
	bool Match(LPCTSTR matchString);
	void SetExpression(LPCTSTR lpExpression, bool bExpressionIsRegExp = false, BOOL bCaseSensitive = FALSE);
	void SetCaseSensitive(BOOL bCaseSensitive = TRUE)
	{
		mbCaseSensitive = bCaseSensitive;
	}
protected:
	void Free();
	CAtlRegExp<> *mRegExp;
	CAtlRegExp<> *mRegExpException;
	BOOL mbCaseSensitive;
};

class CPhoneticStringMatcher : public StringMatcher {
public:
	CPhoneticStringMatcher(LPCTSTR lpExpression = NULL);
	void SetExpression(LPCTSTR lpExpression = NULL);
	bool Match(LPCTSTR matchString);
	unsigned GetMatchWeight() const {return mMatchWeight;}
	int GetMatchIndex() const {return m_iMatchStartInddex;}
	struct PhoneticData {
		CString mStrVowel;
		int vowelCount;
		int consonantCount;
	};
	static CString GetPhoneticString(LPCTSTR inStr, PhoneticData &outPhoneticData, bool bUpdateCounts = true);
	static bool StringHasVowels(LPCTSTR str);
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
	CString mStrOrgExpression;
	CString mStrPhonetic;
	PhoneticData mPhoneticData;
	unsigned mMatchWeight;
	int m_iMatchStartInddex;
};

class CStringMatcherList : public StringMatcher {
public:
	CStringMatcherList(LPCTSTR lpExpression = NULL);
	void SetExpression(LPCTSTR lpExpression = NULL);
	bool Match(LPCTSTR matchString);
	unsigned GetMatchWeight() {return mMatchWeight;}
	INT_PTR GetWordCount() const {return mStrListToMatch.GetCount();}
protected:
	CArrayCString mStrListToMatch;
	unsigned mMatchWeight;
};
class CPhoneticStringMatcherList : public CStringMatcherList {
public:
	CPhoneticStringMatcherList(LPCTSTR lpExpression = NULL);
	void SetExpression(LPCTSTR lpExpression = NULL);
	bool Match(LPCTSTR matchString);
protected:
	typedef CArray<CPhoneticStringMatcher> CArrayPhoneticStringMatcher;
	CArrayPhoneticStringMatcher mPhoneticMatchers;
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
	CString mStrToSearch;
	UINT muFlags;
	MatchCallback mMatchCallback;
	void *m_pUserParam;
};
