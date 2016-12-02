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
    printf("calc: <expression>\n");
    printf("e.g. calc (5+2)*pi/1.5*e-pow(2,3)\n");
	printf("Operations:\n");
	printf("%s", OperatorManager::GetInstance().GetDescription().c_str());
}

static lstring getExpression(int argc, LPCTSTR *argv)
{
    lstring outStr;
    
    if (argc < 2 || !lstrcmpi(argv[1], _T("/?")) || !lstrcmpi(argv[1], _T("-h"))) {
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
	int ret(0);
	Calculator calc;
	Number result(calc.EvaluateExpression(UNICODE_TO_UTF8(expr).c_str()));
	if (result.GetType() == Number::Invalid) {
		printf("%s\n", calc.GetErrorString().c_str());
		ret = 2;
	}
	else
		printf("%s\n", result.GetAsString().c_str());
    return ret;
}
