#include "stdafx.h"
#include "IAccessibleHelper.h"
#include <STLUtils.h>


#pragma comment(lib, "Oleacc.lib")

using namespace STLUtils;

IAccessibleHelper::MapGetterAPI IAccessibleHelper::mMapGetterAPI;

IAccessibleHelper::IAccessibleHelper()
{
    ZeroMemory((void*)mLocation, sizeof(mLocation));
}


IAccessibleHelper::~IAccessibleHelper()
{
}

IAccessibleHelper::operator bool() const
{
    if (m_pIAcc)
        return true;
    return false;
}


#define ADD_MAP_GETTER_API(a) mMapGetterAPI[_T(#a)]=&IAccessibleHelper::a

void IAccessibleHelper::CommonInit()
{
    if (mMapGetterAPI.empty()) {
        ADD_MAP_GETTER_API(ChildCount);
        ADD_MAP_GETTER_API(Name);
        ADD_MAP_GETTER_API(Value);
        ADD_MAP_GETTER_API(Description);
        ADD_MAP_GETTER_API(Role);
        ADD_MAP_GETTER_API(State);
        ADD_MAP_GETTER_API(Help);
        ADD_MAP_GETTER_API(KeyboardShortcut);
        ADD_MAP_GETTER_API(Focus);
        ADD_MAP_GETTER_API(DefaultAction);
        ADD_MAP_GETTER_API(Location);
    }
    if (m_pIAcc)
        m_pIAcc.Release();
    m_vt.Clear();
    m_hWnd = NULL;
}

bool IAccessibleHelper::InitFromPoint(int x, int y)
{
    bool bSuccess(false);
    CommonInit();
    if (x >= 0 && y >= 0)
        bSuccess = SUCCEEDED(AccessibleObjectFromPoint({ x,y }, &m_pIAcc, &m_vt));

    return bSuccess;
}

bool IAccessibleHelper::InitFromWindow(HWND hWnd, DWORD objID /*= OBJID_WINDOW*/)
{
    bool bSuccess(false);
    CommonInit();
    IAccessible *pIAccessible(NULL);
    bSuccess = SUCCEEDED(AccessibleObjectFromWindow(hWnd, objID, IID_IAccessible, (void **)&pIAccessible));
    if (pIAccessible) {
        m_pIAcc = pIAccessible;
        m_vt.vt = VT_I4;
        m_vt.lVal = CHILDID_SELF;
        m_hWnd = hWnd;
    }
    return bSuccess;
}

bool IAccessibleHelper::GetChild(_variant_t &childVt, IAccessibleHelper &outChild) const
{
    bool bSuccess(false);
    if (m_pIAcc) {
        outChild.m_vt = childVt;
        IDispatch *disp = NULL;
        m_pIAcc->get_accChild(outChild.m_vt, &disp);
        if (disp)
            disp->QueryInterface(IID_IAccessible, (void **)&outChild.m_pIAcc);
        if (outChild) {
            //outChild.m_vt.lVal = CHILDID_SELF;
            bSuccess = true;
        }
    }
    return bSuccess;
}

void IAccessibleHelper::GetRect(RECT &outRect, unsigned rectFlags /* = GRF_None */)
{
    if (rectFlags & GRF_CallLocation)
        Location();
    outRect.left = mLocation[0];
    outRect.top = mLocation[1];
    outRect.right = outRect.left + mLocation[2];
    outRect.bottom = outRect.top + mLocation[3];
    if (rectFlags & (GRF_WRTParent|GRF_WRTSelf)) {
        HWND hWnd(GetWindow());
        if (hWnd && (rectFlags&GRF_WRTParent)) {
            HWND hWndp = GetParent(hWnd);
            if (hWndp)
                hWnd = hWndp;
        }
        if (hWnd) {
            LPPOINT pt((LPPOINT)&outRect);
            ScreenToClient(hWnd, pt);
            ScreenToClient(hWnd, pt+1);
        }
    }
}

HWND IAccessibleHelper::GetWindow()
{
    HWND &hWnd(m_hWnd);

    if (!hWnd && m_pIAcc)
        WindowFromAccessibleObject(m_pIAcc, &hWnd);

    return hWnd;
}

lstring IAccessibleHelper::GetValue(const lstring & fieldName) const
{
    lstring outStr;
    auto cit(mMapGetterAPI.find(fieldName));
    if (cit != mMapGetterAPI.end())
        outStr = (this->*cit->second)();
    return outStr;
}

lstring IAccessibleHelper::ChildCount() const
{
    lstring outStr;
    if (m_pIAcc) {
        long childCount(0);
        m_pIAcc->get_accChildCount(&childCount);
        ChangeType(childCount, outStr);
    }
    return outStr;
}

