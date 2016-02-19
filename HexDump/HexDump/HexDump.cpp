// HexDump.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "BinaryFind.h"
#include "Progress.h"
#include "Path.h"

static wchar_t getHexChar(char ch)
{
    ch &= 0xf;
    if (ch > 9)
        return 'A' + ch - 0xA;
    return '0' + ch;
}
static char getHexByte(wchar_t ch)
{
    if (ch >= '0' && ch <= '9')
        return ch - '0';
    if (ch >= 'A' && ch <= 'F')
        return 0xA + ch - 'A';
    if (ch >= 'a' && ch <= 'f')
        return 0xA + ch - 'a';
    return (char)ch;
}
static void getHexStr(char ch, wchar_t *outHex)
{
    outHex[0] = getHexChar(ch >> 4);
    outHex[1] = getHexChar(ch);
    outHex[2] = 0;
}
static char getHexByte(const wchar_t *inStr)
{
    char ch = 0;
    if (inStr) {
        for (int i = 0; i < 2 && inStr[i]; ++i) {
            ch <<= 4;
            ch |= getHexByte(inStr[i]);
        }
    }
    return ch;
}

#define HEX_WITH 16

static long long HexDump(const void *buffer, size_t size, long long startAddress = 0)
{
    const char *buf((const char *)buffer);
    for (size_t i = 0; i < size;) {
        _tprintf(_T("%08llX  "), startAddress+i);
        size_t s = i;
        const char *sBuf(buf);
        for (int j = 0; j < HEX_WITH && s < size; ++j, ++s, ++buf) {
            if (j%4==0)
                _tprintf(_T(" "));
            if (j == (HEX_WITH >> 1))
                _tprintf(_T(" "));
            wchar_t hexStr[4] = { 0 };
            getHexStr(*buf, hexStr);
            _tprintf(_T("%s"), hexStr);
        }
        _tprintf(_T("   "));
        s = i;
        for (int j = 0; j < HEX_WITH && s < size; ++j, ++s, ++sBuf) {
            if (j == (HEX_WITH >> 1))
                _tprintf(_T(" "));
            _tprintf(_T("%c"), isprint((unsigned char)*sBuf) ? *sBuf : '.');
        }
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

#define STR_IS_VALID_PTR(p) (p&&*p)
#define STR_INR_PTR(p) if(STR_IS_VALID_PTR(p)) ++p
#define STR_CHAR_IS_SPACE(c) ((c)==' ' || (c)=='\t' || (c)=='\n' || (c)=='\r')
#define STR_SKIP_SPACE(p) while(STR_IS_VALID_PTR(p)&&STR_CHAR_IS_SPACE(*p)) ++p

static size_t StrToBuffer(const TCHAR *pStr, VecChar &outBuffer, bool asString = false)
{
    if (pStr != NULL)
    {
        while (*pStr)
        {
            if (!asString) {
                STR_SKIP_SPACE(pStr);
                if (!STR_IS_VALID_PTR(pStr))
                    continue;
                outBuffer.push_back(getHexByte(pStr));
                STR_INR_PTR(pStr);
            }
            else
                outBuffer.push_back((char)*pStr);
            STR_INR_PTR(pStr);
        }
    }
    return outBuffer.size();
}


int FindArg(int argc, _TCHAR* argv[], const TCHAR * arg)
{
    size_t len(_tcslen(arg));
    while (--argc > 0) {
        if (_tcsncicmp(argv[argc], arg, len) == 0)
            break;
    }
    return argc;
}
const TCHAR* FindArgValue(int argc, _TCHAR* argv[], const TCHAR * arg)
{
    int argPos(FindArg(argc, argv, arg));
    if (argPos > 0)
        return argv[argPos] + _tcslen(arg);
    return NULL;
}
int FindNextArg(int argc, _TCHAR* argv[], int startArg)
{
    while (++startArg < argc) {
        if (argv[startArg][0] != '-')
            break;
    }
    if (startArg >= argc)
        startArg = 0;
    return startArg;
}
void Help()
{
    _tprintf(_T("Usage:\nHexDump <file or string> [-o=<offset>] [-s=<size>] [-f=<find buffer> [-fs]]\n"));
    _tprintf(_T("[-mp=<match pattern>]\n"));
    _tprintf(_T("[-ep=<exclude pattern>]\n"));
    _tprintf(_T("mp,ep is used if file is folder path\n"));
}
struct HDFDCB_Data
{
    int argc;
    TCHAR **argv;
    BinaryFind &bf;
};
static int HEXDump_FindCallBack(FindData &fd, HDFDCB_Data *pUserParam)
{
    if (!fd.fileMatched)
        return 0;
    int argc(pUserParam->argc);
    TCHAR **argv(pUserParam->argv);
    BinaryFind &bf(pUserParam->bf);
    FILE *fp = NULL;
    _tfopen_s(&fp, fd.fullPath.c_str(), _T("rb"));
    if (fp != NULL)
    {
        bf.SetFindBuffer();
        _tprintf(_T("%s\n"), fd.fullPath.c_str());
        Progress prog;
        const TCHAR *argStr = FindArgValue(argc, argv, _T("-o="));
        if (argStr != NULL) {
            long long offset(getLLfromStr(argStr));
            _fseeki64(fp, offset, offset >= 0 ? SEEK_SET : SEEK_END);
        }
        long long fileOffset(_ftelli64(fp));
        long long sizeToRead(fd.GetFileSize());
        const long long fileSize(sizeToRead);
        argStr = FindArgValue(argc, argv, _T("-s="));
        if (argStr != NULL) {
            long long szRead = getLLfromStr(argStr);
            if (fileOffset + szRead > sizeToRead)
                sizeToRead = sizeToRead - fileOffset;
            else
                sizeToRead = szRead;
        }
        prog.SetTask(sizeToRead);
        VecChar buffer;
        buffer.resize(4 * 1024 * 1024);
        while (sizeToRead > 0)
        {
            size_t cRead = buffer.size();
            if (sizeToRead > 0) {
                if ((long long)cRead > sizeToRead)
                    cRead = sizeToRead;
            }
            cRead = fread_s(&buffer[0], buffer.size(), sizeof(char), cRead, fp);
            if (cRead <= 0)
                break;
            sizeToRead -= cRead;
            if (bf.HasFindPattern()) {
                bf.SetFindBuffer(&buffer[0], cRead);
                while (true)
                {
                    long long findPos = bf.FindNext();
                    if (findPos >= 0)
                        _tprintf(_T("%08llX=-%08llX\n"), fileOffset + findPos, fileSize - (fileOffset + findPos));
                    else break;
                }
            }
            else
                fileOffset = HexDump(&buffer[0], cRead, fileOffset);
            if (prog.UpdateProgress(prog.GetCurrentDone() + cRead))
                _tprintf(_T("\r%02.02f%%\r"), prog.GetCurrentPercentageDone());
        }
        _tprintf(_T("\r            \r"));
        fclose(fp);
    }
    return 0;
}
int _tmain(int argc, _TCHAR* argv[])
{
    int retVal(0);
    int arg = FindNextArg(argc, argv, 0);
    if (arg <= 0)
    {
        Help();
        retVal = 1;
    }
    else
    {
        for (int i = 1; i < argc; ++i)
            _tprintf(_T("%s\n"), argv[i]);
        BinaryFind bf;
        {
            VecChar findBuffer;
            StrToBuffer(FindArgValue(argc, argv, _T("-f=")), findBuffer, FindArgValue(argc, argv, _T("-fs")) != NULL);
            if (!findBuffer.empty())
                bf.SetFindPattern(&findBuffer[0], findBuffer.size());
        }
        const TCHAR *fileOrString(argv[arg]);
        if (Path(fileOrString).Exists()) {
            Path filePath(fileOrString);
            HDFDCB_Data cbData = { argc, argv, bf };
            if (filePath.IsDir()) {
                Finder f((FindCallBack)HEXDump_FindCallBack, &cbData,
                    FindArgValue(argc, argv, _T("-mp=")),
                    FindArgValue(argc, argv, _T("-ep=")));
                f.StartFind(fileOrString);
            }
            else {
                FindData fd(NULL, filePath, true);
                HEXDump_FindCallBack(fd, &cbData);
            }
        }
        else
        {
            if (bf.HasFindPattern()) {
                bf.SetFindBuffer((const void *)fileOrString, _tcslen(fileOrString)*sizeof(fileOrString[0]));
                while (true)
                {
                    long long findPos = bf.FindNext();
                    if (findPos >= 0)
                        _tprintf(_T("%08llX\n"), findPos);
                    else break;
                }
            }
            else
                HexDump((const void *)fileOrString, _tcslen(fileOrString)*sizeof(fileOrString[0]));
        }
    }
    return retVal;
}

