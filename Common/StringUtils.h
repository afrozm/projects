#pragma once

#include "Common.h"
#include <vector>
#include <algorithm>

#define STR_IS_VALID_PTR(p) (p&&*p)
#define STR_INR_PTR(p) if(STR_IS_VALID_PTR(p)) ++p
#define STR_CHAR_IS_LINE(c) ((c)=='\n' || (c)=='\r')
#define STR_CHAR_IS_SPACE_OR_TAB(c) ((c)==' ' || (c)=='\t')
#define STR_CHAR_IS_SPACE(c) (STR_CHAR_IS_SPACE_OR_TAB(c) || STR_CHAR_IS_LINE(c))
#define STR_SKIP_SPACE(p) while(STR_IS_VALID_PTR(p)&&STR_CHAR_IS_SPACE(*p)) ++p


namespace StringUtils
{
	template<typename T>
	std::basic_string<T> ToLower(const std::basic_string<T> &inStr)
	{
		std::basic_string<T> outStr(inStr);
		std::transform(outStr.begin(), outStr.end(), outStr.begin(), ::tolower);
		return outStr;

	}
    typedef std::vector<lstring> VecString;
    int SplitString(VecString &outStrings, const lstring &inStr, const lstring &inSepChars = _T(","), bool bIncludeEmpty = false, int maxCount = -1);
    bool TrimString(lstring &inOutStr, const lstring &inTrimChars = _T(" \t\r\n"), bool bTrimLeft = true, bool bTrimRight = true);
    long long getLLfromStr(const TCHAR *str);
    lstring Format(const TCHAR *fmt, ...);
	template<typename T>
	void Replace(std::basic_string<T> &inOutStr, const std::basic_string<T> &inFindStr, const std::basic_string<T> &inReplaceStr, size_t pos = 0)
	{
		if (inFindStr == inReplaceStr)
			return;
		if (inFindStr.empty())
			return;
		size_t fl(inFindStr.length()), rl(inReplaceStr.length());
		while (true)
		{
			pos = inOutStr.find(inFindStr, pos);
			if (pos == std::basic_string<T>::npos)
				break;
			inOutStr.replace(pos, fl, inReplaceStr);
			pos += rl;
		}
	}

    std::string UnicodeToUTF8(const wchar_t *unicodeString, int len=-1);
    std::wstring UTF8ToUnicode(const char *utf8String, int len = -1);
    std::string UnicodeToUTF8(const std::wstring &unicodeString, int len = -1);
    std::wstring UTF8ToUnicode(const std::string &utf8String, int len = -1);

    lstring WildCardExpToRegExp(const TCHAR *wildCardExp);
    bool WildCardMatch(const lstring &inWildCardExp, const lstring &inStr);
    bool RegMatch(const lstring &inRegExp, const lstring &inStr);
};

#if defined(_UNICODE) || defined(UNICODE)
#define UNICODE_TO_UTF8(x) StringUtils::UnicodeToUTF8(x)
#define UTF8_TO_UNICODE(x) StringUtils::UTF8ToUnicode(x)
#else
#define UNICODE_TO_UTF8(x) x
#define UTF8_TO_UNICODE(x) x
#endif
