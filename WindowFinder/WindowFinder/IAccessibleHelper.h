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
    bool InitFromPoint(int x = -1, int y = -1);
    bool InitFromWindow(HWND hWnd, DWORD objID = OBJID_WINDOW);
    bool GetChild(_variant_t &childVt, IAccessibleHelper &outChild) const;

    enum GetRectFlags {
        GRF_None = 0x0,
        GRF_CallLocation = 0x1,
        GRF_WRTSelf = 0x2,
        GRF_WRTParent = 0x4
    };
    // rectFlags - values from GetRectFlags
    void GetRect(RECT &outRect, unsigned rectFlags = GRF_None); // Must be called after Location
    HWND GetWindow();

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
    void CommonInit();
    typedef lstring (IAccessibleHelper::*GetterAPI)() const;
    typedef std::map<lstring, IAccessibleHelper::GetterAPI> MapGetterAPI;
    static MapGetterAPI mMapGetterAPI;
    IAccessiblePtr m_pIAcc; // IAccessible Object
    _variant_t m_vt;
    mutable long mLocation[4];
    HWND m_hWnd;
};

