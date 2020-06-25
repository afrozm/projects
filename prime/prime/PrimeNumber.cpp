#include "stdafx.h"
#include "PrimeNumber.h"
#include "CountTimer.h"
static unsigned CumulitiveSum(unsigned long long n);
static unsigned CumulitiveSum(const Number &n);
PrimeNumber::PrimeNumber()
    : m_ullRootIdx(0), m_ullIndex(-1), m_ullNumber(0), m_uIncrIndex(0),
    m_ullSQOfRootPN(0), // Square of root prime number
    mbLimitReached(false),
    m_uNumberCumulitiveSum(0)
{
}


PrimeNumber::~PrimeNumber()
{
}

unsigned long long PrimeNumber::IsPrime(unsigned long long number)
{
    bool bNotFound(false);
    bool bIsPrime(mPrimeDatabase.IsPrime(number, &bNotFound));
    if (!bIsPrime && bNotFound) { // compute
        if (!(number & 1)) // 2
            return 2LL;
        m_ullNumber = 5;
        m_uNumberCumulitiveSum = CumulitiveSum(number);
        // divisible by 3
        if (m_uNumberCumulitiveSum == 3 || m_uNumberCumulitiveSum == 6 || m_uNumberCumulitiveSum == 9)
            return 3LL;
        if (number % 5 == 0)
            return 5LL;
        m_uIncrIndex = 5;
        m_uNumberCumulitiveSum = 5;
        m_ullSQOfRootPN = (unsigned long long)sqrtl((long double)number);
        while (m_ullNumber <= m_ullSQOfRootPN)
        {
            // increment to next number
            while (true) {
                NextNumber();
                if (m_uNumberCumulitiveSum == 3 || m_uNumberCumulitiveSum == 6 || m_uNumberCumulitiveSum == 9) continue;
                else break;
            }
            if (!(number % m_ullNumber))
                return m_ullNumber;
        }
        bIsPrime = true;
    }
    return bIsPrime;
}

Number PrimeNumber::IsPrime(const Number& number)
{
    bool bOverflow(false);
    long long ll(number.ToLL(&bOverflow));
    if (!bOverflow)
        return IsPrime(ll);
    // divisible by 2
    if (!(number[0] & 1))
        return 2LL;
    m_uNumberCumulitiveSum = CumulitiveSum(number);
    // divisible by 3
    if (m_uNumberCumulitiveSum == 3 || m_uNumberCumulitiveSum == 6 || m_uNumberCumulitiveSum == 9)
        return 3LL;
    // divisible by 5
    if (number[0] == 5)
        return 5LL;
    Number sqRoot(number.SquareRoot());
    Number numberToCheck(5LL);
    m_uIncrIndex = 5;
    m_uNumberCumulitiveSum = 5;
    while (numberToCheck <= sqRoot) {
        // increment to next number
        while (true) {
            NextNumber(numberToCheck);
            if (m_uNumberCumulitiveSum == 3 || m_uNumberCumulitiveSum == 6 || m_uNumberCumulitiveSum == 9) continue;
            else break;
        }
        if (!(number % numberToCheck))
            return numberToCheck;
    }
    return 1LL;
}

static unsigned CumulitiveSum(unsigned long long n)
{
    unsigned sum(0);
    while (n) {
        sum += n % 10;
        n /= 10;
        if (sum > 9)
            sum -= 9;
    }
    return sum;
}
static unsigned CumulitiveSum(const Number &n)
{
    unsigned sum(0);

    for (int i = (int)n.Size() - 1; i >= 0; --i) {
        sum += n[i];
        if (sum > 9)
            sum -= 9;
    }

    return sum;
}
static std::size_t GetRecommendedAllocSize()
{
    const std::size_t oneGB(1024ULL * 1024ULL * 1024ULL); // 1GB
    std::size_t allocSize(10ULL * oneGB); // 10 GB
    MEMORYSTATUSEX statex;

    statex.dwLength = sizeof(statex);

    GlobalMemoryStatusEx(&statex);

    std::size_t recomSize((statex.ullAvailPhys >> 4) & ~(oneGB - 1));
    if (allocSize > recomSize)
        allocSize = recomSize;

    return allocSize;
}
void PrimeNumber::StartCompute()
{
    if (mCachedPrime.size() == 0) {
        mCachedPrime.resize(GetRecommendedAllocSize() >> 3, 0);
        // mPrimeDatabase.GetPrimeNumbers(&mCachedPrime[0], mCachedPrime.size());
    }
    if (m_ullNumber == 0) {
        m_ullNumber = mPrimeDatabase.GetPrimeNumber(&m_ullIndex, &m_ullRootIdx);
        m_uNumberCumulitiveSum = CumulitiveSum(m_ullNumber);
        m_uIncrIndex = m_ullNumber % 10;
        ComputeRootPN();
    }
}

