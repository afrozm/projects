#include "stdafx.h"
#include "StringUtils.h"


int StringUtils::SplitString(VecString &outStrings, const lstring &inStr, const lstring &inSepChars /* = _T(",") */, bool bIncludeEmpty /* = false */, int maxCount /* = -1 */)
{
    size_t count(outStrings.size());
    size_t sp(0);
    bool bContinue(true);
    while (true) {
        size_t pos = inStr.find_first_of(inSepChars, sp);
        if (pos >= 0) {
            if (bIncludeEmpty || pos > sp) {
                outStrings.push_back(inStr.substr(sp, pos));
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
        outStrings.push_back(inStr.substr(sp, inStr.length()));

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
    bool bMinus(*str == '-');
    if (bMinus)
        ++str;
    long long retVal = std::stoll(str, NULL, 0);
    if (bMinus)
        retVal = -retVal;
    return retVal;
}
