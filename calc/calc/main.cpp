//
//  main.cpp
//  calc
//
//  Created by Afroz Muzammil on 21/11/16.
//  Copyright © 2016 Afroz Muzammil. All rights reserved.
//

#include "stdafx.h"
#include "Calculator.h"
#include "stlutils.h"

static void printHelp()
{
    printf("calc: <expression> [options]\n");
    printf("e.g. calc (5+2)*pi/1.5*e-pow(2,3)\n");
	printf("Operations:\n");
	printf("%s\n", OperatorManager::GetInstance().GetDescription().c_str());
    printf("options:\n\t-b: print binary\n");
    printf("\t-h: print hex\n");
    printf("\t-o: print octal\n");
    printf("\t-p [precision]: print up to decimal precision. No precision given, print all decimals\n");
}

struct Options
{
    int decimalPrecision;
    STLUtils::UniqueVector<Number::StringType> printTypes;
    void Reset()
    {
        decimalPrecision = 2;
        printTypes.clear();
    }
};
static lstring getExpression(int argc, LPCTSTR *argv, Options &outOptions)
{
    lstring outStr;
    outOptions.Reset();
    if (argc < 2 || !lstrcmpi(argv[1], _T("/?")) || !lstrcmpi(argv[1], _T("--help"))) {
        printHelp();
    }
    else for (int i=1; i<argc; ++i) {
        if (!lstrcmpi(argv[i], _T("-b")))
            outOptions.printTypes.AddUnique(Number::Binary);
        else if (!lstrcmpi(argv[i], _T("-h")))
            outOptions.printTypes.AddUnique(Number::Hex);
        else if (!lstrcmpi(argv[i], _T("-o")))
            outOptions.printTypes.AddUnique(Number::Octal);
        else if (!lstrcmpi(argv[i], _T("-p"))) {
            ++i;
            outOptions.decimalPrecision = (int)StringUtils::getLLfromStr(argv[i]);
        }
        else
            outStr += argv[i];
    }
    
    return outStr;
}

int _tmain(int argc, const TCHAR * argv[]) {
    Options opt;
    lstring expr = getExpression(argc, argv, opt);
    if (expr.empty())
        return 1;
	int ret(0);
	Calculator calc;
	Number result(calc.EvaluateExpression(UNICODE_TO_UTF8(expr).c_str()));
	if (result.GetType() == Number::Invalid) {
		printf("%s\n", calc.GetErrorString().c_str());
		ret = 2;
	}
    else {
        printf("%s\n", result.GetAsString(Number::Normal, opt.decimalPrecision).c_str());
        if (result.GetType() == Number::Long) {
            for (auto type : opt.printTypes)
                printf("%s\n", result.GetAsString(type, opt.decimalPrecision).c_str());
        }
    }
    return ret;
}
