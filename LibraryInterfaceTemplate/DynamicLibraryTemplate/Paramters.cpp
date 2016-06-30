#include "stdafx.h"
#include "Paramters.h"



Paramters::Paramters()
{
}


Paramters::Paramters(const StringParam &inParams)
{
    FromString(inParams);
}

const Paramters::StringParam & Paramters::GetParamValue(const StringParam & inParamName, const StringParam & inDefaultValue) const
{
    static const StringParam sEmptyValue;
    auto cit(mParamters.find(inParamName));
    if (cit != mParamters.end())
        return cit->second;
    return sEmptyValue;
}

void Paramters::SetParamValue(const StringParam & inParamName, const StringParam & inValue)
{
    mParamters[inParamName] = inValue;
}

void Paramters::RemoveParam(const StringParam & inParamName)
{
    mParamters.erase(inParamName);
}

Paramters::~Paramters()
{
}

Paramters::StringParam Paramters::ToString() const
{
    StringParam outStr;
    for (auto &cit : mParamters) {
        if (outStr.length() > 0)
            outStr += "\r\n";
        outStr += cit.first + "=" + cit.second;
    }
    return outStr;
}

void Paramters::FromString(const StringParam &inParams)
{
    mParamters.clear();
    const char *str(inParams.c_str());
    enum {
        StateFindKey,
        StateFindValue,
        StateSkipNewLine
    } state = StateFindKey;
    StringParam key, value;
    while (*str)
    {
        switch (state) {
        case StateFindKey:
            if (*str != '=')
                key += *str;
            else
                state = StateFindValue;
            break;
        case StateFindValue:
            if (*str != '\r' && *str != '\n')
                value += *str;
            else {
                mParamters[key] = value;
                key.clear();
                value.clear();
                state = StateSkipNewLine;
            }
            break;
        case StateSkipNewLine:
            if (*str != '\r' && *str != '\n') {
                state = StateFindKey;
                --str;
            }
            break;
        }
        ++str;
    }
    if (!key.empty())
        mParamters[key] = value;
}
