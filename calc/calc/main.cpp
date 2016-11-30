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
    
    return outStr;
}

int _tmain(int argc, const TCHAR * argv[]) {
    lstring expr = getExpression(argc, argv);
    if (expr.empty())
        return 1;
	Calculator calc;
	Number result(calc.EvaluateExpression(UNICODE_TO_UTF8(expr).c_str()));
	if (result.GetType() == Number::Invalid)
		printf("%s\n", calc.GetErrorString().c_str());
	else
		printf("%s\n", result.GetAsString().c_str());
    return 0;
}
