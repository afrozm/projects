#include "stdafx.h"
#include "StringUtils.h"
#include <algorithm>

lstring StringUtils::ToLower(const lstring &inStr)
{
    lstring outStr(inStr);

    std::transform(outStr.begin(), outStr.end(), outStr.begin(), ::tolower);

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
    long long retVal = std::stoll(str, NULL, 0);
    if (bMinus)
        retVal = -retVal;
    return retVal;
}

std::string StringUtils::UnicodeToUTF8(const wchar_t *unicodeString)
{
    std::string sRet;
    if (unicodeString != NULL && unicodeString[0])
    {
        int kMultiByteLength = WideCharToMultiByte(CP_UTF8, 0, unicodeString, -1, 0, 0, NULL, NULL);
        std::vector<char> vecChar(kMultiByteLength);
        if (WideCharToMultiByte(CP_UTF8, 0, unicodeString, -1, &vecChar[0], (int)vecChar.size(), NULL, NULL))
        {
            sRet.assign(&vecChar[0], vecChar.size());
        }
    }
    return sRet;
}
std::wstring StringUtils::UTF8ToUnicode(const char *utf8String)
{
    std::wstring		sRet;
    if (utf8String != NULL && utf8String[0])
    {
        int	kAllocate = MultiByteToWideChar(CP_UTF8, 0, utf8String, -1, NULL, 0);
        if (kAllocate)
        {
            std::vector<wchar_t> vecWide(kAllocate);

            int kCopied = MultiByteToWideChar(CP_UTF8, 0, utf8String, -1, &vecWide[0], (int)vecWide.size());
            if (kCopied)
            {
                sRet.assign(&vecWide[0], vecWide.size());
            }
        }
    }
    return sRet;
}