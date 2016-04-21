#include "stdafx.h"
#include "PrimeNumber.h"
#include "CountTimer.h"

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

bool PrimeNumber::IsPrime(unsigned long long number)
{
    return mPrimeDatabase.IsPrime(number);
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
    m_ullIndex = mPrimeDatabase.AddPrimeNumber(m_ullIndex + 1, m_ullNumber, m_ullRootIdx);
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
