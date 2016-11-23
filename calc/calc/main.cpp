//
//  main.cpp
//  calc
//
//  Created by Afroz Muzammil on 21/11/16.
//  Copyright © 2016 Afroz Muzammil. All rights reserved.
//

#include "stdafx.h"
#include "Calculator.h"

static void printHelp()
{
    _tprintf(_T("calc: <expression>\n"));
    _tprintf(_T("e.g. calc (5+2)*3.4/1.5-2^3\n"));
    _tprintf(_T("operation: all arithmetic\n"));
    _tprintf(_T("operation: * or x : multiply\n"));
    _tprintf(_T("operation: ! : factorial\n"));
    _tprintf(_T("operation: %% : remainder\n"));
    _tprintf(_T("operation: ^ : power\n"));
    _tprintf(_T("operation: sin,cos,tan,sinh,cosh,tanh\n"));
    _tprintf(_T("operation:  log, e\n"));
    _tprintf(_T("more to come\n"));
}

static lstring getExpression(int argc, LPCTSTR *argv)
{
    lstring outStr;
    
    if (argc < 2 || argv[1] == _T("/?") || argv[1]==_T("-h")) {
        printHelp();
    }
    else for (int i=1; i<argc; ++i) {
        outStr += argv[i];
    }
    if (outStr.length() > 0) {
        // remove white spaces
        StringUtils::Replace(outStr, _T(" "), _T(""));
        // remove tab spaces
        StringUtils::Replace(outStr, _T("\t"), _T(""));
        // remove carriage return
        StringUtils::Replace(outStr, _T("\r"), _T(""));
        // remove new line
        StringUtils::Replace(outStr, _T("\n"), _T(""));
    }
    
    return outStr;
}

int _tmain(int argc, const TCHAR * argv[]) {
    lstring expr = getExpression(argc, argv);
    if (expr.empty())
        return 1;
    
    return 0;
}
