#pragma once
#include "BinaryFind.h"

class Number
{
public:
    Number(const char *n);
    Number(long long n = 0);
    Number(const Number& other);
    Number(Number&& other);
    operator bool() const;
    Number& operator=(const Number &other);
    Number& operator=(Number &&other);
    bool operator==(const Number &other) const;
    bool operator!=(const Number &other) const;
    Number operator- () const;
    Number& operator+=(const Number& other);
    Number& operator-=(const Number& other);
    Number operator* (const Number& n) const;
    Number& operator*= (const Number& n);
    Number operator/ (const Number& n) const;
    Number& operator/= (const Number& n);
    Number operator% (const Number& n) const;
    Number& operator%= (const Number& n);
    bool operator< (const Number& other) const;
    bool operator<= (const Number& other) const;
    bool operator> (const Number& other) const;
    bool operator>= (const Number& other) const;
    unsigned char operator[](size_t index) const;
    unsigned char& operator[](size_t index);
    size_t Size() const;
    Number& operator<<= (unsigned s);
    Number& operator>>= (unsigned s);
    std::string ToString() const;
    long long ToLL(bool *bOverflowed = nullptr) const;
    Number Devide(const Number &other, Number &remainder) const;
    Number SquareRoot(Number *outRemainder = nullptr) const;
protected:
    void SetNumber(const char *n = nullptr);
    void SetNumber(long long n);
    Number& PerformSum(const Number& other);
    bool IsLesserThan(const Number& other, bool *isEqual = nullptr, bool compareSign = false) const;
    void SetDataSize(size_t newSize = -1);
    int GetSubstrackCarry(size_t index);
    void ResizeLeadingZeros();
    void Shift(unsigned s, bool bLeft = true);
    int GetDevide(const Number *table) const;
    Number FromRange(size_t start, size_t end) const;
    int SqRootGetFactor(const Number &n, Number &multResult) const;
    BinaryData mNumber;
    bool mbNegative = false;
};

Number operator + (const Number& n1, const Number& n2);
Number operator - (const Number& n1, const Number& n2);
