// Mp4.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "BinaryFind.h"
#include <windows.h>

using namespace BinaryDataUtil;

static bool isValidFOURCC(const BinaryData &inData, size_t offset = 0)
{
    size_t i = 0;
    for (; i < 4 ; ++i) {
        if (!isprint(inData[i + offset]))
            break;
    }
    return i == 4;
}

// mov mp4
int _tmain(int argc, _TCHAR* argv[])
{
    int retVal(ERROR_INVALID_FUNCTION);
    if (argc > 1) {
        FILE *pFile = NULL;
        _tfopen_s(&pFile, argv[1], _T("rb"));
        if (pFile != NULL) {
            int sectionNumber=(0);
            unsigned long long totalSize(0);
            BinaryData data(NULL, 8);
            size_t szRead = data.ReadFromFile(pFile, 4, totalSize);
            bool bIsAvi(isValidFOURCC(data));
            _tprintf(_T("Type:%s\n"), bIsAvi ? _T("avi") : _T("mov,mp4"));
            size_t offsets[2] = { 0,4 };
            while (true)
            {
                unsigned long long size(0);
                szRead = data.ReadFromFile(pFile, 8, totalSize);
                if (szRead == 0) {
                    _tprintf(_T("END: %zd\n"), szRead);
                    break;
                }
                size = GetValueType<unsigned int>(data,offsets[bIsAvi]);
                if (bIsAvi)
                    size = ToggleEndian((unsigned int)size);
                if (!isValidFOURCC(data, offsets[!bIsAvi])) {
                    _tprintf(_T("Invalid FOURCC %c%c%c%c\n"), data[0],data[1],data[2],data[3]);
                    break;
                }
                if (size == 1) {
                    szRead = data.ReadFromFile(pFile, 8);
                    size = GetValueType<unsigned long long>(data);
                    if (bIsAvi)
                        size = ToggleEndian(size);
                }
                if (szRead == 0 || size == 0) {
                    _tprintf(_T("END: %zd:%lld\n"), szRead, size);
                    break;
                }
                ++sectionNumber;
                _tprintf(_T("%d.   0x%08llX:    0x%llx - %lld\n"), sectionNumber, totalSize, size, size);
                totalSize += size;
                if (bIsAvi)
                    totalSize += 8;
            }
            _fseeki64(pFile, 0, SEEK_END);
            long long fileSize = _ftelli64(pFile);
            fclose(pFile);
            _tprintf(_T("Calculated: 0x%08llX - %lld\n"), totalSize, totalSize);
            _tprintf(_T("Actual: 0x%08llX - %lld\n"), fileSize, fileSize);
            retVal = totalSize == fileSize ? 0 : ERROR_INVALID_DATA;
            _tprintf(_T("%sMatch\n"), totalSize == fileSize ? _T("") : _T("Mis"));
        }
        else
            retVal = ERROR_FILE_NOT_FOUND;
    }
    else
        _tprintf(_T("Mp4 <file>\n"));
    return retVal;
}

