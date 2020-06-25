#include "stdafx.h"
#include "Value.h"
#include <stdlib.h>

Value::Value()
    : mType(VTNULL), mNumber(0)
{
}




Value::Value(const Value &other)
    : mType(VTNULL), mNumber(0)
{
    *this = other;
}

int Value::Compare(const Value &other) const
{
    if (this == &other)
        return 0;
    int retVal(mType - other.mType);
    if (retVal)
        return retVal;
    switch (mType)
    {
    case Value::VTNULL:
        break;
    case Value::VTString:
        retVal = mString.compare(other.mString);
        break;
    case Value::VTNumber:
        retVal = (int)(mNumber - other.mNumber);
        break;
    case Value::VTArray:
        retVal = GetArrayCount() - other.GetArrayCount();
        if (retVal == 0) {
            for (size_t i = 0; i < GetArrayCount() && !retVal; ++i)
                retVal = (*this)[i].Compare(other[i]);
        }
        break;
    case Value::VTMap:
        retVal = mMap.size() - other.mMap.size();
        if (retVal == 0) {
            for (auto thisCit = mMap.begin(), oCit = other.mMap.begin(); thisCit != mMap.end(); ++thisCit, ++oCit) {
                retVal = thisCit->first.compare(oCit->first);
                if (retVal)
                    break;
                retVal = thisCit->second->Compare(*oCit->second);
            }
        }
        break;
    default:
        break;
    }
    return retVal;
}

Value::operator bool() const
{
    if (mType == VTNULL)
        return false;
    switch (mType)
    {
    case Value::VTString:
        return !mString.empty();
    case Value::VTNumber:
        return mNumber != 0;
    case Value::VTArray:
        return mArray.size() > 0;
        break;
    case Value::VTMap:
        return mMap.size() > 0;
        break;
    }
    return false;
}

Value::~Value()
{
}

Value::Value(double number)
    : mType(VTNumber), mNumber(number)
{
}

Value::Value(const char *inStr)
    : mType(VTString), mNumber(0), mString(inStr ? inStr : "")
{
}

static Value sNullValue;

Value& Value::operator[](size_t index)
{
    if (index == mArray.size()) {
        mType = VTArray;
        mArray.push_back(PtrValue(new Value));
    }

    if (index < mArray.size())
        return *mArray[index];

    sNullValue = Value();
    return sNullValue;
}

Value& Value::operator[](const char *keyStr)
{
    if (keyStr) {
        if (mMap[keyStr].get() == nullptr) {
            mMap[keyStr] = PtrValue(new Value);
            mType = VTMap;
        }
        return *mMap[keyStr];
    }
    sNullValue = Value();
    return sNullValue;
}

const Value& Value::operator[](size_t index) const
{
    if (index < mArray.size())
        return *mArray[index];

    sNullValue = Value();
    return sNullValue;

}

const Value& Value::operator[](const char *keyStr) const
{
    if (keyStr) {
        auto cit(mMap.find(keyStr));
        if (cit != mMap.end())
            return *cit->second;
    }

    sNullValue = Value();
    return sNullValue;
}

Value& Value::operator=(const Value &other)
{
    mType = other.mType;
    mNumber = other.mNumber;
    mString = other.mString;
    mArray.clear();
    for (const auto & v : other.mArray)
        mArray.push_back(PtrValue(new Value(*v)));
    mMap.clear();
    for (const auto & v : other.mMap)
        mMap[v.first] = PtrValue(new Value(*v.second));
    return *this;
}

static std::string StringFindAndReplace(const std::string &inStr, const std::string &from, const std::string &to,
    std::string::size_type start_pos = 0, std::string::size_type endPos = std::string::npos,
    int nReplace = -1)
{
    std::string outStr(inStr);
    while ((start_pos = outStr.find(from, start_pos)) != std::string::npos) {
        outStr.replace(start_pos, from.length(), to);
        start_pos += to.length(); // ...
        if (nReplace > 0
            && --nReplace <= 0)
            break;
        if (endPos != std::string::npos) {
            endPos += to.length();
            if (start_pos > endPos)
                break;
        }
    }
    return outStr;
}

