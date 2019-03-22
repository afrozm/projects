#pragma once

#include "PrimeDatabase.h"
#include "Number.h"

class PrimeNumber
{
public:
    PrimeNumber();
    ~PrimeNumber();
    unsigned long long IsPrime(unsigned long long number);
    Number IsPrime(const Number& number);
    void StartCompute();
    bool ComputeNextPrime();
    void EndCompute();
    unsigned long long GetCurrentHighestPrime();
    bool Completed() const;
    // start from index 0
    unsigned long long GetNthPrime(unsigned long long n);
    Number GetNthPrime(const Number&  n);
private:
    void ComputeRootPN();
    bool CheckIfPrime();
    void AddPrime();
    void NextNumber();


    PrimeDatabase mPrimeDatabase;
    unsigned long long m_ullIndex, m_ullRootIdx, m_ullNumber, m_ullSQOfRootPN;
    unsigned int m_uIncrIndex, m_uNumberCumulitiveSum;
    bool mbLimitReached;
    std::vector<unsigned long long> mCachedPrime;
};

