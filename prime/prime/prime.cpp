// prime.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "PrimeNumber.h"
#include <conio.h>
#include "StdConsole.h"
#include "Number.h"

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

int _tmain(int argc, _TCHAR* argv[])
{
    if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            bool nNth(!isdigit(argv[i][0]));
            Number number(UNICODE_TO_UTF8(argv[i]+nNth).c_str());
            printf("%s ", number.ToString().c_str());
            if (nNth)
                printf("prime number is %s", PrimeNumber().GetNthPrime(number).ToString().c_str());
            else {
                number = PrimeNumber().IsPrime(number);
                if (!number)
                    printf("cannot determine prime number");
                else if (number > Number(1LL))
                    printf("is not a prime number - divisible by %s", number.ToString().c_str());
                else
                    printf("is a prime number");
            }
            printf("\n");
        }
    }
    else
        BuildPrime();
    return 0;
}

