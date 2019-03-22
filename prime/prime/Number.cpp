#include "stdafx.h"
#include "Number.h"
#include <algorithm>

Number::Number(const char *n /* = nullptr */)
{
    SetNumber(n);
}

Number::Number(long long n)
{
    SetNumber(n);
}

Number::Number(const Number& other)
    : mbNegative(other.mbNegative), mNumber(other.mNumber, other.Size())
{

}
Number& Number::operator=(const Number &other)
{
    mbNegative = other.mbNegative;
    mNumber.SetData(other.mNumber, other.Size());
    return *this;
}

Number::Number(Number&& other)
    : mNumber(std::move(other.mNumber)), mbNegative(other.mbNegative)
{
}

Number::operator bool() const
{
    const size_t size(Size());
    if (size == 0)
        return false;
    for (size_t i = 0; i < size; ++i)
        if (mNumber[i])
            return true;
    return false;
}


Number& Number::operator=(Number &&other)
{
    mNumber = std::move(other.mNumber);
    mbNegative = other.mbNegative;
    return *this;
}

bool Number::operator==(const Number &other) const
{
    bool isEqual(false);
    IsLesserThan(other, &isEqual, true);
    return isEqual;
}

bool Number::operator!=(const Number &other) const
{
    return !(*this == other);
}

Number Number::operator-() const
{
    Number outNumber(*this);
    outNumber.mbNegative = !mbNegative;
    return outNumber;
}

Number& Number::operator-=(const Number& other)
{
    return PerformSum(-other);
}

bool Number::IsLesserThan(const Number& other, bool *isEqual /* = nullptr */, bool compareSign /* = false */) const
{
    if (isEqual)
        *isEqual = false;
    if (compareSign && mbNegative != other.mbNegative)
        return other.mbNegative;
    if (Size() < other.Size())
        return true;
    if (Size() > other.Size())
        return false;
    for (int i = (int)Size() - 1; i >= 0; --i) {
        if (mNumber[i] > other[i])
            return false;
        if (mNumber[i] < other[i])
            return true;
    }
    if (isEqual)
        *isEqual = true;
    return false;
}


Number Number::operator* (const Number& other) const
{
    Number result;
    result.mbNegative = mbNegative != other.mbNegative;
    bool isLess = IsLesserThan(other);
    const Number &top(isLess ? other : *this);
    const Number &bottom(isLess ? *this : other);
    for (size_t bi = 0; bi < bottom.Size(); bi++) {
        if (bottom[bi] == 0) continue;
        Number curMult;
        if (bottom[bi] > 1) {
            int c = 0;
            for (size_t ti = 0; ti < top.Size(); ++ti) {
                int mul = top[ti] * bottom[bi] + c;
                if (mul > 9) {
                    c = mul / 10;
                    mul %= 10;
                }
                curMult[ti] = mul;
            }
            if (c)
                curMult[-1] = c;
        }
        else
            curMult = top;
        curMult.Shift((unsigned)bi);
        curMult.SetDataSize();
        result += curMult;
    }
    return result;
}

Number& Number::operator*=(const Number& n)
{
    *this = *this * n;
    return *this;
}

Number Number::operator/(const Number& n) const
{
    Number r;
    return Devide(n, r);
}

Number& Number::operator/=(const Number& n)
{
    *this = *this / n;
    return *this;
}

Number Number::operator%(const Number& n) const
{
    Number r;
    Devide(n, r);
    return r;
}

Number& Number::operator%=(const Number& n)
{
    *this = *this % n;
    return *this;
}

bool Number::operator<(const Number& other) const
{
    return IsLesserThan(other, nullptr, true);
}

bool Number::operator<=(const Number& other) const
{
    bool isEqual(false);
    bool isLess(IsLesserThan(other, &isEqual, true));
    return isLess || isEqual;
}

bool Number::operator>(const Number& other) const
{
    return *this != other && !(*this < other);
}

bool Number::operator>=(const Number& other) const
{
    return !(*this < other);
}

unsigned char Number::operator[](size_t index) const
{
    return mNumber[index];
}

unsigned char& Number::operator[](size_t index)
{
    return mNumber[index];
}

size_t Number::Size() const
{
    return mNumber.Size();
}

std::string Number::ToString() const
{
    size_t size(Size());
    std::string outStr(size, '0');
    const char *buffer((const char*)(void*)mNumber);
    for (size_t i = 0; i < size; ++i)
        outStr[i] = '0' + buffer[size -1 - i];
    if (outStr.empty())
        outStr = "0";
    return outStr;
}

long long Number::ToLL(bool *bOverflowed /* = nullptr */) const
{
    long long ll(0);
    for (int i = (int)Size() - 1; i >= 0; --i) {
        long long oldLL;
        if (bOverflowed && !*bOverflowed)
            oldLL = ll;
        ll *= 10;
        ll += mNumber[i];
        if (bOverflowed && !*bOverflowed)
            *bOverflowed =  ll/10 != oldLL;
    }
    if (mbNegative)
        ll = -ll;
    return ll;
}

