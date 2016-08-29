#pragma once

#include "Common.h"
#include <oleacc.h>
#include <comdef.h>
#include <map>

class IAccessibleHelper
{
public:
    IAccessibleHelper();
    ~IAccessibleHelper();
    operator bool() const;
    void InitFromPoint(int x = -1, int y = -1);
    bool GetChild(_variant_t &childVt, IAccessibleHelper &outChild) const;

    void GetRect(RECT &outRect, bool bCallLocation = false) const; // Must be called after Location

    lstring GetValue(const lstring &fieldName) const;
    lstring ChildCount() const;
    lstring Name() const;
    lstring Value() const;
    lstring Description() const;
    lstring Role() const;
    lstring State() const;
    lstring Help() const;
    lstring KeyboardShortcut() const;
    lstring Focus() const;
    lstring DefaultAction() const;
    lstring Location() const;


private:
    typedef lstring (IAccessibleHelper::*GetterAPI)() const;
    typedef std::map<lstring, IAccessibleHelper::GetterAPI> MapGetterAPI;
    static MapGetterAPI mMapGetterAPI;
    IAccessiblePtr m_pIAcc; // IAccessible Object
    _variant_t m_vt;
    mutable long mLocation[4];
};

