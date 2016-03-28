#pragma once

#include "stdafx.h"
#include "StdConsole.h"

COORD GetConsolePos(void)
{
    CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE),
        &csbiInfo);
    return csbiInfo.dwCursorPosition;
}

COORD GetConsoleSize(void)
{
    CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE),
        &csbiInfo);
    return csbiInfo.dwSize;
}

void SetConsolePos(COORD newPos)
{
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), newPos);
}
void MoveConsoleCursor(SHORT x, SHORT y)
{
    COORD cp = GetConsolePos();
    cp.X += x;
    cp.Y += y;
    SetConsolePos(cp);
}
AutoConsolePos::AutoConsolePos(PCOORD setPos /* = NULL */)
{
    mCurPos = GetConsolePos();
    if (setPos)
        SetConsolePos(*setPos);
}
AutoConsolePos::~AutoConsolePos()
{
    SetConsolePos(mCurPos);
}

void ConsolePrinter::CopyStr(const TCHAR *str)
{
    //	AllocateSpaceForStr(lstrlen(str));
    lstrcpy(mStr, str);
}
ConsolePrinter::ConsolePrinter()
{
    printPos = GetConsolePos();
    mStr[0] = 0;
    strLen = 0;
}

void ConsolePrinter::Print(const TCHAR *msg /* = NULL */, ...)
{
    lstring str;
    {
        if (msg != NULL && *msg != 0) {
            va_list arg;
            va_start(arg, msg);
            int len = _vsctprintf(msg, arg) + 4 * sizeof(TCHAR); // _vscprintf doesn't count + 1; terminating '\0'
            TCHAR *buf = new TCHAR[len];
            _vstprintf_s(buf, len, msg, arg);
            str = buf;
            delete[] buf;
        }
    }
    if (lstrcmp(mStr, str.c_str())) {
        Erase();
        CopyStr(str.c_str());
        AutoConsolePos acp(&printPos);
        lprintf(_T("%s"), mStr);
    }
}
void ConsolePrinter::Erase()
{
    int len = lstrlen(mStr);
    AutoConsolePos acp(&printPos);
    while (len-- > 0) {
        lprintf(_T(" "));
    }
}
ConsolePrinter::~ConsolePrinter(void)
{
    //		AllocateSpaceForStr(0); // Free space
}

ConsoleProgress::ConsoleProgress() {
    lprintf(_T("\r\t\t\t\t\t"));
    printPos = GetConsolePos();
    printDotPos = printPos;
    dwSize = GetConsoleSize();
    printDotGap = (double)dwSize.X / 100.0;
    printDotPos.X = 0;
    m_iPercentDone = 0;
    lprintf(_T("\r"));
}
ConsoleProgress::~ConsoleProgress() {
}
void ConsoleProgress::ShowPercentage(double percentDone)
{
    if (percentDone > 100)
        percentDone = 100;
    m_iPercentDone = percentDone;
    AutoConsolePos acp;
    int newX((int)(m_iPercentDone*printDotGap));
    SetConsolePos(printDotPos);
    while (printDotPos.X < newX)
    {
        lprintf(_T("."));
        printDotPos.X++;
    }
    SetConsolePos(printPos);
    lprintf(_T(" %02.02f%% "), percentDone);
}
