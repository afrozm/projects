// ShuffleWord.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdlib.h>
#include <time.h>
#include <conio.h>
#include <algorithm>

#define IS_LINE(c) ((c)=='\n'||(c)=='\r')

int _tmain(int argc, _TCHAR* argv[])
{
    srand((unsigned int)time(NULL));
    TCHAR strWord[256] = { 0 };
    _tprintf(_T("Enter word: "));
    _fgetts(strWord, _countof(strWord), stdin);
    int len(lstrlen(strWord));
    while (len > 0 && IS_LINE(strWord[len - 1]))
        --len;
    strWord[len] = 0;
    while (len > 0) {
        Sleep(30);
        if (_kbhit()) {
            if (_gettch() == 27)
                break;
            int i1 = rand() % len;
            int i2 = rand() % len;
            std::swap(strWord[i1], strWord[i2]);
            _tprintf(_T("\r%s"), strWord);
        }
    }
    return 0;
}

