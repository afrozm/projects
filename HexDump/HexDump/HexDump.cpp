// HexDump.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "BinaryFind.h"
#include "Progress.h"
#include "Path.h"

static long long HexDump(const BinaryData &buffer, long long startAddress = 0)
{
    _tprintf(_T("%s"), buffer.HexDump(startAddress).c_str());
    return startAddress + buffer.DataSize();
}

#define STR_IS_VALID_PTR(p) (p&&*p)
#define STR_INR_PTR(p) if(STR_IS_VALID_PTR(p)) ++p
#define STR_CHAR_IS_SPACE(c) ((c)==' ' || (c)=='\t' || (c)=='\n' || (c)=='\r')
#define STR_SKIP_SPACE(p) while(STR_IS_VALID_PTR(p)&&STR_CHAR_IS_SPACE(*p)) ++p
#define STR_SKIP_TILL_CHAR(p,c) while(STR_IS_VALID_PTR(p)&&*p!=c) ++p

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
    _tprintf(_T("[-fs to treat find buffer -f as string]\n"));
    _tprintf(_T("[-d[=<dump size>[,dump offset]]]. Valid only with -f\n"));
    _tprintf(_T("[-mp=<match pattern>]\n"));
    _tprintf(_T("[-ep=<exclude pattern>]\n"));
    _tprintf(_T("mp,ep is used if file is folder path\n"));
}
struct HDFDCB_Data
{
    int argc;
    TCHAR **argv;
    BinaryFind &bf;
    int nFiles, nFound, nMaxMatchPerFile;
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
        int nMatches(0);

        bf.SetFindBuffer();
        _tprintf(_T("%s\n"), fd.fullPath.c_str());
        pUserParam->nFiles++;
        Progress prog;
        const TCHAR *argStr = FindArgValue(argc, argv, _T("-o="));
        if (argStr != NULL) {
            long long offset(StringUtils::getLLfromStr(argStr));
            _fseeki64(fp, offset, offset >= 0 ? SEEK_SET : SEEK_END);
        }
        long long fileOffset(_ftelli64(fp));
        long long sizeToRead(fd.GetFileSize());
        const long long fileSize(sizeToRead);
        argStr = FindArgValue(argc, argv, _T("-s="));
        if (argStr != NULL) {
            long long szRead = StringUtils::getLLfromStr(argStr);
            if (fileOffset + szRead > sizeToRead)
                sizeToRead = sizeToRead - fileOffset;
            else
                sizeToRead = szRead;
        }
        size_t findDumpSize(0), findDumpOffset(-16);
        if (bf.HasFindPattern()) {
            argStr = FindArgValue(argc, argv, _T("-d"));
            if (argStr != NULL) {
                if (*argStr == '=') {
                    findDumpSize = StringUtils::getLLfromStr(argStr + 1);
                    STR_SKIP_TILL_CHAR(argStr, ';');
                    if (*argStr)
                        findDumpOffset = StringUtils::getLLfromStr(argStr + 1);
                }
                if (findDumpSize <= 0)
                    findDumpSize = 48;
            }
        }
        prog.SetTask(sizeToRead);
        BinaryData buffer(NULL, 4 * 1024 * 1024);
        while (sizeToRead > 0)
        {
            buffer.ReadFromFile(fp);
            if (buffer.DataSize() <= 0)
                break;
            sizeToRead -= buffer.DataSize();
            if (bf.HasFindPattern()) {
                bf.SetFindBuffer(buffer);
                while (true)
                {
                    long long findPos = bf.FindNext();
                    if (findPos >= 0) {
                        _tprintf(_T("%08llX=-%08llX\n"), fileOffset + findPos, fileSize - (fileOffset + findPos));
                        if (findDumpSize > 0) {
                            const long long curPos(_ftelli64(fp));
                            long long newPos(fileOffset + findPos + findDumpOffset);
                            if (newPos < 0)
                                newPos = 0;
                            newPos &= ~0xf;
                            BinaryData bd(NULL, findDumpSize);
                            bd.ReadFromFile(fp, 0, newPos);
                            HexDump(bd, newPos);
                            _fseeki64(fp, curPos, SEEK_SET);
                        }
                        ++nMatches;
                    }
                    else break;
                }
            }
            else
                fileOffset = HexDump(buffer, fileOffset);
            if (prog.UpdateProgress(prog.GetCurrentDone() + buffer.DataSize()))
                _tprintf(_T("\r%02.02f%%\r"), prog.GetCurrentPercentageDone());
        }
        _tprintf(_T("\r            \r"));
        fclose(fp);
        if (nMatches > 0) {
            pUserParam->nFound++;
            if (nMatches > pUserParam->nMaxMatchPerFile)
                pUserParam->nMaxMatchPerFile = nMatches;
            if (nMatches > 1)
                _tprintf(_T("%d matches\n"), nMatches);
        }
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
            BinaryData findBuffer;
            findBuffer.BuildFromString(FindArgValue(argc, argv, _T("-f=")), FindArgValue(argc, argv, _T("-fs")) != NULL);
            if (findBuffer.DataSize() > 0)
                bf.SetFindPattern(findBuffer);
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
            if (cbData.nFiles > 1)
                _tprintf(_T("Total files: %d\n"), cbData.nFiles);
            if (cbData.nFound > 1) {
                _tprintf(_T("Total files matching: %d\n"), cbData.nFound);
                if (cbData.nMaxMatchPerFile > 1)
                    _tprintf(_T("max matching in a file : %d\n"), cbData.nMaxMatchPerFile);
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

