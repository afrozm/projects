//
//  Calculator.hpp
//  calc
//
//  Created by Afroz Muzammil on 23/11/16.
//  Copyright © 2016 Afroz Muzammil. All rights reserved.
//

#ifndef Calculator_hpp
#define Calculator_hpp

#include <string>

class Number {
public:
    Number(const char *number);
    Number(long long no=0);
    Number(double number);
    enum StringType {
        Normal,
        Binary,
        Octal,
        Hex,
    };
    std::string GetAsString(int StringType = Normal, int decimalPrecision=2) const;
    double GetDouble() const;
    long long GetLong() const;
    enum NumberType {
        Invalid,
        Long,
        Double
    } ;
    NumberType GetType() const;
private:
    union {
        long long mllNumber;
        double mdNumber;
    } mNumber;
    NumberType mType;
};

class Calculator {
public:
    Calculator();
    Number EvaluateExpression(const char *expression);
    const std::string& GetErrorString() const;
private:
    std::string mErrorStr;
};

#endif /* Calculator_hpp */
