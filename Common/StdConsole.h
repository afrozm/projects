#pragma once

#include "Common.h"
#ifdef _WIN32
#include <windows.h>
#endif // _WIN32


COORD GetConsolePos(void);

COORD GetConsoleSize(void);

void SetConsolePos(COORD newPos);
void MoveConsoleCursor(SHORT x, SHORT y);

class AutoConsolePos {
    COORD mCurPos;
public:
    AutoConsolePos(PCOORD setPos = NULL);
    ~AutoConsolePos();
};

class ConsolePrinter
{
    COORD printPos;
    TCHAR mStr[256];
    int strLen;
    void CopyStr(const TCHAR *str);
public:
    ConsolePrinter();
    void Print(const TCHAR *str = NULL, ...);
    void Erase();
    ~ConsolePrinter(void);
};

struct ConsoleProgress {
    COORD printPos, dwSize;
    COORD printDotPos;
    double m_iPercentDone;
    double printDotGap;
    ConsoleProgress();
    ~ConsoleProgress();
    void ShowPercentage(double percentDone);
};
