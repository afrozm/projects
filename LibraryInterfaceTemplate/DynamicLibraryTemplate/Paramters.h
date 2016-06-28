#pragma once

#include <map>
#include <string>
class Paramters
{
public:
    typedef std::string StringParam;
    Paramters();
    Paramters(const StringParam &inParams);
    const StringParam& GetParamValue(const StringParam &inParamName, const StringParam &inDefaultValue = "") const;
    void SetParamValue(const StringParam &inParamName, const StringParam &inValue);
    void RemoveParam(const StringParam &inParamName);
    ~Paramters();
    StringParam ToString() const;
    void FromString(const StringParam &inParams);

private:
    typedef std::map<StringParam, StringParam> MapStringParam;
    MapStringParam mParamters;
};