// GetDevide - Returns 1 to 9 number divisible
int Number::GetDevide(const Number *table) const
{
    int maxIndex(10), minIndex(0);
    int mid((maxIndex + minIndex) >> 1);
    while (maxIndex - minIndex > 1) {
        const Number &t(table[mid]);
        bool isEqual(false);
        if (IsLesserThan(t, &isEqual))
            maxIndex = mid;
        else if (isEqual)
            break;
        else
            minIndex = mid;
        mid = (maxIndex + minIndex) >> 1;
    }
    return mid+1;
}
Number Number::Devide(const Number &other, Number &remainder) const
{
    Number result;
    if (this == &remainder || &other == &remainder || !other || !*this)
        return result;
    result.mbNegative = mbNegative != other.mbNegative;
    bool isEqual(false);
    bool isLess = IsLesserThan(other, &isEqual);
    if (isLess)
        remainder = *this;
    else if (isEqual)
        result = 1LL;
    else {
        Number table[10];
        for (int i = 0; i < 10; ++i)
            table[i] = other * Number(i+1);
        int startIndex((int)Size());
        int endIndex(startIndex - (int)other.Size());
        remainder = FromRange(startIndex, endIndex);
        while (startIndex > 0) {
            bool remIsLess(remainder.IsLesserThan(other));
            while (endIndex > 0 && remIsLess) {
                --endIndex;
                remainder <<= 1;
                remainder[0] = (*this)[endIndex];
                remainder.ResizeLeadingZeros();
                remIsLess = remainder.IsLesserThan(other);
                if (remIsLess)
                    result <<= 1;
            }
            startIndex = endIndex;
            if (!remainder.IsLesserThan(other)) {
                result <<= 1;
                int mult(remainder.GetDevide(table));
                result[0] = mult;
                remainder -= table[mult-1];
            }
        }
        result.ResizeLeadingZeros();
    }

    return result;
}

Number Number::FromRange(size_t start, size_t end) const
{
    if (end > Size())
        end = Size();
    if (start > end)
        std::swap(start, end);
    Number result;
    size_t size(end - start);
    const char *buffer((const char*)(void*)mNumber);
    buffer += start;
    result.mNumber.SetData(buffer, size);
    result.SetDataSize();
    return result;
}

Number& Number::operator<<=(unsigned s)
{
    Shift(s);
    return *this;
}

Number& Number::operator>>=(unsigned s)
{
    Shift(s, false);
    return *this;
}

void Number::SetNumber(const char *n /*= nullptr*/)
{
    mbNegative = false;
    if (n == nullptr || *n == 0) {
        mNumber.SetData();
    }
    else {
        if (*n == '-') {
            mbNegative = true;
            ++n;
        }        const char *end(n);
        while (*end == '0') ++end;
        while (*end >= '0' && *end <= '9')
            ++end;
        SetDataSize(end - n);
        size_t i = 0;
        char *buffer((char*)(void*)mNumber);
        while (n < end) {
            --end;
            buffer[i++] = *end - '0';
        }
        if (!*this)
            SetNumber();
    }
}

void Number::SetNumber(long long n)
{
    mbNegative = false;
    if (n == 0)
        mNumber.Clear();
    else {
        mbNegative = n < 0;
        if (mbNegative)
            n = -n;
        while (n > 0) {
            mNumber[-1] = n % 10;
            n /= 10;
        }
        SetDataSize();
    }
}

Number& Number::PerformSum(const Number& other)
{
    if (mbNegative == other.mbNegative) {// add
        if (Size() < other.Size())
            SetDataSize(other.Size());
        int c = 0;
        for (size_t i = 0; i < Size(); i++) {
            int sum = mNumber[i] + other[i] + c;
            c = sum > 9;
            if (c)
                sum -= 10;
            mNumber[i] = sum;
        }
        if (c)
            mNumber[Size()] = c;
        SetDataSize();
    }
    else { //subtract
        mbNegative = IsLesserThan(other);
        Number top(mbNegative ? other : *this);
        Number bottom(mbNegative ? *this : other);
        Number result;
        for (size_t i = 0; i < Size(); i++)
            result[i] = ((top[i] < bottom[i]) ? top.GetSubstrackCarry(i) : top[i]) - bottom[i];
        *this = result;
        ResizeLeadingZeros();
    }
    return *this;
}

void Number::SetDataSize(size_t newSize /*= -1*/)
{
    mNumber.SetData(nullptr, newSize == -1 ? Size() : newSize, true, false);
    mNumber.SetDataSize(Size());
}

int Number::GetSubstrackCarry(size_t index)
{
    int retVal((index+1) < mNumber.DataSize() ? mNumber[index]+10 : 0);
    ++index;
    for (; index < mNumber.DataSize(); ++index) {
        if (mNumber[index] == 0)
            mNumber[index] = 9;
        else {
            mNumber[index]--;
            break;
        }
    }
    return retVal;
}

void Number::ResizeLeadingZeros()
{
    // resize - remove leading zeros
    int i = (int)Size() - 1;
    for (; i >= 0; --i)
        if (mNumber[i]) break;
    SetDataSize(i >= 0 ? i + 1 : 0);
}

void Number::Shift(unsigned s, bool bLeft /*= true*/)
{
    if (!bLeft && s > Size())
        s = (unsigned)Size();
    if (s) {
        size_t curSize(Size());
        if (bLeft) {
            SetDataSize(curSize + s);
            char *buffer((char*)(void*)mNumber);
            memmove(buffer + s, buffer, curSize);
            for (unsigned i = 0; i < s; ++i)
                buffer[i] = 0;
        }
        else {
            curSize -= s;
            char *buffer((char*)(void*)mNumber);
            memmove(buffer, buffer + s, curSize);
            SetDataSize(curSize);
        }
    }
}

Number& Number::operator+=(const Number& other)
{
    return PerformSum(other);
}

Number operator+(const Number& n1, const Number& n2)
{
    Number out(n1);
    out += n2;
    return out;
}

Number operator-(const Number& n1, const Number& n2)
{
    Number out(n1);
    out -= n2;
    return out;
}
