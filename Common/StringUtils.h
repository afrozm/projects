#pragma once

#include "Common.h"
#include <vector>

#define STR_IS_VALID_PTR(p) (p&&*p)
#define STR_INR_PTR(p) if(STR_IS_VALID_PTR(p)) ++p
#define STR_CHAR_IS_SPACE(c) ((c)==' ' || (c)=='\t' || (c)=='\n' || (c)=='\r')
#define STR_SKIP_SPACE(p) while(STR_IS_VALID_PTR(p)&&STR_CHAR_IS_SPACE(*p)) ++p


namespace StringUtils
{
    lstring ToLower(const lstring &inStr);
    typedef std::vector<lstring> VecString;
    int SplitString(VecString &outStrings, const lstring &inStr, const lstring &inSepChars = _T(","), bool bIncludeEmpty = false, int maxCount = -1);
    bool TrimString(lstring &inOutStr, const lstring &inTrimChars = _T(" \t\r\n"), bool bTrimLeft = true, bool bTrimRight = true);
    long long getLLfromStr(const TCHAR *str);
    lstring Format(const TCHAR *fmt, ...);
    void Replace(lstring &inOutStr, const lstring &inFindStr, const lstring &inReplaceStr, size_t pos = 0);

    std::string UnicodeToUTF8(const wchar_t *unicodeString);
    std::wstring UTF8ToUnicode(const char *utf8String);
    std::string UnicodeToUTF8(const std::wstring &unicodeString);
    std::wstring UTF8ToUnicode(const std::string &utf8String);
};
