
#pragma once
#include "BinaryFind.h"
#include "TextReader.h"
#include <map>

class WordParser
{
public:
    WordParser();
    ~WordParser();
    INT_PTR ParseWords(const BinaryData &inData, std::vector<std::wstring> &outWords);
private:
    void GetString(const BinaryData &inData, std::wstring &outStr);
    void SplitWords(const wchar_t *pStr, std::vector<std::wstring> &outWords);
    FileEncoding mEncoding;
};

class WordCount
{
public:
    WordCount();
    ~WordCount();
    typedef std::map<std::wstring, int> MapWordCount;
    const MapWordCount& Count(const std::vector<std::wstring> &inWords);
    const MapWordCount& GetCount() const { return m_MapWordCount; }
private:
    MapWordCount m_MapWordCount;
};
