// SwapSum.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <ctype.h>
#include <vector>
#include <functional>
#include <time.h>

typedef std::vector<int> VecInt;

static int VecIntSum(const VecInt &arr)
{
    int sum(0);
    for (auto &a : arr)
        sum += a;
    return sum;
}
static int VecIntFind(const VecInt &arr, int num)
{
    int i = 0;
    for (auto &a : arr) {
        if (a == num)
            return i;
        ++i;
    }
    return -1;
}
static bool VecIntSwapValues(VecInt &arr1, VecInt &arr2, int val1, int val2)
{
    bool bSuccess(false);

    int i1(VecIntFind(arr1, val1)),
        i2(VecIntFind(arr2, val2));

    if (i1 >= 0 && i2 >= 0) {
        bSuccess = true;
        std::swap(arr1[i1], arr2[i2]);
    }

    return bSuccess;
}

struct SwapResult
{
    enum {
        NotPossible,
        Success,
        AlreadySame
    } result;
    int swap1, swap2;
    int nIter;
    SwapResult() : result(NotPossible), swap1(0), swap2(0), nIter(0){}
};

static SwapResult FindSwap(const VecInt &arry1, const VecInt &arry2)
{
    SwapResult res;
    int sum1(VecIntSum(arry1)), sum2(VecIntSum(arry2));
    int diff(sum2 - sum1);

    if (arry1.size() < 2 || arry2.size() < 2 || abs(diff) % 2);
    else if (diff == 0)
        res.result = SwapResult::AlreadySame;
    else {
        const VecInt &largerSumArray(diff > 0 ? arry2 : arry1);
        const VecInt &smallerSumArrya(&largerSumArray == &arry2 ? arry1 : arry2);
        diff = abs(diff);
        diff >>= 1; // half it
        for (auto l : largerSumArray) {
            if (l >= diff) {
                int s = l - diff;
                res.nIter++;
                if (VecIntFind(smallerSumArrya, s) >= 0) {
                    res.result = SwapResult::Success;
                    if (&largerSumArray == &arry1)
                        std::swap(l, s);
                    res.swap1 = s;
                    res.swap2 = l;
                    break;
                }
            }
        }
    }
    return res;
}

static void VecIntPrint(const VecInt &inArr, const _TCHAR *msg = _T(""))
{
    _tprintf(_T("Array %s:\n"), msg);
    for (auto &a : inArr)
        _tprintf(_T("%d "), a);
    _tprintf(_T("\n"));
}

static int VecIntGetFromStd(VecInt &arr, const _TCHAR *msg = _T(""))
{
    _tprintf(_T("Enter array %s:\n"), msg);
    while (1) {
        int val(0);
        if (_tscanf_s(_T("%d"), &val) == 1)
            arr.push_back(val);
        else {
            TCHAR extra[256];
            _tscanf_s(_T("%255s"), extra, _countof(extra));
            break;
        }
    }
    return arr.size();
}

static void VecIntRandomize(VecInt &arr)
{
    arr.clear();
    arr.resize(rand() % 10 + 2);
    for (auto i = 0; i < arr.size(); ++i)
        arr[i] = rand() % 19 + 1;
}
static void VecIntPrintAndSum(const VecInt &inArr, const _TCHAR *msg = _T(""))
{
    VecIntPrint(inArr, msg);
    _tprintf(_T("Sum = %d\n"), VecIntSum(inArr));
}
int _tmain(int argc, _TCHAR* argv[])
{
    srand(time(NULL));
    VecInt arr1, arr2;
    if (argc > 2) {
        VecInt *pV(&arr1);
        for (int i = 1;i < argc; ++i) {
            if (argv[i][0] == '-' || isdigit(argv[i][0])) {
                pV->push_back(_tstoi(argv[i]));
            }
            else if (pV == &arr1)
                pV = &arr2;
            else break;
        }
    }
    else {
        VecIntGetFromStd(arr1, _T("1"));
        VecIntGetFromStd(arr2, _T("2"));
    }
    while (1) {
        bool bStop(false);
        VecIntPrintAndSum(arr1, _T("1"));
        VecIntPrintAndSum(arr2, _T("2"));
        SwapResult sr = FindSwap(arr1, arr2);
        switch (sr.result)
        {
        case SwapResult::NotPossible:
            _tprintf(_T("No solution\nRandomize?[Enter]:"));
            {
                TCHAR ch(0);
                _tscanf_s(_T("%c"), &ch);
                if (ch == '\n') {
                    VecIntRandomize(arr1);
                    VecIntRandomize(arr2);
                }
                else
                    bStop = true;
            }
            break;
        case SwapResult::AlreadySame:
            _tprintf(_T("Already same sum\n"));
            {
                VecInt swapVec;
                if (VecIntGetFromStd(swapVec, _T("only two numbers to swap")) > 1)
                    VecIntSwapValues(arr1, arr2, swapVec[0], swapVec[1]);
                else
                    bStop = true;
            }
            break;
        case SwapResult::Success:
            _tprintf(_T("Success in %d iteration\n"), sr.nIter);
            _tprintf(_T("Swap values: %d and %d\n"), sr.swap1, sr.swap2);
            VecIntSwapValues(arr1, arr2, sr.swap1, sr.swap2);
            break;
        default:
            break;
        }
        if (bStop)
            break;
    }
    return 0;
}