std::string Value::GetAsString(int tabCount/* =0 */) const
{
    std::string outStr;
    switch (mType)
    {
    case Value::VTNULL:
        break;
    case Value::VTString:
        if (mString.length() > 0)
            outStr += "\"" + StringFindAndReplace(mString, "\"", "\"\"") + "\"";
        break;
    case Value::VTNumber:
        outStr += std::to_string(mNumber);
        break;
    case Value::VTArray:
        if (mArray.size() > 0) {
            outStr += "[";
            int count(0);
            for (const auto &v : mArray) {
                if (count > 0)
                    outStr += ", ";
                outStr += v->GetAsString(tabCount+1);
                ++count;
            }
            outStr += "]";
        }
        break;
    case Value::VTMap:
        if (mMap.size() > 0) {
            int count(0);
            if (tabCount < 0 || tabCount > 256)
                tabCount = 0;
            std::string tabStr(tabCount, '\t');
            outStr += /*"\n" + rootTabStr + */"{\n";
            for (const auto &v : mMap) {
                if (count > 0)
                    outStr += ";\n";
                outStr += tabStr + v.first + "=" + v.second->GetAsString(tabCount+1);
                ++count;
            }
            if (tabStr.length() > 0)
                tabStr.erase(tabStr.end()-1);
            outStr += "\n" + tabStr +  "}";
        }
        break;
    default:
        break;
    }
    return outStr;
}


#define STR_SKIP_TILL_CHAR(s,c) while(*s&&*s!=c) ++s
#define STR_SKIP_TILL_TWOCHAR(s,c1,c2) while(*s&&*s!=c1&&*s!=c2) ++s
#define CHAR_IS_WHITE_SPACE(c) (c==' '||c=='\t'||c=='\n'||c=='\r')
#define STR_SKIP_SPACE(s) while(*s&&CHAR_IS_WHITE_SPACE(*s)) ++s
#define STR_SKIP_TILL_SPACE_OR_CHAR(s,c) while(*s&&!CHAR_IS_WHITE_SPACE(*s)&&*s!=c) ++s
#define STR_SKIP_SPACE_OR_CHAR(s,c) while(*s&&(CHAR_IS_WHITE_SPACE(*s)||*s==c)) ++s

const char* Value::FromString(const char *inStr)
{
    *this = Value();
    if (inStr == NULL || *inStr == 0)
        return inStr;
    enum States {
        SearchType,
        SearchMapKey,
        SearchArray,
        SearchString,
        SearchNumber
    };
    States st(SearchType);
    for (; *inStr&&mType==VTNULL; ++inStr) {
        char ch(*inStr);
        switch (st)
        {
        case SearchType:
            if (ch == '{') // map
                st = SearchMapKey;
            else if (ch == '[')
                st = SearchArray;
            else if (ch == '"')
                st = SearchString;
            else if (ch == '-' || ch == '.' || (ch >= '0'&&ch <= '9')) {
                st = SearchNumber;
                --inStr;
            }
            break;
        case SearchMapKey:
        {
            while (*inStr) {
                STR_SKIP_SPACE(inStr);
                const char *startKey(inStr);
                STR_SKIP_TILL_SPACE_OR_CHAR(inStr, '=');
                std::string keyStr(startKey, inStr - startKey);
                STR_SKIP_SPACE(inStr);
                if (*inStr == '=' && keyStr.length() > 0) {
                    ++inStr;
                    inStr = (*this)[keyStr.c_str()].FromString(inStr);
                }
                STR_SKIP_SPACE_OR_CHAR(inStr, ';');
                if (*inStr == '}') // end map
                    break;
            }
        }
            break;
        case SearchArray:
        {
            while (*inStr)
            {
                inStr = (*this)[GetArrayCount()].FromString(inStr);
                STR_SKIP_SPACE(inStr);
                if (*inStr == ']') // end
                    break;
            }
        }
            break;
        case SearchString:
        {
            const char *endPtr(inStr);
            while (*endPtr) {
                STR_SKIP_TILL_CHAR(endPtr, '"');
                if (*endPtr && endPtr[1] != '"')
                    break;
                // skip consecutive '"'
                ++endPtr;
                ++endPtr;
            }
            mString = std::string(inStr, endPtr - inStr);
            mString = StringFindAndReplace(mString, "\"\"", "\"");
            mType = VTString;
            inStr = endPtr;
            if (*inStr == '"')
                ++inStr;
        }
            break;
        case SearchNumber:
        {
            char *endPtr(NULL);
            mNumber = strtod(inStr, &endPtr);
            if (endPtr != NULL && endPtr > inStr) {
                mType = VTNumber;
                inStr = endPtr;
            }
        }
            break;
        default:
            break;
        }
    }
    return inStr;
}

