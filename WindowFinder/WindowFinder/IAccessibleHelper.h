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
    long GetChildCount() const;
    bool GetChild(unsigned childIndex, IAccessibleHelper &outChild) const;
    IAccessibleHelper GetParent() const;
    struct IterCallbackData
    {
        void *pUserData;
        const IAccessibleHelper *childItem;
        const IAccessibleHelper *parentItem;
        const IAccessibleHelper *rootItem;
        int childDepth, childIndex, childCount;
    };
    typedef int(*IterCallback)(const IterCallbackData &inData);
    int IterateChildren(IterCallback callback, void *pUserData = NULL, int maxDepth = 0) const;

    enum GetRectFlags {
        GRF_None = 0x0,
        GRF_WRTSelf = 0x2,
        GRF_WRTParent = 0x4
    };
    // rectFlags - values from GetRectFlags
    bool GetRect(RECT &outRect, unsigned rectFlags = GRF_None); // Must be called after Location
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

    struct IterCallbackDataInt
    {
        IterCallbackData itData;
        IterCallback callback;
        int maxDepth;
    };
    int IterateChildrenInt(IterCallbackDataInt &inData) const;

    typedef lstring (IAccessibleHelper::*GetterAPI)() const;
    typedef std::map<lstring, IAccessibleHelper::GetterAPI> MapGetterAPI;
    static MapGetterAPI mMapGetterAPI;
    IAccessiblePtr m_pIAcc; // IAccessible Object
    _variant_t m_vt;
};

