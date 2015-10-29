#pragma once

#ifndef FLAGBIT

#define FLAGBIT(n) (1<<(n))
#define MASKBIT(u) (FLAGBIT(u)-1)
#define MASKBITS(u,l) (MASKBIT(u) ^ MASKBIT(l))
#define SET_FLAG(f,fb) (f)|=(fb)
#define UNSET_FLAG(f,fb) (f)&=~(fb)
#define SET_UNSET_FLAG(f,fb,s) (f)=(s)?((f)|(fb)):((f)&~(fb))
#define IS_FLAG_SET(f,fb) (((f)&(fb))!=0)

#define SET_FLAGBIT(f,fb) (f)|=FLAGBIT(fb)
#define UNSET_FLAGBIT(f,fb) (f)&=~FLAGBIT(fb)
#define SET_UNSET_FLAGBIT(f,fb,s) (f)=(s)?((f)|FLAGBIT(fb)):((f)&~FLAGBIT(fb))
#define IS_FLAGBIT_SET(f,fb) (((f)&FLAGBIT(fb))!=0)

#define DEFINE_FUNCTION_SET_FLAGBIT(f,fb) void Set##fb(bool bSet##fb=true) {SET_UNSET_FLAGBIT(f,fb,bSet##fb);}
#define DEFINE_FUNCTION_IS_FLAGBIT_SET(f,fb) bool Is##fb() const {return IS_FLAGBIT_SET(f,fb);}
#define DEFINE_FUNCTION_SET_GET_FLAGBIT(f,fb) DEFINE_FUNCTION_SET_FLAGBIT(f,fb)\
	DEFINE_FUNCTION_IS_FLAGBIT_SET(f,fb)

#define MS_FLAGBIT(t) (FLAGBIT((sizeof(t)<<3)-1))
#define MSN_FLAGBIT(t,n) (FLAGBIT((sizeof(t)<<3)-((n)+1)))

#endif


typedef int (__cdecl *GenericCompareFn)(const void * elem1, const void * elem2);

template<class TYPE, class CT>
class CBinarySearch
{
public:
	CBinarySearch(const CT &objRef, GenericCompareFn cmfn = NULL) : mObjRef(objRef), mCompareFn(cmfn) {}
	INT_PTR GetInsertIndex(const TYPE &newElement ) const
	{
		bool bFound(false);
		INT_PTR pos(0);
		if (mObjRef.GetCount() > 0) {
			pos = GetPos(newElement, bFound);
			INT_PTR count(mObjRef.GetCount());
			while (pos >= 0 && pos < count) {
				const TYPE &value2(mObjRef.GetAt(pos));
				int cmpVal(Compare(newElement, value2));
				if (cmpVal > 0) {
					++pos;
					break;
				}
				else if (pos > 0 && cmpVal < 0)
					--pos;
				else break;
			}
		}
		return pos;
	}
	INT_PTR Find(const TYPE &value) const
	{
		bool bFound(false);
		INT_PTR pos(GetPos(value, bFound));
		if (bFound)
			return pos;
		return -1;
	}
	INT_PTR GetPos(const TYPE &value, bool &bFound) const
	{
		INT_PTR low = 0;
		INT_PTR high = mObjRef.GetCount()-1;
		INT_PTR mid = 0;
		bFound = false;
		while (low <= high) {
			mid = (low + high) / 2;
			const TYPE &value2(mObjRef.GetAt(mid));
			int compareRes(Compare(value, value2));
			if (compareRes > 0)
				low = mid + 1; 
			else if (compareRes < 0)
				high = mid-1;
			else {
				bFound = true;
				break;
			}
		}
		return mid;
	}
	INT_PTR HasDuplicates(INT_PTR startIndex = 0) const {
		for (INT_PTR i = startIndex; i < mObjRef.GetCount() -1; ++i)
			if (Compare(mObjRef.GetAt(i), mObjRef.GetAt(i+1)) == 0)
				return i;
		return -1;
	}
protected:
	int Compare(const TYPE &e1, const TYPE e2) const
	{
		if (mCompareFn)
			return mCompareFn(&e1, &e2);
		if (e1 > e2)
			return 1;
		else if (e1 < e2)
			return -1;
		return 0;
	}
	const CT &mObjRef;
	GenericCompareFn mCompareFn;
};

template<class TYPE>
class CSortedArray : public CArray<TYPE>
{
public:
	CSortedArray(GenericCompareFn cmfn = NULL)
		: CArray(), mCompareFn(cmfn)
	{
	}
	INT_PTR Insert(const TYPE &newElement, INT_PTR nCount = 1 )
	{
		CBinarySearch<TYPE, CSortedArray<TYPE>> bs(*this, mCompareFn);
		INT_PTR pos(bs.GetInsertIndex(newElement));
		InsertAt(pos, newElement, nCount);
		return pos;
	}
	void Insert(const CArray &newArray)
	{
		for (INT_PTR i = 0; i < newArray.GetCount(); ++i)
			Insert(newArray.GetAt(i));
	}
	INT_PTR Find(const TYPE &value) const
	{
		CBinarySearch<TYPE, CSortedArray<TYPE>> bs(*this, mCompareFn);
		return bs.Find(value);
	}
	void Sort()
	{
		qsort(GetData(), GetSize(),
			sizeof(TYPE), mCompareFn);
	}
	INT_PTR HasDuplicates()
	{
		CBinarySearch<TYPE, CSortedArray<TYPE>> bs(*this, mCompareFn);
		return bs.HasDuplicates();
	}
	INT_PTR InsertUnique(const TYPE &newElement)
	{
		CBinarySearch<TYPE, CSortedArray<TYPE>> bs(*this, mCompareFn);
		bool bFound(false);
		INT_PTR index(bs.GetPos(newElement, bFound));
		if (bFound)
			GetAt(index) = newElement;
		else
			index = Insert(newElement);
		return index;
	}
protected:
	GenericCompareFn mCompareFn;
};

template <class T, class ARG_TYPE = const T&>
class CArrayEx : public CArray<T, ARG_TYPE> {
public:
	INT_PTR Find(const T& t) const {
		for (INT_PTR i = 0; i < GetCount(); ++i)
			if (GetAt(i) == t)
				return i;
		return -1;
	}
	INT_PTR AddUnique(const T& t) {
		INT_PTR index(Find(t));
		if (index >= 0)
			GetAt(index) = t;
		else
			index = Add(t);
		return index;
	}
	bool Remove(const T& t) {
		INT_PTR index(Find(t));
		if (index >= 0) {
			RemoveAt(index);
			return true;
		}
		return false;
	}
};
