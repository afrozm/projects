#include "stdafx.h"
#include "StringUtils.h"
#include <regex>
#ifdef _WIN32
#include <windows.h>
#endif
#include <algorithm>


StdString::StdString(const char *str)
    :
#if defined(_UNICODE) || defined(UNICODE)
    lstring(UTF8_TO_UNICODE(str))
#else
    lstring(str ? str : "")
#endif
{

}

StdString::StdString(const wchar_t *str)
    :
#if defined(_UNICODE) || defined(UNICODE)
    lstring(str ? str : _T(""))
#else
    lstring(StringUtils::UnicodeToUTF8(str))
#endif
{

}


StdString::StdString(const otherstring &str)
    :
#if defined(_UNICODE) || defined(UNICODE)
    lstring(UTF8_TO_UNICODE(str))
#else
    lstring(StringUtils::UnicodeToUTF8(str))
#endif
{

}

StdString::operator otherstring() const
{
#if defined(_UNICODE) || defined(UNICODE)
    return UNICODE_TO_UTF8(*this);
#else
    return StringUtils::UTF8ToUnicode(*this);
#endif
}


StdString& StdString::MakeLower()
{
	std::transform(begin(), end(), begin(), [](TCHAR c) { return (TCHAR)::tolower((int)c); });
    return *this;
}

StdString StdString::ToLower() const
{
	StdString outString(*this);
	outString.MakeLower();
	return outString;
}

bool StdString::Trim(const StdString & inTrimChars /*= " \t\r\n"*/, bool bTrimLeft /*= true*/, bool bTrimRight /*= true*/)
{
    StdString &outStr(*this);
    bool bContinue(true), bUpdated(false);
    while (bContinue) {
        bContinue = false;
        for (auto cit(inTrimChars.begin()); cit != inTrimChars.end(); ++cit) {
            auto len(outStr.length());
            if (bTrimLeft && len > 0 && outStr[0] == *cit) {
                outStr.erase(0, 1);
                --len;
                bContinue = true;
                bUpdated = true;
            }
            if (bTrimRight && len > 0 && outStr[len - 1] == *cit) {
                outStr.erase(len - 1);
                bContinue = true;
                bUpdated = true;
            }
        }
    }
    return bUpdated;
}

StdString StdString::GetTrimString(const StdString & inTrimChars /*= " \t\r\n"*/, bool bTrimLeft /*= true*/, bool bTrimRight /*= true*/) const
{
    StdString outStr(*this);
    outStr.Trim(inTrimChars, bTrimLeft, bTrimRight);
    return outStr;
}

int StringUtils::SplitString(VecString &outStrings, const lstring &inStr, const lstring &inSepChars /* = _T(",") */, bool bIncludeEmpty /* = false */, int maxCount /* = -1 */)
{
    size_t count(outStrings.size());
    size_t sp(0);
    bool bContinue(true);
    while (true) {
        size_t pos = inStr.find_first_of(inSepChars, sp);
        if (pos != lstring::npos) {
            if (bIncludeEmpty || pos > sp) {
                outStrings.push_back(inStr.substr(sp, pos-sp));
                if (maxCount > 0) {
                    --maxCount;
                    bContinue = maxCount > 0;
                }
            }
            sp = pos + 1;
        }
        else break;
    }
    if (sp < inStr.length())
        outStrings.push_back(inStr.substr(sp, inStr.length()-sp));

    return (int)(outStrings.size()-count);
}

bool StringUtils::TrimString(lstring & inOutStr, const lstring & inTrimChars, bool bTrimLeft, bool bTrimRight)
{
    lstring &outStr(inOutStr);
    bool bContinue(true), bUpdated(false);
    while (bContinue) {
        bContinue = false;
        for (auto cit(inTrimChars.begin()); cit != inTrimChars.end(); ++cit) {
            auto len(outStr.length());
            if (bTrimLeft && len > 0 && outStr[0] == *cit) {
                outStr.erase(0, 1);
                --len;
                bContinue = true;
                bUpdated = true;
            }
            if (bTrimRight && len > 0 && outStr[len - 1] == *cit) {
                outStr.erase(len - 1);
                bContinue = true;
                bUpdated = true;
            }
        }
    }

    return bUpdated;
}

long long StringUtils::getLLfromStr(const TCHAR *str)
{
    if (str == NULL || *str == 0)
        return 0;
    bool bMinus(*str == '-');
    if (bMinus)
        ++str;
    long long retVal = 0;
    try { retVal = std::stoll(str, NULL, 0); } catch (...) {}
    if (bMinus)
        retVal = -retVal;
    return retVal;
}

