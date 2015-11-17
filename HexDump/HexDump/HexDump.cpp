// HexDump.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

static wchar_t getHexChar(char ch)
{
    ch &= 0xf;
    if (ch > 9)
        return 'A' + ch;
    return '0' + ch;
}
static void getHexStr(char ch, wchar_t *outHex)
{
    outHex[0] = getHexChar(ch >> 4);
    outHex[1] = getHexChar(ch);
    outHex[2] = 0;
}

#define HEX_WITH 16

static long long HexDump(const void *buffer, size_t size, long long startAddress = 0)
{
    const char *buf((const char *)buffer);
    for (size_t i = 0; i < size;) {
        _tprintf(_T("%08llX   "), startAddress+i);
        size_t s = i;
        const char *sBuf(buf);
        for (int j = 0; j < HEX_WITH && s < size; ++j, ++s, ++buf) {
            wchar_t hexStr[4] = { 0 };
            getHexStr(*buf, hexStr);
            _tprintf(_T("%s "), hexStr);
            if (j == (HEX_WITH >> 1))
                _tprintf(_T(" "));
        }
        _tprintf(_T("   "));
        s = i;
        for (int j = 0; j < HEX_WITH && s < size; ++j, ++s, ++sBuf)
            _tprintf(_T("%c"), isprint((unsigned char)*sBuf) ? *sBuf : '.');
        i = s;
        _tprintf(_T("\n"));
    }
    return startAddress + size;
}

static long long getLLfromStr(const TCHAR *str)
{
    bool bMinus(*str == '-');
    if (bMinus)
        ++str;
    long long retVal = std::stoll(str, NULL, 0);
    if (bMinus)
        retVal = -retVal;
    return retVal;
}

int _tmain(int argc, _TCHAR* argv[])
{
    int retVal(0);
    if (argc < 2)
    {
        _tprintf(_T("Usage:\nHexDump <file or string> [offset] [size]\n"));
        retVal = 1;
    }
    else
    {
        FILE *fp = NULL;
        _tfopen_s(&fp, argv[1], _T("rb"));
        if (fp != NULL)
        {
            if (argc > 2) {
                long long offset(getLLfromStr(argv[2]));
                _fseeki64(fp, offset, offset >= 0 ? SEEK_SET : SEEK_END);
            }
            long long sizeToRead(-1);
            if (argc > 3)
                sizeToRead = getLLfromStr(argv[3]);
            long long sa(_ftelli64(fp));
            while (true)
            {
                char buffer[4096];
                size_t cRead = _countof(buffer);
                if (sizeToRead > 0) {
                    if ((long long)cRead > sizeToRead)
                        cRead = sizeToRead;
                }
                cRead = fread_s(buffer, sizeof(buffer), sizeof(buffer[0]), cRead, fp);
                sa = HexDump(buffer, cRead, sa);
                if (cRead == 0)
                    break;
                if (sizeToRead > 0) {
                    sizeToRead -= cRead;
                    if (sizeToRead <= 0)
                        break;
                }
            }
            fclose(fp);
        }
        else
        {
            HexDump((const void *)argv[1], _tcslen(argv[1])*sizeof(argv[1][0]));
        }
    }
    return retVal;
}

