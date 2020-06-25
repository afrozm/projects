#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>

class Value
{
public:
    Value();
    ~Value();
    Value(double number);
    Value(const char *inStr);
    Value(const Value &other);
    Value& operator=(const Value &other);
    bool operator < (const Value &other) const { return Compare(other) < 0; }
    bool operator == (const Value &other) const { return Compare(other) == 0; }
    bool operator != (const Value &other) const { return Compare(other) != 0; }
    int Compare(const Value &other) const;
    operator bool() const;
    operator double() const { return mNumber; }
    operator const char *() const { return mString.c_str(); }
    size_t GetArrayCount() const { return mArray.size(); }
    Value& operator[](size_t index);
    Value& operator[](const char *keyStr);
    const Value& operator[](size_t index) const;
    const Value& operator[](const char *keyStr) const;
    std::string GetAsString(int tabCount=0) const;
    const char* FromString(const char *inStr);
private:
    enum Type {
        VTNULL,
        VTString,
        VTNumber,
        VTArray,
        VTMap
    };
    Type mType;
    double mNumber;
    std::string mString;
    typedef std::unique_ptr<Value> PtrValue;
    std::vector<PtrValue> mArray;
    std::map<std::string, PtrValue> mMap;
};

