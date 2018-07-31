// prime.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "PrimeNumber.h"
#include <conio.h>
#include "StdConsole.h"

static void BuildPrime()
{
    PrimeNumber pn;
    pn.StartCompute();
    _tprintf(_T("Highest prime found: "));
    ConsolePrinter cpHPN;
    cpHPN.Print(_T("%llu"), pn.GetCurrentHighestPrime());
    _tprintf(_T("\n"));
    ConsolePrinter cpProgress;
    const TCHAR *strProg(_T("....."));
    const int pglen(lstrlen(strProg));
    int pgIx(pglen-1), incr(-1);
    while (true)
    {
        cpProgress.Print(strProg + pgIx);
        pgIx += incr;
        if (pgIx <= 0 || pgIx >= pglen)
            incr = -incr;
        if (_kbhit() && _getch() == 27) break;
        if (pn.ComputeNextPrime())
            cpHPN.Print(_T("%llu"), pn.GetCurrentHighestPrime());
    }
}

static bool IsPrime(unsigned long long number)
{
    return PrimeNumber().IsPrime(number);
}
static unsigned long long NthPrimeNumber(unsigned long long number)
{
    return PrimeNumber().GetNthPrime(number);
}

int _tmain(int argc, _TCHAR* argv[])
{
    if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            bool nNth(!isdigit(argv[i][0]));
            unsigned long long number(StringUtils::getLLfromStr(argv[i]+nNth));
            _tprintf(_T("%llu%s:%llu\n"), number, nNth ? _T("th") : _T(""),
                nNth ? NthPrimeNumber(number) : IsPrime(number));
        }
    }
    else
        BuildPrime();
    return 0;
}