lstring IAccessibleHelper::Name() const
{
    lstring outStr;
    if (m_pIAcc) {
        BSTR pName = NULL;
        m_pIAcc->get_accName(m_vt, &pName);
        if (pName) {
            outStr = pName;
            SysFreeString(pName);
        }
    }
    return outStr;
}

lstring IAccessibleHelper::Value() const
{
    lstring outStr;
    if (m_pIAcc) {
        BSTR pName = NULL;
        m_pIAcc->get_accValue(m_vt, &pName);
        if (pName) {
            outStr = pName;
            SysFreeString(pName);
        }
    }
    return outStr;
}

lstring IAccessibleHelper::Description() const
{
    lstring outStr;
    if (m_pIAcc) {
        BSTR pName = NULL;
        m_pIAcc->get_accDescription(m_vt, &pName);
        if (pName) {
            outStr = pName;
            SysFreeString(pName);
        }
    }
    return outStr;
}

lstring IAccessibleHelper::Role() const
{
    lstring outStr;
    if (m_pIAcc) {
        BSTR pName = NULL;
        _variant_t vt;
        m_pIAcc->get_accRole(m_vt, &vt);
        if (vt) {
            TCHAR roleText[256];
            roleText[0] = 0;
            GetRoleText(vt.lVal, roleText, _countof(roleText));
            outStr = roleText;
            if (outStr.empty())
                ChangeType(vt.lVal, outStr);
        }
    }
    return outStr;
}

lstring IAccessibleHelper::State() const
{
    lstring outStr;
    if (m_pIAcc) {
        BSTR pName = NULL;
        _variant_t vt;
        m_pIAcc->get_accState(m_vt, &vt);
        if (vt) {
            TCHAR roleText[256];
            roleText[0] = 0;
            GetStateText(vt.lVal, roleText, _countof(roleText));
            outStr = roleText;
            if (outStr.empty())
                ChangeType(vt.lVal, outStr);
        }
    }
    return outStr;
}

lstring IAccessibleHelper::Help() const
{
    lstring outStr;
    if (m_pIAcc) {
        BSTR pName = NULL;
        m_pIAcc->get_accHelp(m_vt, &pName);
        if (pName) {
            outStr = pName;
            SysFreeString(pName);
        }
    }
    return outStr;
}

lstring IAccessibleHelper::KeyboardShortcut() const
{
    lstring outStr;
    if (m_pIAcc) {
        BSTR pName = NULL;
        m_pIAcc->get_accKeyboardShortcut(m_vt, &pName);
        if (pName) {
            outStr = pName;
            SysFreeString(pName);
        }
    }
    return outStr;
}

lstring IAccessibleHelper::Focus() const
{
    lstring outStr;
    if (m_pIAcc) {
        BSTR pName = NULL;
        _variant_t vt;

        if (SUCCEEDED(m_pIAcc->get_accFocus(&vt))) {
            IAccessibleHelper iah;
            const IAccessibleHelper *pIAH(&iah);
            if (vt.vt == VT_EMPTY
                && m_hWnd)
                vt.vt = VT_I4;

            if (vt.vt == VT_I4) {
                if (vt.lVal == CHILDID_SELF)
                    pIAH = this;
                else GetChild(vt, iah);
            }
            else if (vt.vt == VT_DISPATCH){
                iah.m_pIAcc = vt;
                iah.m_vt.vt = VT_I4;
                iah.m_vt.lVal = CHILDID_SELF;
            }
            if (*pIAH) {
                outStr = pIAH->Value();
                if (outStr.empty())
                    outStr = pIAH->Name();
            }
        }
    }
    return outStr;
}

lstring IAccessibleHelper::DefaultAction() const
{
    lstring outStr;
    if (m_pIAcc) {
        BSTR pName = NULL;
        m_pIAcc->get_accDefaultAction(m_vt, &pName);
        if (pName) {
            outStr = pName;
            SysFreeString(pName);
        }
    }
    return outStr;
}

lstring IAccessibleHelper::Location() const
{
    ZeroMemory((void*)mLocation, sizeof(mLocation));
    lstring outStr;
    if (m_pIAcc) {
        if (SUCCEEDED(m_pIAcc->accLocation(mLocation, mLocation +1, mLocation +2, mLocation +3, m_vt))) {
            for (auto p : mLocation)
            {
                lstring str;
                ChangeType(p, str);
                if (!outStr.empty())
                    outStr += _T(",");
                outStr += str;
            }
        }
    }
    return outStr;
}