lstring StringUtils::Format(const TCHAR *msg, ...)
{
    if (msg == NULL || *msg == 0)
        return _T("");
    va_list arg;
    va_start(arg, msg);
    int len = _vsctprintf(msg, arg) + 4 * sizeof(TCHAR); // _vscprintf doesn't count + 1; terminating '\0'
    TCHAR *buf = new TCHAR[len];
    _vstprintf_s(buf, len, msg, arg);
    lstring outStr(buf);
    delete[] buf;
    return outStr;
}

std::string StringUtils::UnicodeToUTF8(const wchar_t *unicodeString, int len /*= -1*/)
{
    std::string sRet;
    if (unicodeString != NULL && unicodeString[0])
    {
#ifdef _WIN32
        int kMultiByteLength = WideCharToMultiByte(CP_UTF8, 0, unicodeString, len, 0, 0, NULL, NULL);
        std::vector<char> vecChar(kMultiByteLength);
        if (WideCharToMultiByte(CP_UTF8, 0, unicodeString, len, &vecChar[0], (int)vecChar.size(), NULL, NULL))
        {
            sRet.assign(&vecChar[0]);
        }
#endif
    }
    return sRet;
}

std::wstring StringUtils::UTF8ToUnicode(const std::string &utf8String, int len /*= -1*/)
{
    return UTF8ToUnicode(utf8String.c_str(), len);
}

std::wstring StringUtils::UTF8ToUnicode(const char *utf8String, int len /*= -1*/)
{
    std::wstring		sRet;
    if (utf8String != NULL && utf8String[0])
    {
#ifdef _WIN32
        int	kAllocate = MultiByteToWideChar(CP_UTF8, 0, utf8String, len, NULL, 0);
        if (kAllocate)
        {
            std::vector<wchar_t> vecWide(kAllocate);

            int kCopied = MultiByteToWideChar(CP_UTF8, 0, utf8String, len, &vecWide[0], (int)vecWide.size());
            if (kCopied)
            {
                sRet.assign(&vecWide[0]);
            }
        }
#endif
    }
    return sRet;
}

std::string StringUtils::UnicodeToUTF8(const std::wstring & unicodeString, int len /*= -1*/)
{
    return UnicodeToUTF8(unicodeString.c_str(), len);
}

static lstring WildCardToRegExp(LPCTSTR wildCard)
{
    LPTSTR regExp = new TCHAR[6 * lstrlen(wildCard) + 1];
    unsigned len = 0;

    while (*wildCard) {
        TCHAR extraCharToAdd = 0;

        switch (*wildCard) {
        case '*':
            extraCharToAdd = '.';
            break;
        case '.':
        case '[':
        case '{':
        case ']':
        case '}':
        case '(':
        case '\\':
            extraCharToAdd = '\\';
            break;
        }
        if (extraCharToAdd)
            regExp[len++] = extraCharToAdd;
        regExp[len++] = *wildCard++;
    }
    regExp[len] = 0;
    lstring regExpStr(regExp);

    delete[] regExp;

    return regExpStr;
}

StdString StringUtils::WildCardExpToRegExp(const TCHAR *wildCardExp)
{
    TCHAR *exp = new TCHAR[lstrlen(wildCardExp) + 1];
    lstrcpy(exp, wildCardExp);
    LPTSTR nexttoken(NULL);
    LPTSTR token = _tcstok_s(exp, _T(";"), &nexttoken);
    StdString regExp;
    while (token != NULL) {
        regExp += _T("(") + WildCardToRegExp(token) + _T(")");
        token = _tcstok_s(NULL, _T(";"), &nexttoken);
        if (token != NULL) {
            regExp += _T("|");
        }
    }
    delete[]exp;
    return regExp;
}

bool StringUtils::WildCardMatch(const lstring &inWildCardExp, const lstring &inStr)
{
    return RegMatch(WildCardExpToRegExp(inWildCardExp.c_str()), inStr);
}

bool StringUtils::RegMatch(const lstring &inRegExp, const lstring &inStr)
{
#if defined(_UNICODE) || defined(UNICODE)
    std::wregex 
#else
    std::regex
#endif
        txt_regex(inRegExp, std::regex_constants::icase);
    return std::regex_match(inStr, txt_regex);
}
