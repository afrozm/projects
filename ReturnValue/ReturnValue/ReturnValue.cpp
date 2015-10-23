// ReturnValue.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


int _tmain(int argc, _TCHAR* argv[])
{
    int returnValue(0);

    if (argc > 1)
        returnValue = _tstoi(argv[1]);
    _tprintf(_T("ReturnValue: %d\n"), returnValue);
    return returnValue;
}

