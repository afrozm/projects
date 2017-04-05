#include "stdafx.h"
#include "WordParser.h"

WordParser::WordParser()
    : mEncoding(FileEncoding_NotInitialized)
{
}

WordParser::~WordParser()
{
}

bool WordParser::IsWordSep(wchar_t ch)
{
    if (ch == 0)
        return true;
    if (STR_CHAR_IS_SPACE(ch))
        return true;
    static char sSeps[256] = { 0 };
    if (sSeps[' '] == 0) {
        const char *pSep = " ~`!@#$%^&*()_-+=|}{\\][';\":/.,?><\r\n\b\t";
        for (const char *p = pSep; *p; ++p)
            sSeps[*p] = *p;
    }
    if (ch < 256)
        return sSeps[ch] != 0;
    return false;
}

bool WordParser::IsCompleteWord(LPCTSTR inString, int pos, int len)
{
    if (inString == nullptr || *inString == 0)
        return true;
    if (pos == 0 || (!IsWordSep(inString[pos]) && IsWordSep(inString[pos-1]))) {
        if (lstring(inString).length() <= pos + len)
            return true;
        return IsWordSep(inString[pos + len]);
    }
    return false;
}

lstring WordParser::GetCompleteWord(LPCTSTR inString, int startPos, int len)
{
    if (inString == nullptr || *inString == 0)
        return lstring();
    if (startPos < 0)
        startPos = 0;
    int endPos(startPos);
    LPCTSTR pStr(inString + startPos);
    while (startPos > 0)
    {
        --pStr;
        if (IsWordSep(*pStr))
            break;
        --startPos;
    }
    pStr = inString + endPos;
    while (*pStr)
    {
        if (len <= 0) {
            if (IsWordSep(*pStr))
                break;
        }
        --len;
        ++pStr;
        ++endPos;
    }
    return lstring(inString + startPos, endPos - startPos);
}

// Integrate ParseWords to content search
INT_PTR WordParser::ParseWords(const BinaryData &inData, std::vector<std::wstring> &outWords)
{
    outWords.clear();
    size_t startIndex = 0;
    if (mEncoding == FileEncoding_NotInitialized) {
        mEncoding = FileEncoding_ANSI;
        if (inData[0] == 0xef && inData[1] == 0xbb && inData[2] == 0xbf) {
            mEncoding = FileEncoding_UTF8;
            startIndex = 3;
        }
        else if (inData[0] == 0xff && inData[1] == 0xfe) {
            mEncoding = FileEncoding_UNICODE;
            startIndex = 2;
        }
        else if (inData[0] == 0xfe && inData[1] == 0xff) {
            mEncoding = FileEncoding_UNICODE_BIG;
            startIndex = 2;
        }
    }
    if (startIndex < inData.DataSize())
    {
        BinaryData bd(inData, inData.DataSize() - startIndex, false);
        std::wstring bigStr;
        // Get string from buffer
        GetString(bd, bigStr);
        // Split it
        SplitWords(bigStr.c_str(), outWords);
    }
    return outWords.size();
}

void WordParser::GetString(const BinaryData &bd, std::wstring &outStr)
{
    switch (mEncoding)
    {
    case FileEncoding_ANSI:
    case FileEncoding_UTF8:
        outStr.assign(SystemUtils::UTF8ToUnicode((const char *)(const void*)bd, (int)bd.DataSize()));
        break;
    case FileEncoding_UNICODE:
        outStr.assign((const wchar_t *)(const void*)bd, (int)bd.DataSize());
        break;
    case FileEncoding_UNICODE_BIG:
    {
        for (size_t i = 0; i < bd.DataSize(); i += 2) {
            wchar_t ch(bd[i]);
            ch <<= 8;
            ch |= bd[i + 1];
            outStr.append(&ch, 1);
        }
    }
    break;
    }
}

#define STR_SKIP_TILL_WORD_START(p) while(*p&&IsWordSep(*p)) ++p
#define STR_SKIP_TILL_WORD_END(p) while(*p&&!IsWordSep(*p)) ++p

void WordParser::SplitWords(const wchar_t *pStr, std::vector<std::wstring> &outWords)
{
    while (*pStr) {
        STR_SKIP_TILL_WORD_START(pStr);
        const wchar_t *pWordStart(pStr);
        STR_SKIP_TILL_WORD_END(pStr);
        std::wstring word(pWordStart, pStr-pWordStart);
        if (!word.empty()&& word.length()<32)
            outWords.push_back(word);
    }
}










WordCount::WordCount()
{
}

WordCount::~WordCount()
{
}

const WordCount::MapWordCount& WordCount::Count(const std::vector<std::wstring> &inWords)
{
    for (auto &w : inWords)
        m_MapWordCount[w]++;
    return m_MapWordCount;
}