//                              0,1,2,3,4,5,6,7,8,9
static const int sIndices[] = { 1,2,1,4,3,2,1,2,1,2 };

bool PrimeNumber::ComputeNextPrime()
{
    bool bPrimeFound(false);
    CountTimer ct;
    while (!ct.UpdateTimeDuration() && !Completed())
    {
        NextNumber();
        if (m_ullSQOfRootPN < m_ullNumber) {
            ++m_ullRootIdx;
            ComputeRootPN();
            if (Completed())
                break;
        }
        if (CheckIfPrime()) {
            AddPrime();
            bPrimeFound = true;
        }
    }
    return bPrimeFound;
}

void PrimeNumber::EndCompute()
{
}

unsigned long long PrimeNumber::GetCurrentHighestPrime()
{
    if (m_ullNumber == 0)
        m_ullNumber = mPrimeDatabase.GetPrimeNumber(&m_ullIndex, &m_ullRootIdx);
    return m_ullNumber;
}

bool PrimeNumber::Completed() const
{
    return mbLimitReached;
}

unsigned long long PrimeNumber::GetNthPrime(unsigned long long n)
{
    unsigned long long outVal(0), *p(&outVal);
    if (n < mCachedPrime.size())
        p = &mCachedPrime[n];
    if (*p == 0)
        *p = mPrimeDatabase.GetPrimeNumber(&n);
    return *p;
}

Number PrimeNumber::GetNthPrime(const Number& n)
{
    bool bOverflow(false);
    long long ll(n.ToLL(&bOverflow));
    if (!bOverflow)
        return GetNthPrime(ll);
    return Number();
}

void PrimeNumber::ComputeRootPN()
{
    unsigned long long rootPN(GetNthPrime(m_ullRootIdx));
    m_ullSQOfRootPN = rootPN * rootPN;
    mbLimitReached = (m_ullSQOfRootPN / rootPN) != rootPN;
}

bool PrimeNumber::CheckIfPrime()
{
    // divisible by 3
    if (m_uNumberCumulitiveSum == 3 || m_uNumberCumulitiveSum == 6 || m_uNumberCumulitiveSum == 9)
        return false;
    // Skip divisible by 5 because we skip number ending with 5
    // check divisible by prime number 7 and onwards till square root of the number
    for (unsigned long long index = 4; index <= m_ullRootIdx; ++index) {
        unsigned long long pn(GetNthPrime(index));
        if ((m_ullNumber % pn) == 0)
            return false;
    }
    return true;
}

void PrimeNumber::AddPrime()
{
    m_ullIndex = mPrimeDatabase.AddPrimeNumber(m_ullNumber);
    if (m_ullIndex < mCachedPrime.size())
        mCachedPrime[m_ullIndex] = m_ullNumber;
}

void PrimeNumber::NextNumber()
{
    const int incr(sIndices[m_uIncrIndex]);
    m_ullNumber += incr;
    m_uIncrIndex += incr;
    m_uNumberCumulitiveSum += incr;
    if (m_uNumberCumulitiveSum > 9)
        m_uNumberCumulitiveSum -= 9;
    if (m_uIncrIndex >= _countof(sIndices))
        m_uIncrIndex -= _countof(sIndices);
}

//                                  0,  1,  2,  3,  4,  5,  6,  7,  8,  9
static const Number sNIndices[] = { 1LL,2LL,1LL,4LL,3LL,2LL,1LL,2LL,1LL,2LL };
void PrimeNumber::NextNumber(Number &number)
{
    number += sNIndices[m_uIncrIndex];
    const int incr(sIndices[m_uIncrIndex]);
    m_uIncrIndex += incr;
    m_uNumberCumulitiveSum += incr;
    if (m_uNumberCumulitiveSum > 9)
        m_uNumberCumulitiveSum -= 9;
    if (m_uIncrIndex >= _countof(sIndices))
        m_uIncrIndex -= _countof(sIndices);
}
